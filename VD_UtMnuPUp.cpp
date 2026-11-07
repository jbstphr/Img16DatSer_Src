// Dataseries-Viewer MFC
// (c) LGPL v3.0; by S. Bochkarev
// file VD_UtMnuPUp.cpp : CViewDlg local pop-up menu
#include "StdAfx.h"
#include "ViewApp.h" // the application-object
#include "VD_UtViewDlg.h" // adhoc main dialog controls' access
#include "VD_MDlg3.h" // dialog [Help]
#include "VD_UtMnuPUp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CViewDlg string-resources
const TCHAR *cListMarkers[cListMarkerN] = {
  _T("0:None"), _T("1:Pixel Inspector"), _T("2:Profile"), _T("3:Histogram"), _T("4:Rectangle") };
  
const TCHAR *cListGamuts[cListGamutN] = {
  _T("explicit"), _T("<0.1%>THist"), _T("<0.1%>Inv THist") };

const TCHAR *cListPixTypes[cListPixTypeN] = {
  _T("signed"), _T("unsigned") };

const TCHAR *cFilExts[cFilExtsN] = {
  _T("bin"), _T("BIN"), _T("raw"), _T("RAW"), _T("img"), _T("IMG"), _T("dat"), _T("DAT") };

/////////////////////////////////////////////////////////////////////////////
// CViewDlg local pop-up menu
#ifndef USE_DIRECT2D
  const tMenuSts cMenuSts[cMenuStsN] = { // [StretchBlt mode]
    {COLORONCOLOR,_T("Fast")},
    {HALFTONE,    _T("Halftone")},
    {WHITEONBLACK,_T("White details")},
    {BLACKONWHITE,_T("Black details")} };
#endif
int xMenuSts = 0; // stub for ini-profile under DirectX

const tMenuCCo cMenuCCo[cMenuCCoN] = { // [ClientCorner]
  {0, 0, _T("RgtBtm")},
  {1, 0, _T("LftBtm")},
  {0, 1, _T("RgtTop")},
  {1, 1, _T("LftTop")},
  {1, 1, _T("Center")} };
int xMenuCCo = 4;

const tMenuPIT cMenuPIT[cMenuPITN] = { // [PixInspType]
  {5, 5, false, _T("5x5")},
  {5, 9, false, _T("5x9")},
  {7,11, false, _T("7x11")},
  {7,11, true,  _T("7x11 hex")},
  {7,17, false, _T("7x17")}};
int xMenuPIT = 1;

const tMenuPfT cMenuPfT[] = { // [ProfileType]
  {true, false, _T("Row")},
  {false,true,  _T("Column")},
  {true, true,  _T("Both")} };
int xMenuPfT = 0;

void UtMnuPUp_AddSubMenus( HMENU AMenu ) { // -- add sub-menus
  #ifndef USE_DIRECT2D
  { HMENU qSMenu = CreatePopupMenu(); // [GDI Stretch]
    for (int qi=0;qi<cMenuStsN;qi++) AppendMenu( qSMenu, MF_STRING, cMenuSts0+qi, cMenuSts[qi].rT );
    CheckMenuItem( qSMenu, cMenuSts0+xMenuSts, MF_CHECKED );
    AppendMenu( AMenu, MF_POPUP|MF_STRING, (UINT_PTR)qSMenu, _T("GDI Stretch") ); };
  #endif // no USE_DIRECT2D
  { HMENU qSMenu = CreatePopupMenu(); // [ClientCorner]
    for (int qi=0;qi<mcASz(cMenuCCo);qi++) AppendMenu( qSMenu, MF_STRING, cMenuCCo0+qi, cMenuCCo[qi].rT );
    CheckMenuItem( qSMenu, cMenuCCo0+xMenuCCo, MF_CHECKED );
    AppendMenu( AMenu, MF_POPUP|MF_STRING, (UINT_PTR)qSMenu, _T("Client corner") ); };
  { HMENU qSMenu = CreatePopupMenu(); // [PixelInspector's type]
    for (int qi=0;qi<mcASz(cMenuPIT);qi++) AppendMenu( qSMenu, MF_STRING, cMenuPIT0+qi, cMenuPIT[qi].rT );
    CheckMenuItem( qSMenu, cMenuPIT0+xMenuPIT, MF_CHECKED );
    AppendMenu( AMenu, MF_POPUP|MF_STRING, (UINT_PTR)qSMenu, _T("Pixel Inspector") ); };
  { HMENU qSMenu = CreatePopupMenu(); // [Profile's type]
    for (int qi=0;qi<mcASz(cMenuPfT);qi++) AppendMenu( qSMenu, MF_STRING, cMenuPfT0+qi, cMenuPfT[qi].rT );
    CheckMenuItem( qSMenu, cMenuPfT0+xMenuPfT, MF_CHECKED );
    AppendMenu( AMenu, MF_POPUP|MF_STRING, (UINT_PTR)qSMenu, _T("Profle") ); }; }

