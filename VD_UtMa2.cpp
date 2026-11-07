// Dataseries-Viewer MFC
// (c) LGPL v3.0; by S. Bochkarev
// file VD_UtMa2.cpp : marker's drawing service
#include "StdAfx.h" // app-wide precompiled header
#include "ViewApp.h" // the application-object
#include "VD_UtViewDlg.h" // adhoc main dialog controls' access
#include "VD_UtMa2.h"

//// hrz,vrt-profiles ////
static const int 
  gLeg = 16; // plot's Y-legend

void Ma2_ProfileSz( tMa2_Profile *AP ) { // -- assign drawing's width,height
  const int cA = gPlot+gLeg+2; SetRect( &AP->rR, 0, 0, cA, cA );
  // grid for marks of gray
  CPoint cGrayBW = theD_GrayBW();
  tGrid1D *qGrid = &AP->rGrayGrid;
  qGrid->rMin = cGrayBW.x; qGrid->rMax = cGrayBW.y; qGrid->Init( cMa2_gN-1 );
  const int cGamut = qGrid->rMax-qGrid->rMin, cN = qGrid->rNumMarks;
  for (int qi=0;qi<cN;qi++) {
    int qVal = AP->rGrayVal[qi] = qGrid->rGridFirst+qi*qGrid->rGridSnap;
    AP->rGrayLin[qi] = ((qVal-qGrid->rMin)*gPlot+(cGamut>>1)-1)/cGamut; }; }
    
struct tSCALE2 { static const unsigned cLg = 16, cIncForLg = (1<<(cLg-1))-1; // -- scaling helper
  unsigned rSpaScale, rGryScale, rGryGam; int rGryMin; bool rGryInv;
  void InitSpa( unsigned AWSrc, unsigned AWDst ) { // -- initialize spatial_scale
    rSpaScale = AWSrc && AWDst ? (AWDst<<cLg)/AWSrc : 1; } 
  inline unsigned SpaScale( unsigned AI ) { return ((AI<<1)+1)*rSpaScale>>(cLg+1); } // -- spatial index
  void InitGry() { const CPoint cGrayBW = theD_GrayBW(); // -- gray_scale
    rGryInv = cGrayBW.x>cGrayBW.y; rGryMin = rGryInv?cGrayBW.y:cGrayBW.x;
    rGryGam = abs(cGrayBW.y-cGrayBW.x); if (!rGryGam) rGryGam = 1; rGryScale = (255<<cLg)/rGryGam; }
  inline unsigned char GryScale( int AV ) { AV -= rGryMin; if (AV<=0) return 0; // -- returns 8-bit gray
    if ((unsigned)AV>=rGryGam) return 255; return ((unsigned)AV*rGryScale+cIncForLg)>>cLg; }
};

void Ma2_ProfileDrawHrz( tDC *ADC, tMa2_Profile *AP ) { // -- draw hrz profile
  CRect qR = theD_Rd(); qR.top = AP->rR.top; qR.bottom = AP->rR.bottom;
  ADC->SolidRect( qR, cUt02_BkColor1 ); // whole rect
  // plot's rect
  qR.top +=1; qR.bottom -= gLeg-1;
  CDC *dc = &ADC->rDC;
  const CRect cRf = theD_Rf();
  const int *cImgRow = theD_Img()+theD_Y()*theA_W()+cRf.left;
  struct tSCALE2 qSc2; // scaling helper
  { // image_data's row
    qSc2.InitSpa( qR.Width(), cRf.Width() ); qSc2.InitGry();
    for (int qx=qR.left;qx<qR.right;qx++) {
      int qGray = qSc2.GryScale( cImgRow[qSc2.SpaScale(qx-qR.left)] );
      if (qGray>0) { int qPlot = (qGray*gPlot+127)>>8;
        if (qSc2.rGryInv) qGray = 256-qGray; ADC->SetPen( PS_SOLID, 1, RGB(qGray,qGray,qGray) ); 
        dc->MoveTo( qx, qR.bottom ); dc->LineTo( qx, qR.bottom-qPlot );
      }; }; // for x
  }; 
  { // horz grid marks of gray
    dc->SetBkMode( OPAQUE ); dc->SetBkColor( cUt02_BkColor0 );
    ADC->SetPen( PS_DOT, 1, cUt02_TxtColor1 );
    const int cN = AP->rGrayGrid.rNumMarks; for (int qi=0;qi<cN;qi++) {
      int qy = qR.bottom-AP->rGrayLin[qi];
      dc->MoveTo( qR.left, qy ); dc->LineTo( qR.right, qy ); }; // for i
  };
  { // vert grid marks of visible columns
    dc->SetBkMode( TRANSPARENT );
    dc->SetTextColor( cUt02_TxtColor2 );
    ADC->SetPen( PS_DOT, 1, cUt02_TxtColor2 );
    const int cNumMarks = qR.Width()/40; if (cNumMarks<2) return;
    ADC->SetFont( ViD_StdFontHeight() );
    tGrid1D qXGrid; qXGrid.rMin = cRf.left; qXGrid.rMax = cRf.right;
    qXGrid.Init( cNumMarks );
    qSc2.InitSpa( cRf.Width(), qR.Width() );
    const int cM = qXGrid.rNumMarks; for (int qx=0;qx<cM;qx++) {
      int qVal = qXGrid.rGridFirst+qx*qXGrid.rGridSnap,
          qLin = qR.left+qSc2.SpaScale( qVal-cRf.left );
      dc->MoveTo( qLin, qR.bottom ); dc->LineTo( qLin, qR.top );
      TCHAR qS[48]; _stprintf( qS, _T("%d"), qVal ); CSize qSz = dc->GetTextExtent( qS );
      int qx0 = qLin-(qSz.cx>>1); if (qx0<0) qx0 = 0; else {
        int qxmx = qR.right-qSz.cx; if (qx0>qxmx) qx0 = qxmx; };
      dc->TextOut( qx0, qR.bottom, qS );
    }; // for m 
  }; } 

