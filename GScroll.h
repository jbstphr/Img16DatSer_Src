// Dataseries-Viewer MFC
// (c) LGPL v3.0; by S. Bochkarev
// Scrolling service (declarations)
#pragma once

// Rclient :CCCC CCCC CddC  At Zoom=1 Rd        Frame :FFFF  Rf locates inside 
// vs      :CCCC dddd CddC  occupies the        vs    :FffF  the frame of Wf*Hf.
// Rdisplay:CCCC dddd CddC  whole wdt or        Rframe:FffF  R stretches into
//         :CCCC CCCC CddC  hgt of Rc.                :FFFF  Rd.
//
//   always :ScrollBarX.ScrollPos=Rf.left, ScrollBarY.ScrollPos=Rf.top
//          :ScrollBarX.ScrollLim=Wf-Rf.Wdt, ScrollBarY.ScrollLim=Hf-Rf.Hgt

void GSCR_CalcScrollRects(
  const CRect ARc,  // display's client rectangle
  int AZoom,        // >0
  int AWf, int AHf, // frame's dimension
  float AWK, float AHK, // relative scrollbars' positions; 0.5=center
  CRect *ARd,       // dst display's rectangle to stretch
  CRect *ARf );     // src frame's rectangle to stretch

int GSCR_GetCfX( const CRect ARd, const CRect ARf, const int ACgX );
int GSCR_GetCfY( const CRect ARd, const CRect ARf, const int ACgY );
bool GSCR_GetCf( const CRect ARd, const CRect ARf, const CPoint ACg, CPoint *ACf );
  // frames' coordinate Cf by GDI's coordinate Cg
  // returns Rf.PtInRect(Cf)

bool GSCR_GetCg( const CRect ARd, const CRect ARf, const CPoint ACf, CPoint *ACg, bool A05=true );
  // GDI's coordinate Cg by frame's coordinate Cf; 05=true:center_display_pix, =false:1st_display_pix
  // returns Rd.PtInRect(Cg)

CPoint GSCR_GetScrollRng( // scrolbabrs' ranges
  int AWf, int AHf,       // frame's dimension
  const CRect ARf );      // caculated src frame's rectangle to stretch
  // returns Wf-Rf.Wdt:Hf-Rf.Hgt

void GSCR_GetScrollK( // returns scrollK
  int AWf, int AHf,   // frame's dimension
  const CRect ARf,    // calculated src frame's rectangle to stretch
  float *AWHK );      // scrollK
  // returns relative scrollbars' positions (i.e. Rf.left/Wf, Rf.top/Hf)

void CSCR_SentinelRf( int AWf, int AHf, CRect *ARf );
  // sentinel for 0:0..Wf:Hf