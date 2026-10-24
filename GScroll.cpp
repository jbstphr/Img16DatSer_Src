// Dataseries-Viewer MFC
// (c) LGPL v3.0; by S. Bochkarev
// Scrolling service (implementation)
#include "StdAfx.h"
#include "GScroll.h"

static void iScroll1DInit( float AScale, int AWc, int AZoom, int AWf, float AWK, int *AMgns ) {
  // initial position of 1D-scroll
  memset( AMgns, 0, 4*sizeof(int) ); // out margins: Dlft,Drgt,Flft,Frgt
  int qWd = (int)(AWf*AScale*AZoom+0.4999f), // virtual width on display
      qGc = qWd-AWc; // virtual gap for display's client area
  if (qGc>0) { // a part of frame within client
    int qGf = (int)(qGc/(AScale*AZoom)+0.4999f), // frame's gap
        qGf0 = AMgns[2] = (int)(qGf*AWK); AMgns[3] = qGf-qGf0;
  } else { // whole frame within client
    int qGd = -qGc, // display's gap
        qGd0 = AMgns[0] = qGd>>1; AMgns[1] = qGd-qGd0;
  }; }

void GSCR_CalcScrollRects(
  const CRect ARc,  // display's client rectangle
  int AZoom,        // >0
  int AWf, int AHf, // frame's dimension
  float AWK, float AHK, // relative scrollbars' positions; 0.5=center
  CRect *ARd,       // dst display's rectangle to stretch
  CRect *ARf ) {    // src frame's rectangle to stretch
  int   qWc = ARc.Width(), qHc = ARc.Height();
  float qScale = (float)qWc/AWf, qScale1 = (float)qHc/AHf;
  if (qScale>qScale1) qScale = qScale1;
  int qMgns[4];
  iScroll1DInit( qScale, qWc, AZoom, AWf, AWK, qMgns );
  ARd->left = qMgns[0]; ARd->right = qWc-qMgns[1];
  ARf->left = qMgns[2]; ARf->right = AWf-qMgns[3];
  iScroll1DInit( qScale, qHc, AZoom, AHf, AHK, qMgns );
  ARd->top = qMgns[0]; ARd->bottom = qHc-qMgns[1];
  ARf->top = qMgns[2]; ARf->bottom = AHf-qMgns[3];
  ARd->OffsetRect( ARc.TopLeft() );
  #ifdef _DEBUG1 // self-test
    CPoint cCf( ARf->right-1, ARf->bottom-1 ), qCg;
    GSCR_GetCg( *ARd, *ARf, cCf, &qCg );
    if (qCg.x!=ARd->right-1 || qCg.y!=ARd->bottom-1) {
      TCHAR qS[80]; _stprintf( qS, _T("ERR GSCR_CalcScrollRects Rd{%d:%d,%d:%d} Rf{%d:%d,%d:%d}"),
        ARd->left, ARd->top, ARd->right, ARd->bottom, ARf->left, ARf->top, ARf->right, ARf->bottom );
      WriteLn( qS );
    };
  #endif
}    

// Destination position for specific source position; inp #0:Lsrc #1:Wsrc #2:Ldst #3:Wdst
// 05=true:center_display_pix, =false:1st_display_pix
inline static int iXdst( const int *ALW, int AXsrc, bool A05=true ) { 
  int qWsrc = ALW[1]; if (!qWsrc) return ALW[2]+ALW[3];
  int qXdst = A05 ? (((AXsrc-ALW[0])<<1)+1)*ALW[3]/(qWsrc<<1) : (AXsrc-ALW[0])*ALW[3]/qWsrc;
  return ALW[2]+qXdst; }

// frame's coordinate Cf by GDI's coordinate Cg
int GSCR_GetCfX( const CRect ARd, const CRect ARf, const int ACgX ) {
  int qLWdf[4] = {ARd.left,ARd.Width(),ARf.left,ARf.Width()}; return iXdst( qLWdf, ACgX ); }
int GSCR_GetCfY( const CRect ARd, const CRect ARf, const int ACgY ) {
  int qLWdf[4] = {ARd.top,ARd.Height(),ARf.top,ARf.Height()}; return iXdst( qLWdf, ACgY ); }
bool GSCR_GetCf( const CRect ARd, const CRect ARf, const CPoint ACg, CPoint *ACf ) {
  CPoint qCf( GSCR_GetCfX(ARd,ARf,ACg.x), GSCR_GetCfY(ARd,ARf,ACg.y) );
  bool qInRf = ARf.PtInRect( qCf )!=FALSE;
  if (qCf.x<0) qCf.x = 0; if (qCf.y<0) qCf.y = 0; // general sentinel when caller assumed unsigned coordinates
  *ACf = qCf; return qInRf; }

// GDI's coordinate Cg by frame's coordinate Cf; 05=true:center_display_pix, =false:1st_display_pix
bool GSCR_GetCg( const CRect ARd, const CRect ARf, const CPoint ACf, CPoint *ACg, bool A05 ) {
  int qLWfd[4]; CPoint qCg;
  qLWfd[0] = ARf.left; qLWfd[1] = ARf.Width(); qLWfd[2] = ARd.left; qLWfd[3] = ARd.Width();
  qCg.x = iXdst( qLWfd, ACf.x, A05 ); 
  qLWfd[0] = ARf.top; qLWfd[1] = ARf.Height(); qLWfd[2] = ARd.top; qLWfd[3] = ARd.Height();
  qCg.y = iXdst( qLWfd, ACf.y, A05 ); 
  bool qInRd = ARd.PtInRect( qCg )==TRUE;
  *ACg = qCg; return qInRd; }

CPoint GSCR_GetScrollRng( // scrolbabrs' range
  int AWf, int AHf,  // frame's dimension
  const CRect ARf ) { // calculated src frame's rectangle to stretch
  return CPoint( AWf-ARf.Width(), AHf-ARf.Height() ); }

inline static float iScrollK( int AScrollRng, int AScrollPos ) { 
  if (AScrollRng<2) return 0.5f; float qK = (float)AScrollPos/AScrollRng; return qK<=1.0f ? qK : 0.5f; }

void GSCR_GetScrollK( // returns relative scrollbars' positions
  int AWf, int AHf,  // frame's dimension
  const CRect ARf,   // calculated src frame's rectangle to stretch
  float *AWHK ) {    // scrollK
  CPoint qScrollRng = GSCR_GetScrollRng( AWf, AHf, ARf );
  AWHK[0] = iScrollK( qScrollRng.x, ARf.left );
  AWHK[1] = iScrollK( qScrollRng.y, ARf.top ); }

void CSCR_SentinelRf( int AWf, int AHf, CRect *ARf ) {
  CRect qR = *ARf; int qV;
  qV = qR.left;         if (qV<0) { qR.left = 0;     qR.right -= qV; } else {
    qV = qR.right-AWf;  if (qV>0) { qR.right = AWf;  qR.left -= qV; }; };
  qV = qR.top;          if (qV<0) { qR.top = 0;      qR.bottom -= qV; } else {
    qV = qR.bottom-AHf; if (qV>0) { qR.bottom = AHf; qR.top -= qV; }; };
  #ifdef _DEBUG1 // self-test
    if (*ARf!=qR) { TCHAR qS[80];
      _stprintf( qS, _T("ERR CSCR_SentinelRf{%d:%d,%d:%d} is out of {%dx%d}"),
      ARf->left, ARf->top, ARf->right, ARf->bottom, AWf, AHf );
      WriteLn( qS ); };
  #endif
  *ARf = qR; }
