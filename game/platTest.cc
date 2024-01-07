//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "platform/platformVideo.h"
#include "platform/platformInput.h"
#include "platform/platformAudio.h"
#include "platform/event.h"
#include "platform/gameInterface.h"
#include "core/tVector.h"
#include "math/mMath.h"
#include "dgl/dgl.h"
#include "dgl/gBitmap.h"
#include "core/resManager.h"
#include "core/fileStream.h"
#include "dgl/gTexManager.h"
#include "dgl/gFont.h"
#include "console/console.h"
#include "console/simBase.h"
#include "gui/guiCanvas.h"
#include "sim/actionMap.h"
#include "core/dnet.h"
#include "game/game.h"
#include "core/bitStream.h"
#include "console/telnetConsole.h"
#include "console/telnetDebugger.h"
#include "console/consoleTypes.h"
#include "math/mathTypes.h"
#include "dgl/gTexManager.h"
#include "core/resManager.h"
#include "interior/interiorRes.h"
#include "interior/interiorInstance.h"
#include "ts/tsShapeInstance.h"
#include "terrain/terrData.h"
#include "terrain/terrRender.h"
#include "editor/terraformer.h"
#include "scenegraph/sceneGraph.h"
#include "dgl/materialList.h"
#include "scenegraph/sceneRoot.h"
#include "game/moveManager.h"
#include "platform/platformVideo.h"
#include "dgl/materialPropertyMap.h"
#include "sim/netStringTable.h"
#include "sim/pathManager.h"
#include "game/GameFunctions.h"
#include "game/particleEngine.h"
#include "game/targetManager.h"
#include "platform/platformRedBook.h"
#include "game/tribesGame.h"
#include "game/netDispatch.h"
#include "sim/decalManager.h"
#include "sim/frameAllocator.h"
#include "scenegraph/detailManager.h"
#include "interior/interiorLMManager.h"
#include "game/version.h"
#include "game/badWordFilter.h"
#include "platform/profiler.h"
#include "game/shapeBase.h"
//#define IHVBUILD
#ifdef IHVBUILD
#include "ihvSignature.h"
#include "crypt/cryptSHA1.h"
#endif

#ifndef BUILD_TOOLS
TribesGame GameObject;
#endif

extern ResourceInstance *constructTerrainFile(Stream &stream);
extern ResourceInstance *constructTSShape(Stream &);

IMPLEMENT_CONOBJECT(Terraformer);

static void cLockMouse(SimObject *, S32, const char **argv)
{
   Platform::setWindowLocked(dAtob(argv[1]));
}

static void cSetNetPort(SimObject *, S32, const char **argv)
{
   Net::openPort(dAtoi(argv[1]));
}

static bool cCreateCanvas(SimObject *, S32, const char **)
{
   AssertISV(!Canvas, "cCreateCanvas: canvas has already been instantiated");

#if !defined(DEBUG) && !defined(INTERNAL_RELEASE)
   if(!Platform::excludeOtherInstances("Tribes2Game"))
      return false;
#endif
   Platform::initWindow(Point2I(800, 600), "Tribes 2");

   // create the canvas, and add it to the manager
   Canvas = new GuiCanvas();
   Canvas->registerObject("Canvas"); // automatically adds to GuiGroup
   return true;
}

static void cSaveJournal(SimObject *, S32, const char **argv)
{
   Game->saveJournal(argv[1]);
}

static void cLoadJournal(SimObject *, S32, const char **argv)
{
   Game->loadJournal(argv[1]);
}

static void cPlayJournal(SimObject *, S32, const char **argv)
{
   Game->playJournal(argv[1]);
}

extern void AIInit();
extern void netInit();
extern void clientNetProcess();
extern void serverNetProcess();
extern void processConnectedReceiveEvent( ConnectedReceiveEvent * event );
extern void processConnectedNotifyEvent( ConnectedNotifyEvent * event );
extern void processConnectedAcceptEvent( ConnectedAcceptEvent * event );
extern void ShowInit();

