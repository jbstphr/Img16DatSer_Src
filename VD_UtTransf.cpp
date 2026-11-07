// Dataseries-Viewer MFC
// (c) LGPL v3.0; by S. Bochkarev
// file VD_UtTransf.cpp : [crop,rotate,flip]
#include "StdAfx.h"
#include "VD_Ut.h" // casual capture utils
#include "ViewApp.h" // the application-object
#include "VD_UtViewDlg.h" // adhoc main dialog controls' access
#include "VD_UtTransf.h"

//// frame=>frame transformations ////

int UtTransf_Average( // -- average of selected *.raw
  CListBox *ALB, TCHAR *APath, TCHAR *ANam, int ASelCnt, short *ASelIdxs, const TCHAR *ADstNam ) {
  const int cNP = theA_W()*theA_H();
  int   *qAcc = new int[cNP]; memset( qAcc, 0, cNP*sizeof(int) );
  short *qBuf = new short[cNP];
  int    qC = 0;
  for (int qi=0;qi<ASelCnt;qi++) { ALB->GetText( ASelIdxs[qi], ANam );
    FILE *qF = _tfopen( APath, _T("rb") ); if (!qF) {
      _tcscat( ANam, _T(" !!! can't read") ); WriteLn( APath ); continue; };
    memset( qBuf, 0, cNP*sizeof(short) );
    fread( qBuf, sizeof(short), cNP, qF ); fclose( qF );
    if (theA_GrayMn()<0) // signed 16-bits
      for (int qn=0;qn<cNP;qn++) qAcc[qn] += qBuf[qn];
    else { // unsigned 16-bits
      unsigned short *qBuu = (unsigned short*)qBuf;
      for (int qn=0;qn<cNP;qn++) qAcc[qn] += qBuu[qn]; };
    qC++; }; // for i
  if (qC>0) {
    for (int qn=0;qn<cNP;qn++) qBuf[qn] = (short)(qAcc[qn]/qC);
    _tcscpy( ANam, ADstNam ); FILE *qF = _tfopen( APath, _T("wb") );
    if (qF) { fwrite( qBuf, sizeof(short), cNP, qF ); fclose( qF ); }
       else { _tcscat( ANam, _T(" !!! can't write") ); WriteLn( APath ); };
  }; // if C>0
  // leave
  delete qAcc; delete qBuf;
  return qC; }

