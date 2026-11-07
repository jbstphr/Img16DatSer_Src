// Dataseries-Viewer MFC
// (c) LGPL v3.0; by S. Bochkarev
// BitMap buffer's service (implementation)
#include "StdAfx.h"
#include "ViewApp.h" // <from the application-object> here use only ~debug trace 
#include "VD_UtViewDlg.h" // adhoc main dialog controls' access
#include "VD_NMDlg1.h" // dialog [BPM]
#include "GBMPSca.h"
#include "GBMPBuf.h"

#ifdef USE_DIRECT2D
  #include <d2d1.h>
  #pragma comment( lib, "d2d1.lib" )
  static int ixD2D_err = 0; 
  static bool iD2D_OK( HRESULT Ahr, const TCHAR *Apfx ) {
    if (SUCCEEDED(Ahr)) return true; if (++ixD2D_err>2) return false;
    TCHAR qS[192], *qCh = qS+_stprintf( qS, _T("ERR: D2D %s:= %X"), Apfx, Ahr );
    WriteLn( qS ); if (ixD2D_err==1) qCh += _stprintf( qCh, _T("\r\n") );
    if (FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL, Ahr, MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT), qCh, 128, NULL )>0) WriteLn( qCh );
    if (ixD2D_err==1) AfxMessageBox( qS, MB_ICONSTOP ); return false; }
  
  // the most standard format for Direct2D
  #define cPixelFormat D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM,D2D1_ALPHA_MODE_PREMULTIPLIED)

  static void iD2D_Construct( GBMPBuf *ASelf ) { // -- construct D2D-objects
    WriteLn( _T("-- GBMPBuf::iD2D_Alloc") );
    ASelf->m_pD2DCRT = NULL; ASelf->m_D2BMPBuf32 = NULL; // ASelf->m_pD2DBrush = NULL;
    const D2D1_RENDER_TARGET_PROPERTIES cProps = D2D1::RenderTargetProperties(
      D2D1_RENDER_TARGET_TYPE_DEFAULT, cPixelFormat,
      0.0f, 0.0f, D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE ); // for use in GDI
    iD2D_OK( AfxGetD2DState()->GetDirect2dFactory()->CreateDCRenderTarget(&cProps,&ASelf->m_pD2DCRT),
      _T("CreateDCRenderTarget") ); }

  static void iD2D_Destruct( GBMPBuf *ASelf ) { // -- destruct D2D-objects
    WriteLn( _T("-- GBMPBuf::iD2D_Release") );
    #if 0 // Direct2D might spontaneously release these objects; 'no release' still doesn't report memory-leaks.
      ID2D1DCRenderTarget *qCRT = m_pD2DCRT; m_pD2DCRT = NULL; try { if (qCRT) qCRT->Release(); } catch(...) {};
    #endif
    if (ixD2D_err) { TCHAR qS[80]; _stprintf( qS,
      _T("WRN: %d errors happened"), ixD2D_err ); WriteLn( qS ); ixD2D_err = 0; }; }

  static void iD2D_Alloc( GBMPBuf *ASelf ) { // -- allocate D2D-objects
    BITMAPINFOHEADER *qD2BMI32 = &ASelf->m_D2BMI32;
    memcpy( qD2BMI32, &ASelf->mBMPHdr.rBMI, sizeof(BITMAPINFOHEADER) );
    qD2BMI32->biClrUsed = qD2BMI32->biClrImportant = 0; qD2BMI32->biBitCount = 32;
    ASelf->m_D2BMPBuf32 = new DWORD[qD2BMI32->biWidth*qD2BMI32->biHeight]; }

  static void iD2D_DeAlloc( GBMPBuf *ASelf ) { // -- deallocate D2D-objects
    if (ASelf->m_D2BMPBuf32) { delete ASelf->m_D2BMPBuf32; ASelf->m_D2BMPBuf32 = NULL; }; }

#else // D2D-stubs
  static void iD2D_Construct( GBMPBuf *ASelf ) {}
  static void iD2D_Destruct( GBMPBuf *ASelf ) {}
  static void iD2D_Alloc( GBMPBuf *ASelf ) {}
  static void iD2D_DeAlloc( GBMPBuf *ASelf ) {}
#endif

