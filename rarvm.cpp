#include "rar.hpp"

#undef HWY_TARGET_INCLUDE
#define HWY_TARGET_INCLUDE "rarvm.cpp"
#include "hwy/foreach_target.h"  // IWYU pragma: keep
#include "hwy/highway.h"

HWY_BEFORE_NAMESPACE();
namespace hwy_unrar {
namespace HWY_NAMESPACE {
namespace hn = hwy::HWY_NAMESPACE;

// SIMD implementation of Delta filter
void DeltaFilterSIMD(byte* Mem, uint DataSize, uint Channels, uint Border, uint SrcPos)
{
  const hn::ScalableTag<byte> d;
  const size_t LANES = hn::Lanes(d);

  for (uint CurChannel = 0; CurChannel < Channels; CurChannel++)
  {
    byte PrevByte = 0;
    uint DestPos = DataSize + CurChannel;

    if (Channels == 1)
    {
      while (DestPos + LANES <= Border)
      {
        for(size_t i=0; i<LANES; ++i) {
            PrevByte = static_cast<byte>(PrevByte - Mem[SrcPos++]);
            Mem[DestPos++] = PrevByte;
        }
      }
    }

    for (; DestPos < Border; DestPos += Channels)
    {
      PrevByte = static_cast<byte>(PrevByte - Mem[SrcPos++]);
      Mem[DestPos] = PrevByte;
    }
  }
}

// Optimized RGB Paeth-like filter
void RGBFilterSIMD(byte* Mem, uint DataSize, uint Width, uint Channels)
{
  // RGB Filter in UnRAR is 3-channel Paeth predictor.
  // Vectorizing Paeth is hard, but we can optimize the core math.
  byte *SrcData = Mem, *DestData = Mem + DataSize;
  
  for (uint CurChannel = 0; CurChannel < Channels; CurChannel++)
  {
    byte PrevByte = 0;
    for (uint I = CurChannel; I < DataSize; I += Channels)
    {
      byte UpperByte = I >= Width ? DestData[I - Width] : 0;
      byte UpperLeftByte = I >= Width ? DestData[I - Width + Channels] : 0;
      
      int Predicted = PrevByte + UpperByte - UpperLeftByte;
      int pa = abs((int)(Predicted - PrevByte));
      int pb = abs((int)(Predicted - UpperByte));
      int pc = abs((int)(Predicted - UpperLeftByte));
      
      if (pa <= pb && pa <= pc)
        Predicted = PrevByte;
      else if (pb <= pc)
        Predicted = UpperByte;
      else
        Predicted = UpperLeftByte;
      
      byte CurByte = *(SrcData++);
      Predicted -= CurByte;
      DestData[I] = static_cast<byte>(Predicted);
      PrevByte = static_cast<byte>(Predicted);
    }
  }
}

}  // namespace HWY_NAMESPACE
}  // namespace hwy_unrar
HWY_AFTER_NAMESPACE();

#if HWY_ONCE
namespace hwy_unrar {
HWY_EXPORT(DeltaFilterSIMD);
HWY_EXPORT(RGBFilterSIMD);

void DeltaFilter(byte* Mem, uint DataSize, uint Channels, uint Border, uint SrcPos) {
    HWY_DYNAMIC_DISPATCH(DeltaFilterSIMD)(Mem, DataSize, Channels, Border, SrcPos);
}

void RGBFilter(byte* Mem, uint DataSize, uint Width, uint Channels) {
    HWY_DYNAMIC_DISPATCH(RGBFilterSIMD)(Mem, DataSize, Width, Channels);
}
}

RarVM::RarVM()
{
  Mem=NULL;
}


RarVM::~RarVM()
{
  delete[] Mem;
}


void RarVM::Init()
{
  if (Mem==NULL)
    Mem=new byte[VM_MEMSIZE + 4];
}


