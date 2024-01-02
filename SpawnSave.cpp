// SpawnSave.cpp
#include "bzfsAPI.h"

int isInRange(int player) {
    if ((player >= 0) && (player <= 199)) {
        return 1;
    } else if ((player < 0) || (player > 199)) {
        return 0;
    } else {
        return -1; // Something weird happened or invalid input.
    }
}

class SpawnSave : public bz_Plugin, bz_CustomSlashCommandHandler
{
    virtual const char* Name(){return "SpawnSave";}
    virtual void Init ( const char* /*config*/ );
    virtual void Event(bz_EventData *eventData );
    virtual void Cleanup ( void );
    virtual bool SlashCommand ( int playerID, bz_ApiString command, bz_ApiString message, bz_APIStringList* params );
    public:
    float playerPos[200][4] = { 0.0 }; // X, Y, Z and finally rotation as last.
    int playerSavedSpawn[200] = { 0 }; // We check if they request a spawn position save.
    // Easy way to reset spawns.
    void resetSpawn(int playerID) {
        playerPos[playerID][0] = 0.0; // x
        playerPos[playerID][1] = 0.0; // Y
        playerPos[playerID][2] = 0.0; // Z
        playerPos[playerID][3] = 0.0; // Rotation
        playerSavedSpawn[playerID] = 1;
    }
    
};

void SpawnSave::Init (const char*config) {
    bz_registerCustomSlashCommand ("save", this);
    Register(bz_ePlayerJoinEvent);
    Register(bz_eGetPlayerSpawnPosEvent);
    Register(bz_ePlayerPartEvent);
}

void SpawnSave::Cleanup (void) {
    bz_removeCustomSlashCommand ("save");
    Flush();
}

bool SpawnSave::SlashCommand ( int playerID, bz_ApiString command, bz_ApiString message, bz_APIStringList* params ) {
    if ((!strcmp("save",command.c_str())) && (isInRange(playerID) == 1)) {
        bz_BasePlayerRecord *pr = bz_getPlayerByIndex(playerID);
        if (pr) {
            playerPos[playerID][0] = pr->lastKnownState.pos[0]; // x
            playerPos[playerID][1] = pr->lastKnownState.pos[1]; // Y
            playerPos[playerID][2] = pr->lastKnownState.pos[2]; // Z
            playerPos[playerID][3] = pr->lastKnownState.rotation; // Rotation
            playerSavedSpawn[playerID] = 1;
            bz_sendTextMessage(BZ_SERVER, playerID, "New spawn position saved.");
            bz_freePlayerRecord(pr);
        }
        return true;
    }
}

void SpawnSave::Event(bz_EventData *eventData ){
   switch (eventData->eventType) {
    
    case bz_ePlayerJoinEvent: {
        bz_PlayerJoinPartEventData_V1* joinData = (bz_PlayerJoinPartEventData_V1*)eventData;
        //
        if (isInRange(joinData->playerID) == 1) {
            resetSpawn(joinData->playerID);
        }
    }break;
    
    case bz_eGetPlayerSpawnPosEvent: {
        bz_GetPlayerSpawnPosEventData_V1* getSpawnData = (bz_GetPlayerSpawnPosEventData_V1*)eventData;
        int player = getSpawnData->playerID;
        //
        if (isInRange(player) == 1) {
            if (playerSavedSpawn[player] == 1) {
                getSpawnData->handled = true;
                getSpawnData->pos[0] = playerPos[player][0];
                getSpawnData->pos[1] = playerPos[player][1];
                getSpawnData->pos[2] = playerPos[player][2];
                getSpawnData->rot = playerPos[player][3];
            }
        }

    }break;
    
    case bz_ePlayerPartEvent: {
        bz_PlayerJoinPartEventData_V1* partData = (bz_PlayerJoinPartEventData_V1*)eventData;
        //
        if (isInRange(partData->playerID) == 1) {
            resetSpawn(partData->playerID);
        }
    }break;
    
    default:{
    }break;
   }
}

BZ_PLUGIN(SpawnSave)
