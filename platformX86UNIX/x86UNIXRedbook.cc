//-----------------------------------------------------------------------------
// Torque Game Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#include "console/console.h"
#include "platformX86UNIX/platformX86UNIX.h"
#include "platform/platformRedBook.h"

#if defined(__linux__)
#include <linux/cdrom.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#endif

#include <SDL/SDL.h>

class UnixRedBookDevice : public RedBookDevice
{
   private:
      S32 mDeviceId;
      SDL_CD *mCD;
      cdrom_volctrl mOriginalVolume;     
      bool mVolumeInitialized;
      bool mPlaying;

      void openVolume();
      void closeVolume();
      void setLastError(const char *);

   public:
      UnixRedBookDevice();
      ~UnixRedBookDevice();

      bool open();
      bool close();
      bool play(U32);
      bool stop();
      bool getTrackCount(U32 *);
      bool getVolume(F32 *);
      bool setVolume(F32);

      bool isPlaying() { return mPlaying; }
      bool updateStatus();
      void setDeviceInfo(S32 deviceId, const char *deviceName);
};

//-------------------------------------------------------------------------------
// Class: UnixRedBookDevice
//-------------------------------------------------------------------------------
UnixRedBookDevice::UnixRedBookDevice()
{
   mVolumeInitialized = false;
   mDeviceId = -1;
   mDeviceName = NULL;
   mCD = NULL;
   mPlaying = false;
}

//------------------------------------------------------------------------------
UnixRedBookDevice::~UnixRedBookDevice()
{
   close();
}

//------------------------------------------------------------------------------
bool UnixRedBookDevice::updateStatus()
{
   AssertFatal(mCD, "mCD is NULL");

   CDstatus status = SDL_CDStatus(mCD);
   if (status == CD_ERROR)
   {
      setLastError("Error accessing device");
      return(false);
   }
   else if (status == CD_TRAYEMPTY)
   {
      setLastError("CD tray empty");
      return false;
   }

   mPlaying = (status == CD_PLAYING);
   return true;
}

//------------------------------------------------------------------------------
void UnixRedBookDevice::setDeviceInfo(S32 deviceId, const char *deviceName)
{
   mDeviceId = deviceId;
   mDeviceName = new char[dStrlen(deviceName) + 1];
   dStrcpy(mDeviceName, deviceName);
}

//------------------------------------------------------------------------------
bool UnixRedBookDevice::open()
{
    if(mAcquired)
    {
       setLastError("Device is already open.");
       return(false);
    }

    // open the device
    mCD = SDL_CDOpen(mDeviceId);
    if (mCD == NULL)
    {
       setLastError(SDL_GetError());
       return false;
    }

    mAcquired = true;

    openVolume();
    setLastError("");
    return(true);
}

//------------------------------------------------------------------------------
bool UnixRedBookDevice::close()
{
   if(!mAcquired)
   {
      setLastError("Device has not been acquired");
      return(false);
   }

   stop();
   closeVolume();

   if (mCD != NULL)
   {
      SDL_CDClose(mCD);
      mCD = NULL;
   }
   
   mAcquired = false;
   setLastError("");
   return(true);
}

//------------------------------------------------------------------------------
bool UnixRedBookDevice::play(U32 track)
{
   if(!mAcquired)
   {
      setLastError("Device has not been acquired");
      return(false);
   }

   U32 numTracks;
   if(!getTrackCount(&numTracks))
      return(false);

   if(track >= numTracks)
   {
      setLastError("Track index is out of range");
      return(false);
   }

   if (!updateStatus())
      return false;

   AssertFatal(mCD, "mCD is NULL");
   if (SDL_CDPlayTracks(mCD, track, 0, 1, 0) == -1)
   {
      setLastError(SDL_GetError());
      return false;
   }

   mPlaying = true;

   setLastError("");
   return(true);
}

//------------------------------------------------------------------------------
bool UnixRedBookDevice::stop()
{
   if(!mAcquired)
   {
      setLastError("Device has not been acquired");
      return(false);
   }

   AssertFatal(mCD, "mCD is NULL");

   if (SDL_CDStop(mCD) == -1)
   {
      setLastError(SDL_GetError());
      return(false);
   }

   mPlaying = false;

   setLastError("");
   return(true);
}

//------------------------------------------------------------------------------
bool UnixRedBookDevice::getTrackCount(U32 * numTracks)
{
   if(!mAcquired)
   {
      setLastError("Device has not been acquired");
      return(false);
   }

   if (!updateStatus())
      return false;

   AssertFatal(mCD, "mCD is NULL");
   *numTracks = mCD->numtracks;
  
   return(true);
}

