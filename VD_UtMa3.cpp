// Dataseries-Viewer MFC
// (c) LGPL v3.0; by S. Bochkarev
// file VD_UtMa3.cpp : marker's drawing service
#include "StdAfx.h" // app-wide precompiled header
#include "ViewApp.h" // the application-object
#include "VD_UtViewDlg.h" // adhoc main dialog controls' access
#include "VD_UtMa3.h"

//// histogram ////
static const int 
  gLeg = 16, // plot's Y-legend
  gLeg1 = gLeg>>1,
  gLegX = 60; // plot's X-legend

static bool iStatsDetails() { return (gFlags&16)!=0; }

void Ma3_HistogramSz( tMa3_Histogram *AHi ) { // -- assign drawing's width,height
  const int cA = gPlot+gLeg+2; SetRect( &AHi->rR, 0, 0, cA, cA );
}

typedef float tScale; // -- scale helper
tScale ScaleInit( unsigned AGamSrc, unsigned AWDst ) { return (float)(AWDst-1)/(AGamSrc-1); } // -- initialize scale
int Scale( const tScale AScale, int AV ) { return (int)((AV*AScale+0.4999f)); } // -- return scaled index

typedef struct tSCALEGRY { static const unsigned cGryLg = 16, cGryIncForLg = (1<<(cGryLg-1))-1; // -- gray scale helper
  unsigned rGryScale, rGryGam; int rGryMin; bool rGryInv;
  void InitGry() { const CPoint cGrayBW = theD_GrayBW(); // -- gray_scale
    rGryInv = cGrayBW.x>cGrayBW.y; rGryMin = rGryInv?cGrayBW.y:cGrayBW.x;
    rGryGam = abs(cGrayBW.y-cGrayBW.x); rGryScale = (255<<cGryLg)/rGryGam; }
  inline unsigned char GryScale( int AV ) { AV -= rGryMin; if (AV<=0) return 0; // -- returns 8-bit gray
    if ((unsigned)AV>=rGryGam) return 255; return ((unsigned)AV*rGryScale+cGryIncForLg)>>cGryLg; }
} tScaleGry;

struct tHIST {
  int  *rHist, rMn, rGam, rNP;
  bool Procure() {
    if (!theApp.m_bHistOk) return false;
    rMn = theApp.m_nMn;
    rGam = theApp.m_nMx-rMn+1;
    rHist = theApp.m_Hist+rMn-theA_GrayMn();
    rNP = theA_W()*theA_H();
    return true; }

  int Occurs( int AArg0, int AArg1 ) { // Args are 'abstract numeric values'
    if (AArg0>AArg1) { int qT = AArg0; AArg0 = AArg1; AArg1 = qT; };
    AArg0 -= rMn; if (AArg0>=rGam) return 0; AArg1 -= rMn; if (AArg1<0) return 0;
    if (AArg0<0) AArg0 = 0; if (AArg1>=rGam) AArg1 = rGam-1;
    int qC = AArg1-AArg0+1, qZ = 0, *cHist = rHist;
    for (int qi=0;qi<qC;qi++) qZ += cHist[AArg0+qi];
    return qC==1 ? qZ : (qZ+(qC>>1)-1)/qC; }

  float Percent( int AArg ) { // Arg is 'abstract numeric value'
    AArg -= rMn; if (AArg<0 || AArg>=rGam) return -1;
    unsigned *cHist = (unsigned*)rHist, qZ = 0;
    for (int qi=0;qi<AArg;qi++) qZ += cHist[qi];
    return 100.0f*qZ/rNP; }
  void Release() { }
};

static int iCurrGray() {
  const int *cImg = theD_Img(); if (!cImg) return 0x80000000;
  CPoint cXY = theD_XY(); return cImg[cXY.y*theA_W()+cXY.x]; }

