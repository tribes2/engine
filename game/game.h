//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _GAME_H_
#define _GAME_H_

struct CameraQuery;

const F32 MinCameraFov              = 1.f;      // min camera FOV
const F32 MaxCameraFov              = 179.f;    // max camera FOV

void GameRenderWorld();
void GameRenderFilters(const CameraQuery& camq);
bool GameProcessCameraQuery(CameraQuery *query);
bool GameGetCameraTransform(MatrixF *mat, Point3F *velocity);
F32 GameGetCameraFov();
void GameSetCameraFov(F32 fov);
void GameSetCameraTargetFov(F32 fov);
void GameUpdateCameraFov();
void GameInit();

void clientProcess(U32 timeDelta);
void serverProcess(U32 timeDelta);

#endif
