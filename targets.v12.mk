V12.AI=\
	ai/aiConnection.cc \
	ai/aiConsole.cc \
	ai/aiDebug.cc \
	ai/aiNavJetting.cc \
	ai/aiNavStep.cc \
	ai/aiObjective.cc \
	ai/aiStep.cc \
	ai/aiTask.cc \
	ai/graph.cc \
	ai/graphBase.cc \
	ai/graphBridge.cc \
	ai/graphBuildLOS.cc \
	ai/graphConjoin.cc \
	ai/graphData.cc \
	ai/graphDebug.cc \
	ai/graphDijkstra.cc \
	ai/graphFind.cc \
	ai/graphFloorPlan.cc \
	ai/graphFloorRender.cc \
	ai/graphForceField.cc \
	ai/graphGenUtils.cc \
	ai/graphGroundPlan.cc \
	ai/graphIndoors.cc \
	ai/graphIsland.cc \
	ai/graphJetting.cc \
	ai/graphLOS.cc \
	ai/graphLocate.cc \
	ai/graphMake.cc \
	ai/graphMath.cc \
	ai/graphOutdoors.cc \
	ai/graphPartition.cc \
	ai/graphPath.cc \
	ai/graphQueries.cc \
	ai/graphRender.cc \
	ai/graphSearchLOS.cc \
	ai/graphSmooth.cc \
	ai/graphSpawn.cc \
	ai/graphThreats.cc \
	ai/graphTransient.cc \
	ai/graphVolume.cc 

V12.AUDIO=\
	audio/audio.cc \
	audio/audioBuffer.cc \
	audio/audioDataBlock.cc \
	audio/audioMss.cc \
	audio/audioNet.cc \
	audio/audioThread.cc \
	audio/audioCodec.cc \
	audio/audioCodecMiles.cc \
	audio/bufferQueue.cc \

#	audio/audioCodecGSM.cc \


V12.COLLISION=\
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

V12.CONSOLE=\
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
	console/telnetDebugger.cc 
	
#	console/yylex.c \
#	console/yyparse.c

V12.CORE=\
	core/BitTables.cc \
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
	core/zipSubStream.cc 

V12.CRYPT=\
	crypt/cryptMGF.cc \
	crypt/cryptRandPool.cc \
	crypt/cryptSHA1.cc 

V12.DGL=\
	dgl/bitmapBM8.cc \
	dgl/bitmapBMP.cc \
	dgl/bitmapGIF.cc \
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

V12.EDITOR=\
	editor/compTest.cc \
	editor/creator.cc \
	editor/editTSCtrl.cc \
	editor/editor.cc \
	editor/editorButtonCtrl.cc \
	editor/editorCheckboxCtrl.cc \
	editor/guiTerrPreviewCtrl.cc \
	editor/missionAreaEditor.cc \
	editor/terraformer.cc \
	editor/terraformerTexture.cc \
	editor/terraformer_noise.cc \
	editor/terrainActions.cc \
	editor/terrainEditor.cc \
	editor/worldEditor.cc 

V12.GUI=\
	gui/channelVector.cc \
	gui/guiArrayCtrl.cc \
	gui/guiAviBitmapCtrl.cc \
	gui/guiBackgroundCtrl.cc \
	gui/guiBitmapCtrl.cc \
	gui/guiBubbleTextCtrl.cc \
	gui/guiButtonCtrl.cc \
	gui/guiCanvas.cc \
	gui/guiChannelVectorCtrl.cc \
	gui/guiChatMenuTreeCtrl.cc \
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
	gui/guiVoteCtrl.cc \
	gui/guiWindowCtrl.cc \
	gui/messageVector.cc \
	
#	gui/guiHelpCtrl.cc \

V12.HUD=\
	hud/hudCtrl.cc \
	hud/hudBitmapCtrl.cc \
	hud/hudBitmapFrameCtrl.cc \
	hud/hudClock.cc \
	hud/hudCompass.cc \
	hud/hudCrosshair.cc \
	hud/hudEnergyDamage.cc \