static bool initLibraries()
{
   if(!Net::init())
   {
      Platform::AlertOK("Network Error", "Unable to initialize the network... aborting.");
      return false;
   }

   // asserts should be created FIRST
   PlatformAssert::create();

   FrameAllocator::init(3 << 20);      // 3 meg frame allocator buffer

//   // Cryptographic pool next
//   CryptRandomPool::init();

   _StringTable::create();
   TextureManager::create();
   ResManager::create();

   // Register known file types here
   ResourceManager->registerExtension(".jpg", constructBitmapJPEG);
   ResourceManager->registerExtension(".png", constructBitmapPNG);
   ResourceManager->registerExtension(".gif", constructBitmapGIF);
   ResourceManager->registerExtension(".dbm", constructBitmapDBM);
   ResourceManager->registerExtension(".bmp", constructBitmapBMP);
   ResourceManager->registerExtension(".bm8", constructBitmapBM8);
   ResourceManager->registerExtension(".gft", constructFont);
   ResourceManager->registerExtension(".dif", constructInteriorDIF);
   ResourceManager->registerExtension(".ter", constructTerrainFile);
   ResourceManager->registerExtension(".dts", constructTSShape);
   ResourceManager->registerExtension(".dml", constructMaterialList);

   RegisterCoreTypes();
   RegisterMathTypes();
   RegisterGuiTypes();
   
   Con::init();
   NetStringTable::create();
   BadWordFilter::create();
   
   RegisterMathFunctions();
   RegisterGameFunctions();
   TelnetConsole::create();
   TelnetDebugger::create();

   Processor::init();
   Math::init();
   Platform::init();    // platform specific initialization
   Audio::init();
   MathConsoleInit();
   InteriorLMManager::init();
   InteriorInstance::init();
   TSShapeInstance::init();
   RedBook::init();
   
   return true;
}


// shut down
static void shutdownLibraries()
{
   // Purge any resources on the timeout list...
   if (ResourceManager)
      ResourceManager->purge();

   RedBook::destroy();
   TSShapeInstance::destroy();
   InteriorInstance::destroy();
   InteriorLMManager::destroy();

   Audio::destroy();
   TextureManager::preDestroy();
   
   Platform::shutdown();
   TelnetDebugger::destroy();
   TelnetConsole::destroy();

   BadWordFilter::destroy();
   NetStringTable::destroy();
   Con::shutdown();

   ResManager::destroy();
   TextureManager::destroy();

   _StringTable::destroy();

//   CryptRandomPool::destroy();

   // asserts should be destroyed LAST
   FrameAllocator::destroy();

   PlatformAssert::destroy();
   Net::shutdown();
}

const char *defaultPaths[] = { // searched left to right
"base"
};

static void cRebuildModPaths(SimObject *, S32, const char **)
{
   ResourceManager->setModPaths(sizeof(defaultPaths)/sizeof(char*), defaultPaths);
}

static void cSetModPaths(SimObject*, S32, const char** argv)
{
   char* buf = new char[dStrlen( argv[1] ) + 6];
   dStrcpy( buf, argv[1] );
   dStrcat( buf, ";base" );
   Vector<char*> paths;
   char* temp = dStrtok( buf, ";" );
   while ( temp )
   {
      if ( temp[0] )
      {
         char* path = (char*) dMalloc( dStrlen( temp ) + 1 );
         dStrcpy( path, temp );
         paths.push_back( path );
      }
      temp = dStrtok( NULL, ";" );
   }

   ResourceManager->setModPaths( paths.size(), (const char**) paths.address() );
   delete [] buf; 
}

static const char* cGetModPaths(SimObject*, S32, const char**)
{
   return( ResourceManager->getModPaths() );
}

static S32 cGetVersionNumber(SimObject *, S32, const char **)
{
   return getVersionNumber();
}

static S32 cGetSimTime(SimObject *, S32, const char **)
{
   return Sim::getCurrentTime();
}

static S32 cGetRealTime( SimObject*, S32, const char** )
{
   return Platform::getRealMilliseconds();
}

static F32 gTimeScale = 1.0;
static U32 gTimeAdvance = 0;
static U32 gFrameSkip = 0;
static U32 gFrameCount = 0;