template <class Type>
static inline Type max(Type v1, Type v2)
{
   if (v1 <= v2)
      return v2;
   else
      return v1;
}
//------------------------------------------------------------------------------
bool UnixRedBookDevice::getVolume(F32 * volume)
{
   if(!mAcquired)
   {
      setLastError("Device has not been acquired");
      return(false);
   }

   if(!mVolumeInitialized)
   {
      setLastError("Volume failed to initialize");
      return(false);
   }

#if defined(__linux__)
   AssertFatal(mCD, "mCD is NULL");

   setLastError("");
   cdrom_volctrl sysvol;
   if (ioctl(mCD->id, CDROMVOLREAD, &sysvol) == -1)
   {
      setLastError(strerror(errno));
      return(false);
   }
   U8 maxVol = max(sysvol.channel0, sysvol.channel1);
   // JMQTODO: support different left/right channel volumes?
   *volume = static_cast<F32>(maxVol) / 255.f;
   return true;
#else
   return(false);
#endif
}

//------------------------------------------------------------------------------
bool UnixRedBookDevice::setVolume(F32 volume)
{
   if(!mAcquired)
   {
      setLastError("Device has not been acquired");
      return(false);
   }

   if(!mVolumeInitialized)
   {
      setLastError("Volume failed to initialize");
      return(false);
   }   

#if defined(__linux__)
   AssertFatal(mCD, "mCD is NULL");

   setLastError("");
   cdrom_volctrl sysvol;
   volume = volume * 255.f;
   if (volume > 255)
      volume = 255;
   if (volume < 0)
      volume = 0;
   sysvol.channel0 = sysvol.channel1 = static_cast<__u8>(volume);
   if (ioctl(mCD->id, CDROMVOLCTRL, &sysvol) == -1)
   {
      setLastError(strerror(errno));
      return(false);
   }
   return true;
#else
   return(false);
#endif
}

//------------------------------------------------------------------------------
void UnixRedBookDevice::openVolume()
{
// Its unforunate that we have to do it this way, but SDL does not currently
// support setting CD audio volume
#if defined(__linux__)
   AssertFatal(mCD, "mCD is NULL");

   setLastError("");
 
   if (ioctl(mCD->id, CDROMVOLREAD, &mOriginalVolume) == -1)
   {
      setLastError(strerror(errno));
      return;
   }

   mVolumeInitialized = true;
#else
   setLastError("Volume failed to initialize");
#endif
}

void UnixRedBookDevice::closeVolume()
{
   if(!mVolumeInitialized)
      return;

#if defined(__linux__)
   AssertFatal(mCD, "mCD is NULL");

   setLastError("");
 
   if (ioctl(mCD->id, CDROMVOLCTRL, &mOriginalVolume) == -1)
   {
      setLastError(strerror(errno));
      return;
   }
#endif

   mVolumeInitialized = false;
}

//------------------------------------------------------------------------------
void UnixRedBookDevice::setLastError(const char * error)
{
   RedBook::setLastError(error);
}

//------------------------------------------------------------------------------
void InstallRedBookDevices()
{
   Con::printf("CD Audio Init:");
   if (SDL_InitSubSystem(SDL_INIT_CDROM) == -1)
   {
      Con::printf("   Unable to initialize CD Audio: %s", SDL_GetError());
      return;
   }

   S32 numDrives = SDL_CDNumDrives();
   if (numDrives == 0)
   {
      Con::printf("   No drives found.");
      return;
   }

   for (int i = 0; i < numDrives; ++i)
   {
      const char * deviceName = SDL_CDName(i);
      Con::printf("   Installing CD Audio device: %s", deviceName);

      UnixRedBookDevice * device = new UnixRedBookDevice;
      device->setDeviceInfo(i, deviceName);
      RedBook::installDevice(device);
   }

   Con::printf(" ");   
}

//------------------------------------------------------------------------------
void PollRedbookDevices()
{
   // JMQTODO: poll at longer intervals if this function is expensive
   static const U32 PollDelay = 1000;

   static U32 lastPollTime = 0;
   U32 curTime = Platform::getVirtualMilliseconds();

   if (lastPollTime != 0 &&
      (curTime - lastPollTime) < PollDelay)
      return;

   lastPollTime = curTime;

   RedBookDevice *rbDevice = RedBook::getCurrentDevice();
   if (rbDevice == NULL)
      return;

   UnixRedBookDevice *device = dynamic_cast<UnixRedBookDevice*>(rbDevice);
   if (device == NULL)
      return;

   if (device->isPlaying())
   {
      device->updateStatus();
      if (!device->isPlaying())
         RedBook::handleCallback(RedBook::PlayFinished);
   }
}
