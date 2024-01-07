SOURCE.AI=\
	ai/aiConnection.cc \
#	ai/aiPlayer.cc

SOURCE.AUDIO=\
	audio/audio.cc \
	audio/audioBuffer.cc \
	audio/audioDataBlock.cc \
	audio/audioFunctions.cc \

SOURCE.COLLISION=\
	collision/abstractPolyList.cc \
	collision/boxConvex.cc \
	collision/clippedPolyList.cc \
	collision/convex.cc \
	collision/depthSortList.cc \
	collision/earlyOutPolyList.cc \
	collision/extrudedPolyList.cc \
	collision/gjk.cc \
	collision/planeExtractor.cc \
	collision/polyhedron.cc \
	collision/polytope.cc 

SOURCE.CONSOLE=\
	console/compiledEval.cc \
	console/compiler.cc \
	console/console.cc \
	console/consoleFunctions.cc \
	console/consoleInternal.cc \
	console/consoleObject.cc \
	console/consoleTypes.cc \
	console/gram.cc \
	console/scan.cc \
	console/scriptObject.cc \
	console/simBase.cc \
	console/simDictionary.cc \
	console/simManager.cc \
	console/telnetConsole.cc \
	console/telnetDebugger.cc \
    console/consoleDoc.cc \
    console/typeValidators.cc
	
#	console/yylex.c \
#	console/yyparse.c

SOURCE.CORE=\
	core/bitTables.cc \
	core/bitRender.cc \
	core/bitStream.cc \
	core/dataChunker.cc \
	core/dnet.cc \
	core/fileObject.cc \
	core/fileStream.cc \
	core/filterStream.cc \
	core/findMatch.cc \
	core/idGenerator.cc \
	core/memStream.cc \
	core/nStream.cc \
	core/nTypes.cc \
	core/resDictionary.cc \
	core/resManager.cc \
	core/resizeStream.cc \
	core/stringTable.cc \
	core/tVector.cc \
	core/tagDictionary.cc \
	core/zipAggregate.cc \
	core/zipHeaders.cc \
	core/zipSubStream.cc \
	core/crc.cc

SOURCE.DGL=\
	dgl/bitmapBm8.cc \
	dgl/bitmapBmp.cc \
	dgl/bitmapGif.cc \
	dgl/bitmapJpeg.cc \
	dgl/bitmapPng.cc \
	dgl/dgl.cc \
	dgl/dglMatrix.cc \
	dgl/gBitmap.cc \
	dgl/gFont.cc \
	dgl/gPalette.cc \
	dgl/gTexManager.cc \
	dgl/lensFlare.cc \
	dgl/materialList.cc \
	dgl/materialPropertyMap.cc \
	dgl/rectClipper.cc \
	dgl/splineUtil.cc \
	dgl/stripCache.cc 

SOURCE.EDITOR=\
	editor/creator.cc \
	editor/editTSCtrl.cc \
	editor/editor.cc \
	editor/guiTerrPreviewCtrl.cc \
	editor/missionAreaEditor.cc \
	editor/terraformer.cc \
	editor/terraformerTexture.cc \
	editor/terraformerNoise.cc \
	editor/terrainActions.cc \
	editor/terrainEditor.cc \
	editor/worldEditor.cc 