bool initGame()
{
   Con::addCommand("getVersionNumber", cGetVersionNumber, "getVersionNumber()", 1, 1);

   Con::addCommand("getSimTime", cGetSimTime, "getSimTime();", 1, 1);
   Con::addCommand("getRealTime", cGetRealTime, "getRealTime()'", 1, 1);
   Con::addCommand("setNetPort", cSetNetPort, "setNetPort(port);", 2, 2);
   Con::addCommand("lockMouse", cLockMouse, "lockMouse(isLocked);", 2, 2);
   Con::addCommand("rebuildModPaths", cRebuildModPaths, "rebuildModPaths();", 1, 1);
   Con::addCommand("setModPaths", cSetModPaths, "setModPaths( paths )", 2, 2 );
   Con::addCommand("getModPaths", cGetModPaths, "getModPaths()", 1, 1 );
   Con::addCommand("createCanvas", cCreateCanvas, "createCanvas();", 1, 1);

   Con::addCommand("saveJournal", cSaveJournal, "saveJournal(jname);", 2, 2);
   Con::addCommand("loadJournal", cLoadJournal, "loadJournal(jname);", 2, 2);
//   Con::addCommand("playJournal", cPlayJournal, "playJournal(jname);", 2, 2);

   Con::setFloatVariable("Video::texResidentPercentage", -1.0f);
   Con::setIntVariable("Video::textureCacheMisses", -1);
   Con::addVariable("timeScale", TypeF32, &gTimeScale);
   Con::addVariable("timeAdvance", TypeS32, &gTimeAdvance);
   Con::addVariable("frameSkip", TypeS32, &gFrameSkip);

#ifdef GATHER_METRICS
   Con::addVariable("Video::numTexelsLoaded", TypeS32, &TextureManager::smTextureSpaceLoaded);
#else
   static U32 sBogusNTL = 0;
   Con::addVariable("Video::numTexelsLoaded", TypeS32, &sBogusNTL);
#endif

   ResourceManager->setModPaths(sizeof(defaultPaths)/sizeof(char*), defaultPaths);
   TerrainRender::init();
   
   netInit();
   dispatchInit();
   GameInit();
   ShowInit();
   AIInit();
   MoveManager::init();

   Sim::init();

   ActionMap* globalMap = new ActionMap;
   globalMap->registerObject("GlobalActionMap");
   Sim::getActiveActionMapSet()->pushObject(globalMap);

   MaterialPropertyMap *map = new MaterialPropertyMap;

   map->registerObject("MaterialPropertyMap");
   Sim::getRootGroup()->addObject(map);

   gClientSceneGraph = new SceneGraph(true);
   gClientSceneRoot  = new SceneRoot;
   gClientSceneGraph->addObjectToScene(gClientSceneRoot);
   gServerSceneGraph = new SceneGraph(false);
   gServerSceneRoot  = new SceneRoot;
   gServerSceneGraph->addObjectToScene(gServerSceneRoot);
   gDecalManager = new DecalManager;
   gClientContainer.addObject(gDecalManager);
   gClientSceneGraph->addObjectToScene(gDecalManager);

   DetailManager::init();
   PathManager::init();
   ParticleEngine::init();
   TargetManager::create();
 
	//execute the console.cs script
   FileStream str;
   if(!str.open("main.cs", FileStream::Read))
      return false;

   U32 size = str.getStreamSize();
   char *script = new char[size + 1];
   str.read(size, script);
   str.close();

   script[size] = 0;
   Con::executef(2, "eval", script);
   delete[] script;
   return true;
}

void shutdownGame()
{
   //exec the script onExit() function
   Con::executef(1, "onExit");

   ParticleEngine::destroy();
   PathManager::destroy();
   DetailManager::shutdown();

   // Note: tho the SceneGraphs are created after the Manager, delete them after, rather
   //  than before to make sure that all the objects are removed from the graph.
   Sim::shutdown();

   gClientSceneGraph->removeObjectFromScene(gDecalManager);
   gClientContainer.removeObject(gDecalManager);
   gClientSceneGraph->removeObjectFromScene(gClientSceneRoot);
   gServerSceneGraph->removeObjectFromScene(gServerSceneRoot);
   delete gClientSceneRoot;
   delete gServerSceneRoot;
   delete gClientSceneGraph;
   delete gServerSceneGraph;
   delete gDecalManager;
   gClientSceneRoot = NULL;
   gServerSceneRoot = NULL;
   gClientSceneGraph = NULL;
   gServerSceneGraph = NULL;
   gDecalManager = NULL;

   TargetManager::destroy();
   TerrainRender::shutdown();
}

extern bool gDGLRender;
bool gShuttingDown   = false;

