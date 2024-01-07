# Microsoft Developer Studio Project File - Name="Engine" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) External Target" 0x0106

CFG=Engine - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "Engine.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "Engine.mak" CFG="Engine - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Engine - Win32 Release" (based on "Win32 (x86) External Target")
!MESSAGE "Engine - Win32 Debug" (based on "Win32 (x86) External Target")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""

!IF  "$(CFG)" == "Engine - Win32 Release"

# PROP BASE Use_MFC
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Cmd_Line "NMAKE /f Engine.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "Engine.exe"
# PROP BASE Bsc_Name "Engine.bsc"
# PROP BASE Target_Dir ""
# PROP Use_MFC
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Cmd_Line "make OS=WIN32 COMPILER=VC6 BUILD=RELEASE DIR.OBJ=out.VC6.RELEASE"
# PROP Rebuild_Opt "/a"
# PROP Target_File "v12_RELEASE.exe"
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "Engine - Win32 Debug"

# PROP BASE Use_MFC
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Cmd_Line "NMAKE /f Engine.mak"
# PROP BASE Rebuild_Opt "/a"
# PROP BASE Target_File "Engine.exe"
# PROP BASE Bsc_Name "Engine.bsc"
# PROP BASE Target_Dir ""
# PROP Use_MFC
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Cmd_Line "make OS=WIN32 COMPILER=VC6 BUILD=DEBUG DIR.OBJ=out.VC6.DEBUG"
# PROP Rebuild_Opt "/a"
# PROP Target_File "v12_DEBUG.exe"
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "Engine - Win32 Release"
# Name "Engine - Win32 Debug"

!IF  "$(CFG)" == "Engine - Win32 Release"

!ELSEIF  "$(CFG)" == "Engine - Win32 Debug"

!ENDIF 

# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "ai"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\ai\aiConnection.cc
# End Source File
# Begin Source File

SOURCE=.\ai\aiConsole.cc
# End Source File
# Begin Source File

SOURCE=.\ai\aiDebug.cc
# End Source File
# Begin Source File

SOURCE=.\ai\aiNavJetting.cc
# End Source File
# Begin Source File

SOURCE=.\ai\aiNavStep.cc
# End Source File
# Begin Source File

SOURCE=.\ai\aiObjective.cc
# End Source File
# Begin Source File

SOURCE=.\ai\aiStep.cc
# End Source File
# Begin Source File

SOURCE=.\ai\aiTask.cc
# End Source File
# Begin Source File

SOURCE=.\ai\graph.cc
# End Source File
# Begin Source File

SOURCE=.\ai\graphBase.cc
# End Source File
# Begin Source File

SOURCE=.\ai\graphBridge.cc
# End Source File
# Begin Source File

SOURCE=.\ai\graphBuildLOS.cc
# End Source File
# Begin Source File

SOURCE=.\ai\graphConjoin.cc
# End Source File
# Begin Source File

SOURCE=.\ai\graphData.cc
# End Source File
# Begin Source File

SOURCE=.\ai\graphDebug.cc
# End Source File
# Begin Source File

SOURCE=.\ai\graphDijkstra.cc
# End Source File
# Begin Source File

SOURCE=.\ai\graphFind.cc
# End Source File
# Begin Source File

SOURCE=.\ai\graphFloorPlan.cc
# End Source File
# Begin Source File

SOURCE=.\ai\graphFloorRender.cc
# End Source File
# Begin Source File

SOURCE=.\ai\graphForceField.cc
# End Source File
# Begin Source File

SOURCE=.\ai\graphGenUtils.cc
# End Source File
# Begin Source File

SOURCE=.\ai\graphGroundPlan.cc
# End Source File
# Begin Source File

SOURCE=.\ai\graphIndoors.cc
# End Source File
# Begin Source File

SOURCE=.\ai\graphIsland.cc
# End Source File
# Begin Source File

SOURCE=.\ai\graphJetting.cc
# End Source File
# Begin Source File

SOURCE=.\ai\graphLocate.cc
# End Source File
# Begin Source File

SOURCE=.\ai\graphLOS.cc
# End Source File
# Begin Source File

SOURCE=.\ai\graphMake.cc
# End Source File
# Begin Source File

SOURCE=.\ai\graphMath.cc
# End Source File
# Begin Source File

SOURCE=.\ai\graphOutdoors.cc
# End Source File
# Begin Source File

SOURCE=.\ai\graphPartition.cc
# End Source File
# Begin Source File

SOURCE=.\ai\graphPath.cc
# End Source File
# Begin Source File

SOURCE=.\ai\graphQueries.cc
# End Source File
# Begin Source File

SOURCE=.\ai\graphRender.cc
# End Source File
# Begin Source File

SOURCE=.\ai\graphSearchLOS.cc
# End Source File
# Begin Source File

SOURCE=.\ai\graphSmooth.cc
# End Source File
# Begin Source File

SOURCE=.\ai\graphSpawn.cc
# End Source File
# Begin Source File

SOURCE=.\ai\graphThreats.cc
# End Source File
# Begin Source File

SOURCE=.\ai\graphTransient.cc
# End Source File
# Begin Source File

SOURCE=.\ai\graphVolume.cc
# End Source File
# End Group
# Begin Group "audio"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\audio\audio.cc
# End Source File
# Begin Source File

SOURCE=.\audio\audioBuffer.cc
# End Source File
# Begin Source File

SOURCE=.\audio\audioCodec.cc
# End Source File
# Begin Source File

SOURCE=.\audio\audioCodecMiles.cc
# End Source File
# Begin Source File

SOURCE=.\audio\audioDataBlock.cc
# End Source File
# Begin Source File

SOURCE=.\audio\audioMss.cc
# End Source File
# Begin Source File

SOURCE=.\audio\audioNet.cc
# End Source File
# Begin Source File

SOURCE=.\audio\audioThread.cc
# End Source File
# Begin Source File

SOURCE=.\audio\bufferQueue.cc
# End Source File
# End Group
# Begin Group "collision"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\collision\abstractPolyList.cc
# End Source File
# Begin Source File

SOURCE=.\collision\boxConvex.cc
# End Source File
# Begin Source File

SOURCE=.\collision\clippedPolyList.cc
# End Source File
# Begin Source File

SOURCE=.\collision\convex.cc
# End Source File
# Begin Source File

SOURCE=.\collision\depthSortList.cc
# End Source File
# Begin Source File

SOURCE=.\collision\earlyOutPolyList.cc
# End Source File
# Begin Source File

SOURCE=.\collision\extrudedPolyList.cc
# End Source File
# Begin Source File

SOURCE=.\collision\gjk.cc
# End Source File
# Begin Source File

SOURCE=.\collision\planeExtractor.cc
# End Source File
# Begin Source File

SOURCE=.\collision\polyhedron.cc
# End Source File
# Begin Source File