SOURCE.GUI=\
	gui/guiArrayCtrl.cc \
	gui/guiAviBitmapCtrl.cc \
	gui/guiBackgroundCtrl.cc \
	gui/guiBitmapCtrl.cc \
	gui/guiBubbleTextCtrl.cc \
	gui/guiButtonBaseCtrl.cc \
	gui/guiButtonCtrl.cc \
	gui/guiCanvas.cc \
	gui/guiCheckBoxCtrl.cc \
	gui/guiChunkedBitmapCtrl.cc \
	gui/guiConsole.cc \
	gui/guiConsoleEditCtrl.cc \
	gui/guiConsoleTextCtrl.cc \
	gui/guiControl.cc \
	gui/guiControlListPopup.cc \
	gui/guiDebugger.cc \
	gui/guiEditCtrl.cc \
	gui/guiFilterCtrl.cc \
	gui/guiFrameCtrl.cc \
	gui/guiInputCtrl.cc \
	gui/guiInspector.cc \
	gui/guiMLTextCtrl.cc \
	gui/guiMLTextEditCtrl.cc \
	gui/guiMessageVectorCtrl.cc \
	gui/guiMouseEventCtrl.cc \
	gui/guiPopUpCtrl.cc \
	gui/guiProgressCtrl.cc \
	gui/guiRadioCtrl.cc \
	gui/guiScrollCtrl.cc \
	gui/guiSliderCtrl.cc \
	gui/guiTSControl.cc \
	gui/guiTextCtrl.cc \
	gui/guiTextEditCtrl.cc \
	gui/guiTextEditSliderCtrl.cc \
	gui/guiTextListCtrl.cc \
	gui/guiTreeViewCtrl.cc \
	gui/guiTypes.cc \
	gui/guiWindowCtrl.cc \
	gui/messageVector.cc \
	
#	gui/guiHelpCtrl.cc \

SOURCE.HUD=\
   hud/hudBarDisplayCtrl.cc \
   hud/hudBezierDisplayCtrl.cc \
   hud/hudBitmapCtrl.cc \
   hud/hudClockCtrl.cc \
   hud/hudCrosshair.cc \
   hud/hudGLEx.cc \
   hud/hudHealthCtrl.cc \
   hud/hudObject.cc \
   hud/hudZoom.cc \
   hud/mBezier2D.cc

SOURCE.INTERIOR=\
	interior/floorPlanRes.cc \
	interior/forceField.cc \
	interior/itfdump.asm \
	interior/interior.cc \
	interior/interiorCollision.cc \
	interior/interiorDebug.cc \
	interior/interiorIO.cc \
	interior/interiorInstance.cc \
	interior/interiorLMManager.cc \
	interior/interiorLightAnim.cc \
	interior/interiorRender.cc \
	interior/interiorRes.cc \
	interior/interiorResObjects.cc \
	interior/interiorSubObject.cc \
	interior/lightUpdateGrouper.cc \
	interior/mirrorSubObject.cc 
	
SOURCE.MATH=\
	math/mBox.cc \
	math/mConsoleFunctions.cc \
	math/mMathFn.cc \
	math/mMath_C.cc \
	math/mMatrix.cc \
	math/mPlaneTransformer.cc \
	math/mQuadPatch.cc \
	math/mQuat.cc \
	math/mRandom.cc \
	math/mSolver.cc \
	math/mSplinePatch.cc \
	math/mathTypes.cc \
	math/mathUtils.cc \
	math/mMathAMD.cc \
	math/mMathAMD_ASM.asm \
	math/mMathSSE.cc \
	math/mMathSSE_ASM.asm

SOURCE.PLATFORM=\
	platform/gameInterface.cc \
	platform/platformAssert.cc \
	platform/platformMemory.cc \
	platform/platformRedBook.cc \
	platform/platformVideo.cc \
	platform/profiler.cc 

SOURCE.PLATFORMPPC=\
	platformPPC/ppcAudio.cc \
	platformPPC/ppcCPUInfo.cc \
	platformPPC/ppcConsole.cc \
	platformPPC/ppcFileio.cc \
	platformPPC/ppcFont.cc \
	platformPPC/ppcGL.cc \
	platformPPC/ppcInput.cc \
	platformPPC/ppcMath.cc \
	platformPPC/ppcMemory.cc \
	platformPPC/ppcNet.cc \
	platformPPC/ppcOGLVideo.cc \
	platformPPC/ppcProcessControl.cc \
	platformPPC/ppcStrings.cc \
	platformPPC/ppcTime.cc \
	platformPPC/ppcUtils.cc \
	platformPPC/ppcWindow.cc 