V12.INTERIOR=\
	interior/FloorPlanRes.cc \
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
	
V12.MATH=\
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
	math/mMathSSE.cc

V12.PLATFORM=\
	platform/gameInterface.cc \
	platform/platformAssert.cc \
	platform/platformMemory.cc \
	platform/platformRedBook.cc \
	platform/platformVideo.cc \
	platform/profiler.cc 

V12.PLATFORMPPC=\
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

V12.PLATFORMWIN32=\
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

#	platformWin32/D3DGL.cc \
#	platformWin32/GLU2D3D.cc \
#	platformWin32/OpenGL2D3D.cc \


V12.SHELL=\
	shell/shellFancyArray.cc \
	shell/shellFancyTextList.cc \
	shell/shellScrollCtrl.cc \
	shell/shellTextEditCtrl.cc \

V12.SIM=\
	sim/actionMap.cc \
	sim/cannedChatDataBlock.cc \
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

V12.GAME=\
	game/debris.cc \
	game/debugView.cc \
	game/gameFunctions.cc \
	game/stationFXPersonal.cc \
	game/stationFXVehicle.cc \
	game/ambientAudioManager.cc \
	game/audioEmitter.cc \
	game/badWordFilter.cc \
	game/banList.cc \
	game/bombSight.cc \
	game/camera.cc \
	game/cameraFXMgr.cc \
	game/collisionTest.cc \
	game/commanderMapIcon.cc \
	game/explosion.cc \
	game/fireballAtmosphere.cc \
	game/flyingVehicle.cc \
	game/forceFieldBare.cc \
	game/game.cc \
	game/gameBase.cc \
	game/gameConnection.cc \
	game/gameConnectionEvents.cc \
	game/gameConnectionMoves.cc \
	game/gameProcess.cc \
	game/gameTSCtrl.cc \
	game/guiNoMouseCtrl.cc \
	game/guiPlayerView.cc \
	game/guiServerBrowser.cc \
	game/hoverVehicle.cc \
	game/httpObject.cc \
	game/item.cc \
	game/lightning.cc \
	game/linearProjectile.cc \
	game/missionArea.cc \
	game/missionMarker.cc \
	game/motionBlurLine.cc \
	game/net.cc \
	game/netDispatch.cc \
	game/netTest.cc \
	game/particleEmitter.cc \
	game/particleEngine.cc \
	game/physicalZone.cc \
	game/platTest.cc \
	game/player.cc \
	game/precipitation.cc \
	game/projBomb.cc \
	game/projELF.cc \
	game/projEnergy.cc \
	game/projFlareGrenade.cc \
	game/projGrenade.cc \
	game/projLinearFlare.cc \
	game/projRepair.cc \
	game/projSeeker.cc \
	game/projShockLance.cc \
	game/projSniper.cc \
	game/projTargeting.cc \
	game/projTracer.cc \
	game/projectile.cc \
	game/rigid.cc \
	game/scopeAlwaysShape.cc \
	game/sensor.cc \
	game/serverQuery.cc \
	game/shadow.cc \
	game/shapeBase.cc \
	game/shapeCollision.cc \
	game/shapeImage.cc \
	game/shieldImpact.cc \
	game/shockwave.cc \
	game/showTSShape.cc \
	game/sphere.cc \
	game/splash.cc \
	game/staticShape.cc \
	game/targetManager.cc \
	game/tcpObject.cc \
	game/trigger.cc \
	game/tsStatic.cc \
	game/turret.cc \
	game/underLava.cc \
	game/vehicle.cc \
	game/vehicleBlocker.cc \
	game/weaponBeam.cc \
	game/wheeledVehicle.cc \
	game/version.cc