void Ma2_ProfileDrawVrt( tDC *ADC, tMa2_Profile *AP ) { // -- draw vrt profile
  CRect qR = theD_Rd(); qR.left = AP->rR.left; qR.right = AP->rR.right;
  ADC->SolidRect( qR, cUt02_BkColor2 ); // whole rect
  // plot's rect
  qR.left += gLeg+1; qR.right--;
  CDC *dc = &ADC->rDC;
  const CRect cRf = theD_Rf(); const int cW = theA_W();
  const int  *cImgCol = theD_Img()+theD_X()+cRf.top*cW;
  struct tSCALE2 qSc2; // scaling helper
  { // image_data's row
    qSc2.InitSpa( qR.Height(), cRf.Height() ); qSc2.InitGry();
    // ADC->SetPen( PS_SOLID, 1, 0 ); 
    for (int qy=qR.top;qy<qR.bottom;qy++) {
      int qGray = qSc2.GryScale( cImgCol[qSc2.SpaScale(qy-qR.top)*cW] );
      if (qGray>0) { int qPlot = (qGray*gPlot+127)>>8;
        if (qSc2.rGryInv) qGray = 256-qGray; ADC->SetPen( PS_SOLID, 1, RGB(qGray,qGray,qGray) ); 
        dc->MoveTo( qR.left, qy ); dc->LineTo( qR.left+qPlot, qy );
    }; }; // for y
  }; 
  { // vert grid marks of gray
    dc->SetBkMode( OPAQUE ); dc->SetBkColor( cUt02_BkColor0 );
    ADC->SetPen( PS_DOT, 1, cUt02_TxtColor1 );
    const int cN = AP->rGrayGrid.rNumMarks; for (int qi=0;qi<cN;qi++) {
      int qx = qR.left+AP->rGrayLin[qi];
      dc->MoveTo( qx, qR.top ); dc->LineTo( qx, qR.bottom ); }; // for i
  }; 
  { // horz grid marks of visible rows
    dc->SetBkMode( TRANSPARENT );
    dc->SetTextColor( cUt02_TxtColor2 );
    ADC->SetPen( PS_DOT, 1, cUt02_TxtColor2 );
    const int cNumMarks = qR.Height()/40; if (cNumMarks<2) return;
    LOGFONT qLF; theDlg.m_Font.GetLogFont( &qLF );
    qLF.lfHeight = ViD_StdFontHeight(); qLF.lfWeight = 800; qLF.lfEscapement = -900;
    ADC->SetFont( &qLF );
    tGrid1D qYGrid; qYGrid.rMin = cRf.top; qYGrid.rMax = cRf.bottom;
    qYGrid.Init( cNumMarks );
    qSc2.InitSpa( cRf.Height(), qR.Height() );
    const int cM = qYGrid.rNumMarks; for (int qy=0;qy<cM;qy++) {
      int qVal = qYGrid.rGridFirst+qy*qYGrid.rGridSnap,
          qLin = qR.top+qSc2.SpaScale( qVal-cRf.top );
      dc->MoveTo( qR.left, qLin ); dc->LineTo( qR.right, qLin );
      TCHAR qS[48]; _stprintf( qS, _T("%d"), qVal ); CSize qSz = dc->GetTextExtent( qS );
      int qy0 = qLin-(qSz.cx>>1); if (qy0<0) qy0 = 0; else {
        int qymx = qR.bottom-qSz.cx; if (qy0>qymx) qy0 = qymx; };
      dc->TextOut( qR.left-1, qy0, qS );
    }; // for m
  }; } 

void Ma2_ProfileDrawBox( tDC *ADC, tMa2_Profile *AP ) { // -- draw profiles box
  CDC         *dc = &ADC->rDC;
  dc->SetBkColor( cUt02_BkColor2 );
  dc->SetTextColor( cUt02_TxtColor2 );
  LOGFONT qLF; theDlg.m_Font.GetLogFont( &qLF );
  qLF.lfHeight = ViD_StdFontHeight(); qLF.lfWeight = 1600; qLF.lfItalic = 1;
  ADC->SetFont( &qLF );
  dc->SetTextColor( cUt02_TxtColor1 );
  dc->SetBkColor( cUt02_BkColor0 );
  dc->SetBkMode( OPAQUE );
  CRect qR = AP->rR; 
  TCHAR qS[48]; CSize qSz;
  const int cN = AP->rGrayGrid.rNumMarks; for (int qi=0;qi<cN;qi++) {
    _stprintf( qS, _T("%d"), AP->rGrayVal[qi] );
    qSz = dc->GetTextExtent( qS );
    int qX = gLeg+qR.left+AP->rGrayLin[qi]-(qSz.cx>>1);
    if (qX<0) qX = 0; else { int qXmx = qR.right-qSz.cx; if (qX>qXmx) qX = qXmx;}
    int qY = qR.bottom-AP->rGrayLin[qi]-gLeg-(qSz.cy>>1); if (qY<qR.top) qY = qR.top;
    dc->TextOut( qX, qY, qS );
  }; } // for i