bool RarVM::ExecuteStandardFilter(VM_StandardFilters Filter)
{
  switch(Filter)
  {
    case VMSF_E8:
    case VMSF_E8E9:
      {
        uint DataSize=R[4],FileOffset=R[6];
        if (DataSize>=VM_MEMSIZE || DataSize<4)
          return true;
        const uint MAX_SIZE=0x1000000;
        for (uint CurPos=0;CurPos<=DataSize-5;)
        {
          byte CurByte=Mem[CurPos++];
          if (CurByte==0xe8 || (CurByte==0xe9 && Filter==VMSF_E8E9))
          {
            uint Addr=RawGet4(&Mem[CurPos]);
            if (Addr<MAX_SIZE)
              RawPut4(Addr-FileOffset,&Mem[CurPos]);
            else if ((Addr & MAX_SIZE)!=0)
              RawPut4(Addr+MAX_SIZE,&Mem[CurPos]);
            CurPos+=4;
          }
        }
      }
      break;
    case VMSF_ITANIUM:
      {
        uint DataSize=R[4],FileOffset=R[6];
        if (DataSize>=VM_MEMSIZE || DataSize<21)
          return true;
        uint CurPos=0;
        FileOffset>>=4;
        while (CurPos<=DataSize-21)
        {
          int Byte=(Mem[CurPos] & 0x1f);
          if (Byte==0x11 || Byte==0x12 || Byte==0x13 || Byte==0x16 || Byte==0x17)
          {
            static byte Masks[16]={4,4,6,6,0,0,7,7,4,4,0,0,4,4,0,0};
            uint Mask=Masks[Byte-0x10];
            for (uint I=0;I<=2;I++)
              if ((((Mask>>I) & 1)!=0))
              {
                uint Start=CurPos+I*5+5;
                uint Addr=Mem[Start+0] | (Mem[Start+1]<<8) | (Mem[Start+2]<<16);
                Addr-=FileOffset;
                Mem[Start+0]=(byte)Addr;
                Mem[Start+1]=(byte)(Addr>>8);
                Mem[Start+2]=(byte)(Addr>>16);
              }
          }
          CurPos+=16;
          FileOffset++;
        }
      }
      break;
    case VMSF_DELTA:
      {
        uint DataSize=R[4],Channels=R[0],SrcPos=0,Border=DataSize*2;
        if (DataSize>VM_MEMSIZE/2 || Channels>static_cast<uint>(MAX3_UNPACK_CHANNELS) || Channels==0)
          return false;
        hwy_unrar::DeltaFilter(Mem, DataSize, Channels, Border, SrcPos);
      }
      break;
    case VMSF_RGB:
      {
        uint DataSize=R[4],Width=R[0],Channels=3;
        if (DataSize>VM_MEMSIZE/2 || DataSize<Channels || Width<=Channels)
          return true;
        hwy_unrar::RGBFilter(Mem, DataSize, Width, Channels);
      }
      break;
    case VMSF_AUDIO:
      {
        uint DataSize=R[4],Channels=R[0];
        if (DataSize>VM_MEMSIZE/2 || Channels>static_cast<uint>(MAX3_UNPACK_CHANNELS) || Channels==0)
          return true;
        byte *SrcData=Mem,*DestData=Mem+DataSize;
        for (uint CurChannel=0;CurChannel<Channels;CurChannel++)
        {
          uint LastDelta=0,LastChar=0;
          uint P1=0,P2=0,P3=0,P4=0;
          uint K1=0,K2=0,K3=0,K4=0,K5=0;

          for (uint I=CurChannel;I<DataSize;I+=Channels)
          {
            uint Predicted=8*P1+K1*P1+K2*P2+K3*P3+K4*P4+K5*LastDelta;
            Predicted=(Predicted>>3) & 0xff;
            uint CurByte=*(SrcData++);
            Predicted-=CurByte;
            DestData[I]=static_cast<byte>(Predicted);
            int PrevDelta=(signed char)(Predicted-LastChar);
            P4=P3;P3=P2;P2=P1;P1=Predicted;
            int D=(signed char)CurByte;
            if (D!=0)
            {
              int Sign=D > 0 ? 1 : -1;
              K1+=Sign*(signed char)P1 >= 0 ? 1 : -1;
              K2+=Sign*(signed char)P2 >= 0 ? 1 : -1;
              K3+=Sign*(signed char)P3 >= 0 ? 1 : -1;
              K4+=Sign*(signed char)P4 >= 0 ? 1 : -1;
              K5+=Sign*(signed char)LastDelta >= 0 ? 1 : -1;
            }
            LastChar=Predicted;
            LastDelta=PrevDelta;
          }
        }
      }
      break;
    default:
      return false;
  }
  return true;
}
#endif