SOURCE=.\collision\polytope.cc
# End Source File
# End Group
# Begin Group "console"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\console\compiledEval.cc
# End Source File
# Begin Source File

SOURCE=.\console\compiler.cc
# End Source File
# Begin Source File

SOURCE=.\console\console.cc
# End Source File
# Begin Source File

SOURCE=.\console\consoleFunctions.cc
# End Source File
# Begin Source File

SOURCE=.\console\consoleInternal.cc
# End Source File
# Begin Source File

SOURCE=.\console\consoleObject.cc
# End Source File
# Begin Source File

SOURCE=.\console\consoleTypes.cc
# End Source File
# Begin Source File

SOURCE=.\console\gram.cc
# End Source File
# Begin Source File

SOURCE=.\console\scan.cc
# End Source File
# Begin Source File

SOURCE=.\console\scriptObject.cc
# End Source File
# Begin Source File

SOURCE=.\console\simBase.cc
# End Source File
# Begin Source File

SOURCE=.\console\simDictionary.cc
# End Source File
# Begin Source File

SOURCE=.\console\simManager.cc
# End Source File
# Begin Source File

SOURCE=.\console\telnetConsole.cc
# End Source File
# Begin Source File

SOURCE=.\console\telnetDebugger.cc
# End Source File
# End Group
# Begin Group "core"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\core\bitRender.cc
# End Source File
# Begin Source File

SOURCE=.\core\bitStream.cc
# End Source File
# Begin Source File

SOURCE=.\core\BitTables.cc
# End Source File
# Begin Source File

SOURCE=.\core\dataChunker.cc
# End Source File
# Begin Source File

SOURCE=.\core\dnet.cc
# End Source File
# Begin Source File

SOURCE=.\core\fileObject.cc
# End Source File
# Begin Source File

SOURCE=.\core\fileStream.cc
# End Source File
# Begin Source File

SOURCE=.\core\filterStream.cc
# End Source File
# Begin Source File

SOURCE=.\core\findMatch.cc
# End Source File
# Begin Source File

SOURCE=.\core\idGenerator.cc
# End Source File
# Begin Source File

SOURCE=.\core\memStream.cc
# End Source File
# Begin Source File

SOURCE=.\core\nStream.cc
# End Source File
# Begin Source File

SOURCE=.\core\nTypes.cc
# End Source File
# Begin Source File

SOURCE=.\core\resDictionary.cc
# End Source File
# Begin Source File

SOURCE=.\core\resizeStream.cc
# End Source File
# Begin Source File

SOURCE=.\core\resManager.cc
# End Source File
# Begin Source File

SOURCE=.\core\stringTable.cc
# End Source File
# Begin Source File

SOURCE=.\core\tagDictionary.cc
# End Source File
# Begin Source File

SOURCE=.\core\tVector.cc
# End Source File
# Begin Source File

SOURCE=.\core\zipAggregate.cc
# End Source File
# Begin Source File

SOURCE=.\core\zipHeaders.cc
# End Source File
# Begin Source File

SOURCE=.\core\zipSubStream.cc
# End Source File
# End Group
# Begin Group "crypt"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\crypt\cryptMGF.cc
# End Source File
# Begin Source File

SOURCE=.\crypt\cryptRandPool.cc
# End Source File
# Begin Source File

SOURCE=.\crypt\cryptSHA1.cc
# End Source File
# End Group
# Begin Group "dgl"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\dgl\bitmapBM8.cc
# End Source File
# Begin Source File

SOURCE=.\dgl\bitmapBMP.cc
# End Source File
# Begin Source File

SOURCE=.\dgl\bitmapGIF.cc
# End Source File
# Begin Source File

SOURCE=.\dgl\bitmapJpeg.cc
# End Source File
# Begin Source File

SOURCE=.\dgl\bitmapPng.cc
# End Source File
# Begin Source File

SOURCE=.\dgl\dgl.cc
# End Source File
# Begin Source File

SOURCE=.\dgl\dglMatrix.cc
# End Source File
# Begin Source File

SOURCE=.\dgl\gBitmap.cc
# End Source File
# Begin Source File

SOURCE=.\dgl\gFont.cc
# End Source File
# Begin Source File

SOURCE=.\dgl\gPalette.cc
# End Source File
# Begin Source File

SOURCE=.\dgl\gTexManager.cc
# End Source File
# Begin Source File

SOURCE=.\dgl\lensFlare.cc
# End Source File
# Begin Source File

SOURCE=.\dgl\materialList.cc
# End Source File
# Begin Source File

SOURCE=.\dgl\materialPropertyMap.cc
# End Source File
# Begin Source File

SOURCE=.\dgl\rectClipper.cc
# End Source File
# Begin Source File

SOURCE=.\dgl\splineUtil.cc
# End Source File
# Begin Source File

SOURCE=.\dgl\stripCache.cc
# End Source File
# End Group
# Begin Group "editor"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\editor\compTest.cc
# End Source File
# Begin Source File

SOURCE=.\editor\creator.cc
# End Source File
# Begin Source File

SOURCE=.\editor\editor.cc
# End Source File
# Begin Source File

SOURCE=.\editor\editorButtonCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\editor\editorCheckboxCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\editor\editTSCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\editor\guiTerrPreviewCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\editor\missionAreaEditor.cc
# End Source File
# Begin Source File

SOURCE=.\editor\terraformer.cc
# End Source File
# Begin Source File

SOURCE=.\editor\terraformer_noise.cc
# End Source File
# Begin Source File

SOURCE=.\editor\terraformerTexture.cc
# End Source File
# Begin Source File

SOURCE=.\editor\terrainActions.cc
# End Source File
# Begin Source File

SOURCE=.\editor\terrainEditor.cc
# End Source File
# Begin Source File

SOURCE=.\editor\worldEditor.cc
# End Source File
# End Group
# Begin Group "game"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\game\ambientAudioManager.cc
# End Source File
# Begin Source File

SOURCE=.\game\audioEmitter.cc
# End Source File
# Begin Source File

SOURCE=.\game\badWordFilter.cc
# End Source File
# Begin Source File

SOURCE=.\game\banList.cc
# End Source File
# Begin Source File

SOURCE=.\game\bombSight.cc
# End Source File
# Begin Source File

SOURCE=.\game\camera.cc
# End Source File
# Begin Source File

SOURCE=.\game\cameraFXMgr.cc
# End Source File
# Begin Source File

SOURCE=.\game\collisionTest.cc
# End Source File
# Begin Source File

SOURCE=.\game\commanderMapIcon.cc
# End Source File
# Begin Source File

SOURCE=.\game\debris.cc
# End Source File
# Begin Source File

SOURCE=.\game\debugView.cc
# End Source File
# Begin Source File

SOURCE=.\game\explosion.cc
# End Source File
# Begin Source File

SOURCE=.\game\fireballAtmosphere.cc
# End Source File
# Begin Source File

SOURCE=.\game\flyingVehicle.cc
# End Source File
# Begin Source File

