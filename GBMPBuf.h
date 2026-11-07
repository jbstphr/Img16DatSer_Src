// Dataseries-Viewer MFC
// (c) LGPL v3.0; by S. Bochkarev
// BitMap buffer's service (declarations).
#pragma once

//#include "afxwin.h"

class GBMPBuf {
  CRITICAL_SECTION rSemaphore; // binary access-sentinel (without 'das Ordnung')
  int        rWstride; // >0 indicates allocated buffers
  BYTE      *rBMPBuf;  // bitmap's buffer (ready-to-display)
  void iAlloc( int AW, int AH ); // alloc BYTE-buffer
  void iImageToBMP( int ATag, const void *AImg  ); // <alloc and> fill BYTE-buffer
  void iStretchDIB( CDC *ADC, const CRect ARd, const CRect ARf ); // sretchblt
  int  rScaMn, rScaGam; bool rScaInv;
public:
  #pragma pack(2)
  struct { BITMAPINFOHEADER rBMI; RGBQUAD rPal[256]; } mBMPHdr; // bitmap's header
  #pragma pack()
  #ifdef USE_DIRECT2D // D2D-objects; ~reflect the GDI's ones
    ID2D1DCRenderTarget *m_pD2DCRT; BITMAPINFOHEADER m_D2BMI32; DWORD *m_D2BMPBuf32;
  #endif
  GBMPBuf(); ~GBMPBuf();
  int       *mImgBuf;  // 16-bit image-buffer
  bool       mHaveImg; // at least once ImageToBMP with current dimensions
  int        mBlack, mWhite; // grayscale transform 16-bit to 8-bit
  void Alloc( int AW, int AH ); // facultative pre-alloc BYTE-buffer
  void ImageToBMP( int AW, int AH, int ATag, const void *AImg ); // <alloc and> fill BYTE-buffer
    // "Tag=0":signed 16-bit, "=1":unsigned
    // if Img<>nil then store input data; if Img=nil then update BMP for stored data
  void StretchDIB( CDC *ADC, const CRect ARd, const CRect ARf ); // sretchblt
  int PixelAt( int AX, int AY ); // 16-bit gray-value
  void CopyToClipboardBMP(); // copy Displayed bitmap to clipboard
  void CopyToClipboardText(); // copy text to clipboard
}; 