GBMPBuf::GBMPBuf() {
  rWstride = 0; rBMPBuf = NULL; mHaveImg = false;
  mBlack = 0; mWhite = (1<<14)-1;
  memset( &mBMPHdr, 0, sizeof(mBMPHdr) ); mBMPHdr.rBMI.biSize = sizeof(BITMAPINFOHEADER);
  iD2D_Construct( this ); // construct D2D-objects
  InitializeCriticalSection( &rSemaphore ); }; 

GBMPBuf::~GBMPBuf() { iD2D_Destruct( this ); // detruct D2D-objects
  Alloc( -1, -1 ); DeleteCriticalSection( &rSemaphore ); }

void GBMPBuf::iAlloc( int AW, int AH ) { // -- alloc BYTE-buffer 
  BITMAPINFOHEADER *qBMI = &mBMPHdr.rBMI;
  if ((qBMI->biWidth==AW) && (qBMI->biHeight==AH)) return; mHaveImg = false;
  #ifdef _DEBUG
    TCHAR qS[80]; _stprintf( qS, _T("-- GBMPBuf::iAlloc(%dx%d)"), AW, AH ); WriteLn( qS );
  #endif
  qBMI->biWidth = AW; qBMI->biHeight = AH;
  if (mImgBuf) { delete mImgBuf; mImgBuf = NULL; };
  if (rBMPBuf) { delete rBMPBuf; rBMPBuf = NULL; };
  iD2D_DeAlloc( this ); if (AW<=0) return; // deallocate D2D-objects
  mImgBuf = new int[AW*AH]; memset( mImgBuf, 0, AW*AH*sizeof(int) );
  AW = (AW+3)&~3; //if (AW&3) AW += 4-(AW&3);
  qBMI->biSizeImage = AW*AH;
  qBMI->biBitCount = 8; qBMI->biPlanes = 1; qBMI->biClrImportant = qBMI->biClrUsed = 256;
  // qBMI->biXPelsPerMeter = (long)(1000/0.20f); qBMI->biYPelsPerMeter = (long)(1000/0.20f);
  for (int qi3=0;qi3<256;qi3++) { RGBQUAD *qPal = mBMPHdr.rPal+qi3;
    qPal->rgbRed = qPal->rgbGreen = qPal->rgbBlue = qi3; };
  rBMPBuf = new BYTE[qBMI->biSizeImage]; rWstride = AW;
  iD2D_Alloc( this ); } // allocate D2D0objects

void GBMPBuf::iImageToBMP( int ATag, const void *AImg ) { // -- fill BYTE-buffer
  int      qWstride = rWstride; if (!qWstride) return; mHaveImg = true;
  int      qW = mBMPHdr.rBMI.biWidth, qH = mBMPHdr.rBMI.biHeight,
           qN = qW*qH, *qImg = mImgBuf;
  if (AImg) switch (ATag) { // Tag=0:signed 16-bit, =1:unsigned
    case  1: { const unsigned short *qSrc = (const unsigned short*)AImg; // 1:unsigned 
      for (int qi=0;qi<qN;qi++) qImg[qi] = qSrc[qi]; }; break;
    default: { const short *qSrc = (const short*)AImg; // 0:signed
      for (int qi=0;qi<qN;qi++) qImg[qi] = qSrc[qi]; }; };
  BYTE    *qBB = rBMPBuf+qWstride*(qH-1); qWstride = -qWstride;
  for (int qi=0;qi<qH;qi++) {
    GBMPSca_32to8( qW, mBlack, mWhite, qImg, qBB );
    qImg += qW; qBB += qWstride; };  // for i
  // the entries #0.. of Pal could be modified ad-hoc
  BPM_DIB8Update( &mBMPHdr.rBMI, mBMPHdr.rPal, rBMPBuf ); }

static const int cGray = 199;