V12.PLATFORMLINUX=\
	platformLinux/audio.cc \
	platformLinux/linuxAL.cc \
	platformLinux/linuxALStub.cc \
	platformLinux/linuxAsmBlit.cc \
	platformLinux/linuxCPUInfo.cc \
	platformLinux/linuxCodeMap.cc \
	platformLinux/linuxConsole.cc \
	platformLinux/linuxFileio.cc \
	platformLinux/linuxFont.cc \
	platformLinux/linuxGL.cc \
	platformLinux/linuxInput.cc \
	platformLinux/linuxMath.cc \
	platformLinux/linuxMemory.cc \
	platformLinux/linuxMutex.cc \
	platformLinux/linuxNet.cc \
	platformLinux/linuxOGLVideo.cc \
	platformLinux/linuxProcessControl.cc \
	platformLinux/linuxRedBook.cc \
	platformLinux/linuxSemaphore.cc \
	platformLinux/linuxStrings.cc \
	platformLinux/linuxThread.cc \
	platformLinux/linuxTime.cc \
	platformLinux/linuxWindow.cc \
	platformLinux/linuxOpenAL.cc \
	platformLinux/lokiOpenAL.cc

V12.SCENEGRAPH=\
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

V12.TERRAIN=\
	terrain/FluidQuadTree.cc \
	terrain/FluidRender.cc \
	terrain/FluidSupport.cc \
	terrain/Sky.cc \
	terrain/Sun.cc \
	terrain/blender.cc \
	terrain/bvQuadTree.cc \
	terrain/terrCollision.cc \
	terrain/terrData.cc \
	terrain/terrLighting.cc \
	terrain/terrRender.cc \
	terrain/terrRender2.cc \
	terrain/waterBlock.cc 

V12.TS=\
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

V12.ALL=\
	$(V12.AI) \
	$(V12.AUDIO) \
	$(V12.COLLISION) \
	$(V12.CONSOLE) \
	$(V12.CORE) \
	$(V12.CRYPT) \
	$(V12.DGL) \
	$(V12.EDITOR) \
	$(V12.GUI) \
	$(V12.HUD) \
	$(V12.INTERIOR) \
	$(V12.MATH) \
	$(V12.PLATFORM) \
	$(V12.PLATFORMWIN32) \
	$(V12.SHELL) \
	$(V12.SIM) \
	$(V12.SCENEGRAPH) \
	$(V12.TS) \
	$(V12.TERRAIN) \
	$(V12.GAME) \




OBJ.ALL:=$(addprefix $(DIR.OBJ)/, $(addsuffix $O, $(basename $(V12.ALL))) )
SOURCES += $(V12.ALL)
targetsclean += v12clean

OPENGL2D3D=glFOO
GLU2D3D=gluFOO

$(DIR.OBJ)/v12_$(BUILD)$(EXT.EXE): CFLAGS += -I../lib/directx -I../lib/zlib -I../lib/lungif -I../lib/lpng -I../lib/ljpeg -I../lib/mss -I- -I../lib/openal/win32 \
	-DWIN32 -DUSEASSEMBLYTERRBLEND -DPNG_NO_READ_tIME -DPNG_NO_WRITE_TIME \
	-DOPENGL2D3D=\"$(OPENGL2D3D).dll\" -DGLU2D3D=\"$(GLU2D3D).dll\" \
	-DNO_MILES_OPENAL

$(DIR.OBJ)/v12_$(BUILD)$(EXT.EXE): LIB.PATH += \
	../lib/$(DIR.OBJ) \
	../lib/mss \

$(DIR.OBJ)/v12_$(BUILD)$(EXT.EXE): LINK.LIBS.GENERAL += \
	ljpeg$(EXT.LIB) \
	lpng$(EXT.LIB) \
	lungif$(EXT.LIB) \
	zlib$(EXT.LIB) \
	Mss32$(EXT.LIB) \
	vfw32$(EXT.LIB)

$(DIR.OBJ)/v12_$(BUILD)$(EXT.EXE): dirlist $(OBJ.ALL)
	$(DO.LINK.CONSOLE.EXE)
	cp $(DIR.OBJ)/v12_$(BUILD)* ../example
	
v12clean:
ifneq ($(wildcard v12_DEBUG.*),)
	-$(RM)  v12_DEBUG*
endif
ifneq ($(wildcard v12_RELEASE.*),)
	-$(RM)  v12_RELEASE*
endif

