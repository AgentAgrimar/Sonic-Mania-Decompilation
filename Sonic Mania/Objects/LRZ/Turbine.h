#ifndef OBJ_TURBINE_H
#define OBJ_TURBINE_H

#include "SonicMania.h"

typedef enum { TURBINE_HANDLES, TURBINE_SPIKES, TURBINE_WALLDECOR } TurbineTypes;

// Object Class
typedef struct {
    RSDK_OBJECT
    uint16 aniFrames;
    uint16 sfxTurbine;
    Hitbox hitbox1;
    Hitbox hitboxHurt;
    bool32 playingTurbineSfx;
} ObjectTurbine;

// Entity Class
typedef struct {
    RSDK_ENTITY
    StateMachine(state);
    uint8 type;
    uint8 activePlayers;
    int32 playerAngles[4];
    int32 playerTimers[4];
    Animator animator;
} EntityTurbine;

// Object Struct
extern ObjectTurbine *Turbine;

// Standard Entity Events
void Turbine_Update(void);
void Turbine_LateUpdate(void);
void Turbine_StaticUpdate(void);
void Turbine_Draw(void);
void Turbine_Create(void* data);
void Turbine_StageLoad(void);
#if RETRO_INCLUDE_EDITOR
void Turbine_EditorDraw(void);
void Turbine_EditorLoad(void);
#endif
void Turbine_Serialize(void);

// Extra Entity Functions
void Turbine_State_Handles(void);
void Turbine_State_Spikes(void);

#endif //!OBJ_TURBINE_H