//// ad-hoc text to clipboard ////
static const int cTxtN = 1<<11; static int ixTxtN = 0; static char ixTxt[cTxtN];
void UtMnu_ClipboardTxt( const TCHAR *ATxt ) { // add string to clipboard_text; nil empties the buffer
  if (!ATxt) { ixTxtN = 0; return; }; int qL = (int)_tcslen( ATxt ); if (cTxtN-ixTxtN-1<qL) return;
  VaU_TCHAR_ASCII( ATxt, ixTxt+ixTxtN, qL+1 ); ixTxtN += qL; }
bool UtMnu_ClipboardTxtHave() { return ixTxtN>0; } // -- ret true if the buffer isn't empty
void UtMnu_ClipboardTxtCopy() { // -- copy the buffer to clipboard
  if (!UtMnu_ClipboardTxtHave() || !OpenClipboard( theDlg.m_hWnd )) return;
  HANDLE qMemHandle = GlobalAlloc( GMEM_MOVEABLE, ixTxtN*sizeof(char) );
  if (!qMemHandle) { CloseClipboard(); return; };
  char *qData = (char*)GlobalLock( qMemHandle );
  if (!qData) { CloseClipboard(); return; };
  EmptyClipboard();
  memcpy( qData, ixTxt, ixTxtN*sizeof(char) );
  SetClipboardData( CF_TEXT, qMemHandle ); // CF_UNICODETEXT 
  GlobalUnlock( qMemHandle );
  CloseClipboard(); }

//// adhoc procedures ////
void UtMnuPUp_OffsetAccessoryRect( RECT *AR ) { // -- offset accessory's rectangle by [ClientCorner]
  CRect qRcli = theD_Rcli();
  CPoint qP0( qRcli.Width()-AR->right, qRcli.Height()-AR->bottom ); // RgtBtm
  qP0.x *= cMenuCCo[xMenuCCo].rMulX; qP0.y *= cMenuCCo[xMenuCCo].rMulY;
  OffsetRect( AR, qP0.x, qP0.y ); }