SOURCE.PLATFORMWIN32=\
	platformWin32/winAsmBlit.cc \
	platformWin32/winCPUInfo.cc \
	platformWin32/winConsole.cc \
	platformWin32/winD3DVideo.cc \
	platformWin32/winDInputDevice.cc \
	platformWin32/winDirectInput.cc \
	platformWin32/winFileio.cc \
	platformWin32/winFont.cc \
	platformWin32/winGL.cc \
	platformWin32/winInput.cc \
	platformWin32/winMath.cc \
	platformWin32/winMath_ASM.cc \
	platformWin32/winMemory.cc \
	platformWin32/winMutex.cc \
	platformWin32/winNet.cc \
	platformWin32/winOGLVideo.cc \
	platformWin32/winOpenAL.cc \
	platformWin32/winProcessControl.cc \
	platformWin32/winRedbook.cc \
	platformWin32/winSemaphore.cc \
	platformWin32/winStrings.cc \
	platformWin32/winThread.cc \
	platformWin32/winTime.cc \
	platformWin32/winV2Video.cc \
	platformWin32/winWindow.cc \

SOURCE.SIM=\
	sim/actionMap.cc \
	sim/decalManager.cc \
	sim/frameAllocator.cc \
	sim/netConnection.cc \
	sim/netEvent.cc \
	sim/netGhost.cc \
	sim/netObject.cc \
	sim/netStringTable.cc \
	sim/pathManager.cc \
	sim/sceneObject.cc \
	sim/simPath.cc 

SOURCE.GAME=\
	game/main.cc \
	game/debris.cc \
	game/debugView.cc \
	game/gameFunctions.cc \
	game/ambientAudioManager.cc \
	game/audioEmitter.cc \
	game/banList.cc \
	game/camera.cc \
	game/cameraFXMgr.cc \
	game/collisionTest.cc \
	game/explosion.cc \
	game/flyingVehicle.cc \
	game/game.cc \
	game/gameBase.cc \
	game/gameConnection.cc \
	game/gameConnectionEvents.cc \
	game/gameConnectionMoves.cc \
	game/gameProcess.cc \
	game/gameTSCtrl.cc \
	game/guiNoMouseCtrl.cc \
	game/guiPlayerView.cc \
	game/hoverVehicle.cc \
	game/httpObject.cc \
	game/item.cc \
	game/lightning.cc \
	game/missionArea.cc \
	game/missionMarker.cc \
	game/net.cc \
	game/netDispatch.cc \
	game/netTest.cc \
	game/particleEmitter.cc \
	game/particleEngine.cc \
	game/physicalZone.cc \
	game/player.cc \
	game/precipitation.cc \
	game/projectile.cc \
	game/rigid.cc \
	game/scopeAlwaysShape.cc \
	game/serverQuery.cc \
	game/shadow.cc \
	game/shapeBase.cc \
	game/shapeCollision.cc \
	game/shapeImage.cc \
	game/showTSShape.cc \
	game/sphere.cc \
	game/splash.cc \
	game/staticShape.cc \
	game/tcpObject.cc \
	game/trigger.cc \
	game/tsStatic.cc \
	game/underLava.cc \
	game/vehicle.cc \
	game/vehicleBlocker.cc \
	game/wheeledVehicle.cc \
	game/version.cc

SOURCE.GAME.FPS=\
	game/fps/shapeNameHud.cc

SOURCE.PLATFORMGAY=\
	platformLinux/linuxAsmBlit.cc \
	platformLinux/linuxCPUInfo.cc \
	platformLinux/linuxConsole.cc \
	platformLinux/linuxFileio.cc \
	platformLinux/linuxFont.cc \
	platformLinux/linuxGL.cc \
	platformLinux/linuxInput.cc \
	platformLinux/linuxIO.cc \
	platformLinux/linuxMath.cc \
	platformLinux/linuxMemory.cc \
	platformLinux/linuxMutex.cc \
	platformLinux/linuxNet.cc \
	platformLinux/linuxOGLVideo.cc \
	platformLinux/linuxOpenAL.cc \
	platformLinux/linuxProcessControl.cc \
	platformLinux/linuxSemaphore.cc \
	platformLinux/linuxStrings.cc \
	platformLinux/linuxThread.cc \
	platformLinux/linuxTime.cc \
	platformLinux/linuxWindow.cc 

######################################### this is dead
#	platformLinux/linuxAL.cc \
#	platformLinux/linuxALStub.cc \
#	platformLinux/linuxCodeMap.cc \
#	platformLinux/linuxRedBook.cc \
#########################################