int UtTransf_ToBMP( // -- convert selected *.raw to *.bmp
  CListBox *ALB, TCHAR *APath, TCHAR *ANam, int ASelCnt, short *ASelIdxs ) {
  const bool cUnSigned = theA_GrayMn()>=0;
  const int cGamut = theD_Gamut(); // 0:explict, 1:cut_hist, 2:cut_hist_inv
  const bool cCutByHist = cGamut>0;
  TCHAR qS[256], *qCh = qS+_stprintf( qS,
    _T("-- UtTransf_ToBMP(%s G=%d "), cUnSigned?_T("uns"):_T("sig"), cGamut );
  int qWhite, qBlack; // whether explicit gamut is specified...
  // to invoke _Hist16BaseBuild,_HistCut; save modified fields of theApp
  struct { int rCutWhite,rCutBlack,saMn,saMx,*saHist; bool saOk; } qCut;
  if (cCutByHist) {
    qCut.rCutWhite = VaUt_SpinInt( &theDlg, IDC_EDIT8 ); // [cut white in 0.1%]
    qCut.rCutBlack = VaUt_SpinInt( &theDlg, IDC_EDIT10 ); // [cut black in 0.1%]
    qCut.saHist = theApp.m_Hist; theApp.m_Hist = new int[1<<16];
    qCut.saMn = theApp.m_nMn; qCut.saMx = theApp.m_nMx;
    qCut.saOk = theApp.m_bHistOk; theApp.m_bHistOk = true;
    qCh += _stprintf( qCh, _T("cutB=%d cutW=%d)"), qCut.rCutBlack, qCut.rCutWhite );
  } else {
    qWhite = VaUt_SpinInt( &theDlg, IDC_EDIT9 ); // [explicit white]
    qBlack = VaUt_SpinInt( &theDlg, IDC_EDIT11 ); // [explicit black]
    qCh += _stprintf( qCh, _T("B=%d W=%d)"), qBlack, qWhite );
  };
  WriteLn( qS );
  const int cW = theA_W(), cH = theA_W(), cNP = cW*cH;
  int qW4 = cW; if (qW4&3) qW4 += 4-(qW4&3); int qSizeImage = qW4*cH;
  // windows bitmap
  #pragma pack(2) // bitmap's header
  struct { BITMAPFILEHEADER rFil; BITMAPINFOHEADER rBMI; RGBQUAD rPal[256]; } qHdr;
  #pragma pack()
  memset( &qHdr, 0, sizeof(qHdr) ); qHdr.rFil.bfType = 0x4D42;
  qHdr.rFil.bfOffBits = sizeof(qHdr); qHdr.rFil.bfSize = sizeof(qHdr)+qSizeImage;
  qHdr.rBMI.biSize = sizeof(BITMAPINFOHEADER); qHdr.rBMI.biWidth = cW; qHdr.rBMI.biHeight = cH;
  qHdr.rBMI.biPlanes = 1; qHdr.rBMI.biBitCount = 8; qHdr.rBMI.biSizeImage = qSizeImage;
  for (int qip=0;qip<256;qip++) // bitmap's palette
    qHdr.rPal[qip].rgbRed = qHdr.rPal[qip].rgbGreen = qHdr.rPal[qip].rgbBlue = qip;
  // loop by files
  short *qBuf = new short[cNP]; BYTE *qBufB = new BYTE[qSizeImage];
  int qC = 0;
  for (int qif=0;qif<ASelCnt;qif++) { ALB->GetText( ASelIdxs[qif], ANam );
    // read *.raw
    FILE *qF = _tfopen( APath, _T("rb") ); if (!qF) {
      _tcscat( ANam, _T(" !!! can't read") ); WriteLn( APath ); continue; };
    memset( qBuf, 0, cNP*sizeof(short) );
    fread( qBuf, sizeof(short), cNP, qF ); fclose( qF );
    if (cCutByHist) { // optionally determine Black & White
      ViD_Hist16BaseBuild( qBuf, false, false ); 
      qWhite = ViD_HistCutWhite( qCut.rCutWhite );
      qBlack = ViD_HistCutBlack( qCut.rCutBlack );
      if (qWhite<qBlack) mcSwap( int, qWhite, qBlack ) else if (qWhite==qBlack) qWhite++;
      if (cGamut==2) mcSwap( int, qWhite, qBlack ); // if cut_hist_inv
    };
    // convert to BMP
    float qScale = 255.0f/(qWhite-qBlack); // simplest float grayscale
    const short *cSrc = qBuf; BYTE *qDst = qBufB+qSizeImage-qW4;
    for (int qrow=0;qrow<cH;qrow++) {// by rows
      for (int qcol=0;qcol<cW;qcol++) { // by cols
        int qV = cUnSigned ? *(unsigned short*)cSrc : *cSrc; cSrc++; 
        qV = (int)((qV-qBlack)*qScale); qDst[qcol] = mcRng( qV, 0, 255 ); }; // by cols
      qDst -= qW4; }; //by rows
    // write BMP
    _tcscpy( _tcsrchr(ANam,_T('.')), _T(".bmp") );
    qF = _tfopen( APath, _T("wb") ); if (!qF) {
      _tcscat( ANam, _T(" !!! can't write") ); WriteLn( APath ); continue; }; 
    fwrite( &qHdr, 1, sizeof(qHdr), qF ); fwrite( qBufB, 1, qSizeImage, qF ); fclose( qF );
    _stprintf( qS, _T(" - %s B=%d W=%d"), ANam, qBlack, qWhite );
    WriteLn( qS ); qC++;
  }; // for if
  // leave
  if (cCutByHist) { delete theApp.m_Hist; theApp.m_Hist = qCut.saHist;
    theApp.m_nMn = qCut.saMn; theApp.m_nMx = qCut.saMx; theApp.m_bHistOk = qCut.saOk; };
  delete qBuf; delete qBufB;
  return qC; }

//// frame=>image transformations ////