void UtMnu_HelpDlg() { // -- the main help dialog 
  HelpDlg( theApp.m_pszAppName, &theDlg,
    _T("Main\n\
= The main functionality of this utility:\n\
 - read 16bit little-endian pixels from \"image_file\",\n\
 - hold the pixels in memory as 32bit signed ints,\n\
 - displays those as 8bit grayscale.\n\
Specific builds:\n\
 - \"WindowsGDI\" is default,\n\
 - \"DirectX 2D\" supports only \"halftone bitblt-stretch\"; this could\n\
   become rational in ~ old Linux-PCs (slow WinE-GDI).\n\n\
= ThisExeName defines initial \"pixels' interpretation\":\n\
 - default       :\t- [signed]\n\
 - *U.exe, *u.exe:\t- [unsigned]\n\n\
\tthe input:\n\
= section [View] in ini-profile\n\
 - this_exe_directory/%IniProfileOfSpecificSDK% (if one exists)\n\
 - this_exe_directory/this_exe_name.ini (otherwise)\n\
= optional cmdline 'w h wildcard' or \"wildcard\", where\n\
wildcard composes as \"directory/fname_wildcard\"; e.g. ~:\n\
\t256 256 image_file\n\t640 480 z:/img/1????.ext\n\t\"/home/user/img/*.ext\"\n\n\
\twindow-title:\n\
- <prefix> file_name min..max =median ~average\n\
- '?'=\"no image\", '<'=\"[w]*[h]<file_size\",..."),
    _T("View-controls\n\
= [w><h]        :\t- swaps the width,height\n\
= [>>]          :\t- browse & load image_file<s>\n\
= [combo box] (interprets pixels in image_file)\n\
  - signed      :\t- default -32768..32767\n\
  - unsigned    :\t- 0..65535 (when named as \"View16U.exe\"\n\
= [combo box] (gray_gamut)\n\
  - explicit    :\t- black=>RGB(0,0,0), white=>RGB(255,255,255)\n\
  - THist       :\t- cut 0.1% of pixels\n\
  - inv THist   :\t- cut 0.1% of pixels and invert\n\
= [Zoom]        :\t- magnifies frame's rectangle versus displayed one\n\
= [combo Marker]:\t- draws an accessory alongside image\n\
= [edits X,Y]   :\t- Marker's location\n\
= [area]        :\t- Marker's size"),
    _T("Pop-up menu\n\
\tAppears by [F9], right mouse btn\n\n\
\tMaintenance\n\
= the dialogs (show particular help by [F1])\n\
  - Settings   :\t- UI-settings\n\
  - Files      :\t- operate with items of files' list\n\
  - BPMM editor:\t- operate with BadPixels'Mask-overlays\n\
= the commands\n\
  - Copy displayng BMP-area to clipboard\n\
  - Copy ROI text to clipboard\n\
  - Copy recent stats to clipboard\n\
  - [Execute] : invokes ~ShellExecute('open')\n\n\
\tGDI-stretch selection <when allowed>\n\
- Fast               :\t- StretchBlt(COLORONCOLOR)\n\
- Halftone           :\t- HALFTONE (the only option in DirectX-build)\n\
- White|Black details:\t- WHITEONBLACK|BLACKONWHITE\n\n\
\tClient corner's selection\n\
- to locate the image (when aspect_ratio reduces client_area)\n\
- to locate accessories in ~opposite corner\n\n\
\tPixel Inspector's selection\n\
- 5x5 | 7x7 | etc\n\n\
\tProfile's selection\n\
- Row | Column | Both"),
    _T("The markers\n\
\t1:Pixel inspector\n\
- ROI-center is view's spin-edits [X,Y]\n\
- ROI-area is selection in pop-up menu\n\n\
\t2:Profile\n\
- the lines Row|Column|Both follow selection in pop-up menu\n\
- the grids follow the view\n\n\
\t3:Histogram\n\
- the grids follow [Settings]\n\n\
\t4:Rectangle\n\
- ROI-lft_top is view's [XY]\n\
- ROI-area is view's [area]\n\
- presenting statistics follow [Settings]")
  ); }

void UtMnu_ExplorerExec() { // -- call ~ShellExecute('open')
  TCHAR qS[256]; int qL = (int)(theApp.m_sNam-theApp.m_sPath);
  if (qL>0) { _tcsncpy( qS, theApp.m_sPath, qL ); qS[qL-1] = 0; }
       else { _tcscpy( qS, theApp.m_IF.rFName ); VaU_Sfx( qS, _T('\\') )[-1] = 0; };
  if (theApp.m_bWinEstrAPI) {
    _stprintf( qS, _T("file:%s"), VaUt_PathToUIStr(qS) );
    ShellExecute( NULL, _T("OPEN"), _T("WINEBROWSER"), qS, NULL, SW_SHOWNORMAL );
  } else
    ShellExecute( theDlg.m_hWnd, _T("OPEN"), _T("EXPLORER.EXE"), qS, NULL, SW_SHOWNORMAL );
}
 
void UtMnu_IniProfile( bool AWrite ) { // -- read/write initialization profile
  if (AWrite) { TCHAR qS[256]; _stprintf( qS,
    _T("%d %d %d %d; Sts CCo PIT PfT"), xMenuSts, xMenuCCo, xMenuPIT, xMenuPfT );
    theApp.VDI_SetStr( _T("UtMnu"), qS );
    return; };
  if (theApp.VDI_Ints( _T("UtMnu"), NULL )==4) {
    int *qI = theApp.VDI_Ints();
    xMenuSts = qI[0]; xMenuCCo = qI[1]; xMenuPIT = qI[2]; xMenuPfT = qI[3];
  }; }