SOURCE=.\game\forceFieldBare.cc
# End Source File
# Begin Source File

SOURCE=.\game\game.cc
# End Source File
# Begin Source File

SOURCE=.\game\gameBase.cc
# End Source File
# Begin Source File

SOURCE=.\game\gameConnection.cc
# End Source File
# Begin Source File

SOURCE=.\game\gameConnectionEvents.cc
# End Source File
# Begin Source File

SOURCE=.\game\gameConnectionMoves.cc
# End Source File
# Begin Source File

SOURCE=.\game\gameFunctions.cc
# End Source File
# Begin Source File

SOURCE=.\game\gameProcess.cc
# End Source File
# Begin Source File

SOURCE=.\game\gameTSCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\game\guiNoMouseCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\game\guiPlayerView.cc
# End Source File
# Begin Source File

SOURCE=.\game\guiServerBrowser.cc
# End Source File
# Begin Source File

SOURCE=.\game\hoverVehicle.cc
# End Source File
# Begin Source File

SOURCE=.\game\httpObject.cc
# End Source File
# Begin Source File

SOURCE=.\game\item.cc
# End Source File
# Begin Source File

SOURCE=.\game\lightning.cc
# End Source File
# Begin Source File

SOURCE=.\game\linearProjectile.cc
# End Source File
# Begin Source File

SOURCE=.\game\missionArea.cc
# End Source File
# Begin Source File

SOURCE=.\game\missionMarker.cc
# End Source File
# Begin Source File

SOURCE=.\game\motionBlurLine.cc
# End Source File
# Begin Source File

SOURCE=.\game\net.cc
# End Source File
# Begin Source File

SOURCE=.\game\netDispatch.cc
# End Source File
# Begin Source File

SOURCE=.\game\netTest.cc
# End Source File
# Begin Source File

SOURCE=.\game\particleEmitter.cc
# End Source File
# Begin Source File

SOURCE=.\game\particleEngine.cc
# End Source File
# Begin Source File

SOURCE=.\game\physicalZone.cc
# End Source File
# Begin Source File

SOURCE=.\game\platTest.cc
# End Source File
# Begin Source File

SOURCE=.\game\player.cc
# End Source File
# Begin Source File

SOURCE=.\game\precipitation.cc
# End Source File
# Begin Source File

SOURCE=.\game\projBomb.cc
# End Source File
# Begin Source File

SOURCE=.\game\projectile.cc
# End Source File
# Begin Source File

SOURCE=.\game\projELF.cc
# End Source File
# Begin Source File

SOURCE=.\game\projEnergy.cc
# End Source File
# Begin Source File

SOURCE=.\game\projFlareGrenade.cc
# End Source File
# Begin Source File

SOURCE=.\game\projGrenade.cc
# End Source File
# Begin Source File

SOURCE=.\game\projLinearFlare.cc
# End Source File
# Begin Source File

SOURCE=.\game\projRepair.cc
# End Source File
# Begin Source File

SOURCE=.\game\projSeeker.cc
# End Source File
# Begin Source File

SOURCE=.\game\projShockLance.cc
# End Source File
# Begin Source File

SOURCE=.\game\projSniper.cc
# End Source File
# Begin Source File

SOURCE=.\game\projTargeting.cc
# End Source File
# Begin Source File

SOURCE=.\game\projTracer.cc
# End Source File
# Begin Source File

SOURCE=.\game\rigid.cc
# End Source File
# Begin Source File

SOURCE=.\game\scopeAlwaysShape.cc
# End Source File
# Begin Source File

SOURCE=.\game\sensor.cc
# End Source File
# Begin Source File

SOURCE=.\game\serverQuery.cc
# End Source File
# Begin Source File

SOURCE=.\game\shadow.cc
# End Source File
# Begin Source File

SOURCE=.\game\shapeBase.cc
# End Source File
# Begin Source File

SOURCE=.\game\shapeCollision.cc
# End Source File
# Begin Source File

SOURCE=.\game\shapeImage.cc
# End Source File
# Begin Source File

SOURCE=.\game\shieldImpact.cc
# End Source File
# Begin Source File

SOURCE=.\game\shockwave.cc
# End Source File
# Begin Source File

SOURCE=.\game\showTSShape.cc
# End Source File
# Begin Source File

SOURCE=.\game\sphere.cc
# End Source File
# Begin Source File

SOURCE=.\game\splash.cc
# End Source File
# Begin Source File

SOURCE=.\game\staticShape.cc
# End Source File
# Begin Source File

SOURCE=.\game\targetManager.cc
# End Source File
# Begin Source File

SOURCE=.\game\tcpObject.cc
# End Source File
# Begin Source File

SOURCE=.\game\trigger.cc
# End Source File
# Begin Source File

SOURCE=.\game\tsStatic.cc
# End Source File
# Begin Source File

SOURCE=.\game\turret.cc
# End Source File
# Begin Source File

SOURCE=.\game\underLava.cc
# End Source File
# Begin Source File

SOURCE=.\game\vehicle.cc
# End Source File
# Begin Source File

SOURCE=.\game\vehicleBlocker.cc
# End Source File
# Begin Source File

SOURCE=.\game\version.cc
# End Source File
# Begin Source File

SOURCE=.\game\weaponBeam.cc
# End Source File
# Begin Source File

SOURCE=.\game\wheeledVehicle.cc
# End Source File
# End Group
# Begin Group "gui"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\gui\channelVector.cc
# End Source File
# Begin Source File

SOURCE=.\gui\guiArrayCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\gui\guiAviBitmapCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\gui\guiBackgroundCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\gui\guiBitmapCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\gui\guiBubbleTextCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\gui\guiButtonCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\gui\guiCanvas.cc
# End Source File
# Begin Source File

SOURCE=.\gui\guiChannelVectorCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\gui\guiChatMenuTreeCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\gui\guiCheckBoxCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\gui\guiChunkedBitmapCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\gui\guiConsole.cc
# End Source File
# Begin Source File

SOURCE=.\gui\guiConsoleEditCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\gui\guiConsoleTextCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\gui\guiControl.cc
# End Source File
# Begin Source File

SOURCE=.\gui\guiControlListPopup.cc
# End Source File
# Begin Source File

SOURCE=.\gui\guiDebugger.cc
# End Source File
# Begin Source File

SOURCE=.\gui\guiEditCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\gui\guiFilterCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\gui\guiFrameCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\gui\guiInputCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\gui\guiInspector.cc
# End Source File
# Begin Source File

SOURCE=.\gui\guiMessageVectorCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\gui\guiMLTextCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\gui\guiMLTextEditCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\gui\guiMouseEventCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\gui\guiPopUpCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\gui\guiProgressCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\gui\guiRadioCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\gui\guiScrollCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\gui\guiSliderCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\gui\guiTextCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\gui\guiTextEditCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\gui\guiTextEditSliderCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\gui\guiTextListCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\gui\guiTreeViewCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\gui\guiTSControl.cc
# End Source File
# Begin Source File

