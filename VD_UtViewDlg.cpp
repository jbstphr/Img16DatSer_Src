// Dataseries-Viewer MFC
// (c) LGPL v3.0; by S. Bochkarev
// file VD_UtViewDlg.cpp : adhoc main dialog controls' access
#include "StdAfx.h"
#include "ViewApp.h" // main dialog
#include "VD_UtMnuPUp.h" // CViewDlg local pop-up menu
#include "VD_UtViewDlg.h"

//// main dialog's features ////

CRect theA_Clip( const CRect AROI ) { // -- clip a frame's rectangle
  CRect qROI; qROI.IntersectRect( AROI, CRect(CPoint(0,0),theA_WH()) ); return qROI; }

int theD_Marker() { // -- marker's type index
  return (int)theDlg.SendDlgItemMessage( IDC_COMBO1, CB_GETCURSEL ); }

int theD_Gamut() { // -- gamut's type index
  return (int)theDlg.SendDlgItemMessage( IDC_COMBO2, CB_GETCURSEL ); }

CPoint theD_XYa() { // -- [marker's area]
  return CPoint( theDlg.GetDlgItemInt(IDC_EDIT6), theDlg.GetDlgItemInt(IDC_EDIT7) ); }

CPoint theD_XYo() { // -- the origin of symmetric [marker's area]
  CPoint cArea = theD_XYa(), cXY = theD_XY(); // marker's area
  return CPoint( cXY.x-(cArea.x>>1), cXY.y-(cArea.y>>1) ); }

CRect theD_ROI() { // -- UI's [marker ROI]
  CRect qROI(CPoint(0,0),theD_XYa()); switch (theD_Marker()) { // marker
    case  1: qROI.OffsetRect( theD_XYo() ); break; // [pixel inspector]
    case  4: qROI.OffsetRect( theD_XY() ); break; // [rectangle]
    default: qROI.SetRectEmpty(); 
  }; return qROI; }

CRect theD_Rcli() { // -- client's rectangle
  CRect qRX, qRY; theDlg.GetSBRects( &qRX, &qRY );
  return theDlg.m_Z>1 ? CRect( qRX.left, qRY.top, qRY.left, qRX.top )
                      : CRect( qRX.left, qRY.top, qRY.right, qRX.bottom ); };

CRect theD_Rd() { // -- current rectangle in buffer DC
  CRect qRd = theDlg.m_Rd; qRd.OffsetRect( -theD_Rcli().TopLeft() ); return qRd; }
CRect theD_Rf() { // -- current rectangle in the frame
  return theA_Clip(theDlg.m_Rf); }

int *theD_Img() { return theDlg.m_BMPBuf.mImgBuf; } // -- returns image-buffer

CPoint theD_GrayBW() { // -- explicit gray-scale's black,white
  return CPoint( theDlg.m_BMPBuf.mBlack, theDlg.m_BMPBuf.mWhite );}

void theD_Repaint( bool AEraseBkg ) { // -- repaint
  theDlg.InvalidateRect( theD_Rcli(), AEraseBkg ); }

void theD_OnPresentationUpdate() { // -- called by ~BPM-editor
  theDlg.OnPresentationUpdate( false ); }

void ViD_Hist16BaseBuild( // builds histogram & calcs basic stats
  const short *ABuf, bool AMed, bool AAve ) {
  int *qHist = theApp.m_Hist; memset( qHist, 0, (1<<16)*sizeof(int) );
  const int cGrayMn = theA_GrayMn(), cNP = theA_W()*theA_H();
  if (cGrayMn<0) { // signed 16-bits
    for (int qi=0;qi<cNP;qi++) qHist[ABuf[qi]-cGraySigMn]++; }
  else { // unsigned 16-bits
    unsigned short *qBuu = (unsigned short*)ABuf;
    for (int qi=0;qi<cNP;qi++) qHist[qBuu[qi]-cGrayUnsMn]++; };
  int qMn = 0; while (!qHist[qMn]) qMn++; theApp.m_nMn = qMn+cGrayMn;
  int qMx = (1<<16)-1; while (!qHist[qMx]) qMx--; theApp.m_nMx = qMx+cGrayMn;
  if (!AMed) return; // if median requested
  tLL qZ = 0; int qNP = 0, qMed = qMn; for (;qMed<=qMx;qMed++) {
    int qV = qHist[qMed]; qZ += (tLL)(qMed+cGrayMn)*qV; qNP += qV; if (qNP>cNP>>1) break; };
  theApp.m_nMed = qMed+cGrayMn;  
  if (!AAve) return; // if average requested
  for (;qMed<=qMx;qMed++) qZ += (tLL)(qMed+cGrayMn)*qHist[qMed];
  theApp.m_fAve = (float)qZ/cNP; }

