//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "console/console.h"
#include "Platform/platformRedBook.h"

//------------------------------------------------------------------------------
// Class: RedBookDevice
//------------------------------------------------------------------------------
RedBookDevice::RedBookDevice()
{
   mAcquired = false;
   mDeviceName = 0;
}

RedBookDevice::~RedBookDevice()
{
   delete [] mDeviceName;
}

//------------------------------------------------------------------------------
// Class: RedBook
//------------------------------------------------------------------------------
Vector<RedBookDevice *>    RedBook::smDeviceList(__FILE__, __LINE__);
RedBookDevice *            RedBook::smCurrentDevice;
char                       RedBook::smLastError[1024];

//------------------------------------------------------------------------------
void RedBook::init()
{
   installConsoleCommands();
}

void RedBook::destroy()
{
   close();

   for( Vector<RedBookDevice*>::iterator i = smDeviceList.begin( ); i != smDeviceList.end( ); i++ ) {
	   delete *i;
   }

   smDeviceList.clear( );
}

//------------------------------------------------------------------------------
void RedBook::installDevice(RedBookDevice * device)
{
   smDeviceList.push_back(device);
}

RedBookDevice * RedBook::getCurrentDevice()
{
   return(smCurrentDevice);
}

U32 RedBook::getDeviceCount()
{
   return(smDeviceList.size());
}

const char * RedBook::getDeviceName(U32 idx)
{
   if(idx >= getDeviceCount())
   {
      setLastError("Invalid device index");
      return("");
   }
   return(smDeviceList[idx]->mDeviceName);
}

void RedBook::setLastError(const char * error)
{
   if(!error || dStrlen(error) >= sizeof(smLastError))
      setLastError("Invalid error string passed");
   else
      dStrcpy(smLastError, error);
}

const char * RedBook::getLastError()
{
   return(smLastError);
}

void RedBook::handleCallback(U32 type)
{
   switch(type)
   {
      case PlayFinished:
         Con::executef(2, "RedBookCallback", "PlayFinished");
         break;
   }
}

//------------------------------------------------------------------------------
bool RedBook::open(const char * deviceName)
{
   if(!deviceName)
   {
      setLastError("Invalid device name");
      return(false);
   }

   for(U32 i = 0; i < smDeviceList.size(); i++)
      if(!dStricmp(deviceName, smDeviceList[i]->mDeviceName))
         return(open(smDeviceList[i]));

   setLastError("Failed to find device");
   return(false);
}

bool RedBook::open(RedBookDevice * device)
{
   if(!device)
   {  
      setLastError("Invalid device passed");
      return(false);
   }

   close();
   smCurrentDevice = device;
   return(smCurrentDevice->open());
}

bool RedBook::close()
{
   if(smCurrentDevice)
   {
      bool ret = smCurrentDevice->close();
      smCurrentDevice = 0;
      return(ret);
   }

   setLastError("No device is currently open");
   return(false);
}

bool RedBook::play(U32 track)
{
   if(!smCurrentDevice)
   {
      setLastError("No device is currently open");
      return(false);
   }
   return(smCurrentDevice->play(track));
}

bool RedBook::stop()
{
   if(!smCurrentDevice)
   {
      setLastError("No device is currently open");
      return(false);
   }
   return(smCurrentDevice->stop());
}

bool RedBook::getTrackCount(U32 * trackCount)
{
   if(!smCurrentDevice)
   {
      setLastError("No device is currently open");
      return(false);
   }
   return(smCurrentDevice->getTrackCount(trackCount));
}

bool RedBook::getVolume(F32 * volume)
{
   if(!smCurrentDevice)
   {
      setLastError("No device is currently open");
      return(false);
   }
   return(smCurrentDevice->getVolume(volume));
}

bool RedBook::setVolume(F32 volume)
{
   if(!smCurrentDevice)
   {
      setLastError("No device is currently open");
      return(false);
   }
   return(smCurrentDevice->setVolume(volume));   
}

//------------------------------------------------------------------------------
// console methods
//------------------------------------------------------------------------------
static bool cOpen(SimObject *, S32 argc, const char ** argv)
{
   if(argc == 1)
      return(RedBook::open(RedBook::getDeviceName(0)));
   else
      return(RedBook::open(argv[1]));
}

static bool cClose(SimObject *, S32, const char **)
{
   return(RedBook::close());
}

static bool cPlay(SimObject *, S32, const char ** argv)
{
   return(RedBook::play(dAtoi(argv[1])));
}

static bool cStop(SimObject *, S32, const char **)
{
   return(RedBook::stop());
}

static S32 cGetTrackCount(SimObject *, S32, const char **)
{
   U32 trackCount;
   if(!RedBook::getTrackCount(&trackCount))
      return(0);
   return(trackCount);
}

static F32 cGetVolume(SimObject *, S32, const char **)
{
   F32 vol;
   if(!RedBook::getVolume(&vol))
      return(0.f);
   else
      return(vol);
}

static bool cSetVolume(SimObject *, S32, const char ** argv)
{
   return(RedBook::setVolume(dAtof(argv[1])));
}

static S32 cGetDeviceCount(SimObject *, S32, const char **)
{
   return(RedBook::getDeviceCount());
}

static const char * cGetDeviceName(SimObject *, S32, const char ** argv)
{
   return(RedBook::getDeviceName(dAtoi(argv[1])));
}

static const char * cGetLastError(SimObject *, S32, const char **)
{
   return(RedBook::getLastError());
}

void RedBook::installConsoleCommands()
{
   Con::addCommand("redbookOpen", cOpen, "redbookOpen(<device>)", 1, 2);
   Con::addCommand("redbookClose", cClose, "redbookClose()", 1, 1);
   Con::addCommand("redbookPlay", cPlay, "redbookPlay(track)", 2, 2);
   Con::addCommand("redbookStop", cStop, "redbookStop()", 1, 1);
   Con::addCommand("redbookGetTrackCount", cGetTrackCount, "redbookGetTrackCount()", 1, 1);
   Con::addCommand("redbookGetVolume", cGetVolume, "redbookGetVolume", 1, 1);
   Con::addCommand("redbookSetVolume", cSetVolume, "redbookSetVolume", 2, 2);

   Con::addCommand("redbookGetDeviceCount", cGetDeviceCount, "redbookGetDeviceCount()", 1, 1);
   Con::addCommand("redbookGetDeviceName", cGetDeviceName, "redbookGetDeviceName(idx)", 2, 2);
   Con::addCommand("redbookGetLastError", cGetLastError, "redbookGetLastError()", 1, 1);
}