static void iHistogramDrawHrz( tDC *ADC, tMa3_Histogram *AHi, const CRect AR ) { 
  TCHAR qS[48]; CSize qSz;
  CRect qR( AR ); qR.left += gLegX; qR.top += gLeg1; qR.right -= gLeg1; qR.bottom -= gLeg;
  CDC *dc = &ADC->rDC;
  dc->SetBkMode( OPAQUE ); dc->SetBkColor( cUt02_BkColor1 );
  ADC->SetFont( ViD_StdFontHeight() );
  tGrid1D qGrid; // grid of gray values
  int qCurrGray = iCurrGray(), qOccuCurrGray=0x80000000; float qPercCurrGray=146; // current gray
  { // histogram data
    struct tHIST qHist; if (qHist.Procure()) {
      ADC->SetPen( PS_SOLID, 1, cUt02_TxtColor1 );
      const int cRGam = qR.Width();
      tScale qSc = ScaleInit( cRGam, gOccGrayMx-gOccGrayMn+1 ),
             qScY = ScaleInit( gOccMax, qR.Height() ); // scaling helper by Y
      int qArg0 = Scale( qSc, -1 )+gOccGrayMn,
          qArg1 = Scale( qSc, 0 )+gOccGrayMn;
      tScaleGry qScGry; qScGry.InitGry();
      for (int qx=0;qx<cRGam;qx++) { 
        int qArg2 = Scale( qSc, qx+1 )+gOccGrayMn,
            qV = qHist.Occurs( qArg0+1, qArg2-1 );
        qV = Scale( qScY, qV ); if (qV) {
          int qxd = qR.left+qx; // 'x to draw'
          unsigned char cGry = qScGry.GryScale( qArg1 ); // 'gray to draw'
          ADC->SetPen( PS_SOLID, 1, RGB(cGry,cGry,cGry) );
          dc->MoveTo( qxd, qR.bottom ); 
          qV = qR.bottom-qV; if (qV<=qR.top) qV = qR.top-1;
          dc->LineTo( qxd, qV ); };
        qArg0 = qArg1; qArg1 = qArg2; }; // for x
      qPercCurrGray = qHist.Percent( qCurrGray );
      if (iStatsDetails()) qOccuCurrGray = qHist.Occurs( qCurrGray, qCurrGray );
      qHist.Release();
    }; // if Hist.Procure
  };
  int   qLinCurrGray = -1;
  { // vert grid marks of visible gray
    ADC->SetPen( PS_DOT, 1, cUt02_TxtColor2 );
    dc->SetTextColor( cUt02_TxtColor2 );
    qGrid.rMin = gOccGrayMn; qGrid.rMax = gOccGrayMx; qGrid.Init( qR.Width()/40 );
    tScale qSc = ScaleInit( gOccGrayMx-gOccGrayMn+1, qR.Width() );
    const int cN = qGrid.rNumMarks; for (int qi=0;qi<cN;qi++) {
      int qVal = qGrid.rGridFirst+qi*qGrid.rGridSnap,
          qLin = qR.left+Scale( qSc, qVal-qGrid.rMin );
      dc->MoveTo( qLin, qR.top ); dc->LineTo( qLin, qR.bottom );
      _stprintf( qS, _T("%d"), qVal ); qSz = dc->GetTextExtent( qS );
      int qx0 = qLin-(qSz.cx>>1); if (qx0<AR.left) qx0 = AR.left; else {
        int qxmx = AR.right-qSz.cx; if (qx0>qxmx) qx0 = qxmx; };
      dc->TextOut( qx0, qR.bottom, qS );
    }; // for i
    if (qCurrGray!=0x80000000)
      qLinCurrGray = qR.left+Scale( qSc, qCurrGray-gOccGrayMn );
  }; 
  { // horz grid marks of occurencies
    ADC->SetPen( PS_DOT, 1, cUt02_TxtColor1 );
    dc->SetTextColor( cUt02_TxtColor1 );
    qGrid.rMin = 0; qGrid.rMax = gOccMax; qGrid.Init( qR.Height()/20 );
    tScale qSc = ScaleInit( gOccMax, qR.Height() );
    const int cN = qGrid.rNumMarks; for (int qi=0;qi<cN;qi++) {
      int qVal = qGrid.rGridFirst+qi*qGrid.rGridSnap,
          qLin = qR.bottom-Scale( qSc, qVal-qGrid.rMin );
      dc->MoveTo( qR.left, qLin ); dc->LineTo( qR.right, qLin );
      _stprintf( qS, _T("%d"), qVal ); qSz = dc->GetTextExtent( qS );
      int qymx = qR.bottom-qSz.cy+1, qy = qLin-(qSz.cy>>1);
      dc->TextOut( qR.left-qSz.cx-gLeg1, __min(qymx,qy), qS );
    }; // for i
  };
  { // current gray
    if (qLinCurrGray>=qR.left) { ADC->SetPen( PS_SOLID, 1, cUt02_TxtColor0 );
      dc->MoveTo( qLinCurrGray, qR.top ); dc->LineTo( qLinCurrGray, qR.bottom );
      if (qCurrGray!=0x80000000) {
        dc->SetTextColor( cUt02_TxtColor1 );
        dc->SetBkColor( cUt02_BkColor0);
        _stprintf( qS, _T("%.2f%%"), qPercCurrGray );
        if (qOccuCurrGray!=0x80000000)
          _stprintf( qS+_tcslen(qS), _T(" =%d #%d"), qCurrGray, qOccuCurrGray );
        qSz = dc->GetTextExtent( qS );  
        int qX = qLinCurrGray-(qSz.cx>>1); if (qX<qR.left) qX = qR.left; else {
          int qX1 = qR.right-qSz.cx; if (qX>qX1) qX = qX1; };
        dc->TextOut( qX, qR.top-(gLeg1*2/3), qS );
      };
    };
  }; }

