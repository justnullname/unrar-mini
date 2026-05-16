#ifndef _RAR_DATAIO_
#define _RAR_DATAIO_

#include "os.hpp"
#include "rartypes.hpp"
#include "hash.hpp"
#include "file.hpp"
#include "headers.hpp"

class CmdAdd;
class Archive;
class Unpack;

class ComprDataIO
{
  private:
    // Memory source/dest for QuickView zero-copy pipeline.
    const byte *UnpSrcAddr = nullptr;
    size_t UnpSrcSize = 0;
    size_t UnpSrcPos = 0;

    byte *UnpDstAddr = nullptr;
    size_t UnpDstSize = 0;
    size_t UnpDstPos = 0;

    int64 UnpPackedSize = 0;
    int64 CurPackRead = 0;

  public:
    ComprDataIO() : UnpVolume(false) {}
    ~ComprDataIO() {}
    void Init() { UnpSrcPos = 0; UnpDstPos = 0; CurPackRead = 0; }

    // === QuickView DOD: wire memory regions ===
    void SetMemorySource(const byte *Addr, size_t Size) {
      UnpSrcAddr = Addr;
      UnpSrcSize = Size;
      UnpSrcPos = 0;
    }

    void SetMemoryDest(byte *Addr, size_t Size) {
      UnpDstAddr = Addr;
      UnpDstSize = Size;
      UnpDstPos = 0;
    }

    void SetPackedSizeToRead(int64 Size) { UnpPackedSize = Size; CurPackRead = 0; }
    void SetMemoryPos(size_t Pos) { UnpSrcPos = Pos; }

    int UnpRead(byte *Addr, size_t Count) {
      if (UnpSrcAddr == nullptr) return -1;
      int64 remain = UnpPackedSize - CurPackRead;
      if (remain <= 0) return 0;
      size_t srcRemain = UnpSrcSize - UnpSrcPos;
      size_t toRead = Count;
      if ((int64)toRead > remain) toRead = (size_t)remain;
      if (toRead > srcRemain) toRead = srcRemain;
      if (toRead == 0) return 0;
      memcpy(Addr, UnpSrcAddr + UnpSrcPos, toRead);
      UnpSrcPos += toRead;
      CurPackRead += (int64)toRead;
      return (int)toRead;
    }

    void UnpWrite(byte *Addr, size_t Count) {
      if (UnpDstAddr == nullptr) return;
      size_t dstRemain = UnpDstSize - UnpDstPos;
      size_t toWrite = (Count < dstRemain) ? Count : dstRemain;
      if (toWrite > 0) {
        memcpy(UnpDstAddr + UnpDstPos, Addr, toWrite);
        UnpDstPos += toWrite;
      }
    }

    bool UnpVolume;
    bool Suspended;
    DataHash UnpHash;

    void SetFiles(File *src, File *dest) { this->SrcFile = src; this->DestFile = dest; }
    void SetSuspended(bool suspended) { this->Suspended = suspended; }
    void SetPackedSize(int64 Size) { UnpPackedSize = Size; }
    void SetTestMode(bool mode) { (void)mode; }
    void SetSkipBefore(int64 size) { (void)size; }
    void SetEncryption(bool password, CRYPT_METHOD method, SecPassword *passwordPtr,
         const byte *salt, const byte *initV, uint lg2Count,
         const byte *hashKey, const byte *pswCheck) 
    { 
        (void)password; (void)method; (void)passwordPtr; (void)salt; (void)initV; (void)lg2Count; (void)hashKey; (void)pswCheck;
    }
    void SetSubHeader(FileHeader *hd, int64 *pos) { (void)hd; (void)pos; }
    void SetNoFileHeader(bool mode) { (void)mode; }
    void SetUnpackToMemory(byte *Addr, uint Size) { SetMemoryDest(Addr, Size); }
    void GetUnpackedData(byte **data, size_t *size) { (void)data; (void)size; }
    void SetCurrentCommand(wchar cmd) { (void)cmd; }
    
    int64 CurUnpRead = 0;
    int64 CurUnpWrite = 0;

    size_t GetWrittenSize() const { return UnpDstPos; }

    // Compatibility stubs
    void EnableShowProgress(bool /*Show*/) {}
    void SetSkipUnpCRC(bool /*Skip*/) {}
    int64 UnpArcSize = 0;
    int64 UnpFileCRC = 0;
    bool PackFromMemory = false;
    bool UnpackToMemory = false;
    bool Encryption = false;
    bool Decryption = false;
    File *SrcFile = nullptr;
    File *DestFile = nullptr;
    Archive *SrcArc = nullptr;
};

#endif