SOURCE=.\gui\guiTypes.cc
# End Source File
# Begin Source File

SOURCE=.\gui\guiVoteCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\gui\guiWindowCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\gui\messageVector.cc
# End Source File
# End Group
# Begin Group "hud"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\hud\hudBitmapCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\hud\hudBitmapFrameCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\hud\hudClock.cc
# End Source File
# Begin Source File

SOURCE=.\hud\hudCompass.cc
# End Source File
# Begin Source File

SOURCE=.\hud\hudCrosshair.cc
# End Source File
# Begin Source File

SOURCE=.\hud\hudCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\hud\hudEnergyDamage.cc
# End Source File
# End Group
# Begin Group "interior"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\interior\FloorPlanRes.cc
# End Source File
# Begin Source File

SOURCE=.\interior\forceField.cc
# End Source File
# Begin Source File

SOURCE=.\interior\interior.cc
# End Source File
# Begin Source File

SOURCE=.\interior\interiorCollision.cc
# End Source File
# Begin Source File

SOURCE=.\interior\interiorDebug.cc
# End Source File
# Begin Source File

SOURCE=.\interior\interiorInstance.cc
# End Source File
# Begin Source File

SOURCE=.\interior\interiorIO.cc
# End Source File
# Begin Source File

SOURCE=.\interior\interiorLightAnim.cc
# End Source File
# Begin Source File

SOURCE=.\interior\interiorLMManager.cc
# End Source File
# Begin Source File

SOURCE=.\interior\interiorRender.cc
# End Source File
# Begin Source File

SOURCE=.\interior\interiorRes.cc
# End Source File
# Begin Source File

SOURCE=.\interior\interiorResObjects.cc
# End Source File
# Begin Source File

SOURCE=.\interior\interiorSubObject.cc
# End Source File
# Begin Source File

SOURCE=.\interior\itfdump.asm
# End Source File
# Begin Source File

SOURCE=.\interior\lightUpdateGrouper.cc
# End Source File
# Begin Source File

SOURCE=.\interior\mirrorSubObject.cc
# End Source File
# End Group
# Begin Group "math"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\math\mathTypes.cc
# End Source File
# Begin Source File

SOURCE=.\math\mathUtils.cc
# End Source File
# Begin Source File

SOURCE=.\math\mBox.cc
# End Source File
# Begin Source File

SOURCE=.\math\mConsoleFunctions.cc
# End Source File
# Begin Source File

SOURCE=.\math\mMath_C.cc
# End Source File
# Begin Source File

SOURCE=.\math\mMathAMD.cc
# End Source File
# Begin Source File

SOURCE=.\math\mMathFn.cc
# End Source File
# Begin Source File

SOURCE=.\math\mMathSSE.cc
# End Source File
# Begin Source File

SOURCE=.\math\mMatrix.cc
# End Source File
# Begin Source File

SOURCE=.\math\mPlaneTransformer.cc
# End Source File
# Begin Source File

SOURCE=.\math\mQuadPatch.cc
# End Source File
# Begin Source File

SOURCE=.\math\mQuat.cc
# End Source File
# Begin Source File

SOURCE=.\math\mRandom.cc
# End Source File
# Begin Source File

SOURCE=.\math\mSolver.cc
# End Source File
# Begin Source File

SOURCE=.\math\mSplinePatch.cc
# End Source File
# End Group
# Begin Group "platform"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\platform\gameInterface.cc
# End Source File
# Begin Source File

SOURCE=.\platform\platformAssert.cc
# End Source File
# Begin Source File

SOURCE=.\platform\platformMemory.cc
# End Source File
# Begin Source File

SOURCE=.\platform\platformRedBook.cc
# End Source File
# Begin Source File

SOURCE=.\platform\platformVideo.cc
# End Source File
# Begin Source File

SOURCE=.\platform\profiler.cc
# End Source File
# End Group
# Begin Group "platformWIN32"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\platformWIN32\winAsmBlit.cc
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winConsole.cc
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winCPUInfo.cc
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winD3DVideo.cc
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winDInputDevice.cc
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winDirectInput.cc
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winFileio.cc
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winFont.cc
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winGL.cc
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winInput.cc
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winMath.cc
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winMath_ASM.cc
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winMemory.cc
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winMutex.cc
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winNet.cc
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winOGLVideo.cc
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winOpenAL.cc
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winProcessControl.cc
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winRedbook.cc
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winSemaphore.cc
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winStrings.cc
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winThread.cc
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winTime.cc
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winV2Video.cc
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winWindow.cc
# End Source File
# End Group
# Begin Group "scenegraph"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\scenegraph\detailManager.cc
# End Source File
# Begin Source File

SOURCE=.\scenegraph\lightManager.cc
# End Source File
# Begin Source File

SOURCE=.\scenegraph\sceneGraph.cc
# End Source File
# Begin Source File

SOURCE=.\scenegraph\sceneLighting.cc
# End Source File
# Begin Source File

SOURCE=.\scenegraph\sceneRoot.cc
# End Source File
# Begin Source File

SOURCE=.\scenegraph\sceneState.cc
# End Source File
# Begin Source File

SOURCE=.\scenegraph\sceneTraversal.cc
# End Source File
# Begin Source File

SOURCE=.\scenegraph\sgUtil.cc
# End Source File
# Begin Source File

SOURCE=.\scenegraph\shadowVolumeBSP.cc
# End Source File
# Begin Source File

SOURCE=.\scenegraph\windingClipper.cc
# End Source File
# End Group
# Begin Group "shell"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\shell\shellFancyArray.cc
# End Source File
# Begin Source File

SOURCE=.\shell\shellFancyTextList.cc
# End Source File
# Begin Source File

SOURCE=.\shell\shellScrollCtrl.cc
# End Source File
# Begin Source File

SOURCE=.\shell\shellTextEditCtrl.cc
# End Source File
# End Group
# Begin Group "sim"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\sim\actionMap.cc
# End Source File
# Begin Source File

SOURCE=.\sim\cannedChatDataBlock.cc
# End Source File
# Begin Source File

SOURCE=.\sim\decalManager.cc
# End Source File
# Begin Source File

SOURCE=.\sim\frameAllocator.cc
# End Source File
# Begin Source File

SOURCE=.\sim\netConnection.cc
# End Source File
# Begin Source File

SOURCE=.\sim\netEvent.cc
# End Source File
# Begin Source File

SOURCE=.\sim\netGhost.cc
# End Source File
# Begin Source File

SOURCE=.\sim\netObject.cc
# End Source File
# Begin Source File

SOURCE=.\sim\netStringTable.cc
# End Source File
# Begin Source File

SOURCE=.\sim\pathManager.cc
# End Source File
# Begin Source File

SOURCE=.\sim\sceneObject.cc
# End Source File
# Begin Source File