static void iHistogramDrawVrt( tDC *ADC, tMa3_Histogram *AHi, const CRect AR, bool ADir ) { 
  TCHAR qS[48]; CSize qSz;
  CRect qR( AR ); qR.left += gLeg; qR.top += gLegX; qR.right -= gLeg1; qR.bottom -= gLeg1;
  qR.OffsetRect( -1, 0 ); // empirical shift left by 1 pix
  CDC *dc = &ADC->rDC;
  dc->SetBkMode( OPAQUE ); dc->SetBkColor( cUt02_BkColor1 );
  LOGFONT qLF; theDlg.m_Font.GetLogFont( &qLF );
  qLF.lfHeight = ViD_StdFontHeight(); qLF.lfEscapement = -900;
  ADC->SetFont( &qLF );
  tGrid1D qGrid; // grid of gray values
  int qCurrGray = iCurrGray(), qOccuCurrGray=0x80000000; float qPercCurrGray=146; // current gray
  { // histogram data
    struct tHIST qHist; if (qHist.Procure()) {
      ADC->SetPen( PS_SOLID, 1, cUt02_TxtColor1 );
      const int cRGam = qR.Height();
      tScale qSc = ScaleInit( cRGam, gOccGrayMx-gOccGrayMn+1 ),
             qScX = ScaleInit( gOccMax, qR.Width() ); // scaling helper by X
      int qArg0 = Scale( qSc, -1 )+gOccGrayMn,
          qArg1 = Scale( qSc, 0 )+gOccGrayMn;
      tScaleGry qScGry; qScGry.InitGry();
      for (int qy=0;qy<cRGam;qy++) {
        int qArg2 = Scale( qSc, qy+1 )+gOccGrayMn,
            qV = qHist.Occurs( qArg0+1, qArg2-1 );
        qV = Scale( qScX, qV ); if (qV) {
          int qyd = ADir?qR.bottom-qy-1:qy+qR.top; // 'y to draw'
          unsigned char cGry = qScGry.GryScale( qArg1 ); // 'gray to draw'
          ADC->SetPen( PS_SOLID, 1, RGB(cGry,cGry,cGry) );
          dc->MoveTo( qR.left, qyd  ); 
          qV = qR.left+qV; if (qV>=qR.right) qV = qR.right-1;
          dc->LineTo( qV, qyd ); };
        qArg0 = qArg1; qArg1 = qArg2; }; // for y
      qPercCurrGray = qHist.Percent( qCurrGray );
      if (iStatsDetails()) qOccuCurrGray = qHist.Occurs( qCurrGray, qCurrGray );
      qHist.Release();
    }; // if Hist.Procure
  };
  int qLinCurrGray = -1;
  { // horz grid marks of visible gray
    ADC->SetPen( PS_DOT, 1, cUt02_TxtColor2 );
    dc->SetTextColor( cUt02_TxtColor2 );
    qGrid.rMin = gOccGrayMn; qGrid.rMax = gOccGrayMx; qGrid.Init( qR.Height()/40 );
    tScale qSc = ScaleInit( gOccGrayMx-gOccGrayMn+1, qR.Height() );
    const int cN = qGrid.rNumMarks; for (int qi=0;qi<cN;qi++) {
      int qVal = qGrid.rGridFirst+qi*qGrid.rGridSnap,
          qLin = Scale( qSc, qVal-qGrid.rMin );
      if (ADir) qLin = qR.bottom-qLin-1;
           else qLin = qR.top+qLin;
      dc->MoveTo( qR.left, qLin ); dc->LineTo( qR.right, qLin );
      _stprintf( qS, _T("%d"), qVal ); qSz = dc->GetTextExtent( qS );
      int qy0 = qLin-(qSz.cx>>1); if (qy0<AR.top) qy0 = AR.top; else {
        int qymx = AR.bottom-qSz.cx; if (qy0>qymx) qy0 = qymx; };
      dc->TextOut( qR.left, qy0, qS );
    }; // for i
    if (qCurrGray!=0x80000000) {
      qLinCurrGray = Scale( qSc, qCurrGray-gOccGrayMn );
      if (ADir) qLinCurrGray = qR.bottom-qLinCurrGray-1;
           else qLinCurrGray = qR.top+qLinCurrGray;
    };
  };
  { // vert grid marks of occurencies
    ADC->SetPen( PS_DOT, 1, cUt02_TxtColor1 );
    dc->SetTextColor( cUt02_TxtColor1 );
    qGrid.rMin = 0; qGrid.rMax = gOccMax; qGrid.Init( qR.Width()/20 );
    tScale qSc = ScaleInit( gOccMax, qR.Width() );
    const int cN = qGrid.rNumMarks; for (int qi=0;qi<cN;qi++) {
      int qVal = qGrid.rGridFirst+qi*qGrid.rGridSnap,
          qLin = qR.left+Scale( qSc, qVal-qGrid.rMin );
      dc->MoveTo( qLin, qR.top ); dc->LineTo( qLin, qR.bottom );
      _stprintf( qS, _T("%d"), qVal ); qSz = dc->GetTextExtent( qS );
      int qxmn = qR.left+qSz.cy-1, qx = qLin+(qSz.cy>>1);
      dc->TextOut( __max(qxmn,qx), qR.top-qSz.cx-gLeg1, qS );
    }; // for i
  };
  { // current gray
    if (qLinCurrGray>=qR.top) { ADC->SetPen( PS_SOLID, 1, cUt02_TxtColor0 );
      dc->MoveTo( qR.left, qLinCurrGray ); dc->LineTo( qR.right, qLinCurrGray );
      if (qCurrGray!=0x80000000) {
        dc->SetTextColor( cUt02_TxtColor1 );
        dc->SetBkColor( cUt02_BkColor0 );
        _stprintf( qS, _T("%.2f%%"), qPercCurrGray );
        if (qOccuCurrGray!=0x80000000)
          _stprintf( qS+_tcslen(qS), _T(" =%d #%d"), qCurrGray, qOccuCurrGray );
        qSz = dc->GetTextExtent( qS );
        int qY = qLinCurrGray-(qSz.cx>>1); if (qY<qR.top) qY = qR.top; else {
          int qY1 = qR.bottom-qSz.cx; if (qY>qY1) qY = qY1; };
        dc->TextOut( qR.right+gLeg1*2/3, qY, qS );
      };
    };
  }; }

void Ma3_HistogramDraw( tDC *ADC, tMa3_Histogram *AHi ) { // -- draw histogram
  CRect cRcli = theD_Rcli(), qR(0,0,cRcli.Width(),cRcli.Height());
  RECT *cRbase = &AHi->rR;
  bool cHrz = !cRbase->top==!cRbase->left;
  if (cHrz) { qR.top = cRbase->top; qR.bottom = cRbase->bottom; }
       else { qR.left = cRbase->left; qR.right = cRbase->right; };
  ADC->SolidRect( qR, cUt02_BkColor1 ); // whole rect
  if (cHrz) iHistogramDrawHrz( ADC, AHi, qR );
       else iHistogramDrawVrt( ADC, AHi, qR, !cRbase->top ); }
