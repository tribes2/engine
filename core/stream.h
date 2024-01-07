//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _STREAM_H_
#define _STREAM_H_

//Includes
#ifndef _PLATFORM_H_
#include "Platform/platform.h"
#endif


// This should ideally be done with templates...
//
#define DECLARE_OVERLOADED_READ(type)      \
   bool read(type* out_read) {             \
      return read(sizeof(type), out_read); \
   }
#define DECLARE_OVERLOADED_WRITE(type)       \
   bool write(type in_write) {               \
      return write(sizeof(type), &in_write); \
   }

#define DECLARE_ENDIAN_OVERLOADED_READ(type)       \
   bool read(type* out_read) {                     \
      type temp;                                   \
      bool success = read(sizeof(type), &temp);    \
      *out_read = convertLEndianToHost(temp);      \
      return success;                              \
   }
#define DECLARE_ENDIAN_OVERLOADED_WRITE(type)      \
   bool write(type in_write) {                     \
      type temp = convertHostToLEndian(in_write);  \
      return write(sizeof(type), &temp);           \
   }


class ColorI;
class ColorF;

//------------------------------------------------------------------------------
//-------------------------------------- Base Stream class
//
class Stream {
   // Public structs and enumerations...
  public:
   enum Status {
      Ok = 0,           // obv.
      IOError,          // Read or Write error
      EOS,              // End of Stream reached (mostly for reads)
      IllegalCall,      // An unsupported operation used.  Always w/ accompanied by AssertWarn
      Closed,           // Tried to operate on a closed stream (or detached filter)
      UnknownError      // Catchall
   };

   enum Capability {
      StreamWrite    = U32(1 << 0),
      StreamRead     = U32(1 << 1),
      StreamPosition = U32(1 << 2)
   };

   // Accessible only through inline accessors
  private:
   Status m_streamStatus;

   // Derived accessible data modifiers...
  protected:
   void setStatus(const Status in_newStatus) { m_streamStatus = in_newStatus; }

  public:
   Stream();
   virtual ~Stream();

   Status getStatus() const { return m_streamStatus; }
   static const char* getStatusString(const Status in_status);

   // Derived classes must override these...
  protected:
   virtual bool _read(const U32 in_numBytes,  void* out_pBuffer)      = 0;
   virtual bool _write(const U32 in_numBytes, const void* in_pBuffer) = 0;
  public:
   virtual bool hasCapability(const Capability) const = 0;

   virtual U32  getPosition() const                      = 0;
   virtual bool setPosition(const U32 in_newPosition) = 0;
   virtual U32  getStreamSize() = 0;

   void readLine(U8 *buffer, U32 bufferSize);
   void writeLine(U8 *buffer);
   
   const char *readSTString(bool casesens = false);
   virtual void readString(char stringBuf[256]);
   void readLongString(U32 maxStringLen, char *stringBuf);
   void writeLongString(U32 maxStringLen, const char *string);

   virtual void writeString(const char *stringBuf, S32 maxLen=255);

   bool write(const ColorI&);
   bool write(const ColorF&);
   bool read(ColorI*);
   bool read(ColorF*);


   // Overloaded write and read ops..
  public:
   bool read(const U32 in_numBytes,  void* out_pBuffer) {
      return _read(in_numBytes, out_pBuffer);
   }
   bool write(const U32 in_numBytes, const void* in_pBuffer) {
      return _write(in_numBytes, in_pBuffer);
   }
   DECLARE_OVERLOADED_WRITE(S8)
   DECLARE_OVERLOADED_WRITE(U8)
   DECLARE_OVERLOADED_WRITE(F64)

   DECLARE_ENDIAN_OVERLOADED_WRITE(S16)
   DECLARE_ENDIAN_OVERLOADED_WRITE(S32)
   DECLARE_ENDIAN_OVERLOADED_WRITE(U16)
   DECLARE_ENDIAN_OVERLOADED_WRITE(U32)
   DECLARE_ENDIAN_OVERLOADED_WRITE(F32)

   DECLARE_OVERLOADED_READ(S8)
   DECLARE_OVERLOADED_READ(U8)
   DECLARE_OVERLOADED_READ(F64)

   DECLARE_ENDIAN_OVERLOADED_READ(S16)
   DECLARE_ENDIAN_OVERLOADED_READ(S32)
   DECLARE_ENDIAN_OVERLOADED_READ(U16)
   DECLARE_ENDIAN_OVERLOADED_READ(U32)
   DECLARE_ENDIAN_OVERLOADED_READ(F32)

   // We have to do the bool's by hand, since they are different sizes
   //  on different compilers...
   //
   bool read(bool* out_pRead) {
      U8 translate;
      bool success = read(&translate);
      if (success == false)
         return false;

      *out_pRead = translate != 0;
      return true;
   }
   bool write(const bool& in_rWrite) {
      U8 translate = in_rWrite ? U8(1) : U8(0);
      return write(translate);
   }
};

#endif //_STREAM_H_
