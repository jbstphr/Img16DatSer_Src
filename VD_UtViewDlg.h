// Dataseries-Viewer MFC
// (c) LGPL v3.0; by S. Bochkarev
// file VD_UtViewDlg.h : adhoc main dialog controls' access
#pragma once

#define cUt02_TxtColor0   RGB(255,0,0) // bright text
#if 0 // reference background const
  #define cUt02_Bkg 99 
#else // reference background adjusted
  #define cUt02_Bkg gBkg
#endif
#define cUt02_TxtColor1 (gFlags&2?RGB(0,0,0):RGB(200,255,0)) // ordinary text (pixel's brightness)
#define cUt02_TxtColor2 (gFlags&2?RGB(255,255,0):RGB(0,0,0)) // grid marks
#define cUt02_BkColorBaseMn 55 // ~as rational
#define cUt02_BkColorBaseMx (255-66) // '255-brightest_background'
#define cUt02_BkColor0  RGB(0,cUt02_Bkg+44,cUt02_Bkg+66) // brighter background
#define cUt02_BkColor1  RGB(0,cUt02_Bkg+22,cUt02_Bkg+44) // darker background (whole rect's)
#define cUt02_BkColor2  RGB(0,cUt02_Bkg,   cUt02_Bkg+22) // darker background (vert profile)

//// main dialog's features ////

// frame's dimension
const int // histogram's entry #0
  cGraySigMn = -(1<<15), // signed 16-bits
  cGrayUnsMn = 0; // unsigned
inline int theA_GrayMn() { return theApp.m_nGrayMn; };
inline int theA_W() { return theApp.m_nW; }; inline int theA_H() { return theApp.m_nH; };
inline CPoint theA_WH() { return CPoint( theA_W(), theA_H() ); }; // the frame
CRect theA_Clip( const CRect AROI ); // clip a frame's rectangle

// location of the marker
inline int theD_X() { return __min(theDlg.m_X,theA_W()-1); };
inline int theD_Y() { return __min(theDlg.m_Y,theA_H()-1); };
inline CPoint theD_XY() { return CPoint( theD_X(), theD_Y() ); };
int theD_Marker(); // marker's type index
int theD_Gamut(); // gamut's type index
CPoint theD_XYa(); // [marker's area]
CPoint theD_XYo(); // the origin of symmetric [marker's area]; might be out of A_WH
CRect theD_ROI(); // _XYa.offset('_XY' or '_XYo'); [marker's ROI] in GUI
CRect theD_Rcli(); // client's rectangle
CRect theD_Rd(); // current rectangle in buffer DC
CRect theD_Rf(); // current rectangle in the frame
int *theD_Img(); // returns image-buffer
CPoint theD_GrayBW(); // explicit gray-scale's black,white
void theD_Repaint( bool AEraseBkg=false ); // repaint
void theD_OnPresentationUpdate(); // called by ~BPM-editor

void ViD_Hist16BaseBuild( // builds histogram & calcs basic stats
  const short *ABuf, bool AMed=true, bool AAve=true );

//// called mainly in the main dialog ////
int ViD_HistCutBlack( int ACut ); // to assign ~IDC_EDIT11
int ViD_HistCutWhite( int ACut ); // to assign ~IDC_EDIT9

//// adjustments in DSeg (mostly for code in _Ut_Ma<rker>.cpp) ////
extern int
  gFlags, // ~ #0:adim #1:drktxt #2:flbfont #3:flbsort #4:sttdet #5:WinEdp
  gBkg, // reference background
  gBmpCpyMxDim, // stretch copying BMP ~> BmpCpyMxDim*BmpCpyMxDim
  gPlot, // plots' gutter 
  gOccMax, // max grid for occurencies
  gOccGrayMn, // min gray for occurencies
  gOccGrayMx; // max gray for occurencies

extern TCHAR gWildCardDef[]; // default wildcard; "*."+default_extension
inline TCHAR *gFilExtDef() { return gWildCardDef+2; } // +2 is default_extension
inline TCHAR *gFilExtDef1() { return gWildCardDef+1; } // +1 is .default_extension
  
inline int ViD_StdFontHeight() {
  return gFlags&4?14:13; } // the most standard adhoc font

void ViD_IniProfile( bool AWrite ); // read/write initialization profile