SOURCE=.\sim\simPath.cc
# End Source File
# End Group
# Begin Group "terrain"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\terrain\blender.cc
# End Source File
# Begin Source File

SOURCE=.\terrain\bvQuadTree.cc
# End Source File
# Begin Source File

SOURCE=.\terrain\FluidQuadTree.cc
# End Source File
# Begin Source File

SOURCE=.\terrain\FluidRender.cc
# End Source File
# Begin Source File

SOURCE=.\terrain\FluidSupport.cc
# End Source File
# Begin Source File

SOURCE=.\terrain\Sky.cc
# End Source File
# Begin Source File

SOURCE=.\terrain\Sun.cc
# End Source File
# Begin Source File

SOURCE=.\terrain\terrCollision.cc
# End Source File
# Begin Source File

SOURCE=.\terrain\terrData.cc
# End Source File
# Begin Source File

SOURCE=.\terrain\terrLighting.cc
# End Source File
# Begin Source File

SOURCE=.\terrain\terrRender.cc
# End Source File
# Begin Source File

SOURCE=.\terrain\terrRender2.cc
# End Source File
# Begin Source File

SOURCE=.\terrain\waterBlock.cc
# End Source File
# End Group
# Begin Group "ts"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\ts\tsAnimate.cc
# End Source File
# Begin Source File

SOURCE=.\ts\tsCollision.cc
# End Source File
# Begin Source File

SOURCE=.\ts\tsDecal.cc
# End Source File
# Begin Source File

SOURCE=.\ts\tsDump.cc
# End Source File
# Begin Source File

SOURCE=.\ts\tsIntegerSet.cc
# End Source File
# Begin Source File

SOURCE=.\ts\tsLastDetail.cc
# End Source File
# Begin Source File

SOURCE=.\ts\tsMaterialList.cc
# End Source File
# Begin Source File

SOURCE=.\ts\tsMesh.cc
# End Source File
# Begin Source File

SOURCE=.\ts\tsPartInstance.cc
# End Source File
# Begin Source File

SOURCE=.\ts\tsShape.cc
# End Source File
# Begin Source File

SOURCE=.\ts\tsShapeAlloc.cc
# End Source File
# Begin Source File

SOURCE=.\ts\tsShapeConstruct.cc
# End Source File
# Begin Source File

SOURCE=.\ts\tsShapeInstance.cc
# End Source File
# Begin Source File

SOURCE=.\ts\tsShapeOldRead.cc
# End Source File
# Begin Source File

SOURCE=.\ts\tsSortedMesh.cc
# End Source File
# Begin Source File

SOURCE=.\ts\tsThread.cc
# End Source File
# Begin Source File

SOURCE=.\ts\tsTransform.cc
# End Source File
# End Group
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "ai headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\ai\aiConnection.h
# End Source File
# Begin Source File

SOURCE=.\ai\aiNavJetting.h
# End Source File
# Begin Source File

SOURCE=.\ai\aiNavStep.h
# End Source File
# Begin Source File

SOURCE=.\ai\aiObjective.h
# End Source File
# Begin Source File

SOURCE=.\ai\aiStep.h
# End Source File
# Begin Source File

SOURCE=.\ai\aiTask.h
# End Source File
# Begin Source File

SOURCE=.\ai\graph.h
# End Source File
# Begin Source File

SOURCE=.\ai\graphBase.h
# End Source File
# Begin Source File

SOURCE=.\ai\graphBridge.h
# End Source File
# Begin Source File

SOURCE=.\ai\graphBSP.h
# End Source File
# Begin Source File

SOURCE=.\ai\graphData.h
# End Source File
# Begin Source File

SOURCE=.\ai\graphDefines.h
# End Source File
# Begin Source File

SOURCE=.\ai\graphFloorPlan.h
# End Source File
# Begin Source File

SOURCE=.\ai\graphForceField.h
# End Source File
# Begin Source File

SOURCE=.\ai\graphGenUtils.h
# End Source File
# Begin Source File

SOURCE=.\ai\graphGroundPlan.h
# End Source File
# Begin Source File

SOURCE=.\ai\graphGroundVisit.h
# End Source File
# Begin Source File

SOURCE=.\ai\graphJetting.h
# End Source File
# Begin Source File

SOURCE=.\ai\graphLocate.h
# End Source File
# Begin Source File

SOURCE=.\ai\graphLOS.h
# End Source File
# Begin Source File

SOURCE=.\ai\graphMath.h
# End Source File
# Begin Source File

SOURCE=.\ai\graphNodes.h
# End Source File
# Begin Source File

SOURCE=.\ai\graphPartition.h
# End Source File
# Begin Source File

SOURCE=.\ai\graphPath.h
# End Source File
# Begin Source File

SOURCE=.\ai\graphSearches.h
# End Source File
# Begin Source File

SOURCE=.\ai\graphThreats.h
# End Source File
# Begin Source File

SOURCE=.\ai\graphTransient.h
# End Source File
# Begin Source File

SOURCE=.\ai\oVector.h
# End Source File
# Begin Source File

SOURCE=.\ai\tBinHeap.h
# End Source File
# Begin Source File

SOURCE=.\ai\texturePreload.h
# End Source File
# End Group
# Begin Group "audio headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\audio\audio.h
# End Source File
# Begin Source File

SOURCE=.\audio\audioBuffer.h
# End Source File
# Begin Source File

SOURCE=.\audio\audioCodec.h
# End Source File
# Begin Source File

SOURCE=.\audio\audioCodecMiles.h
# End Source File
# Begin Source File

SOURCE=.\audio\audioDataBlock.h
# End Source File
# Begin Source File

SOURCE=.\audio\audioMss.h
# End Source File
# Begin Source File

SOURCE=.\audio\audioNet.h
# End Source File
# Begin Source File

SOURCE=.\audio\audioThread.h
# End Source File
# Begin Source File

SOURCE=.\audio\bufferQueue.h
# End Source File
# End Group
# Begin Group "collsion headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\collision\abstractPolyList.h
# End Source File
# Begin Source File

SOURCE=.\collision\boxConvex.h
# End Source File
# Begin Source File

SOURCE=.\collision\clippedPolyList.h
# End Source File
# Begin Source File

SOURCE=.\collision\collision.h
# End Source File
# Begin Source File

SOURCE=.\collision\convex.h
# End Source File
# Begin Source File

SOURCE=.\collision\depthSortList.h
# End Source File
# Begin Source File

SOURCE=.\collision\earlyOutPolyList.h
# End Source File
# Begin Source File

SOURCE=.\collision\extrudedPolyList.h
# End Source File
# Begin Source File

SOURCE=.\collision\gjk.h
# End Source File
# Begin Source File

SOURCE=.\collision\planeExtractor.h
# End Source File
# Begin Source File

SOURCE=.\collision\polyhedron.h
# End Source File
# Begin Source File