//// called mainly in the main dialog ////
static inline int ithe_A_HistN( int ACut ) { return (int)((tLL)theA_W()*theA_H()*ACut/1000); }
int ViD_HistCutBlack( int ACut ) { if (!theApp.m_bHistOk) return 0; if (!ACut) return theApp.m_nMn; // -- to assign ~IDC_EDIT11
  int qN = ithe_A_HistN( ACut ), qZ = 0, qi = 0, *qHist = theApp.m_Hist;
  do qZ += qHist[qi++]; while (qZ<=qN); return qi+theA_GrayMn(); }
int ViD_HistCutWhite( int ACut ) { if (!theApp.m_bHistOk) return 4095; if (!ACut) return theApp.m_nMx; // -- to assign ~IDC_EDIT9
  int qN = ithe_A_HistN( ACut ), qZ = 0, qi = (1<<16)-1, *qHist = theApp.m_Hist;
  do qZ += qHist[qi--]; while (qZ<=qN); return qi+theA_GrayMn(); }

//// adjustments in DSeg (mostly for code in _Ut_Ma<rker>.cpp) ////
int 
  gFlags = 1,
  gPlot = 99,
  gBkg = 123,
  gBmpCpyMxDim = 1999,
  gOccMax = 5000,
  gOccGrayMn = 0,
  gOccGrayMx = 1<<14;

TCHAR gWildCardDef[8] = _T("*.");

void ViD_IniProfile( bool AWrite ) { // -- read/write initialization profile
  if (AWrite) { TCHAR qS[256];
    _stprintf( qS, _T("%X; hex #0:adim #1:drktxt #2:flbfnt #3:flbsrt #4:sttdet #5:WinEdp"), gFlags );
    theApp.VDI_SetStr( _T("ViD_Flg"), qS );
    _stprintf( qS, _T("%d %d %d; plot bkg bmpcmx"), gPlot, gBkg, gBmpCpyMxDim );
    theApp.VDI_SetStr( _T("ViD_Widgets"), qS );
    _stprintf( qS, _T("%d %d %d; o_mx g_mn g_mx"), gOccMax, gOccGrayMn, gOccGrayMx );
    theApp.VDI_SetStr( _T("ViD_Occ"), qS );
    theApp.VDI_SetStr( _T("ViD_FilExtDef"), gFilExtDef() );
    return; };
  if (theApp.VDI_Hexs( _T("ViD_Flg"), NULL )==1) {
    gFlags = theApp.VDI_Ints()[0]; };
  if (theApp.VDI_Ints( _T("ViD_Widgets"), NULL )==3) { int *qI = theApp.VDI_Ints();
    gPlot = qI[0]; gBkg = qI[1]; gBmpCpyMxDim = qI[2]; };
  if (theApp.VDI_Ints( _T("ViD_Occ"), NULL )==3) { int *qI = theApp.VDI_Ints();
    gOccMax = qI[0]; gOccGrayMn = qI[1]; gOccGrayMx = qI[2];  };
  _tcsncpy( gFilExtDef(), theApp.VDI_Str(_T("ViD_FilExtDef"),cFilExts[0]), mcASz(gWildCardDef)-3 );
  gWildCardDef[mcASz(gWildCardDef)-1] = 0; }