void UtTransf_ImgDim( // -- transformed image's dimension
  const tUtTransf *ARec, int *AW, int *AH ) {
  int qW = ARec->rSttW, qH = ARec->rSttH;
  if (qW<4 || qW>=(1<<15) || qH<4 || qH>=(1<<15)) {
    qW = theA_W(); qW -= ARec->rLft+ARec->rRgt; 
    qH = theA_H(); qH -= ARec->rTop+ARec->rBtm; };
  if (ARec->rROF&0x30) { *AW = qH; *AH = qW; }
                  else { *AW = qW; *AH = qH; }; }

// stretch service
template <class tA> inline float interpolatorB3Conv( float AT, const tA AA[4] ) { // bicubic spline from wikipedia
  tA m0 = AA[1]-AA[0], m1 = AA[3]-AA[2]; return (((2*AA[1]+m0-2*AA[2]+m1)*AT-3*AA[1]+3*AA[2]-2*m0-m1)*AT+m0)*AT+AA[1]; }
template <class tA> inline int interpolatorB3Conv2D( int AW, int AH, const tA *AData, float ATX, float ATY ) {
  int qI = (int)ATX; ATX -= qI;
  int qIX[4];
  qIX[0] = qI-1; if (qIX[0]<0) qIX[0] = 0;
  qIX[1] = qI;
  qIX[2] = qI+1; if (qIX[2]>=AW) qIX[2] = AW-1;
  qIX[3] = qI+2; if (qIX[3]>=AW) qIX[3] = AW-1;
  qI = (int)ATY; ATY -= qI;
  int qIY[4];
  qIY[0] = qI-1; if (qIY[0]<0) qIY[0] = 0;
  qIY[1] = qI;
  qIY[2] = qI+1; if (qIY[2]>=AH) qIY[2] = AH-1;
  qIY[3] = qI+2; if (qIY[3]>=AH) qIY[3] = AH-1;
  // return AData[qIY[1]*AWidth+qIX[1]];
  float qB[4]; for (int qi=0;qi<4;qi++) { const tA *cData = AData+qIY[qi]*AW; int qA[4];
    for (int qj=0;qj<4;qj++) qA[qj] = cData[qIX[qj]]; qB[qi] = interpolatorB3Conv( ATX, qA ); };
  int qRes = (int)interpolatorB3Conv( ATY, qB );
  return qRes; }

static short *iTransf( const tUtTransf *ARec, int AWfrm, int AHfrm, short *ASrcFrm, short *ABuf ) { // frame=>image; 
  // returns address of result-buffer; the dimension shall coincide with output of _ImgDim
  short *qInp = ASrcFrm, *qOut = ABuf;
  int qW = theA_W(), qH = theA_H(), qWout, qHout;
  // crop
  qWout = qW-ARec->rLft-ARec->rRgt; qHout = qH-ARec->rTop-ARec->rBtm;
  if (qWout!=qW || qHout!=qH) {
    short *qSrc = qInp+ARec->rTop*qW+ARec->rLft, *qDst = qOut;
    for (int qi=0;qi<qHout;qi++) { 
      memcpy( qDst, qSrc, qWout*sizeof(short) ); qSrc += qW; qDst += qWout; };
    qW = qWout; qH = qHout; mcSwap( short*, qInp, qOut );
  };
  // ~primitive slow stretch
  qWout = ARec->rSttW; qHout = ARec->rSttH;
  if (qWout && qHout && (qWout!=qW || qHout!=qH)) { short *qDst = qOut;
    float cScaleX = (float)(qW-1)/(qWout-1), cScaleY = (float)(qH-1)/(qHout-1);
    for (int qi=0;qi<qHout;qi++) { float qTY = qi*cScaleY; 
      for (int qj=0;qj<qWout;qj++) { float qTX = qj*cScaleX;
        if (theA_GrayMn()<0) { // signed 16-bits
          int qV = interpolatorB3Conv2D( qW, qH, qInp, qTX, qTY );
          qDst[qj] = mcRng( qV, -(1<<15)+1, (1<<15)-1 ); 
        } else { // unsigned 16-bits
          int qV = interpolatorB3Conv2D( qW, qH, (unsigned short*)qInp, qTX, qTY );
          qDst[qj] = mcRng( qV, 0, (1<<16)-1 );
        }; }; // for j
      qDst += qWout; }; // for i
    qW = qWout; qH = qHout; mcSwap( short*, qInp, qOut );
  };
  // rotate
  if (ARec->rROF&0x30) { short *qDst = qOut;
    switch (ARec->rROF&0x30) {
      case 0x10: { for (int qi=0;qi<qH;qi++) { short *qSrc = qInp+qH-1-qi; // rot_CCW
        for (int qj=0;qj<qW;qj++) { *qDst++ = *qSrc; qSrc += qH; }; };
      }; break;
      case 0x20: { for (int qi=0;qi<qH;qi++) { short *qSrc = qInp+(qW-1)*qH+qi; // rot_CW
        for (int qj=0;qj<qW;qj++) { *qDst++ = *qSrc; qSrc -= qH; }; };
      }; break;
    }; mcSwap( int, qW, qH ); mcSwap( short*, qInp, qOut );
  };
  // flip
  if (ARec->rROF&1) { short *qDat = qInp; // flip horz
    for (int qi=0;qi<qH;qi++) { short *qDat0 = qDat, *qDat1 = qDat+qW-1;
      do { short qT = *qDat0; *qDat0 = *qDat1; *qDat1 = qT; } while (++qDat0<--qDat1); qDat += qW; };
  };
  if (ARec->rROF&2) { short *qDat0 = qInp, *qDat1 = qDat0+qW*(qH-1); // flip vert
    for (int qi=0;qi<qH>>1;qi++) { memcpy( qOut, qDat0, qW*sizeof(short) );
      memcpy( qDat0, qDat1, qW*sizeof(short) ); memcpy( qDat1, qOut, qW*sizeof(short) ); qDat0 += qW; qDat1 -= qW; };
  };
  return qInp; }