SOURCE=.\collision\polytope.h
# End Source File
# End Group
# Begin Group "console headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\console\ast.h
# End Source File
# Begin Source File

SOURCE=.\console\compiler.h
# End Source File
# Begin Source File

SOURCE=.\console\console.h
# End Source File
# Begin Source File

SOURCE=.\console\consoleInternal.h
# End Source File
# Begin Source File

SOURCE=.\console\consoleObject.h
# End Source File
# Begin Source File

SOURCE=.\console\consoleTypes.h
# End Source File
# Begin Source File

SOURCE=.\console\gram.h
# End Source File
# Begin Source File

SOURCE=.\console\objectTypes.h
# End Source File
# Begin Source File

SOURCE=.\console\simBase.h
# End Source File
# Begin Source File

SOURCE=.\console\simDictionary.h
# End Source File
# Begin Source File

SOURCE=.\console\telnetConsole.h
# End Source File
# Begin Source File

SOURCE=.\console\telnetDebugger.h
# End Source File
# End Group
# Begin Group "core headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\core\bitMatrix.h
# End Source File
# Begin Source File

SOURCE=.\core\bitRender.h
# End Source File
# Begin Source File

SOURCE=.\core\bitSet.h
# End Source File
# Begin Source File

SOURCE=.\core\bitStream.h
# End Source File
# Begin Source File

SOURCE=.\core\BitTables.h
# End Source File
# Begin Source File

SOURCE=.\core\bitVector.h
# End Source File
# Begin Source File

SOURCE=.\core\bitVectorW.h
# End Source File
# Begin Source File

SOURCE=.\core\color.h
# End Source File
# Begin Source File

SOURCE=.\core\coreRes.h
# End Source File
# Begin Source File

SOURCE=.\core\dataChunker.h
# End Source File
# Begin Source File

SOURCE=.\core\dnet.h
# End Source File
# Begin Source File

SOURCE=.\core\fileio.h
# End Source File
# Begin Source File

SOURCE=.\core\fileObject.h
# End Source File
# Begin Source File

SOURCE=.\core\fileStream.h
# End Source File
# Begin Source File

SOURCE=.\core\filterStream.h
# End Source File
# Begin Source File

SOURCE=.\core\findMatch.h
# End Source File
# Begin Source File

SOURCE=.\core\idGenerator.h
# End Source File
# Begin Source File

SOURCE=.\core\llist.h
# End Source File
# Begin Source File

SOURCE=.\core\memstream.h
# End Source File
# Begin Source File

SOURCE=.\core\polyList.h
# End Source File
# Begin Source File

SOURCE=.\core\realComp.h
# End Source File
# Begin Source File

SOURCE=.\core\resizeStream.h
# End Source File
# Begin Source File

SOURCE=.\core\resManager.h
# End Source File
# Begin Source File

SOURCE=.\core\stream.h
# End Source File
# Begin Source File

SOURCE=.\core\stringTable.h
# End Source File
# Begin Source File

SOURCE=.\core\tagDictionary.h
# End Source File
# Begin Source File

SOURCE=.\core\tAlgorithm.h
# End Source File
# Begin Source File

SOURCE=.\core\tSparseArray.h
# End Source File
# Begin Source File

SOURCE=.\core\tVector.h
# End Source File
# Begin Source File

SOURCE=.\core\zipAggregate.h
# End Source File
# Begin Source File

SOURCE=.\core\zipHeaders.h
# End Source File
# Begin Source File

SOURCE=.\core\zipSubStream.h
# End Source File
# End Group
# Begin Group "crypt headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\crypt\cryptMGF.h
# End Source File
# Begin Source File

SOURCE=.\crypt\cryptRandPool.h
# End Source File
# Begin Source File

SOURCE=.\crypt\cryptSHA1.h
# End Source File
# End Group
# Begin Group "dgl headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\dgl\dgl.h
# End Source File
# Begin Source File

SOURCE=.\dgl\gBitmap.h
# End Source File
# Begin Source File

SOURCE=.\dgl\gChunkedTexManager.h
# End Source File
# Begin Source File

SOURCE=.\dgl\gFont.h
# End Source File
# Begin Source File

SOURCE=.\dgl\gPalette.h
# End Source File
# Begin Source File

SOURCE=.\dgl\gTexManager.h
# End Source File
# Begin Source File

SOURCE=.\dgl\lensFlare.h
# End Source File
# Begin Source File

SOURCE=.\dgl\materialList.h
# End Source File
# Begin Source File

SOURCE=.\dgl\materialPropertyMap.h
# End Source File
# Begin Source File

SOURCE=.\dgl\rectClipper.h
# End Source File
# Begin Source File

SOURCE=.\dgl\splineUtil.h
# End Source File
# Begin Source File

SOURCE=.\dgl\stripCache.h
# End Source File
# End Group
# Begin Group "editor headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\editor\creator.h
# End Source File
# Begin Source File

SOURCE=.\editor\editor.h
# End Source File
# Begin Source File

SOURCE=.\editor\editorButtonCtrl.h
# End Source File
# Begin Source File

SOURCE=.\editor\editorCheckboxCtrl.h
# End Source File
# Begin Source File

SOURCE=.\editor\editTSCtrl.h
# End Source File
# Begin Source File

SOURCE=.\editor\guiTerrPreviewCtrl.h
# End Source File
# Begin Source File

SOURCE=.\editor\missionAreaEditor.h
# End Source File
# Begin Source File

SOURCE=.\editor\terraformer.h
# End Source File
# Begin Source File

SOURCE=.\editor\terraformer_noise.h
# End Source File
# Begin Source File

SOURCE=.\editor\terrainActions.h
# End Source File
# Begin Source File

SOURCE=.\editor\terrainEditor.h
# End Source File
# Begin Source File

SOURCE=.\editor\worldEditor.h
# End Source File
# End Group
# Begin Group "game headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\game\ambientAudioManager.h
# End Source File
# Begin Source File

SOURCE=.\game\audioEmitter.h
# End Source File
# Begin Source File

SOURCE=.\game\auth.h
# End Source File
# Begin Source File

SOURCE=.\game\badWordFilter.h
# End Source File
# Begin Source File

SOURCE=.\game\banList.h
# End Source File
# Begin Source File

SOURCE=.\game\bombSight.h
# End Source File
# Begin Source File

SOURCE=.\game\camera.h
# End Source File
# Begin Source File

SOURCE=.\game\cameraFXMgr.h
# End Source File
# Begin Source File

SOURCE=.\game\collisionTest.h
# End Source File
# Begin Source File

SOURCE=.\game\commanderMapIcon.h
# End Source File
# Begin Source File

SOURCE=.\game\debris.h
# End Source File
# Begin Source File

SOURCE=.\game\debugView.h
# End Source File
# Begin Source File

SOURCE=.\game\explosion.h
# End Source File
# Begin Source File

SOURCE=.\game\fireballAtmosphere.h
# End Source File
# Begin Source File

