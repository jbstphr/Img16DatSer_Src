// Dataseries-Viewer MFC
// (c) LGPL v3.0; by S. Bochkarev
// file VD_UtMnuPUp.h : CViewDlg local pop-up menu
// The all of these ad-hoc declarations:
// - could be local vars in DSeg of ViewDlg.cpp,
// - but are separated to be less ambiguous when one maintains code.
#pragma once

// string-resources
const int cListMarkerN = 5;
extern const TCHAR *cListMarkers[cListMarkerN];
const int cListGamutN = 3;
extern const TCHAR *cListGamuts[cListGamutN];
const int cListPixTypeN = 2;
extern const TCHAR *cListPixTypes[cListPixTypeN];
const int cFilExtsN = 8;
extern const TCHAR *cFilExts[cFilExtsN];

// local pop-up menu
const int cMenu0 = 9000;

#ifndef USE_DIRECT2D
  // [StretchBlt mode]
  const int cMenuSts0 = cMenu0+10, cMenuStsN = 4;
  typedef struct {int rV; const TCHAR *rT;} tMenuSts;
  extern const tMenuSts cMenuSts[cMenuStsN];
  extern int xMenuSts;
#endif

// [ClientCorner]
const int cMenuCCo0 = cMenu0+20, cMenuCCoN = 5;
typedef struct {int rMulX,rMulY; const TCHAR *rT;} tMenuCCo;
extern const tMenuCCo cMenuCCo[cMenuCCoN];
extern int xMenuCCo;

// [PixInspType]
const int cMenuPIT0 = cMenu0+30, cMenuPITN = 5;
typedef struct {int rSzX,rSzY; bool rHex; const TCHAR *rT;} tMenuPIT;
extern const tMenuPIT cMenuPIT[cMenuPITN];
extern int xMenuPIT;

// [ProfileType]
const int cMenuPfT0 = cMenu0+40, cMenuPfTN = 3;
typedef struct {bool rHrz,rVrt; const TCHAR *rT;} tMenuPfT;
extern const tMenuPfT cMenuPfT[cMenuPfTN];
extern int xMenuPfT;

void UtMnuPUp_AddSubMenus( HMENU AMenu ); // add sub-menus

//// ad-hoc text to clipboard ////
void UtMnu_ClipboardTxt( const TCHAR *ATxt ); // add string to clipboard_text; nil empties the buffer
bool UtMnu_ClipboardTxtHave(); // ret true if the buffer isn't empty
void UtMnu_ClipboardTxtCopy(); // copy the buffer to clipboard

//// adhoc procedures ////
void UtMnuPUp_OffsetAccessoryRect( RECT *AR ); // offset accessory's rectangle by [ClientCorner]
  // inp:"R(0,0,W,H)"; out:"R(x,y,x+W,y+H)", which locates in a corner of "theD_Rcli"
void UtMnu_HelpDlg(); // the main help dialog
void UtMnu_ExplorerExec(); // call ~ShellExecute('open')
void UtMnu_IniProfile( bool AWrite ); // read/write initialization profile