int UtTransf( const tUtTransf *ARec, const TCHAR *ATgtSubDir, // -- transform selected *.raw
  // returns # processed files; the dimension shall coincide with output of _ImgDim
  CListBox *ALB, const TCHAR *APath, TCHAR *ANam, int ASelCnt, const short *ASelIdxs ) {
  TCHAR qTgtPath[256], *qTgtNam;
  if (ATgtSubDir) { *ANam = 0; // [write to subdir]
    _stprintf( qTgtPath, _T("%s\\%s"), APath, ATgtSubDir );
    VaU_RemoveBackSlash( qTgtPath ); CreateDirectory( qTgtPath, NULL );
    qTgtNam = VaU_AddBackSlash( qTgtPath );
  } else { // [overwrite]
    _tcscpy( qTgtPath, APath ); qTgtNam = qTgtPath+(ANam-APath);
  };
  int qWfrm = theA_W(), qHfrm = theA_H(), qNfrm = qWfrm*qHfrm; 
  int qWimg, qHimg; UtTransf_ImgDim( ARec, &qWimg, &qHimg ); int qNimg = qWimg*qHimg; 
  int qNmax = __max( qNfrm, qNimg );
  short *qFrm = new short[qNmax], *qBuf = new short[qNmax];
  int qC = 0;
  for (int qi=0;qi<ASelCnt;qi++) {
    // read frame
    ALB->GetText( ASelIdxs[qi], ANam );
    FILE *qF = _tfopen( APath, _T("rb") ); if (!qF) {
      _tcscat( ANam, _T(" !!! can't read") ); WriteLn( APath ); continue; };
    int qNrea = (int)fread( qFrm, sizeof(short), qNfrm, qF ); fclose( qF );
    memset( qFrm+qNrea, 0, (qNfrm-qNrea)*sizeof(short) );
    // process frame to image
    short *qImg = iTransf( ARec, qWfrm, qHfrm, qFrm, qBuf );
    // write image
    _tcscpy( qTgtNam, ANam ); qF = _tfopen( qTgtPath, _T("wb") );
    if (qF) { fwrite( qImg, sizeof(short), qNimg, qF ); fclose( qF ); }
       else { _tcscat( qTgtNam, _T(" !!! can't write") ); WriteLn( qTgtPath ); continue; };
    // continue
    qC++; }; // for i
  delete qFrm; delete qBuf;
  return qC; }