SOURCE=.\game\flyingVehicle.h
# End Source File
# Begin Source File

SOURCE=.\game\forceFieldBare.h
# End Source File
# Begin Source File

SOURCE=.\game\game.h
# End Source File
# Begin Source File

SOURCE=.\game\gameBase.h
# End Source File
# Begin Source File

SOURCE=.\game\gameConnection.h
# End Source File
# Begin Source File

SOURCE=.\game\gameConnectionEvents.h
# End Source File
# Begin Source File

SOURCE=.\game\gameFunctions.h
# End Source File
# Begin Source File

SOURCE=.\game\gameTSCtrl.h
# End Source File
# Begin Source File

SOURCE=.\game\guiPlayerView.h
# End Source File
# Begin Source File

SOURCE=.\game\guiServerBrowser.h
# End Source File
# Begin Source File

SOURCE=.\game\hoverVehicle.h
# End Source File
# Begin Source File

SOURCE=.\game\httpObject.h
# End Source File
# Begin Source File

SOURCE=.\game\item.h
# End Source File
# Begin Source File

SOURCE=.\game\lightning.h
# End Source File
# Begin Source File

SOURCE=.\game\linearProjectile.h
# End Source File
# Begin Source File

SOURCE=.\game\missionArea.h
# End Source File
# Begin Source File

SOURCE=.\game\missionMarker.h
# End Source File
# Begin Source File

SOURCE=.\game\motionBlurLine.h
# End Source File
# Begin Source File

SOURCE=.\game\moveManager.h
# End Source File
# Begin Source File

SOURCE=.\game\netDispatch.h
# End Source File
# Begin Source File

SOURCE=.\game\objectTypes.h
# End Source File
# Begin Source File

SOURCE=.\game\particleEmitter.h
# End Source File
# Begin Source File

SOURCE=.\game\particleEngine.h
# End Source File
# Begin Source File

SOURCE=.\game\physicalZone.h
# End Source File
# Begin Source File

SOURCE=.\game\player.h
# End Source File
# Begin Source File

SOURCE=.\game\precipitation.h
# End Source File
# Begin Source File

SOURCE=.\game\projBomb.h
# End Source File
# Begin Source File

SOURCE=.\game\projectile.h
# End Source File
# Begin Source File

SOURCE=.\game\projELF.h
# End Source File
# Begin Source File

SOURCE=.\game\projEnergy.h
# End Source File
# Begin Source File

SOURCE=.\game\projFlareGrenade.h
# End Source File
# Begin Source File

SOURCE=.\game\projGrenade.h
# End Source File
# Begin Source File

SOURCE=.\game\projLinearFlare.h
# End Source File
# Begin Source File

SOURCE=.\game\projRepair.h
# End Source File
# Begin Source File

SOURCE=.\game\projSeeker.h
# End Source File
# Begin Source File

SOURCE=.\game\projShockLance.h
# End Source File
# Begin Source File

SOURCE=.\game\projSniper.h
# End Source File
# Begin Source File

SOURCE=.\game\projTargeting.h
# End Source File
# Begin Source File

SOURCE=.\game\projTracer.h
# End Source File
# Begin Source File

SOURCE=.\game\resource.h
# End Source File
# Begin Source File

SOURCE=.\game\rigid.h
# End Source File
# Begin Source File

SOURCE=.\game\sensor.h
# End Source File
# Begin Source File

SOURCE=.\game\serverQuery.h
# End Source File
# Begin Source File

SOURCE=.\game\shadow.h
# End Source File
# Begin Source File

SOURCE=.\game\shapeBase.h
# End Source File
# Begin Source File

SOURCE=.\game\shieldImpact.h
# End Source File
# Begin Source File

SOURCE=.\game\shockwave.h
# End Source File
# Begin Source File

SOURCE=.\game\showTSShape.h
# End Source File
# Begin Source File

SOURCE=.\game\sphere.h
# End Source File
# Begin Source File

SOURCE=.\game\splash.h
# End Source File
# Begin Source File

SOURCE=.\game\staticShape.h
# End Source File
# Begin Source File

SOURCE=.\game\targetManager.h
# End Source File
# Begin Source File

SOURCE=.\game\tcpObject.h
# End Source File
# Begin Source File

SOURCE=.\game\tribesGame.h
# End Source File
# Begin Source File

SOURCE=.\game\trigger.h
# End Source File
# Begin Source File

SOURCE=.\game\tsStatic.h
# End Source File
# Begin Source File

SOURCE=.\game\turret.h
# End Source File
# Begin Source File

SOURCE=.\game\underLava.h
# End Source File
# Begin Source File

SOURCE=.\game\vehicle.h
# End Source File
# Begin Source File

SOURCE=.\game\vehicleBlocker.h
# End Source File
# Begin Source File

SOURCE=.\game\version.h
# End Source File
# Begin Source File

SOURCE=.\game\weaponBeam.h
# End Source File
# Begin Source File

SOURCE=.\game\wheeledVehicle.h
# End Source File
# End Group
# Begin Group "gui headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\gui\channelVector.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiArrayCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiAviBitmapCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiBackgroundCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiBitmapCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiBubbleTextCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiButtonCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiCanvas.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiChannelVectorCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiChatMenuTreeCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiCheckBoxCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiConsole.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiConsoleEditCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiConsoleTextCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiControl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiDebugger.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiEditCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiFilterCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiFrameCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiHelpCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiInputCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiInspector.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiMessageVectorCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiMLTextCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiMLTextEditCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiMouseEventCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiPopUpCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiProgressCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiRadioCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiScrollCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiSliderCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiTextCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiTextEditCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiTextEditSliderCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiTextListCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiTreeViewCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiTSControl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiTypes.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiVoteCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiWindowCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\messageVector.h
# End Source File
# End Group
# Begin Group "hud headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\hud\hudBitmapCtrl.h
# End Source File
# Begin Source File

SOURCE=.\hud\hudBitmapFrameCtrl.h
# End Source File
# Begin Source File

SOURCE=.\hud\hudCtrl.h
# End Source File
# End Group
# Begin Group "interior headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\interior\FloorPlanRes.h
# End Source File
# Begin Source File

SOURCE=.\interior\forceField.h
# End Source File
# Begin Source File

SOURCE=.\interior\interior.h
# End Source File
# Begin Source File

SOURCE=.\interior\interiorInstance.h
# End Source File
# Begin Source File

SOURCE=.\interior\interiorLMManager.h
# End Source File
# Begin Source File

SOURCE=.\interior\interiorRes.h
# End Source File
# Begin Source File

SOURCE=.\interior\interiorResObjects.h
# End Source File
# Begin Source File

SOURCE=.\interior\interiorSubObject.h
# End Source File
# Begin Source File

SOURCE=.\interior\lightUpdateGrouper.h
# End Source File
# Begin Source File

