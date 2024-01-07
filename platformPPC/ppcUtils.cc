//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "PlatformPPC/platformPPC.h"
#include "PlatformPPC/ppcUtils.h"

void ppcGetWorkingDirectory()
{
	WDPBRec rec;
	rec.ioCompletion = NULL;
	rec.ioNamePtr = NULL;
	
	// get the current vRefNum and dirID
	PBHGetVolSync( &rec );
	
	ppcState.volRefNum = rec.ioWDVRefNum;
	ppcState.dirID     = rec.ioWDDirID;
}