#ifdef USE_DIRECT2D
  void GBMPBuf::iStretchDIB( CDC *ADC, const CRect ARd, const CRect ARf ) { // -- sretchblt by Direct2D
    //int qStrtchBltMode = ADC->GetStretchBltMode();
    int qRes = 0; if (mHaveImg) { qRes = -1;
      // create transient 32-bit bitmap
      int    cWstride = rWstride;
      BYTE  *qBMPBuf = rBMPBuf+mBMPHdr.rBMI.biSizeImage-cWstride*(ARf.top+1)+ARf.left ;
      DWORD *qD2BMPBuf32 = m_D2BMPBuf32; DWORD *cPal = (DWORD*)mBMPHdr.rPal; 
      int    cw = m_D2BMI32.biWidth = ARf.Width(), ch = m_D2BMI32.biHeight = ARf.Height();
      m_D2BMI32.biSizeImage = cw*ch<<2; 
      for (int qr=0;qr<ch;qr++) {
        for (int qc=0;qc<cw;qc++) *qD2BMPBuf32++ = cPal[qBMPBuf[qc]];
        qBMPBuf -= cWstride ; };
      // stretch transient 32-bit bitmap
      m_pD2DCRT->BindDC( ADC->m_hDC, &ARd ); m_pD2DCRT->BeginDraw();
      m_pD2DCRT->Clear( D2D1::ColorF(D2D1::ColorF::Black) );
      const D2D1_BITMAP_PROPERTIES cbmpProps = D2D1::BitmapProperties( cPixelFormat, 0.0f, 0.0f );
      ID2D1Bitmap *qD2DBitmap; HRESULT qhr =
        m_pD2DCRT->CreateBitmap( D2D1::SizeU(cw,ch), m_D2BMPBuf32, cw*4, &cbmpProps, &qD2DBitmap );
      if (iD2D_OK(qhr,_T("CreateBitmap")) && qD2DBitmap) {
        const D2D1_RECT_F
          crectDst = D2D1::RectF( 0, 0, (FLOAT)ARd.Width(), (FLOAT)ARd.Height() ),
          crectSrc = D2D1::RectF( 0, 0, (FLOAT)cw, (FLOAT)ch );
        m_pD2DCRT->DrawBitmap( qD2DBitmap, &crectDst, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, &crectSrc );
        qD2DBitmap->Release(); qRes = 1;
      }; // if D2DBitmap
      iD2D_OK( m_pD2DCRT->EndDraw(), _T("EndDraw") );
    }; // if HaveImg
    if (qRes<1) ADC->FillSolidRect( ARd, !qRes?GetSysColor(COLOR_BTNFACE):RGB(cGray,cGray,cGray) );
  }
#else
  void GBMPBuf::iStretchDIB( CDC *ADC, const CRect ARd, const CRect ARf ) { // -- sretchblt by GDI
    ADC->SetBrushOrg( 0, 0 ); 
    int qRes = !mHaveImg ? -1 : qRes = StretchDIBits( ADC->m_hDC,
      ARd.left, ARd.top, ARd.Width(), ARd.Height(),
      ARf.left, mBMPHdr.rBMI.biHeight-ARf.bottom, ARf.Width(), ARf.Height(),
      rBMPBuf, (BITMAPINFO*)&mBMPHdr,
      DIB_RGB_COLORS, SRCCOPY );
    if (qRes<1) ADC->FillSolidRect( ARd, !qRes?GetSysColor(COLOR_BTNFACE):RGB(cGray,cGray,cGray) );
  }
#endif

void GBMPBuf::Alloc( int AW, int AH ) { // -- facultative pre-alloc BYTE-buffer
  EnterCriticalSection( &rSemaphore ); __try { iAlloc( AW, AH );
  } __finally { LeaveCriticalSection( &rSemaphore ); }; }

void GBMPBuf::ImageToBMP( int AW, int AH, int ATag, const void *AImg ) { // -- alloc and fill BYTE-buffer
  EnterCriticalSection( &rSemaphore ); __try {
    if (AW) iAlloc( AW, AH ); iImageToBMP( ATag, AImg );
  } __finally { LeaveCriticalSection( &rSemaphore ); }; }

void GBMPBuf::StretchDIB( CDC *ADC, const CRect ARd, const CRect ARf ) { // -- sretchblt 
  EnterCriticalSection( &rSemaphore ); __try { iStretchDIB( ADC, ARd, ARf );
  } __finally { LeaveCriticalSection( &rSemaphore ); }; }

int GBMPBuf::PixelAt( int AX, int AY ) { // -- 16-bit gray-value
  if (!mHaveImg || AY<0 || AY>=mBMPHdr.rBMI.biHeight || AX<0) return 0x80000000;
  int qW = mBMPHdr.rBMI.biWidth; if (AX>=qW ) return 0x80000000;
  return mImgBuf[AX+AY*qW]; }