SOURCE=.\interior\mirrorSubObject.h
# End Source File
# End Group
# Begin Group "math headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\math\mathIO.h
# End Source File
# Begin Source File

SOURCE=.\math\mathTypes.h
# End Source File
# Begin Source File

SOURCE=.\math\mathUtils.h
# End Source File
# Begin Source File

SOURCE=.\math\mBox.h
# End Source File
# Begin Source File

SOURCE=.\math\mConstants.h
# End Source File
# Begin Source File

SOURCE=.\math\mMath.h
# End Source File
# Begin Source File

SOURCE=.\math\mMathFn.h
# End Source File
# Begin Source File

SOURCE=.\math\mMatrix.h
# End Source File
# Begin Source File

SOURCE=.\math\mPlane.h
# End Source File
# Begin Source File

SOURCE=.\math\mPlaneTransformer.h
# End Source File
# Begin Source File

SOURCE=.\math\mPoint.h
# End Source File
# Begin Source File

SOURCE=.\math\mQuadPatch.h
# End Source File
# Begin Source File

SOURCE=.\math\mQuat.h
# End Source File
# Begin Source File

SOURCE=.\math\mRandom.h
# End Source File
# Begin Source File

SOURCE=.\math\mRect.h
# End Source File
# Begin Source File

SOURCE=.\math\mSphere.h
# End Source File
# Begin Source File

SOURCE=.\math\mSplinePatch.h
# End Source File
# Begin Source File

SOURCE=.\math\mTrig.h
# End Source File
# End Group
# Begin Group "platform headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\platform\3DFX.H
# End Source File
# Begin Source File

SOURCE=.\platform\event.h
# End Source File
# Begin Source File

SOURCE=.\platform\gameInterface.h
# End Source File
# Begin Source File

SOURCE=.\platform\platform.h
# End Source File
# Begin Source File

SOURCE=.\platform\platformAssert.h
# End Source File
# Begin Source File

SOURCE=.\platform\platformAudio.h
# End Source File
# Begin Source File

SOURCE=.\platform\platformInput.h
# End Source File
# Begin Source File

SOURCE=.\platform\platformMutex.h
# End Source File
# Begin Source File

SOURCE=.\platform\platformRedBook.h
# End Source File
# Begin Source File

SOURCE=.\platform\platformSemaphore.h
# End Source File
# Begin Source File

SOURCE=.\platform\platformThread.h
# End Source File
# Begin Source File

SOURCE=.\platform\platformVideo.h
# End Source File
# Begin Source File

SOURCE=.\platform\profiler.h
# End Source File
# Begin Source File

SOURCE=.\platform\types.h
# End Source File
# Begin Source File

SOURCE=.\platform\typesLinux.h
# End Source File
# Begin Source File

SOURCE=.\platform\typesPPC.h
# End Source File
# Begin Source File

SOURCE=.\platform\typesWin32.h
# End Source File
# End Group
# Begin Group "platformWIN32 headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\platformWIN32\gllist.h
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\platformAL.h
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\platformGL.h
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\platformWin32.h
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winConsole.h
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winD3DVideo.h
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winDInputDevice.h
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winDirectInput.h
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winOGLVideo.h
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winV2Video.h
# End Source File
# End Group
# Begin Group "scenegraph headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\scenegraph\detailManager.h
# End Source File
# Begin Source File

SOURCE=.\scenegraph\lightManager.h
# End Source File
# Begin Source File

SOURCE=.\scenegraph\sceneGraph.h
# End Source File
# Begin Source File

SOURCE=.\scenegraph\sceneLighting.h
# End Source File
# Begin Source File

SOURCE=.\scenegraph\sceneRoot.h
# End Source File
# Begin Source File

SOURCE=.\scenegraph\sceneState.h
# End Source File
# Begin Source File

SOURCE=.\scenegraph\sgUtil.h
# End Source File
# Begin Source File

SOURCE=.\scenegraph\shadowVolumeBSP.h
# End Source File
# Begin Source File

SOURCE=.\scenegraph\windingClipper.h
# End Source File
# End Group
# Begin Group "shell headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\shell\shellFancyArray.h
# End Source File
# Begin Source File

SOURCE=.\shell\shellFancyTextList.h
# End Source File
# Begin Source File

SOURCE=.\shell\shellScrollCtrl.h
# End Source File
# Begin Source File

SOURCE=.\shell\shellTextEditCtrl.h
# End Source File
# End Group
# Begin Group "sim headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\sim\actionMap.h
# End Source File
# Begin Source File

SOURCE=.\sim\cannedChatDataBlock.h
# End Source File
# Begin Source File

SOURCE=.\sim\decalManager.h
# End Source File
# Begin Source File

SOURCE=.\sim\frameAllocator.h
# End Source File
# Begin Source File

SOURCE=.\sim\netConnection.h
# End Source File
# Begin Source File

SOURCE=.\sim\netObject.h
# End Source File
# Begin Source File

SOURCE=.\sim\netStringTable.h
# End Source File
# Begin Source File

SOURCE=.\sim\pathManager.h
# End Source File
# Begin Source File

SOURCE=.\sim\sceneObject.h
# End Source File
# Begin Source File

SOURCE=.\sim\simPath.h
# End Source File
# End Group
# Begin Group "terrain headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\terrain\blender.h
# End Source File
# Begin Source File

SOURCE=.\terrain\bvQuadTree.h
# End Source File
# Begin Source File

SOURCE=.\terrain\Fluid.h
# End Source File
# Begin Source File

SOURCE=.\terrain\Sky.h
# End Source File
# Begin Source File

SOURCE=.\terrain\Sun.h
# End Source File
# Begin Source File

SOURCE=.\terrain\terrData.h
# End Source File
# Begin Source File

SOURCE=.\terrain\terrRender.h
# End Source File
# Begin Source File

SOURCE=.\terrain\waterBlock.h
# End Source File
# End Group
# Begin Group "ts headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\ts\tsDecal.h
# End Source File
# Begin Source File

SOURCE=.\ts\tsIntegerSet.h
# End Source File
# Begin Source File

SOURCE=.\ts\tsLastDetail.h
# End Source File
# Begin Source File

SOURCE=.\ts\tsMesh.h
# End Source File
# Begin Source File

SOURCE=.\ts\tsPartInstance.h
# End Source File
# Begin Source File

SOURCE=.\ts\tsShape.h
# End Source File
# Begin Source File

SOURCE=.\ts\tsShapeAlloc.h
# End Source File
# Begin Source File

SOURCE=.\ts\tsShapeConstruct.h
# End Source File
# Begin Source File

SOURCE=.\ts\tsShapeInstance.h
# End Source File
# Begin Source File

SOURCE=.\ts\tsSortedMesh.h
# End Source File
# Begin Source File

SOURCE=.\ts\tsTransform.h
# End Source File
# End Group
# End Group
# Begin Source File

SOURCE=.\targets.v12.mk
# End Source File
# End Target
# End Project