S32 TribesGame::main(S32 argc, const char **argv)
{
//   if (argc == 1) {
//      static const char* argvFake[] = { "dtest.exe", "-jload", "test.jrn" };
//      argc = 3;                                                               
//      argv = argvFake;
//   }

//   Memory::enableLogging("testMem.log");
//   Memory::setBreakAlloc(104717);

   if(!initLibraries())
      return 0;

#ifdef IHVBUILD
   char* pVer = new char[sgVerStringLen + 1];
   U32 hi;
   for (hi = 0; hi < sgVerStringLen; hi++)
      pVer[hi] = sgVerString[hi] ^ 0xFF;
   pVer[hi] = '\0';

   SHA1Context hashCTX;
   hashCTX.init();
   hashCTX.hashBytes(pVer, sgVerStringLen);
   hashCTX.finalize();

   U8 hash[20];
   hashCTX.getHash(hash);

   for (hi = 0; hi < 20; hi++)
      if (U8(hash[hi]) != U8(sgHashVer[hi]))
         return 0;
#endif

   // Set up the command line args for the console scripts...
   Con::setIntVariable("Game::argc", argc);
   U32 i;
   for (i = 0; i < argc; i++)
      Con::setVariable(avar("Game::argv%d", i), argv[i]);
   if (initGame() == false)
      return 0;

#ifdef IHVBUILD
   char* pPrint = new char[dStrlen(sgVerPrintString) + 1];
   for (U32 pi = 0; pi < dStrlen(sgVerPrintString); pi++)
      pPrint[pi] = sgVerPrintString[pi] ^ 0xff;
   pPrint[dStrlen(sgVerPrintString)] = '\0';

   Con::printf("");
   Con::errorf(ConsoleLogEntry::General, pPrint, pVer);
   delete [] pVer;
#endif


//   extern void QGL_EnableLogging(bool);
//   QGL_EnableLogging(true);

   while(Game->isRunning())
   {
      PROFILE_START(MainLoop);
      Game->journalProcess();
      //if(Game->getJournalMode() != GameInterface::JournalLoad)
      //{
         Net::process();      // read in all events
         Platform::process(); // keys, etc.
         TelConsole->process();
         TelDebugger->process();
         TimeManager::process(); // guaranteed to produce an event
      //}
      PROFILE_END();
   }
   shutdownGame();
   shutdownLibraries();

   gShuttingDown = true;
//   QGL_EnableLogging(false);

#if 0
// tg: Argh! This should OS version check should be part of Platform, not here...
//
   // check os
   OSVERSIONINFO osInfo;
   dMemset(&osInfo, 0, sizeof(OSVERSIONINFO));
   osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

   // see if osversioninfoex fails
   if(!GetVersionEx((OSVERSIONINFO*)&osInfo))
   {
      osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
      if(!GetVersionEx((OSVERSIONINFO*)&osInfo))
         return 0;
   }

   // terminate the process if win95 only!
   if((osInfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) &&      // 95, 98, ME
      (osInfo.dwMajorVersion == 4) &&                             // 95, 98, ME, NT
      (osInfo.dwMinorVersion == 0))                               // 95
   {
      AssertWarn(0, "Forcing termination of app (Win95)!  Upgrade your OS now!");
      TerminateProcess(GetCurrentProcess(), 0xffffffff);
   }
#endif

   return 0;
}


static bool serverTick = false;


static F32 fpsRealStart;
static F32 fpsRealLast;
//static F32 fpsRealTotal;
static F32 fpsReal;
static F32 fpsVirtualStart;
static F32 fpsVirtualLast;
//static F32 fpsVirtualTotal;
static F32 fpsVirtual;
static F32 fpsFrames;
static F32 fpsNext;
static bool fpsInit = false;
const F32 UPDATE_INTERVAL = 0.25f;

//--------------------------------------
void fpsReset()
{
   fpsRealStart    = (F32)Platform::getRealMilliseconds()/1000.0f;      // Real-World Tick Count
   fpsVirtualStart = (F32)Platform::getVirtualMilliseconds()/1000.0f;   // Engine Tick Count (does not vary between frames)
   fpsNext         = fpsRealStart + UPDATE_INTERVAL;

//   fpsRealTotal= 0.0f;
   fpsRealLast = 0.0f;
   fpsReal     = 0.0f;
//   fpsVirtualTotal = 0.0f;
   fpsVirtualLast  = 0.0f;
   fpsVirtual      = 0.0f;
   fpsFrames = 0;
   fpsInit   = true;
}   