SOURCE.PLATFORMLINUX=\
	platformX86UNIX/x86UNIXAsmBlit.cc \
	platformX86UNIX/x86UNIXCPUInfo.cc \
	platformX86UNIX/x86UNIXConsole.cc \
	platformX86UNIX/x86UNIXFileio.cc \
	platformX86UNIX/x86UNIXFont.cc \
	platformX86UNIX/x86UNIXGL.cc \
	platformX86UNIX/x86UNIXInput.cc \
	platformX86UNIX/x86UNIXIO.cc \
	platformX86UNIX/x86UNIXMath.cc \
	platformX86UNIX/x86UNIXMemory.cc \
	platformX86UNIX/x86UNIXMutex.cc \
	platformX86UNIX/x86UNIXNet.cc \
	platformX86UNIX/x86UNIXOGLVideo.cc \
	platformX86UNIX/x86UNIXOpenAL.cc \
	platformX86UNIX/x86UNIXProcessControl.cc \
	platformX86UNIX/x86UNIXSemaphore.cc \
	platformX86UNIX/x86UNIXStrings.cc \
	platformX86UNIX/x86UNIXThread.cc \
	platformX86UNIX/x86UNIXTime.cc \
	platformX86UNIX/x86UNIXWindow.cc 

SOURCE.PLATFORMOpenBSD=\
	platformX86UNIX/x86UNIXAsmBlit.cc \
	platformX86UNIX/x86UNIXCPUInfo.cc \
	platformX86UNIX/x86UNIXConsole.cc \
	platformX86UNIX/x86UNIXFileio.cc \
	platformX86UNIX/x86UNIXFont.cc \
	platformX86UNIX/x86UNIXGL.cc \
	platformX86UNIX/x86UNIXInput.cc \
	platformX86UNIX/x86UNIXIO.cc \
	platformX86UNIX/x86UNIXMath.cc \
	platformX86UNIX/x86UNIXMemory.cc \
	platformX86UNIX/x86UNIXMutex.cc \
	platformX86UNIX/x86UNIXNet.cc \
	platformX86UNIX/x86UNIXOGLVideo.cc \
	platformX86UNIX/x86UNIXOpenAL.cc \
	platformX86UNIX/x86UNIXProcessControl.cc \
	platformX86UNIX/x86UNIXSemaphore.cc \
	platformX86UNIX/x86UNIXStrings.cc \
	platformX86UNIX/x86UNIXThread.cc \
	platformX86UNIX/x86UNIXTime.cc \
	platformX86UNIX/x86UNIXWindow.cc 

SOURCE.SCENEGRAPH=\
	sceneGraph/detailManager.cc \
	sceneGraph/lightManager.cc \
	sceneGraph/sceneGraph.cc \
	sceneGraph/sceneLighting.cc \
	sceneGraph/sceneRoot.cc \
	sceneGraph/sceneState.cc \
	sceneGraph/sceneTraversal.cc \
	sceneGraph/sgUtil.cc \
	sceneGraph/shadowVolumeBSP.cc \
	sceneGraph/windingClipper.cc 

SOURCE.TERRAIN=\
	terrain/fluidQuadTree.cc \
	terrain/fluidRender.cc \
	terrain/fluidSupport.cc \
	terrain/sky.cc \
	terrain/sun.cc \
	terrain/blender.cc \
	terrain/bvQuadTree.cc \
	terrain/terrCollision.cc \
	terrain/terrData.cc \
	terrain/terrLighting.cc \
	terrain/terrRender.cc \
	terrain/terrRender2.cc \
	terrain/waterBlock.cc 

SOURCE.TS=\
	ts/tsAnimate.cc \
	ts/tsCollision.cc \
	ts/tsDecal.cc \
	ts/tsDump.cc \
	ts/tsIntegerSet.cc \
	ts/tsLastDetail.cc \
	ts/tsMaterialList.cc \
	ts/tsMesh.cc \
	ts/tsPartInstance.cc \
	ts/tsShape.cc \
	ts/tsShapeAlloc.cc \
	ts/tsShapeConstruct.cc \
	ts/tsShapeInstance.cc \
	ts/tsShapeOldRead.cc \
	ts/tsSortedMesh.cc \
	ts/tsThread.cc \
	ts/tsTransform.cc 


