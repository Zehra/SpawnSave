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
  const char* Name(){return "SpawnSave";}
  void Init (const char* commandLine) {
    bz_registerCustomSlashCommand ("save", this);
    Register(bz_ePlayerJoinEvent);
    Register(bz_eGetPlayerSpawnPosEvent);
    Register(bz_ePlayerPartEvent);
  }
void Event(bz_EventData *eventData );
  void Cleanup (void) {
    bz_removeCustomSlashCommand ("save");
    Flush();
  }
bool SlashCommand ( int playerID, bz_ApiString command, bz_ApiString message, bz_APIStringList* params );
    public:
    struct PlayerInfo {
      float playerPosition[4];
      int savedSpawn=0;
      int resetAfter=0;
    };
    struct PlayerInfo PD[200];
    // Easy way to reset spawns.
    void resetSpawn(int playerID) {
        for (int i=0;i<=3;i++) {
          PD[playerID].playerPosition[i] = 0.0;
        } // X // Y  // Z // Rotation
        PD[playerID].savedSpawn = 0;
        PD[playerID].resetAfter = 0;
    }
    
};

bool SpawnSave::SlashCommand ( int playerID, bz_ApiString command, bz_ApiString message, bz_APIStringList* params ) {
  if ((!strcmp("save",command.c_str())) && (isInRange(playerID) == 1)) {
    if (params->size()== 1) {
      if ((!strcmp("pos",params->get(0).c_str())) || (!strcmp("fixed",params->get(0).c_str()))) {
        // Could be rewritten, we quite often do this.
        bz_BasePlayerRecord *pr = bz_getPlayerByIndex(playerID);
        if (pr) {
          for (int i=0;i<=2;i++) { // 0=x, y=1, z=2.
            PD[playerID].playerPosition[i] = pr->lastKnownState.pos[i];
          }
          PD[playerID].playerPosition[3] = pr->lastKnownState.rotation; // Rotation
          PD[playerID].savedSpawn = 1;
          bz_sendTextMessage(BZ_SERVER, playerID, "New spawn position saved.");  
        }
        bz_freePlayerRecord(pr);
        // In theory it should be a more efficient operation.
        if (!strcmp("pos",params->get(0).c_str())) {
          PD[playerID].resetAfter = 1;
        } else {
          PD[playerID].resetAfter = 0;
        }
      
      }
      if (!strcmp("clear",params->get(0).c_str())) {
        resetSpawn(playerID);
      }
      
      
    } else {
      bz_sendTextMessage(BZ_SERVER,playerID,"SpawnSave commands help:");
      bz_sendTextMessage(BZ_SERVER,playerID,"/save         -  Current help page");
      bz_sendTextMessage(BZ_SERVER,playerID,"/save pos     -  Saves position for only the next spawn.");
		  bz_sendTextMessage(BZ_SERVER,playerID,"/save clear   -  Clears saved data.");
		  bz_sendTextMessage(BZ_SERVER,playerID,"/save fixed   -  Saves position for all future spawns.");
    }   
  return true;
  }
  return false;
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
            if (PD[player].savedSpawn == 1) {
                getSpawnData->handled = true;
                getSpawnData->pos[0] = PD[player].playerPosition[0];
                getSpawnData->pos[1] = PD[player].playerPosition[1];
                getSpawnData->pos[2] = PD[player].playerPosition[2];
                getSpawnData->rot = PD[player].playerPosition[3];
                if (PD[player].resetAfter == 1) {
                  PD[player].savedSpawn = 0;
                }
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

