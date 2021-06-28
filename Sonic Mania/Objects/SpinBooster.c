#include "../SonicMania.h"

ObjectSpinBooster *SpinBooster;

void SpinBooster_Update(void)
{
    RSDK_THIS(SpinBooster);

    byte negAngle = -entity->angle;

    foreach_active(Player, player)
    {
        int pid   = RSDK.GetEntityID(player);
        int distX = (player->position.x - entity->position.x) >> 8;
        int distY = (player->position.y - entity->position.y) >> 8;
        int x     = (distY * RSDK.Sin256(negAngle)) + distX * RSDK.Cos256(negAngle) + entity->position.x;
        int y     = (distY * RSDK.Cos256(negAngle)) - distX * RSDK.Sin256(negAngle) + entity->position.y;

        if (abs(x - entity->position.x) >= 0x180000 || abs(y - entity->position.y) >= entity->size << 19) {
            if (x > entity->position.x) {
                entity->activePlayers &= ~(1 << pid);
            }
        }
        else {
            if (x > entity->position.x) {
                if ((1 << pid) & entity->activePlayers) {
                    if (!entity->forwardOnly) {
                        if (player->state == Player_State_ForceRoll || player->state == Player_State_RollLock) {
                            player->nextAirState    = StateMachine_None;
                            player->nextGroundState = StateMachine_None;
                            if (!entity->allowTubeInput)
                                player->controlLock = 0;
                            player->tileCollisions = true;
                            if (player->onGround)
                                player->state = Player_State_Roll;
                            else
                                player->state = Player_State_Air;
                        }
                    }
                }
                entity->activePlayers &= ~(1 << pid);
            }
            else if (!((1 << pid) & entity->activePlayers)) {
                SpinBooster_Unknown5(player);
                entity->activePlayers |= (1 << pid);
            }
        }
    }

    entity->visible = DebugMode->debugActive;
}

void SpinBooster_LateUpdate(void) {}

void SpinBooster_StaticUpdate(void) {}

void SpinBooster_Draw(void) { SpinBooster_DrawSprites(); }

void SpinBooster_Create(void *data)
{
    RSDK_THIS(SpinBooster);
    RSDK.SetSpriteAnimation(SpinBooster->aniFrames, 0, &entity->animator, true, 0);
    entity->drawFX |= FX_FLIP;
    entity->animator.frameID = 4;
    entity->activePlayers    = 0;
    if (RSDK_sceneInfo->inEditor) {
        if (!entity->boostPower)
            entity->boostPower = 15;
    }
    else {
        switch (entity->direction) {
            case FLIP_NONE: entity->angle = 0; break;
            case FLIP_X: entity->angle = 0x40; break;
            case FLIP_Y: entity->angle = -0x80; break;
            case FLIP_XY: entity->angle = 0xC0; break;
        }
        entity->active = ACTIVE_BOUNDS;

        if (entity->size * RSDK.Sin256(entity->angle) << 11 >= 0)
            entity->updateRange.x = entity->size * RSDK.Sin256(entity->angle) << 11;
        else
            entity->updateRange.x = -0x800 * entity->size * RSDK.Sin256(entity->angle);
        entity->updateRange.x += 0x200000;

        if (entity->size * RSDK.Cos256(entity->angle) << 11 >= 0)
            entity->updateRange.y = entity->size * RSDK.Cos256(entity->angle) << 11;
        else
            entity->updateRange.y = -0x800 * entity->size * RSDK.Cos256(entity->angle);
        entity->updateRange.y += 0x200000;
        entity->visible   = false;
        entity->drawOrder = Zone->drawOrderLow;
    }
}

void SpinBooster_StageLoad(void) { SpinBooster->aniFrames = RSDK.LoadSpriteAnimation("Global/PlaneSwitch.bin", SCOPE_STAGE); }

