// Dataseries-Viewer MFC
// (c) LGPL v3.0; by S. Bochkarev
// file VD_UtMa1.cpp : marker's drawing service
#include "StdAfx.h" // app-wide precompiled header
#include "ViewApp.h" // the application-object
#include "VD_UtViewDlg.h" // adhoc main dialog controls' access
#include "VD_NMDlg1.h" // dialog [BPM]
#include "VD_UtMa1.h"

//// pixel-inspector ////
void Ma1_PixelInspectorSz( tMa1_PixelInspector *API ) { // -- assign drawing's width,height
  const CPoint cArea = theD_XYa(); // marker's area
  API->rR.left = API->rR.top = 0;
  const int cH = API->rH = cArea.x<7?15:(theApp.m_bUnderWinE?12:13), // cell's height
            cW = API->rW = theApp.m_bUnderWinE?cH*3+8:cH*3+3; // cell's width
  API->rR.right = (1+cArea.x)*cW+2;
  API->rR.bottom = ((API->rHex?1:2)+cArea.y)*cH+2; } 

void Ma1_PixelInspectorDraw( tDC *ADC, tMa1_PixelInspector *API, CRect AROI ) { // -- draw pixel inspector
  ADC->SolidRect( API->rR, cUt02_BkColor0 ); // whole rect
  AROI = theA_Clip( AROI );
  TCHAR        qS[256];
  const CPoint cAWH = theA_WH(), // frame's dimension
               cLocation = theD_XY(), // marker's coordinates
               cArea = theD_XYa(), // marker's area
               cXYo = theD_XYo(); // current symmetric XYo (might be out of AWH)
  const bool   cHex = API->rHex;
  const int    cY0 = cHex?0:1;
  CDC         *dc = &ADC->rDC;
  dc->SetBkMode( TRANSPARENT );
  LOGFONT qLF; theDlg.m_Font.GetLogFont( &qLF );
  const int cW = API->rW, cH = API->rH; qLF.lfWeight = 400; qLF.lfHeight = cH;
  ADC->SetFont( &qLF );
  const TCHAR *cCellFmt = _T("%6d"), *cCellFmtHex = _T("%6X");
  { // columns
    for (int qx=AROI.left;qx<AROI.right;qx++) { _stprintf( qS, cCellFmt, qx );
    dc->SetTextColor( qx==cLocation.x?cUt02_TxtColor0:cUt02_TxtColor2 );
    dc->TextOut( API->rR.left+1+(1+qx-cXYo.x)*cW, API->rR.top+1+cY0*cH, qS ); };
  };
  { // rows  
    for (int qy=AROI.top;qy<AROI.bottom;qy++) { _stprintf( qS, cCellFmt, qy );
    dc->SetTextColor( qy==cLocation.y?cUt02_TxtColor0:cUt02_TxtColor2 );
    dc->TextOut( API->rR.left+1, API->rR.top+1+(1+cY0+qy-cXYo.y)*cH, qS ); };
  };
  int qN = 0, qA = 0, qMn = 0x7FFFFFFF, qMx = -0x7FFFFFFF;
  { // pixels
    int *qImg = theD_Img(); if (qImg) for (int qy=AROI.top;qy<AROI.bottom;qy++)
    for (int qx=AROI.left;qx<AROI.right;qx++) { int qV = qImg[cAWH.x*qy+qx];
      if (cHex) {
        if (qV<0) { TCHAR s[16]; _tcscpy( qS, s+_stprintf(s,_T("    -%X"),-qV)-6 ); }
             else _stprintf( qS, cCellFmtHex, qV );
      } else { _stprintf( qS, cCellFmt, qV );
        if (qMn>qV) qMn = qV; if (qMx<qV) qMx = qV; qA += qV; qN++; };
      COLORREF qBkColor = BPM_PixelBkColor( CPoint(qx,qy) );
      if (qBkColor) { dc->SetBkMode( OPAQUE ); dc->SetBkColor( qBkColor ); }
               else dc->SetBkMode( TRANSPARENT );
      dc->SetTextColor( qx==cLocation.x && qy==cLocation.y?cUt02_TxtColor0:cUt02_TxtColor1 );
      dc->TextOut( API->rR.left+1+(1+qx-cXYo.x)*cW, API->rR.top+1+(1+cY0+qy-cXYo.y)*cH, qS ); }; // for x,y
  }; 
  if (qN) { // stats
    qA = (qA+(qN>>1)-1)/qN;
    _stprintf( qS, _T("%d..%d ~%d"), qMn, qMx, qA );
    dc->SetBkMode( TRANSPARENT ); dc->SetTextColor( cUt02_TxtColor0 );
    dc->TextOut( API->rR.left+10, API->rR.top+1, qS );
  }; }