void GBMPBuf::CopyToClipboardBMP() { // -- copy Displayed bitmap to clipboard
  if (!mHaveImg || !OpenClipboard( theDlg.m_hWnd )) return;
  const   CRect cRf = theDlg.m_Rf;
  int     qW = cRf.Width(), qW1 = qW, qH = cRf.Height(), qH1 = qH;
  // this is mainly for limitation of WinE's clipboard ...
  float   cScale = (float)(gBmpCpyMxDim*gBmpCpyMxDim)/(qW*qH); // stretch copying BMP ~> BmpCpyMxDim*BmpCpyMxDim
  if (cScale<0.999999f) { qW1 = (int)(qW1*cScale); qH1 = (int)(qH1*cScale); };
  int     qW1stride = (qW1+3)&~3, qMemSizeImage = qW1stride*qH1;
  HANDLE  qMemHandle = GlobalAlloc( GMEM_MOVEABLE, sizeof(mBMPHdr)+qMemSizeImage ); 
  if (!qMemHandle) { CloseClipboard(); return; };
  char   *qData = (char*)GlobalLock( qMemHandle );
  if (!qData) { CloseClipboard(); GlobalFree( qMemHandle ); return; }; EmptyClipboard();
  // clipboard bitmap's header
  BITMAPINFOHEADER *qB = (BITMAPINFOHEADER*)qData; memcpy( qB, &mBMPHdr, sizeof(mBMPHdr) );
  qB->biWidth = qW1; qB->biHeight = qH1; qB->biSizeImage = qMemSizeImage;
  // stretch BMPBuf to cliboard bitmap's data
  HDC       qHMemDC = CreateCompatibleDC( NULL ); if (qHMemDC) { void *qMemBMPBuf = NULL; 
    HBITMAP qHMemB = CreateDIBSection( qHMemDC, (BITMAPINFO*)qB, DIB_RGB_COLORS, &qMemBMPBuf, NULL, 0 );
    if (qHMemB) {
      HBITMAP qHOldBmp = (HBITMAP)SelectObject( qHMemDC, qHMemB );
      SetStretchBltMode( qHMemDC, HALFTONE ); SetBrushOrgEx( qHMemDC, 0, 0, NULL ); 
      StretchDIBits( qHMemDC, 0, 0, qW1, qH1,
        cRf.left, mBMPHdr.rBMI.biHeight-cRf.bottom, qW, qH,
        rBMPBuf, (BITMAPINFO*)&mBMPHdr, DIB_RGB_COLORS, SRCCOPY );
      memcpy( qData+sizeof(mBMPHdr), qMemBMPBuf, qMemSizeImage );
      SelectObject( qHMemDC, qHOldBmp ); DeleteObject( qHMemB );
    } else // if MemB=nil
      WriteLn( _T("ERR CopyToClipboardBMP.CreateDIBSection") );
    DeleteDC( qHMemDC );
  } else // if MemDC=nil
    WriteLn( _T("ERR CopyToClipboardBMP.CreateCompatibleDC") );
  // leave
  SetClipboardData( CF_DIB, qMemHandle );
  GlobalUnlock( qMemHandle );
  CloseClipboard(); }

void GBMPBuf::CopyToClipboardText() { // -- copy text to clipboard
  if (!mHaveImg || !OpenClipboard( theDlg.m_hWnd )) return;
  CRect qROI = theA_Clip( theD_ROI() );
  if (qROI.IsRectEmpty()) { CloseClipboard(); return; };
  if (qROI.Width()>99) qROI.right = qROI.left+99;
  if (qROI.Height()>99) qROI.bottom = qROI.top+99;
  int     qBufSize = (qROI.Width()+1)*(qROI.Height()+1)*8+qROI.Height()+1;
  HANDLE  qMemHandle = GlobalAlloc( GMEM_MOVEABLE, qBufSize*sizeof(char) );
  if (!qMemHandle) { CloseClipboard(); return; };
  char   *qData = (char*)GlobalLock( qMemHandle );
  if (!qData) { CloseClipboard(); return; };
  EmptyClipboard();
  char   *qCh = qData; qCh += sprintf( qCh, "      " );
  for (int qcol=qROI.left;qcol<qROI.right;qcol++)
    qCh += sprintf( qCh, "%6d", qcol );
  qCh += sprintf( qCh, "\n" );
  for (int qrow=qROI.top;qrow<qROI.bottom;qrow++) {
    qCh += sprintf( qCh, "%6d", qrow );
    for (int qcol=qROI.left;qcol<qROI.right;qcol++) {
      qCh += sprintf( qCh, "%6d", PixelAt(qcol,qrow) ); };
    qCh += sprintf( qCh, "\n" );
  };
  SetClipboardData( CF_TEXT, qMemHandle ); // CF_UNICODETEXT 
  GlobalUnlock( qMemHandle );
  CloseClipboard(); }
  