int SpinBooster_Unknown1(EntityPlayer *player)
{
    RSDK_THIS(SpinBooster);

    switch (entity->direction) {
        case FLIP_NONE:
            if (entity->autoGrip != 5) {
                if ((entity->autoGrip == 7 || entity->autoGrip == 8) && player->up)
                    return 2;
                return 0;
            }
            if (player->down) {
                return 0;
            }
            if (player->up) {
                return 2;
            }

            if (!entity->bias)
                return 0;
            else
                return 2;
        case FLIP_X:
            if (entity->autoGrip != 5) {
                if (entity->autoGrip == 7) {
                    if (!player->left) {
                        return -1;
                    }
                    return 3;
                }

                if (entity->autoGrip == 6) {
                    if (!player->right)
                        return -1;
                }
                if (entity->autoGrip != 8)
                    return -1;
                if (!player->left) {
                    if (!player->right)
                        return -1;
                    return 1;
                }
                else {
                    return 3;
                }
            }
            else {
                if (player->left)
                    return 3;
                if (player->right)
                    return 1;
                return 2 * (entity->bias != 0) + 1;
            }
        case FLIP_Y:
            if (entity->autoGrip != 5) {
                if ((entity->autoGrip == 6 || entity->autoGrip == 8) && player->up)
                    return 2;
                return 0;
            }
            if (player->down)
                return 0;
            if (player->up)
                return 2;

            if (!entity->bias)
                return 2;
            else
                return 0;
        case FLIP_XY:
            if (entity->autoGrip == 5) {
                return 2 * (player->left != 0) + 1;
            }
            else if (entity->autoGrip == 6) {
                if (!player->left)
                    return -1;
                return 3;
            }
            else {
                if (entity->autoGrip == 7) {
                    if (!player->right)
                        return -1;
                }
                if (entity->autoGrip != 8)
                    return -1;
                if (!player->left) {
                    if (!player->right)
                        return -1;
                    return 1;
                }
                else {
                    return 3;
                }
            }
        default: break;
    }
    return -1;
}
void SpinBooster_Unknown2(EntityPlayer *player)
{
    RSDK_THIS(SpinBooster);

    if (entity->autoGrip) {
        byte cMode = 0;
        if (entity->autoGrip < 5) {
            cMode = entity->autoGrip - 1;
        }
        else {
            cMode = SpinBooster_Unknown1(player);
        }

        if ((cMode & 0x80) != 0) {
            player->onGround = false;
            return;
        }
        int offsetX  = 0;
        int offsetY  = 0;
        int plrAngle = 0;
        int angle    = 0;

        if (cMode) {
            switch (cMode) {
                case CMODE_FLOOR:
                    plrAngle = 0x00;
                    angle    = 0x40;
                    break;
                case CMODE_LWALL:
                    plrAngle = -0x40;
                    angle    = 0x00;
                    break;
                case CMODE_ROOF:
                    plrAngle = -0x80;
                    angle    = -0x40;
                    break;
                case CMODE_RWALL:
                    plrAngle = 0x40;
                    angle    = -0x80;
                    break;
                default:
                    plrAngle = 0x00;
                    angle    = 0x00;
                    break;
            }
        }

        Hitbox *playerHitbox = Player_GetHitbox(player);
        switch (cMode) {
            case CMODE_FLOOR: offsetY = playerHitbox->bottom << 16; break;
            case CMODE_LWALL: offsetX = playerHitbox->right << 16; break;
            case CMODE_ROOF: offsetY = playerHitbox->top << 16; break;
            case CMODE_RWALL: offsetX = playerHitbox->left << 16; break;
            default: break;
        }
        player->position.x += RSDK.Cos256(angle) << 8;
        player->position.y += RSDK.Sin256(angle) << 8;
        RSDK.ObjectTileGrip(player, Zone->fgLayers, cMode, 0, offsetX, offsetY, 1);
        player->angle = plrAngle;
        if (player->onGround) {
            byte mode = 0;
            switch (player->collisionMode) {
                case CMODE_FLOOR: mode = CMODE_ROOF; break;
                case CMODE_LWALL: mode = CMODE_RWALL; break;
                case CMODE_ROOF: mode = CMODE_FLOOR; break;
                case CMODE_RWALL: mode = CMODE_LWALL; break;
            }

            if (cMode == mode) {
                player->groundVel = -player->groundVel;
            }
        }
        else {
            switch (cMode) {
                case FLIP_NONE: player->groundVel = player->velocity.x; break;
                case FLIP_X: player->groundVel = -player->velocity.y; break;
                case FLIP_Y: player->groundVel = -player->velocity.x; break;
                case FLIP_XY: player->groundVel = player->velocity.y; break;
                default: break;
            }
        }
        player->collisionMode = cMode;
        player->onGround      = true;
    }
}
void SpinBooster_Unknown3(EntityPlayer *player)
{
    RSDK_THIS(SpinBooster);
    if (player->onGround) {
        int entAng = RSDK.Sin256(entity->angle) + RSDK.Cos256(entity->angle);
        int plrAng = RSDK.Sin256(player->angle) + RSDK.Cos256(player->angle);
        int power  = (entity->boostPower << 15) * ((plrAng > 0) - (plrAng < 0)) * ((entAng > 0) - (entAng < 0));
        if (entity->boostPower >= 0)
            player->groundVel += power;
        else
            player->groundVel = power;
    }
    else {
        int x = (RSDK.Cos256(entity->angle) << 7) * entity->boostPower;
        int y = (-0x80 * RSDK.Sin256(entity->angle)) * entity->boostPower;
        if (entity->boostPower >= 0) {
            player->velocity.x += x;
            player->velocity.y += y;
        }
        else {
            player->velocity.x = x;
            player->velocity.y = y;
        }
    }
    if (entity->boostPower < 0 && !entity->forwardOnly) {
        if (player->state == Player_State_ForceRoll || player->state == Player_State_RollLock) {
            player->nextAirState    = StateMachine_None;
            player->nextGroundState = StateMachine_None;
            if (!entity->allowTubeInput)
                player->controlLock = 0;
            player->tileCollisions = true;
            if (player->onGround)
                player->state = Player_State_Roll;
            else
                player->state = Player_State_Air;
        }
    }
}
void SpinBooster_Unknown4(uint colour, int X1, int Y1, int X2, int Y2)
{
    RSDK.DrawLine(X1, Y1, X2, Y2, colour, 127, INK_ADD, 0);
    int angle = RSDK.ATan2(X1 - X2, Y1 - Y2);
    int c1    = RSDK.Cos256(angle + 12) << 12;
    int s1    = RSDK.Sin256(angle + 12) << 12;
    int c2    = RSDK.Cos256(angle - 12) << 12;
    int s2    = RSDK.Sin256(angle - 12) << 12;
    RSDK.DrawLine(X2, Y2, X2 + c1, Y2 + s1, colour, 0x7F, INK_ADD, false);
    RSDK.DrawLine(X2, Y2, X2 + c2, Y2 + s2, colour, 0x7F, INK_ADD, false);
}
void SpinBooster_DrawSprites(void)
{
    RSDK_THIS(SpinBooster);
    Vector2 drawPos;

    drawPos.x = entity->position.x;
    drawPos.y = entity->position.y;
    drawPos.y -= entity->size << 19;
    Zone_Unknown3((Entity *)entity, &drawPos, entity->angle);
    for (int i = 0; i < entity->size; ++i) {
        RSDK.DrawSprite(&entity->animator, &drawPos, false);
        drawPos.x += RSDK.Sin256(entity->angle) << 12;
        drawPos.y += RSDK.Cos256(entity->angle) << 12;
    }

    if (RSDK_sceneInfo->inEditor) {
        byte negAngle = -entity->angle;
        int power     = entity->boostPower;
        int X1        = entity->position.x;
        int Y1        = entity->position.y;
        int clr       = ((power >> 31) & 0xFE0001) + 0xFFFF;
        if (!power)
            power = 1;
        SpinBooster_Unknown4(clr, X1, Y1, (power * (RSDK.Cos256(negAngle) << 11)) + entity->position.x,
                             power * (RSDK.Sin256(negAngle) << 11) + entity->position.y);

        switch (entity->autoGrip) {
            case 1:
            case 2:
            case 3:
            case 4: {
                int angle = 0;
                switch (entity->autoGrip) {
                    case 1: angle = 0x40; break;
                    case 2: angle = 0; break;
                    case 3: angle = -0x40; break;
                    case 4: angle = 0x80; break;
                }
                SpinBooster_Unknown4(0xFF00, X1, Y1, (RSDK.Cos256(angle) << 14) + entity->position.x,
                                     (RSDK.Sin256(angle) << 14) + entity->position.y);
                break;
            }
            case 5:
                if (!entity->bias)
                    clr = 0x00FF00;
                else
                    clr = 0xFFFF00;
                SpinBooster_Unknown4(clr, X1, Y1, (RSDK.Cos256((byte)(negAngle + 64)) << 14) + entity->position.x,
                                     (RSDK.Sin256((byte)(negAngle + 64)) << 14) + entity->position.y);
                SpinBooster_Unknown4(clr, X1, Y1, (RSDK.Cos256((byte)(negAngle - 64)) << 14) + entity->position.x,
                                     (RSDK.Sin256((byte)(negAngle - 64)) << 14) + entity->position.y);
                break;
            case 6:
                clr = 0xFFFF00;
                SpinBooster_Unknown4(clr, X1, Y1, (RSDK.Cos256((byte)(negAngle + 64)) << 14) + entity->position.x,
                                     (RSDK.Sin256((byte)(negAngle + 64)) << 14) + entity->position.y);
                SpinBooster_Unknown4(0xFF00, X1, Y1, (RSDK.Cos256(negAngle) << 14) + entity->position.x,
                                     (RSDK.Sin256(negAngle) << 14) + entity->position.y);
                break;
            case 7:
                clr = 0xFFFF00;
                SpinBooster_Unknown4(clr, X1, Y1, (RSDK.Cos256((byte)(negAngle - 64)) << 14) + entity->position.x,
                                     (RSDK.Sin256((byte)(negAngle - 64)) << 14) + entity->position.y);
                SpinBooster_Unknown4(0xFF00, X1, Y1, (RSDK.Cos256(negAngle) << 14) + entity->position.x,
                                     (RSDK.Sin256(negAngle) << 14) + entity->position.y);
                break;
            case 8:
                clr = 0xFFFF00;
                SpinBooster_Unknown4(clr, X1, Y1, (RSDK.Cos256((byte)(negAngle + 64)) << 14) + entity->position.x,
                                     (RSDK.Sin256((byte)(negAngle + 64)) << 14) + entity->position.y);
                SpinBooster_Unknown4(clr, X1, Y1, (RSDK.Cos256((byte)(negAngle - 64)) << 14) + entity->position.x,
                                     (RSDK.Sin256((byte)(negAngle - 64)) << 14) + entity->position.y);
                SpinBooster_Unknown4(0xFF00, X1, Y1, (RSDK.Cos256(negAngle) << 14) + entity->position.x,
                                     (RSDK.Sin256(negAngle) << 14) + entity->position.y);
                break;
        }
    }
}
void SpinBooster_Unknown5(EntityPlayer *player)
{
    RSDK_THIS(SpinBooster);
    player->tileCollisions = true;
    SpinBooster_Unknown2(player);

    if (player->state == Player_State_ForceRoll || player->state == Player_State_RollLock) {
        if (entity->boostAlways)
            SpinBooster_Unknown3(player);
    }
    else {
        if (entity->playSound)
            RSDK.PlaySFX(Player->sfx_Roll, 0, 0xFF);
        if (player->playerAnimator.animationID != ANI_JUMP) {
            RSDK.SetSpriteAnimation(player->spriteIndex, ANI_JUMP, &player->playerAnimator, false, 0);
            if (!player->collisionMode && player->onGround)
                player->position.y += player->cameraOffset;
            player->pushing = false;
        }
        player->nextAirState    = StateMachine_None;
        player->nextGroundState = StateMachine_None;
        if (!entity->allowTubeInput)
            player->controlLock = 0xFFFF;

        if (player->onGround)
            player->state = Player_State_ForceRoll;
        else
            player->state = Player_State_RollLock;

        if (abs(player->groundVel) < 0x10000) {
            if (entity->direction & FLIP_X)
                player->groundVel = -0x40000;
            else
                player->groundVel = 0x40000;
        }
        SpinBooster_Unknown3(player);
    }
}

void SpinBooster_EditorDraw(void) {}

void SpinBooster_EditorLoad(void) {}

void SpinBooster_Serialize(void)
{
    RSDK_EDITABLE_VAR(SpinBooster, VAR_UINT8, direction);
    RSDK_EDITABLE_VAR(SpinBooster, VAR_UINT8, autoGrip);
    RSDK_EDITABLE_VAR(SpinBooster, VAR_UINT8, bias);
    RSDK_EDITABLE_VAR(SpinBooster, VAR_ENUM, size);
    RSDK_EDITABLE_VAR(SpinBooster, VAR_INT32, boostPower);
    RSDK_EDITABLE_VAR(SpinBooster, VAR_BOOL, boostAlways);
    RSDK_EDITABLE_VAR(SpinBooster, VAR_BOOL, forwardOnly);
    RSDK_EDITABLE_VAR(SpinBooster, VAR_BOOL, playSound);
    RSDK_EDITABLE_VAR(SpinBooster, VAR_BOOL, allowTubeInput);
}