//--------------------------------------
void fpsUpdate()
{
   if (!fpsInit)
      fpsReset();

   const float alpha  = 0.07f;
   F32 realSeconds    = (F32)Platform::getRealMilliseconds()/1000.0f;
   F32 virtualSeconds = (F32)Platform::getVirtualMilliseconds()/1000.0f;

   fpsFrames++;
   if (fpsFrames > 1)
   {
      fpsReal    = fpsReal*(1.0-alpha) + (realSeconds-fpsRealLast)*alpha;     
      fpsVirtual = fpsVirtual*(1.0-alpha) + (virtualSeconds-fpsVirtualLast)*alpha;     
   }
//   fpsRealTotal    = fpsFrames/(realSeconds - fpsRealStart);
//   fpsVirtualTotal = fpsFrames/(virtualSeconds - fpsVirtualStart);

   fpsRealLast    = realSeconds;
   fpsVirtualLast = virtualSeconds;

   // update variables every few frames
   F32 update = fpsRealLast - fpsNext;
   if (update > 0.5f)
   {
//      Con::setVariable("fps::realTotal",    avar("%4.1f", fpsRealTotal));
//      Con::setVariable("fps::virtualTotal", avar("%4.1f", fpsVirtualTotal));
      Con::setVariable("fps::real",    avar("%4.1f", 1.0f/fpsReal));
      Con::setVariable("fps::virtual", avar("%4.1f", 1.0f/fpsVirtual));
      if (update > UPDATE_INTERVAL)
         fpsNext  = fpsRealLast + UPDATE_INTERVAL;
      else
         fpsNext += UPDATE_INTERVAL;
   }
}   


//--------------------------------------------------------------------------
//-------------------------------------- NOTE: Entropy distilling occurs in this
//                                              function.  Talk to dmoore for details.
//


void TribesGame::processMouseMoveEvent(MouseMoveEvent * mEvent)
{
//         CryptRandomPool::submitEntropy(mEvent->xPos, 2);      // Take the least significant 2 bits of the mouse pos
//         CryptRandomPool::submitEntropy(mEvent->yPos, 2);
   if (Canvas) 
      Canvas->processMouseMoveEvent(mEvent);
}

void TribesGame::processInputEvent(InputEvent *event)
{
   PROFILE_START(ProcessInputEvent);
   if (!ActionMap::handleEventGlobal(event))
   {
      // Other input consumers here...
      if (!(Canvas && Canvas->processInputEvent(event)))
         ActionMap::handleEvent(event);
   }
   PROFILE_END();
}

void TribesGame::processQuitEvent()
{
   setRunning(false);
}

void TribesGame::refreshWindow()
{
   if(Canvas)
      Canvas->resetUpdateRegions();
}

void TribesGame::processConsoleEvent(ConsoleEvent *event)
{
   char *argv[2];
   argv[0] = "eval";
   argv[1] = event->data;
   Sim::postCurrentEvent(Sim::getRootGroup(), new SimConsoleEvent(2, const_cast<const char**>(argv), false));
}

void TribesGame::processTimeEvent(TimeEvent *event)
{
   PROFILE_START(ProcessTimeEvent);
   U32 elapsedTime = event->elapsedTime;
   // cap the elapsed time to one second
   // if it's more than that we're probably in a bad catch-up situation

   if(elapsedTime > 1024)
      elapsedTime = 1024;
   
   U32 timeDelta;
   
   if(gTimeAdvance)
      timeDelta = gTimeAdvance;
   else
      timeDelta = elapsedTime * gTimeScale;

   Platform::advanceTime(elapsedTime);

   PROFILE_START(ServerProcess);
   serverProcess(timeDelta);
   PROFILE_END();
   PROFILE_START(ServerNetProcess);
   serverNetProcess();
   PROFILE_END();

   PROFILE_START(SimAdvanceTime);
   Sim::advanceTime(timeDelta);
   PROFILE_END();

   PROFILE_START(ClientProcess);
   clientProcess(timeDelta);
   PROFILE_END();
   PROFILE_START(ClientNetProcess);
   clientNetProcess();
   PROFILE_END();

   if(Canvas && gDGLRender)
   {
      bool preRenderOnly = false;
      if(gFrameSkip && gFrameCount % gFrameSkip)
         preRenderOnly = true;

      PROFILE_START(RenderFrame);
      ShapeBase::sLastRenderFrame++;
      Canvas->renderFrame(preRenderOnly);
      PROFILE_END();
      gFrameCount++;
   }
   dispatchCheckTimeouts();
   fpsUpdate();
   PROFILE_END();
}

void GameReactivate()
{
   if ( !Input::isEnabled() )
      Input::enable();

   if ( !Input::isActive() )
      Input::reactivate();

   gDGLRender = true;
   if ( Canvas )
      Canvas->resetUpdateRegions();
}

void GameDeactivate( bool noRender )
{
   if ( Input::isActive() )
      Input::deactivate();

   if ( Input::isEnabled() )
      Input::disable();

   if ( noRender )
      gDGLRender = false;
}

void TribesGame::textureKill()
{
   TextureManager::makeZombie();
}

void TribesGame::textureResurrect()
{
   TextureManager::resurrect();
}

