// Dataseries-Viewer MFC
// (c) LGPL v3.0; by S. Bochkarev
// file VD_UtMa3.cpp : marker's drawing service
#include "StdAfx.h" // app-wide precompiled header
#include "math.h"
#include "ViewApp.h" // the application-object
#include "VD_UtViewDlg.h" // adhoc main dialog controls' access 
#include "VD_UtMnuPUp.h" // CViewDlg local pop-up menu
#include "VD_UtMa4.h"

//// rectangle marker ////

static const int gW = 50, // cell'x width
                 gH = 16; // cell's height

static bool iStatsDetails() { return (gFlags&16)!=0; }

void Ma4_RectSttSz( tMa4_RectStt *ARS ) { // -- assign drawing's width,height
  SetRect( &ARS->rR, 0, 0, 6+gW*2, 6+gH*(iStatsDetails()?11:7) );
}

POINT xMa4_MarkerArea = {10,10}; // -- recent rectangle ROI

static CDC *ixTODC; static CPoint ixTOP0; static int ixTOY;
static void iTO( int AXI, const TCHAR *AStr ) {
  ixTODC->SetTextColor( AXI?cUt02_TxtColor1:cUt02_TxtColor2 );
  int qX = AXI ? ixTOP0.x+gW*2-ixTODC->GetTextExtent( AStr ).cx : ixTOP0.x;
  ixTODC->TextOut( qX, ixTOP0.y+ixTOY*gH, AStr ); if (AXI) ixTOY++;
  TCHAR qS[48]; if (AXI) {
    if (AStr[0]==_T('=') || AStr[0]==_T('~')) AStr++;
    TCHAR *qCh = qS; *qCh++ = _T(' '); _tcscpy( qCh, AStr );
    qCh += _tcslen(qCh); while (qCh>qS && qCh[-1]==_T(' ')) *qCh--;
    *qCh = 0; AStr = qS; }; // if XI
  UtMnu_ClipboardTxt( AStr ); if (AXI>0) UtMnu_ClipboardTxt( _T(";") ); }

void Ma4_RectSttDraw( tDC *ADC, tMa4_RectStt *ARS, const CRect AROI ) { // -- draw rectangle's statistics
  CRect qR = theD_Rd(); qR.top = ARS->rR.top; qR.bottom = ARS->rR.bottom;
  ADC->SolidRect( ARS->rR, cUt02_BkColor0 ); // whole rect
  // non-symm and already-clipped ROI
  if (AROI.IsRectEmpty()) return;
  xMa4_MarkerArea.x = AROI.Width(); xMa4_MarkerArea.y = AROI.Height();
  int *qImg = theD_Img(); if (!qImg) return; // image-buffer
  // get the stats
  const CPoint cAWH = theA_WH(); // frame's dimension
  const int cN = xMa4_MarkerArea.x*xMa4_MarkerArea.y;
  tLL qZZ = 0; int qMn = 1<<16, qMx =-(1<<15); CPoint qMnP, qMxP;
  // don't use ~theA_GrayMn as here could be pixels' differentials
  const int cMn = -(1<<16), cGam = 1<<17;
  int *qHis = new int[cGam]; memset( qHis, 0, cGam*sizeof(int) );
  for (int qy=AROI.top;qy<AROI.bottom;qy++) for (int qx=AROI.left;qx<AROI.right;qx++) {
    int qV = qImg[cAWH.x*qy+qx]; qZZ += qV;
    if (qMn>qV) { qMn = qV; qMnP.SetPoint( qx, qy ); };
    if (qMx<qV) { qMx = qV; qMxP.SetPoint( qx, qy ); };
    qV -= cMn; qHis[mcRng(qV,0,cGam-1)]++; };
  float qA = (float)qZZ/cN; // average
  int qHm = 0; qZZ = 0; do qZZ += qHis[qHm++]; while (qZZ<(cN>>1)); qHm += cMn; // median
  int qHw, qHb; if (iStatsDetails()) { const int cn = cN/20+1; // black-point, white-point 
    qHb = 0; qZZ = 0; do qZZ += qHis[qHb++]; while (qZZ<cn); qHb += cMn; // black-point
    qHw = cGam-1; qZZ = 0; do qZZ += qHis[qHw--]; while (qZZ<cn); qHw += cMn; }; // white-point
  delete qHis;
  int qa = (int)qA; qZZ = 0; // std deviation
  for (int qy=AROI.top;qy<AROI.bottom;qy++) for (int qx=AROI.left;qx<AROI.right;qx++) {
    int qV = qImg[cAWH.x*qy+qx]-qa; qZZ += qV*qV; };
  float qD = (float)sqrt( (double)qZZ/cN );
  // draw
  CDC   *dc = &ADC->rDC;
  dc->SetBkMode( TRANSPARENT );
  ADC->SetFont( gH );
  ixTODC = dc; ixTOP0.SetPoint( 3+ARS->rR.left, 3+ARS->rR.top ); ixTOY = 0;
  UtMnu_ClipboardTxt( NULL ); // empty the clipboard_text
  TCHAR  qS[256]; const int cSttDetX = iStatsDetails()?-1:1;
  _stprintf( qS, _T("%04d:%04d    "), AROI.left, AROI.top ); iTO( -1, qS );
  _stprintf( qS, _T("%04d:%04d"), AROI.right, AROI.bottom ); iTO( 1, qS ); 
  iTO( 0, _T("max") ); _stprintf( qS, _T("%05d"), qMx ); iTO( cSttDetX, qS );
  if (cSttDetX<0) {
    _stprintf( qS, _T("%d:%d"), qMxP.x, qMxP.y ); iTO( 1, qS );
    iTO( 0, _T("5%brt") ); _stprintf( qS, _T("%05d"), qHw ); iTO( 1, qS ); };
  iTO( 0, _T("med") ); _stprintf( qS, _T("=%05d"), qHm ); iTO( 1, qS );
  iTO( 0, _T("ave") ); _stprintf( qS, _T("~%.2f"), qA ); iTO( 1, qS );
  if (cSttDetX<0) {
    iTO( 0, _T("5%drk") ); _stprintf( qS, _T("%05d"), qHb ); iTO( 1, qS ); };
  iTO( 0, _T("min") ); _stprintf( qS, _T("%05d"), qMn ); iTO( cSttDetX, qS );
  if (cSttDetX<0) {
    _stprintf( qS, _T("%d:%d"), qMnP.x, qMnP.y ); iTO( 1, qS ); };
  iTO( 0, _T("stdev") ); _stprintf( qS, _T("%.2f"), qD ); iTO( 1, qS );
}