SOURCE.ENGINE =\
	$(SOURCE.COLLISION) \
	$(SOURCE.CONSOLE) \
	$(SOURCE.CORE) \
	$(SOURCE.DGL) \
	$(SOURCE.INTERIOR) \
	$(SOURCE.MATH) \
	$(SOURCE.PLATFORM) \
	$(SOURCE.PLATFORM$(OS)) \
	$(SOURCE.SCENEGRAPH) \
	$(SOURCE.SIM) \
	$(SOURCE.TERRAIN) \
	$(SOURCE.TS)


SOURCE.TESTAPP =\
	$(SOURCE.AI) \
	$(SOURCE.AUDIO) \
	$(SOURCE.COLLISION) \
	$(SOURCE.CONSOLE) \
	$(SOURCE.CORE) \
	$(SOURCE.DGL) \
	$(SOURCE.EDITOR) \
	$(SOURCE.GUI) \
	$(SOURCE.GAME) \
	$(SOURCE.GAME.FPS) \
	$(SOURCE.HUD) \
	$(SOURCE.INTERIOR) \
	$(SOURCE.MATH) \
	$(SOURCE.PLATFORM) \
	$(SOURCE.PLATFORM$(OS)) \
	$(SOURCE.SCENEGRAPH) \
	$(SOURCE.SIM) \
	$(SOURCE.TERRAIN) \
	$(SOURCE.TS) 


SOURCE.TESTAPP.OBJ:=$(addprefix $(DIR.OBJ)/, $(addsuffix $O, $(basename $(SOURCE.TESTAPP))) )
SOURCE.ENGINE.OBJ:=$(addprefix $(DIR.OBJ)/, $(addsuffix $O, $(basename $(SOURCE.ENGINE))) )
SOURCE.ALL += $(SOURCE.TESTAPP)
targetsclean += torqueClean


#----------------------------------------
torqueDemo: $(DIR.OBJ)/torqueDemo$(EXT.EXE)

$(DIR.OBJ)/torqueDemo$(EXT.EXE): CFLAGS += -I../lib/zlib -I../lib/lungif -I../lib/lpng -I../lib/ljpeg -I- -I../lib/directx8 -I../lib/openal/$(OS) \
	-DPNG_NO_READ_TIME -DPNG_NO_WRITE_TIME \

$(DIR.OBJ)/torqueDemo$(EXT.EXE): LIB.PATH +=../lib/$(DIR.OBJ) \

$(DIR.OBJ)/torqueDemo$(EXT.EXE): LINK.LIBS.GENERAL += \
	$(PRE.LIBRARY.LIB)ljpeg$(EXT.LIB) \
	$(PRE.LIBRARY.LIB)lpng$(EXT.LIB) \
	$(PRE.LIBRARY.LIB)lungif$(EXT.LIB) \
	$(PRE.LIBRARY.LIB)zlib$(EXT.LIB)


$(DIR.OBJ)/torqueDemo$(EXT.EXE): dirlist $(SOURCE.TESTAPP.OBJ) 
	$(DO.LINK.CONSOLE.EXE)
	$(CP) $(DIR.OBJ)/torqueDemo$(BUILD_SUFFIX).* ../example


#----------------------------------------
$(DIR.OBJ)/engine$(EXT.LIB): CFLAGS += -I../lib/zlib -I../lib/lungif -I../lib/lpng -I../lib/ljpeg -I- -I../lib/directx8 -I../lib/openal/$(OS)  \
	-DPNG_NO_READ_tIME -DPNG_NO_WRITE_TIME -DMAX_UTIL \

$(DIR.OBJ)/engine$(EXT.LIB): dirlist $(SOURCE.ENGINE.OBJ)
	$(DO.LINK.LIB)


#----------------------------------------
torqueClean:
ifneq ($(wildcard torqueDemo_DEBUG.*),)
	-$(RM)  torqueDemo$(BUILD_SUFFIX)*
endif
ifneq ($(wildcard torqueDemo_RELEASE.*),)
	-$(RM)  torqueDemo_RELEASE*
endif

