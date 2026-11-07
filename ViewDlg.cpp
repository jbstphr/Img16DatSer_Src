// Dataseries-Viewer MFC
// (c) LGPL v3.0; by S. Bochkarev
// EViewDlg.cpp : main dialog
#include "StdAfx.h"
#include "IO.h"
#include "GScroll.h" // Scrolling service
#include "ViewApp.h" // the application-object
#include "VD_UtViewDlg.h" // adhoc main dialog controls' access
#include "VD_UtMnuPUp.h" // CViewDlg local pop-up menu
#include "VD_UtMa1.h" // marker's driving service
#include "VD_UtMa2.h" // marker's driving service
#include "VD_UtMa3.h" // marker's driving service
#include "VD_UtMa4.h" // marker's driving service
#include "VD_MDlg1.h" // dialog [Settings]
#include "VD_MDlg2.h" // dialog [Files]
#include "VD_MDlg3.h" // dialog [Help]
#include "VD_NMDlg1.h" // dialog [BPM]
#include "VD_NMDlg2.h" // dialog [flip,cut,rot]
#include "ViewDlg.h"
#include <shlwapi.h>   // For StrCmpLogicalW
#pragma comment(lib, "shlwapi.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CViewDlg::CViewDlg( CWnd* pParent ) : CDialog( CViewDlg::IDD, pParent ) {
  m_bInitialized = m_bPresentationUpdate = m_Combo3DropDown = false;
  m_Z = 1; m_X = m_Y = m_Combo3Flg = m_LastDlgCtrlId = 0; m_DPzoo = -1; }
  
void CViewDlg::DoDataExchange( CDataExchange* pDX ) {
  CDialog::DoDataExchange( pDX );
  DDX_Control( pDX, IDC_SCROLLBAR1, m_SBY );
  DDX_Control( pDX, IDC_SCROLLBAR2, m_SBX );
  DDX_Control( pDX, IDC_STATIC1, m_Sta1 );
  DDX_Control( pDX, IDC_LIST1, m_List1 );
  DDX_Control( pDX, IDC_COMBO3, m_Combo3 );
}

BEGIN_MESSAGE_MAP(CViewDlg, CDialog)
  ON_WM_PAINT()
  ON_WM_QUERYDRAGICON()
  ON_WM_LBUTTONDOWN()
  ON_WM_LBUTTONUP()
  ON_WM_MOUSEMOVE()
  ON_WM_SIZE()
  ON_WM_CTLCOLOR()
  ON_WM_DROPFILES()
END_MESSAGE_MAP()

static const int cMsg0 = WM_USER+2048;

/////////////////////////////////////////////////////////////////////////////
// CViewDlg local pop-up menu
void CViewDlg::iDisplayLocalMenu( LPARAM APoint ) {
  HMENU qMenu = CreatePopupMenu();
  { HMENU qSMenu = CreatePopupMenu(); // [Maintenance]
    AppendMenu( qSMenu, MF_STRING, cMenu0+0, _T("Settings [ctrl+S]") );
    AppendMenu( qSMenu, MF_STRING, cMenu0+1, _T("Files [F2]") );
    if (!BPMDlg_Active()) AppendMenu( qSMenu, MF_STRING, cMenu0+2, _T("BPMM editor [ctrl+B]") );
    AppendMenu( qSMenu, MF_SEPARATOR, 0, NULL );
    if (theD_Img())
      AppendMenu( qSMenu, MF_STRING, cMenu0+5, _T("Copy bitmap to clipboard") );
    if (!theA_Clip(theD_ROI()).IsRectEmpty())
      AppendMenu( qSMenu, MF_STRING, cMenu0+6, _T("Copy ROI text to clipboard") );
    if (UtMnu_ClipboardTxtHave())
      AppendMenu( qSMenu, MF_STRING, cMenu0+7, _T("Copy recent stats to clipboard") );
    AppendMenu( qSMenu, MF_STRING, cMenu0+8,
      theApp.m_bWinEstrAPI?_T("WinE Browser [shift+F2]"):_T("Windows Explorer [shift+F2]") );
    AppendMenu( qMenu, MF_POPUP|MF_STRING, (UINT_PTR)qSMenu, _T("Maintenance") ); };
  UtMnuPUp_AddSubMenus( qMenu );
  AppendMenu( qMenu, MF_STRING, cMenu0+3, _T("Help") );
  // display menu
  VaUt_DisplayMenu( this, APoint, qMenu ); }

/////////////////////////////////////////////////////////////////////////////
// CViewDlg services
void CViewDlg::iOnSizeControls( int ACX, int ACY ) { // -- as if called by OnSize
  CRect qRX, qRY; GetSBRects( &qRX, &qRY );
  CRect qR1; m_Sta1.GetWindowRect( &qR1 ); ScreenToClient( &qR1 ); 
  m_SBY.MoveWindow( ACX-qRY.Width(), qRY.top, qRY.Width(), ACY-qRX.Height()-qRY.top );
  m_SBX.MoveWindow( qRX.left, ACY-qRX.Height(), ACX-qR1.Width()-qRX.left, qRX.Height() );
  m_Sta1.MoveWindow( ACX-qR1.Width(), ACY-qR1.Height(), qR1.Width(), qR1.Height() );
  // upper-rgt section of clientrect
  m_Combo3.GetWindowRect( &qR1 ); ScreenToClient( &qR1 );
  qR1.right = ACX-2; m_Combo3.MoveWindow( qR1 );
  m_Combo3.SetEditSel( -1, 0 ); m_Combo3.Invalidate( TRUE );
  // files' list on bottom-lft section of clientrect
  m_List1.GetWindowRect( &qR1 ); ScreenToClient( &qR1 ); 
  qR1.bottom = ACY-2; m_List1.MoveWindow( &qR1 ); }

void CViewDlg::iSetMarkerArea( int ACX, int ACY, bool AEnabled ) { // -- set [marker Area]
  const int cIC[] = {IDC_EDIT6,IDC_SPIN6,IDC_EDIT7,IDC_SPIN7};
  VaUt_EnableControls( m_hWnd, cIC, mcASz(cIC), AEnabled );
  m_bInitialized = false; VaUt_SetInt( this, IDC_EDIT6, ACX );
  VaUt_SetInt( this, IDC_EDIT7, ACY ); m_bInitialized = true; }

void CViewDlg::iSetCutGamut() { // -- set [cut Gamut] (if one is active)
  int qWhite = ViD_HistCutWhite( VaUt_SpinInt(this,IDC_EDIT8) ),
      qBlack = ViD_HistCutBlack( VaUt_SpinInt(this,IDC_EDIT10) );
  if (qWhite<qBlack) mcSwap( int, qWhite, qBlack ) else if (qWhite==qBlack) qWhite++;
  switch (theD_Gamut()) {
    case  1: m_BMPBuf.mWhite = qWhite; m_BMPBuf.mBlack = qBlack; break;
    case  2: m_BMPBuf.mWhite = qBlack; m_BMPBuf.mBlack = qWhite; break;
    default: return; };
  m_bInitialized = false; VaUt_SetInt( this, IDC_EDIT9, m_BMPBuf.mWhite );
  VaUt_SetInt( this, IDC_EDIT11, m_BMPBuf.mBlack ); m_bInitialized = true; }

CRect CViewDlg::iDrawMarkerROI( tDC *ADC, const CRect ARd, bool ASymmROI ) { // -- draw 'theD_ROI'
  CRect qROI = theD_ROI(); if (!ASymmROI) qROI = theA_Clip( qROI ); // clip non-symm ROIs here
  CPoint qM0; GSCR_GetCg( ARd, m_Rf, qROI.TopLeft(), &qM0, false ); // the left of display's pixel
  CPoint qM1; GSCR_GetCg( ARd, m_Rf, qROI.BottomRight(), &qM1, false );
  CRect qR( qM0, qM1 ); // min rectangle (to be recognizable)
  int qDmn; if (ASymmROI) {
    qDmn = 5-qR.Width(); if (qDmn>0) { qR.left -= qDmn>>1; qR.right += qDmn-(qDmn>>1); };
    qDmn = 5-qR.Height(); if (qDmn>0) { qR.top -= qDmn>>1; qR.bottom += qDmn-(qDmn>>1); };  
  } else {
    qDmn = 3-qR.Width(); if (qDmn>0) qR.right += qDmn;
    qDmn = 3-qR.Height(); if (qDmn>0) qR.bottom += qDmn; };
  // draw and leave
  ADC->SetPen( PS_SOLID, 1, cUt02_TxtColor0 );
  ADC->SetBrush( BS_HOLLOW, 0, 0 );
  ADC->rDC.Rectangle( qR ); return qROI; }

void CViewDlg::iCombo3DimSet( int AI, int AW, int AH ) { // -- set dim
  m_Combo3.SetItemData( AI, (AW&0xFFFF)|(AH<<16) ); }
void CViewDlg::iCombo3StrSet( int AI, const TCHAR *APath, int AW, int AH ) { // -- set path
  const TCHAR *cPath = VaUt_PathToUIStr( APath );
  if (AI<0) AI = m_Combo3.AddString( cPath );
       else m_Combo3.InsertString( AI, cPath );
  iCombo3DimSet( AI, AW, AH ); }
void CViewDlg::iCombo3StrGet( int AI, TCHAR *APath, int *AW, int *AH, bool AUIStr ) { // -- get path
  if (APath) { m_Combo3.GetLBText( AI, APath );
    if (!AUIStr) _tcscpy( APath, VaUt_UIStrToPath(APath) ); };
  if (!AW) return; DWORD qWH = (DWORD)m_Combo3.GetItemData( AI );
  *AW = qWH&0xFFFF; *AH = (qWH>>16)&0xFFFF; }
void CViewDlg::iCombo3WndTxtSet( const TCHAR *APath ) { // -- set wildcard's text
  m_Combo3.SetWindowText( VaUt_PathToUIStr(APath) ); }
int CViewDlg::iCombo3WndTxtGet( TCHAR *APath ) { // -- get wildcard's text
  TCHAR qS[192]; m_Combo3.GetWindowText( qS, 192 ); VaU_TrimStr( qS );
  _tcscpy( APath, VaUt_UIStrToPath(qS) ); return m_Combo3.FindString( -1, qS ); }

static int fn_qsort_Windows( const void *A1, const void *A2 ) { return StrCmpLogicalW( (const LPCWSTR)A1, (const LPCWSTR)A2 ); } 
static int fn_qsort_Std( const void *A1, const void *A2 ) { return wcsicmp( (const LPCWSTR)A1, (const LPCWSTR)A2 ); }
  
void CViewDlg::iList1Sort( int *ASel, int *AMaxLen ) { // -- sort List1 in Windows Explorer Mode
  const int cMaxLen = 64; int qC = m_List1.GetCount();
  WCHAR *qBuf = new WCHAR[cMaxLen*qC+256];
  for (int qi1=0;qi1<qC;qi1++) {
    #ifndef UNICODE
      char qS[256]; m_List1.GetText( qi1, qS ); 
      MultiByteToWideChar( CP_ACP, 0, qS, -1, qBuf+qi1*cMaxLen, cMaxLen-1 );
    #else
      m_List1.GetText( qi1, qBuf+qi1*cMaxLen );
    #endif
  }; // for i1
  qsort( qBuf, qC, cMaxLen*sizeof(WCHAR), gFlags&8?fn_qsort_Windows:fn_qsort_Std );
  m_List1.ResetContent();
  int qSel = m_Combo3Nam[0] ? -1 : 0x7FFFFFFF, qMaxLen = 0;
  for (int qi2=0;qi2<qC;qi2++) {
    WCHAR *qB = qBuf+qi2*cMaxLen;
    #ifndef UNICODE
      char qS[256]; WideCharToMultiByte( CP_ACP, 0, qB, -1, qS, cMaxLen-1, NULL, NULL );
      m_List1.AddString( qS ); if (qSel<0 && !stricmp( m_Combo3Nam, qS )) qSel = qi2;
      int qLen = (int)strlen( qS );
    #else
      if (qSel<0 && !wcsicmp( m_Combo3Nam, qB )) qSel = qi2;
      m_List1.AddString( qB );
      int qLen = (int)wcslen( qB );
    #endif
    if (qMaxLen<qLen) qMaxLen = qLen;
  }; // for i2
  delete qBuf; *ASel = qSel==0x7FFFFFFF?-1:qSel; *AMaxLen = qMaxLen; }

/////////////////////////////////////////////////////////////////////////////
// CViewDlg ~API-internals
void CViewDlg::jOnChangeRdRf( bool AOnSize ) { // -- set label<s> for Zoom>1 when change "RECTdisplay"
  // check that XY locates within frame
  CPoint cXY = theD_XY(), cAWH = theA_WH();
  if (cXY.x>=cAWH.x || cXY.y>=cAWH.y) {
    m_X = cAWH.x-1; m_Y = cAWH.y-1; bool cInitialized = m_bInitialized;
    m_bInitialized = false; SetDlgItemInt( IDC_EDIT4, m_X );
    SetDlgItemInt( IDC_EDIT5, m_Y ); m_bInitialized = cInitialized; };
  if (m_Z>1) { TCHAR qS[80]; _stprintf( qS, _T("%d:%d %d:%d"), // display RECTframe
    m_Rf.left, m_Rf.top, m_Rf.right, m_Rf.bottom ); m_Sta1.SetWindowText( qS ); };
  theD_Repaint( AOnSize ); }

void CViewDlg::jReCalcRdRf( int ATag ) { // -- recalculate "RECTdisplay", "RECTframe"
  const CPoint cAWH = theA_WH(); float qWHK[2];
  if (ATag&2 || m_Z==1) qWHK[0] = qWHK[1] = 0.5f; // bit#1: called from SetImgDim (scroll to the center)
                   else GSCR_GetScrollK( cAWH.x, cAWH.y, &m_Rf, qWHK );
  CRect qRcli = theD_Rcli();
  GSCR_CalcScrollRects( qRcli, m_Z, cAWH.x, cAWH.y, qWHK[0], qWHK[1], &m_Rd, &m_Rf );
  switch (xMenuCCo) { // attach "RECTdisplay" to opposite corner of PixelInspector
    case 0: m_Rd.OffsetRect( qRcli.BottomRight()-m_Rd.BottomRight() ); break;
    case 1: m_Rd.OffsetRect( qRcli.left-m_Rd.left, qRcli.bottom-m_Rd.bottom ); break;
    case 2: m_Rd.OffsetRect( qRcli.right-m_Rd.right, qRcli.top-m_Rd.top ); break;
    case 3: m_Rd.OffsetRect( qRcli.TopLeft()-m_Rd.TopLeft() ); break; };
  if (m_Z>1) { // set ranges of visible scrollbars
    CPoint qSR( GSCR_GetScrollRng( cAWH.x, cAWH.y, m_Rf ) );
    m_SBX.SetScrollRange( 0, qSR.x ); m_SBX.SetScrollPos( m_Rf.left );
    m_SBY.SetScrollRange( 0, qSR.y ); m_SBY.SetScrollPos( m_Rf.top ); }; // if Z>1
  jOnChangeRdRf( ATag&1 ); } // bit#0: called from OnSize

void CViewDlg::jProcessScrollMsg( CScrollBar *ASBar, WPARAM AWParam ) { // -- message from scrollbar
  int qPos = ASBar->GetScrollPos(), qLim = ASBar->GetScrollLimit()-1;
  if (qLim<0) qLim = 0;
  switch (LOWORD( AWParam )) {
    case SB_LINEUP: qPos -= 1; break;
    case SB_LINEDOWN: qPos += 1; break;
    case SB_PAGEUP: qPos -= __max(qLim/10,10); break;
    case SB_PAGEDOWN: qPos += __max(qLim/10,10); break;
    case SB_TOP: qPos = 0; break;
    case SB_BOTTOM: qPos = qLim; break;
    case SB_THUMBPOSITION: qPos = (short)HIWORD(AWParam); break;
    case SB_THUMBTRACK:
    case SB_ENDSCROLL: return; };
  qPos = mcRng( qPos, 0, qLim );
  ASBar->SetScrollPos( qPos );
  // shift RECTframe only
  if (ASBar==&m_SBX) m_Rf.OffsetRect( qPos-m_Rf.left, 0 ); else 
  if (ASBar==&m_SBY) m_Rf.OffsetRect( 0, qPos-m_Rf.top );
  jOnChangeRdRf(); }

void CViewDlg::jProcessOwnerDrawMessage( UINT message, WPARAM wParam, LPARAM lParam, int ATag ) { // -- owner_draw
  TCHAR  qS[256]; CFont *qFont = gFlags&4?&m_Font:&m_FontSmall;
  switch (message) {
    case WM_MEASUREITEM: { CDC *qDC = GetDC();
      CFont *qOldFont = qDC->SelectObject( qFont );
      TEXTMETRIC qTM; qDC->GetTextMetrics( &qTM ); qDC->SelectObject( qOldFont ); ReleaseDC( qDC );
      ((LPMEASUREITEMSTRUCT)lParam)->itemHeight = qTM.tmHeight+0;
    }; break; // case WM_MEASUREITEM
    case WM_DRAWITEM: {
      LPDRAWITEMSTRUCT qDIS = (LPDRAWITEMSTRUCT)lParam; int cItem = qDIS->itemID; if (cItem==-1) return;
      COLORREF qTextColor, qBkColor; switch (qDIS->itemState&(ODS_FOCUS|ODS_SELECTED)) {
        case ODS_FOCUS|ODS_SELECTED: qTextColor = cUt02_TxtColor0; qBkColor = cUt02_BkColor1; break;
        case ODS_FOCUS:              qTextColor = cUt02_TxtColor0; qBkColor = GetSysColor(COLOR_WINDOW); break;
        case ODS_SELECTED:           qTextColor = cUt02_TxtColor1; qBkColor = cUt02_BkColor1; break;
        default:                     qTextColor = GetSysColor(COLOR_WINDOWTEXT); qBkColor = GetSysColor(COLOR_WINDOW); };
      HDC dc = qDIS->hDC; RECT *qR = &qDIS->rcItem;
      SetTextColor( dc, qTextColor ); SetBkColor( dc, qBkColor );
      HBRUSH qBrush = CreateSolidBrush( GetBkColor(dc) );
      FillRect( dc, qR, qBrush ); DeleteObject( qBrush );
      switch (ATag) { // acquire the text to display (depending on the "Tag")
        case 0: { // [wildcard] combo-box
          int qW, qH; TCHAR s[192]; iCombo3StrGet( cItem, s, &qW, &qH, true );
          _stprintf( qS, _T("%04dx%04d %s"), qW, qH, s );
        }; break;
        case 1: { // files' list
          m_List1.GetText( cItem, qS );
        }; break;
      }; // switch Tag
      HFONT qOldFont = (HFONT)SelectObject( dc, qFont->m_hObject );
      DrawText( dc, qS, (int)_tcslen(qS), qR, DT_LEFT|DT_SINGLELINE|DT_VCENTER );
      SelectObject( dc, qOldFont );
    }; break; // case WM_DRAWITEM
  }; } // switch message

void CViewDlg::jCombo3Validate() { // -- change edit by wildcard-shadow
  // if wildcard was changed then update it's list-box & edit-box
  int qCombo3Flg = m_Combo3Flg; m_Combo3Flg = qCombo3Flg&0xFF;
  if (!(qCombo3Flg&0x200)) return; // if no [wildcard editor changed]
  if (theApp.m_nSizeDiff) return;
  // update [wildcard]'s list-box
  TCHAR qS[192]; int qOld = iCombo3WndTxtGet( qS ); 
  if (!qOld) { // the path is already from the item#0 
    iCombo3DimSet( 0, theA_W(), theA_H() ); return; };
  m_Combo3.SetRedraw( FALSE ); 
  if (qOld<0) { qOld = m_Combo3.GetCount()-1; if (qOld<15) qOld = -1; };
  if (qOld>0) m_Combo3.DeleteString( qOld );
  iCombo3StrSet( 0, qS, theA_W(), theA_H() );
  m_Combo3.SetCurSel( 0 ); m_Combo3.SetRedraw( TRUE );
  m_Combo3.Invalidate(); }

/////////////////////////////////////////////////////////////////////////////
// CViewDlg message handlers

BOOL CViewDlg::OnInitDialog() {
  TCHAR qS[256];
  CDialog::OnInitDialog();
  SetWindowText( theApp.m_pszAppName );
  SetTimer( 1, 222, NULL );
  //TCHAR qS[256];
  HICON qIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME); // from ~default constructor to here
  SetIcon( qIcon, TRUE ); SetIcon( qIcon, FALSE ); // Set big & small icon
  // fonts
  LOGFONT qLF; memset( &qLF, 0, sizeof(LOGFONT) ); // font
  _tcscpy( qLF.lfFaceName, theApp.StandardMonoFontName() );
  qLF.lfHeight = 14; qLF.lfWeight = 888; m_Font.CreateFontIndirect( &qLF );
  _tcscpy( qLF.lfFaceName, _T("ARIAL") );
  qLF.lfHeight = 13; m_FontSmall.CreateFontIndirect( &qLF );
  m_Sta1.SetFont( &m_FontSmall );
  GetDlgItem(IDC_BUTTON1)->SetFont( &m_FontSmall );
  GetDlgItem(IDC_COMBO2)->SetFont( &m_FontSmall );
  GetDlgItem(IDC_COMBO4)->SetFont( &m_FontSmall );
  // spins
  SendDlgItemMessage( IDC_SPIN1,  UDM_SETRANGE32, 0, 0x7FFF ); // [Width]
  SendDlgItemMessage( IDC_SPIN2,  UDM_SETRANGE32, 0, 0x7FFF ); // [Height]
  SendDlgItemMessage( IDC_SPIN3,  UDM_SETRANGE32, 1, 33 ); SetDlgItemInt( IDC_EDIT3, m_Z ); // [Zoom]
  SendDlgItemMessage( IDC_SPIN6,  UDM_SETRANGE32, 1, 0xFFFF ); // [size X]
  SendDlgItemMessage( IDC_SPIN7,  UDM_SETRANGE32, 0xFFFF, 1 ); // [size Y]
  SendDlgItemMessage( IDC_SPIN8,  UDM_SETRANGE32, 9999, 0 ); // [Gamut cut% white]
  SendDlgItemMessage( IDC_SPIN10, UDM_SETRANGE32, 0, 9999 ); // [Gamut cut% black]
  SendDlgItemMessage( IDC_SPIN9,  UDM_SETRANGE32, -(1<<16)+1, (1<<16)-1 ); // [Gamut white explicit]
  SendDlgItemMessage( IDC_SPIN11, UDM_SETRANGE32, -(1<<16)+1, (1<<16)-1 ); // [Gamut black explicit]
  // INI-profile
  // theApp.VDI_SetSectName( _T("View") ); // set in InitInstance
  CRect qR; const int *cI = theApp.VDI_Ints();
  { // recent [rectangle]'s marker
    if (theApp.VDI_Ints(_T("MarkerRect"),NULL)==2) {
      xMa4_MarkerArea.x = cI[0]; xMa4_MarkerArea.y = cI[1]; }; };
  { // window's size & position
    GetWindowRect( &qR ); m_WIWmn = qR.Width(); m_WIHmn = qR.Height();
    if (theApp.VDI_Ints(_T("WndPos"),NULL)==4) { qR.SetRect( cI[0], cI[1], cI[2], cI[3] ); 
      if (qR.top>0 && qR.left>0 && qR.Width()<9999 && qR.Height()<9999) {
        int qWmon, qHmon; { HMONITOR qHM = MonitorFromWindow( m_hWnd, MONITOR_DEFAULTTONEAREST );
        MONITORINFO qMI; qMI.cbSize = sizeof(MONITORINFO); if (GetMonitorInfo( qHM, &qMI )) {
          qWmon = qMI.rcMonitor.right-qMI.rcMonitor.left; qHmon = qMI.rcMonitor.bottom-qMI.rcMonitor.top;
        } else {
          qWmon = GetSystemMetrics( SM_CXSCREEN ); qHmon = GetSystemMetrics( SM_CYSCREEN ); };
        int qXmx = qWmon-(qR.Width()>>1), qYmx = qHmon-(qR.Height()>>1);
        MoveWindow( __min(qR.left,qXmx), __min(qR.top,qYmx), qR.Width(), qR.Height() ); };
    }; // if top>=0 and left>09999
    GetClientRect( &qR ); iOnSizeControls( qR.Width(), qR.Height() ); }; // if _Ints=4
  };
  { // [signed/unsigned]
    for (int qi=0;qi<cListPixTypeN;qi++)
      SendDlgItemMessage( IDC_COMBO4, CB_ADDSTRING, 0, (LPARAM)cListPixTypes[qi] );
    SendDlgItemMessage( IDC_COMBO4, CB_SETCURSEL, theA_GrayMn()<0?0:1 ); };
  { // [grayscale]
    SetDlgItemInt( IDC_EDIT8, 10 ); SetDlgItemInt( IDC_EDIT10, 10 ); // cut white,black ~1.0%
    SetDlgItemInt( IDC_EDIT9, 4095 ); SetDlgItemInt( IDC_EDIT11, 0 ); // explicit white,black
    for (int qi=0;qi<cListGamutN;qi++)
      SendDlgItemMessage( IDC_COMBO2, CB_ADDSTRING, 0, (LPARAM)cListGamuts[qi] );
    theApp.VDI_Ints( _T("Gray"), _T("0 0 4095") );
    const int cId = cI[0] ? IDC_EDIT8 : IDC_EDIT9;
    SendDlgItemMessage( IDC_COMBO2, CB_SETCURSEL, cI[0] );
    SetDlgItemInt( cId+2, cI[1] ); SetDlgItemInt( cId, cI[2] );
    m_BMPBuf.mBlack = cI[1]; m_BMPBuf.mWhite = cI[2];
    PostMessage( WM_COMMAND, MAKEWPARAM(IDC_COMBO2,CBN_SELCHANGE) ); }; 
  { // [marker]
    for (int qi=0;qi<cListMarkerN;qi++)
      SendDlgItemMessage( IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)cListMarkers[qi] );
    theApp.VDI_Ints( _T("Marker"), _T("0") );      
    SendDlgItemMessage( IDC_COMBO1, CB_SETCURSEL, cI[0] );
    PostMessage( WM_COMMAND, MAKEWPARAM(IDC_COMBO1,CBN_SELCHANGE) ); };
  { // [wildcard]
    theApp.CommandLineCheck(); // could assign W,H,Path:=Mask
    bool cInitEdit = theApp.m_sPath[0]!=0; // as if [wildcard editor changed]
    if (cInitEdit) { if (theApp.m_sNam) { // if specific file 
      _tcscpy( m_Combo3Nam, theApp.m_sNam ); _tcscpy( theApp.m_sNam, gWildCardDef ); };
      iCombo3WndTxtSet( theApp.m_sPath ); // here is no auto '_EDITCHANGE'
    } else
      _tcscpy( m_Combo3Nam, theApp.VDI_Str(_T("FNam"),_T("")) );
    theApp.VDI_Ints( _T("DirN"), _T("0") ); const int cN = theApp.VDI_Ints()[0];
    for (int qi=0;qi<cN;qi++) { _stprintf( qS, _T("Dir%d"), qi );
      const TCHAR *cS = theApp.VDI_Str( qS, NULL ); if (!cS) continue;
      int qW, qH, qLenPfx; if (_stscanf( cS,_T("%dx%d%n"),&qW,&qH,&qLenPfx)!=2) continue;
      iCombo3StrSet( -1, cS+qLenPfx+1, qW, qH );
      if (cInitEdit && !theA_W() && !_tcsicmp(theApp.m_sPath,cS+qLenPfx+1)) {
        theApp.m_nW = qW; theApp.m_nH = qH; };
    }; // for i
    if (!m_Combo3.GetCount()) iCombo3StrSet( -1, gWildCardDef, 256, 256 );
    if (cInitEdit) { // assign spin-edits [Width], [Height]
      if (!theA_W()) iCombo3StrGet( 0, NULL, &theApp.m_nW, &theApp.m_nH );
      SetDlgItemInt( IDC_EDIT1, theApp.m_nW ); SetDlgItemInt( IDC_EDIT2, theApp.m_nH );
    } else {
      m_Combo3.SetCurSel( 0 ); 
    };
    PostMessage( WM_COMMAND, MAKEWPARAM(IDC_COMBO3,cInitEdit?CBN_EDITCHANGE:CBN_SELCHANGE) ); }; 
  // leave
  m_Combo3.SetFocus();
  m_bInitialized = true;
  return FALSE; } // return TRUE unless you set the focus to a control

BOOL CViewDlg::OnCommand( WPARAM wParam, LPARAM lParam ) {
  TCHAR qS[256];
  WORD qNC = HIWORD(wParam), qID = LOWORD(wParam);
  // tag of the last changed control
  if (qID>=IDC_EDIT1 && qID<=IDC_EDIT35) m_LastDlgCtrlId = qID; else
  if (qID>=IDC_SPIN1 && qID<=IDC_SPIN35) m_LastDlgCtrlId = qID+(IDC_EDIT1-IDC_SPIN1);
  if (qID!=IDC_SPIN3 && qID!=IDC_EDIT3) m_DPzoo = -1; // ????
  switch (qID) {
    case IDCANCEL: // ignore escape in this application
         break; 
    case IDC_BUTTON1: { // [w><h]
      int qW = VaUt_SpinInt(this,IDC_EDIT1), qH = VaUt_SpinInt(this,IDC_EDIT2 );
      m_bInitialized = false; VaUt_SetInt( this, IDC_EDIT1, qH );
      m_bInitialized = true;  VaUt_SetInt( this, IDC_EDIT2, qW );
    }; break; 
    case IDC_BUTTON2: { const CPoint cAWH = theA_WH(); // [Center]
      if (m_Z==1) { // marker to frames' center
        m_bInitialized = false; SetDlgItemInt( IDC_EDIT4, cAWH.x>>1 ); 
        m_bInitialized = true;  SetDlgItemInt( IDC_EDIT5, cAWH.y>>1 );
      } else { // center Rf to marker
        CPoint qRng = GSCR_GetScrollRng( cAWH.x, cAWH.y, m_Rf ), qMarkerXY = theD_XY();
        int qPosX = qMarkerXY.x-(m_Rf.Width()>>1);  qPosX = mcRng( qPosX, 0, qRng.x );
        int qPosY = qMarkerXY.y-(m_Rf.Height()>>1); qPosY = mcRng( qPosY, 0, qRng.y );
        m_SBX.SetScrollPos( qPosX ); m_Rf.OffsetRect( qPosX-m_Rf.left, 0 ); // move horizontal
        m_SBY.SetScrollPos( qPosY ); m_Rf.OffsetRect( 0, qPosY-m_Rf.top ); // move vertical
        jOnChangeRdRf();
      };
    }; break; 
    case IDC_BUTTON3: { iCombo3WndTxtGet( qS ); // [Load] wildcard
      if (VaU_OpenFileDlg( this, qS, NULL, NULL, NULL )) {
        TCHAR *qNam = VaU_Sfx( qS, _T('\\') ); _tcscpy( m_Combo3Nam, qNam );
        _tcscpy( qNam, gWildCardDef ); iCombo3WndTxtSet( qS );
        m_Combo3Flg |= 2; PostMessage( cMsg0+0 ); break; // as if [wildcard editor changed]
      }; 
    }; break; 

    case IDC_EDIT1: case IDC_EDIT2: // [Width], [Height]
      if (m_bInitialized && qNC==EN_CHANGE) { // [wildcard editor changed]
        m_Combo3Flg |= 2; // PostMessage( cMsg0+0 );
      }; break;
    case IDC_EDIT3: // [Zoom]
      if (m_bInitialized && qNC==EN_CHANGE) {
        m_Z = VaUt_SpinInt( this, IDC_EDIT3 );
        int qJekyllOrHyde = m_Z>1?SW_SHOW:SW_HIDE;
        m_SBX.ShowWindow( qJekyllOrHyde ); m_SBY.ShowWindow( qJekyllOrHyde );
        m_Sta1.ShowWindow( qJekyllOrHyde );
        jReCalcRdRf();
      }; break;
    case IDC_EDIT4: case IDC_EDIT5: // [marker Location]
      if (m_bInitialized && qNC==EN_CHANGE) {
        m_X = VaUt_SpinInt( this, IDC_EDIT4 ); m_Y = VaUt_SpinInt( this, IDC_EDIT5 );
        theD_Repaint();
      }; break;
    case IDC_EDIT6: case IDC_EDIT7: // [marker Area]
      if (m_bInitialized && qNC==EN_CHANGE) {
        theD_Repaint();
      }; break;
    case IDC_EDIT8: case IDC_EDIT10: // [cut Gamut]
      if (m_bInitialized && qNC==EN_CHANGE) {
        iSetCutGamut();
        OnPresentationUpdate( false );
      }; break;
    case IDC_EDIT9: case IDC_EDIT11: // [explicit Gamut]
      if (m_bInitialized && qNC==EN_CHANGE) {
        m_BMPBuf.mWhite = VaUt_SpinInt( this, IDC_EDIT9 );
        m_BMPBuf.mBlack = VaUt_SpinInt( this, IDC_EDIT11 );
        OnPresentationUpdate( false );
      }; break;
    
    case IDC_COMBO1: // [Marker]
      if (qNC==CBN_SELCHANGE) {
        UtMnu_ClipboardTxt( NULL ); // empty the clipboard_text
        switch (theD_Marker()) {
          case 0: // [none]
            iSetMarkerArea( 1, 1, false ); break;
          case 1: // [pixel inspector]
            iSetMarkerArea( cMenuPIT[xMenuPIT].rSzX, cMenuPIT[xMenuPIT].rSzY, false ); break;
          case 2: // [profile]
          case 3: // [histogram]
            iSetMarkerArea( theA_W(), theA_H(), false ); break;
          case 4: // [rectangle]
            iSetMarkerArea( xMa4_MarkerArea.x, xMa4_MarkerArea.y, true ); break;
        }; // switch
        if (m_bInitialized) theD_Repaint();
      }; break;
    case IDC_COMBO2: // [Gamut]
      if (qNC==CBN_SELCHANGE) {
        const int cICA[] = {IDC_EDIT8,IDC_SPIN8,IDC_EDIT10,IDC_SPIN10},
                  cIC[] =  {IDC_EDIT9,IDC_SPIN9,IDC_EDIT11,IDC_SPIN11},
                  cGamut = theD_Gamut();
        const bool cExplicit = !cGamut;
        VaUt_EnableControls( m_hWnd, cIC, mcASz(cIC), cExplicit );
        VaUt_ShowControls( m_hWnd, cICA, mcASz(cICA), cExplicit?SW_HIDE:SW_SHOW );
        if (m_bInitialized) PostMessage( WM_COMMAND, MAKEWPARAM(cExplicit?IDC_EDIT9:IDC_EDIT8,EN_CHANGE) );
      }; break;
    case IDC_COMBO3: // [Wildcard]
      switch (qNC) {
        case CBN_SELCHANGE: m_Combo3Flg |= 1; // [wildcard listbox selected]
          PostMessage( cMsg0+0 );
          break; 
        case CBN_EDITCHANGE: m_Combo3Flg |= 2; // [wildcard editor changed]
          if (!theD_Img()) // if an image isn't loaded yet then
            PostMessage( cMsg0+0 ); // process this event a bit faster
          break; 
        case CBN_DROPDOWN: // don't Combo3Validate while dropped down 
          m_Combo3DropDown = true; break;
        case CBN_CLOSEUP: if (m_Combo3DropDown) { // this message might denote just lost focus
          m_Combo3Flg |= 2; // as if [wildcard editor changed]
          m_Combo3DropDown = false; }; break;
      }; break;
    case IDC_COMBO4: // [signed/unsigned]
      if (qNC==CBN_SELCHANGE) {
        theApp.PixelTypeTagSet( (int)SendDlgItemMessage(IDC_COMBO4,CB_GETCURSEL) );
        PostMessage( WM_COMMAND, MAKEWPARAM(IDC_LIST1,LBN_SELCHANGE) );
      }; break;
      
    case IDC_LIST1: // [files' list]
      if (qNC==LBN_SELCHANGE) { int qSel = m_List1.GetCaretIndex();
        //_stprintf( qS, _T("dbg WinE LBN_SELCHANGE Sel=%d"), qSel ); WriteLn( qS ); // !!!!
        if (qSel>=0) { m_List1.GetText( qSel, m_Combo3Nam );
          _tcscpy( theApp.m_sNam, m_Combo3Nam );
          theApp.LoadImage(); };
      }; break;
            
    case cMenu0+0: { // [Settings]
      CVD_MDlg1 qDlg( this );
      if (qDlg.DoModal()!=IDOK) return TRUE;
      m_Combo3Flg |= 2; // as if [wildcard editor changed]
      iCombo3WndTxtGet( qS ); TCHAR *qExt = VaU_Sfx( qS, _T('.') );
      if (*qExt && _tcsicmp(qExt+1,gFilExtDef1()) ) {
        _tcscpy( qExt+1, gFilExtDef() ); iCombo3WndTxtSet( qS ); };
      PostMessage( cMsg0+0 );
    }; return TRUE;
    case cMenu0+1: { // [Files]
      CVD_MDlg2 qDlg( this, &m_List1 );
      if (qDlg.DoModal()!=IDOK || !qDlg.m_bModifHave) return TRUE;
      m_Combo3Flg |= 2; // as if [wildcard editor changed]
      PostMessage( qDlg.m_bModifFile?cMsg0+0:cMsg0+1 ); // Msg0+1:keep List1
    }; return TRUE;
    case cMenu0+2: BPMDlg_Show(); return TRUE; // [BPM-editor]
    case cMenu0+3: UtMnu_HelpDlg(); return TRUE; // [Help]
    case cMenu0+5: m_BMPBuf.CopyToClipboardBMP(); return TRUE; // [copy bitmap to cllpboard]
    case cMenu0+6: m_BMPBuf.CopyToClipboardText(); return TRUE; // [copy ROI text to clipboard]
    case cMenu0+7: UtMnu_ClipboardTxtCopy(); return TRUE; // [copy recent stats to clipboard]
    case cMenu0+8: UtMnu_ExplorerExec(); return TRUE; // [Explore]
    #ifndef USE_DIRECT2D
      case cMenuSts0+0: case cMenuSts0+1: case cMenuSts0+2: case cMenuSts0+3:
        xMenuSts = qID-cMenuSts0; theD_Repaint(); return TRUE;
    #endif // no USE_DIRECT2D
    case cMenuCCo0+0: case cMenuCCo0+1: case cMenuCCo0+2: case cMenuCCo0+3: case cMenuCCo0+4:
      xMenuCCo = qID-cMenuCCo0; jReCalcRdRf(); return TRUE; // [Client's corner]
    case cMenuPIT0+0: case cMenuPIT0+1: case cMenuPIT0+2: case cMenuPIT0+3: case cMenuPIT0+4:
      xMenuPIT = qID-cMenuPIT0; SendDlgItemMessage( IDC_COMBO1, CB_SETCURSEL, 1 );  // [PixelInspector's type]
      PostMessage( WM_COMMAND, MAKEWPARAM(IDC_COMBO1,CBN_SELCHANGE) ); return TRUE;
    case cMenuPfT0+0: case cMenuPfT0+1: case cMenuPfT0+2: 
      xMenuPfT = qID-cMenuPfT0; SendDlgItemMessage( IDC_COMBO1, CB_SETCURSEL, 2 );
      PostMessage( WM_COMMAND, MAKEWPARAM(IDC_COMBO1,CBN_SELCHANGE) ); return TRUE; // [Profile's type]
  }; // switch
  return CDialog::OnCommand( wParam, lParam ); }

LRESULT CViewDlg::WindowProc( UINT message, WPARAM wParam, LPARAM lParam ) {
  TCHAR qS[256];
  switch (message) {
    case WM_CONTEXTMENU: 
      iDisplayLocalMenu( lParam ); return 1;
    case WM_HELP: // the main help dialog
      UtMnu_HelpDlg(); return 1;

    case WM_DESTROY: {
      m_bInitialized = false;
      KillTimer( 1 );
      // INI-profile
      theApp.VDI_SetSectName( _T("View") );
      { // [marker]
        _stprintf( qS, _T("%d; 0:none 1:pi 2:pf 3:his 4:rect"), (int)SendDlgItemMessage(IDC_COMBO1,CB_GETCURSEL) ); 
        theApp.VDI_SetStr( _T("Marker"), qS ); };
      { // grayscale
        const int cI = theD_Gamut(), cId = cI?IDC_EDIT8:IDC_EDIT9;
        _stprintf( qS, _T("%d %d %d"), cI, VaUt_SpinInt(this,cId+2), VaUt_SpinInt(this,cId) );
        theApp.VDI_SetStr( _T("Gray"), qS ); }; 
      { // [wildcard]
        theApp.VDI_SetStr( _T("FNam"), m_Combo3Nam );
        const int cN = m_Combo3.GetCount();
        _stprintf( qS, _T("%d"), cN ); theApp.VDI_SetStr( _T("DirN"), qS );
        for (int qi=0;qi<cN;qi++) { _stprintf( qS, _T("Dir%d"), qi );
          int qW, qH; TCHAR s[192]; iCombo3StrGet( qi, s, &qW, &qH );
          _stprintf( qS+32, _T("%dx%d %s"), qW, qH, s );
          theApp.VDI_SetStr( qS, qS+32 ); }; }; // for i
      { // window's size & position
        CRect qR; GetWindowRect( &qR ); if (qR.left>0 && qR.left<9999 && qR.top>0 && qR.top<9999) {
          _stprintf( qS, _T("%d %d %d %d; lft top rgt btm"), qR.left, qR.top, qR.right, qR.bottom );
          theApp.VDI_SetStr( _T("WndPos"), qS ); }; };
      { // recent [rectangle]'s marker
        _stprintf( qS, _T("%d %d; W H"), xMa4_MarkerArea.x, xMa4_MarkerArea.y );
        theApp.VDI_SetStr( _T("MarkerRect"), qS ); };
      // the objects
      m_BMPBuf.Alloc( 0, 0 );
      // non-functional GUI-objects
      if (m_FontSmall.m_hObject) m_FontSmall.DeleteObject();
      if (m_Font.m_hObject) m_Font.DeleteObject();
    }; break;

    case WM_TIMER: if (wParam==1) { // process in poset message
      if (m_bPresentationUpdate) OnPresentationUpdate( true ); // DIB changed
                            else if (m_Combo3Flg&255) PostMessage( cMsg0+0 );
      return TRUE;
    }; break;

    case WM_GETMINMAXINFO: {
      MINMAXINFO *qInfo = (MINMAXINFO*)lParam;
      qInfo->ptMinTrackSize.x = m_WIWmn; qInfo->ptMinTrackSize.y = m_WIHmn; 
    }; break;
      
    case WM_SYSCOMMAND:
      switch (wParam) {
        case SC_MINIMIZE: ShowWindow( SW_SHOWMINIMIZED ); return 0; // kb293215
      }; // switch wParam
      break;

    case WM_HSCROLL: if (lParam==(LPARAM)m_SBX.m_hWnd) { jProcessScrollMsg( &m_SBX, wParam ); return 0; }; break;
    case WM_VSCROLL: if (lParam==(LPARAM)m_SBY.m_hWnd) { jProcessScrollMsg( &m_SBY, wParam ); return 0; }; break;
    case WM_MEASUREITEM: case WM_DRAWITEM: switch (wParam) {
      case IDC_COMBO3: jProcessOwnerDrawMessage( message, wParam, lParam, 0 ); return 1;
      case IDC_LIST1:  jProcessOwnerDrawMessage( message, wParam, lParam, 1 ); return 1;
    }; break;
    
    // [wildcard changed] (editor or selected)
    case cMsg0+0:   // reassign List1
    case cMsg0+1: { // keep List1
      int qCombo3Flg = m_Combo3Flg&255; if (!qCombo3Flg) return 1;
      m_List1.SetRedraw( FALSE );
      m_Combo3Flg = qCombo3Flg<<8; // flg 'wildcard was recently changed'
      if (qCombo3Flg&1) { // [wildcard listbox selected] 
        iCombo3StrGet( m_Combo3.GetCurSel(), qS, &theApp.m_nW, &theApp.m_nH );
        m_bInitialized = false; VaUt_SetInt( this, IDC_EDIT1, theApp.m_nW );
        VaUt_SetInt( this, IDC_EDIT2, theApp.m_nH ); m_bInitialized = true;
        m_Combo3.SetEditSel( -1, -1 );
      } else { // 'Combo3Flg&2' [wildcard editor changed] 
        iCombo3WndTxtGet( qS );
      };
      int qSelOld, qCntOld = m_List1.GetCount(), qCnt; theApp.m_sPath[0] = 0;
      if (message==cMsg0+0) { qSelOld = -1; // reassign List1
        m_List1.ResetContent(); qCnt = 0; // 'Dir' with custom sorting
        struct _tfinddata_t qFindData; intptr_t qFFH = _tfindfirst( qS, &qFindData );
        if (qFFH>=0) { labRepeat:
          if (!(qFindData.attrib&_A_SUBDIR)) {
            m_List1.AddString( qFindData.name ); qCnt++; };
          if (!_tfindnext( qFFH, &qFindData )) goto labRepeat;
          _findclose( qFFH ); }; // if FFH>0
      } else { // keep List1
        qSelOld = m_List1.GetCurSel(); qCnt = qCntOld; qCntOld = 0x7FFFFFFF;
      };
      if (qCnt>0) { // sorting
        int qSel, qMaxLen; iList1Sort( &qSel, &qMaxLen );
        if (qSel<0) qSel = qSelOld; if (qSel<0 || qSel>=qCnt) qSel = qCnt-1;
        m_List1.SetSel( qSel, TRUE ); m_List1.SetCaretIndex( qSel, TRUE );
        *VaU_Sfx(qS,_T('\\')) = 0; _tcscpy( theApp.m_sPath, qS );
        int qFntWdtCnd = gFlags&4 ? 8 : 7; // ~"courier14 vs arial13" for "horz scroll-range"
        m_List1.SendMessage( LB_SETHORIZONTALEXTENT, __max(qMaxLen,10)*qFntWdtCnd, 0 );
      }; // if Cnt>0
      // reset current file's path
      theApp.LoadResetFlags();
      theApp.m_sNam = theApp.m_sPath+_tcslen(theApp.m_sPath);
      DWORD qWH = theApp.MetaInfoWxH(); if (qWH) { // try read metainformation WxH
        m_bInitialized = false; VaUt_SetInt( this, IDC_EDIT1, LOWORD(qWH) );
        VaUt_SetInt( this, IDC_EDIT2, HIWORD(qWH) ); m_bInitialized = true; };
      theApp.m_nW = VaUt_SpinInt( this, IDC_EDIT1 );
      theApp.m_nH = VaUt_SpinInt( this, IDC_EDIT2 );
      m_List1.SetRedraw( TRUE );
      if (qCnt|qCntOld) {
        m_List1.Invalidate(); m_List1.UpdateWindow();
        if (qCnt>0) // also it would update "[wildcard] combo" 
          PostMessage( WM_COMMAND, MAKEWPARAM(IDC_LIST1,LBN_SELCHANGE) );
        else // treat repeated empty List1 as "as if no valid loaded image"
          theApp.m_nSizeDiff = (int)0x80000000;
      }; // if Cnt<>CntOld
    }; return 1;
  };
  // leave
  return CDialog::WindowProc( message, wParam, lParam ); }

HCURSOR CViewDlg::OnQueryDragIcon() {
  return (HCURSOR)AfxGetApp()->LoadIcon(IDR_MAINFRAME); }
  
void CViewDlg::OnPaint() {
  CPaintDC qPaintDC( this ); // it shall be constructed anyway
  // do not call CDialog::OnPaint() for painting messages 
  if (!theD_Img()) return;
  // WriteLn( _T("-- CViewDlg::OnPaint") );
  TCHAR    qS[48]; CSize qSz;
  CRect    cRd = theD_Rd(), cRcli = theD_Rcli();
  CPoint   tl = cRcli.TopLeft();
  tDC      qDC; qDC.Create( cRcli.Width(), cRcli.Height(), GetSysColor(COLOR_BTNFACE), &qPaintDC );
  CDC     *dc = &qDC.rDC;
  #ifndef USE_DIRECT2D
    dc->SetStretchBltMode( cMenuSts[xMenuSts].rV ); // [Stretch GDI]
  #endif
  // draw 8-bit contents of BMPBuf
  m_BMPBuf.StretchDIB( dc, cRd, m_Rf );
  // stack-frame's container
  union {
    tMa1_PixelInspector rPI; // pixel inspector's record
    tMa2_Profile rPR; // profile's record
    tMa3_Histogram rHi; // histogram's record
    tMa4_RectStt rRS; // rectangle's statistics
  } Q;
  // marker's location on Rdisplay
  CPoint cXY = theD_XY();
  CPoint qMd; bool cGetCg = GSCR_GetCg( cRd, m_Rf, cXY, &qMd );
  // [marker's accessories]; typically invoke 'DrawMarkerROI=draw 'theD_ROI''
  const int cMarker = theD_Marker(); switch (cMarker) {
    case 1: { // [pixel inspector]
      Q.rPI.rHex = cMenuPIT[xMenuPIT].rHex;
      Ma1_PixelInspectorSz( &Q.rPI ); // assign drawing's width,height
      UtMnuPUp_OffsetAccessoryRect( &Q.rPI.rR ); // draw pixel_inspector's marker,text
      Ma1_PixelInspectorDraw( &qDC, &Q.rPI, iDrawMarkerROI(&qDC,cRd,true) );
    }; break;
    case 2: { // [profile] if (cGetCg)
      Ma2_ProfileSz( &Q.rPR ); // assign drawing's width,height
      UtMnuPUp_OffsetAccessoryRect( &Q.rPR.rR );
      if (xMenuCCo<2) { // change the order of horz/vert profiles
        if (cMenuPfT[xMenuPfT].rVrt) Ma2_ProfileDrawVrt( &qDC, &Q.rPR );
        if (cMenuPfT[xMenuPfT].rHrz) Ma2_ProfileDrawHrz( &qDC, &Q.rPR );
      } else {
        if (cMenuPfT[xMenuPfT].rHrz) Ma2_ProfileDrawHrz( &qDC, &Q.rPR );
        if (cMenuPfT[xMenuPfT].rVrt) Ma2_ProfileDrawVrt( &qDC, &Q.rPR );
      };
      Ma2_ProfileDrawBox( &qDC, &Q.rPR );
    }; break;
    case 3: { // [histogram]
      Ma3_HistogramSz( &Q.rHi ); // assign drawing's width,height
      UtMnuPUp_OffsetAccessoryRect( &Q.rHi.rR );
      Ma3_HistogramDraw( &qDC, &Q.rHi );
    }; break;
    case 4: { // [rectangle]
      Ma4_RectSttSz( &Q.rRS ); // assign drawing's width,height
      UtMnuPUp_OffsetAccessoryRect( &Q.rRS.rR ); // draw rectangle's marker,text
      Ma4_RectSttDraw( &qDC, &Q.rRS, iDrawMarkerROI(&qDC,cRd,false) );
    }; break;
  }; // switch marker
  // [marker's cross]
  if (cGetCg) { const int cMarkerCRad[] = {11,0,9999,22,0};
    int qMarkerCRad = cMarkerCRad[__min(cMarker,mcASz(cMarkerCRad)-1)];
    if (qMarkerCRad>0) {
      // text's visual attributes
      LOGFONT qLF; memset( &qLF, 0, sizeof(LOGFONT) );
      _tcscpy( qLF.lfFaceName, _T("ARIAL") ); qLF.lfHeight = 15; qLF.lfWeight = 800;
      qDC.SetFont( &qLF );
      qDC.SetPen( PS_SOLID, 1, cUt02_TxtColor0 );
      COLORREF qBkColor = BPM_PixelBkColor( cXY );
      if (!qBkColor) qBkColor = cUt02_BkColor0;
      dc->SetBkColor( qBkColor ); dc->SetBkMode( OPAQUE ); 
      dc->SetTextColor( cUt02_TxtColor0 );
      // text'a location
      int qt;
      qt = qMd.x-qMarkerCRad; dc->MoveTo( __max(qt,0), qMd.y ); 
      qt = qMd.x+qMarkerCRad; dc->LineTo( __min(qt,cRcli.Width()-1), qMd.y );
      qt = qMd.y-qMarkerCRad; dc->MoveTo( qMd.x, __max(qt,0) ); 
      qt = qMd.y+qMarkerCRad; dc->LineTo( qMd.x, __min(qt,cRcli.Height()-1) );
      _stprintf( qS, _T("%d"), theD_Img()[cXY.y*theA_W()+cXY.x] );
      qSz = dc->GetTextExtent( qS );
      int qX = qMd.x+2; if (qX+qSz.cx>=cRcli.Width())  qX = qMd.x-2-qSz.cx;
      int qY = qMd.y+2; if (qY+qSz.cy>=cRcli.Height()) qY = qMd.y-2-qSz.cy;
      dc->TextOut( qX, qY, qS );
    }; // if MarkerCRad
  }; // if GetCg
  { // BPM-mark
    CRect qR; if (BPM_MarkGet( &qR )) { //
      CPoint qM0; GSCR_GetCg( cRd, m_Rf, qR.TopLeft(), &qM0 );
      CPoint qM1; GSCR_GetCg( cRd, m_Rf, qR.BottomRight(), &qM1 );
      qR.SetRect( qM0, qM1 );
      int cDmnX = 4-qR.Width(), cDmnY = 4-qR.Height();
      if (cDmnX>0 || cDmnY>0) { // if BPM's marker is small
        qDC.SetPen( PS_SOLID, 1, cBPM_ColorMarkPale ); qDC.SetBrush( BS_HOLLOW, 0, 0 );
        if (cDmnX>0 && cDmnY>0) {
          qR.left -= cDmnX>>1; qR.right += cDmnX-(cDmnX>>1); 
          qR.top -= cDmnY>>1; qR.bottom += cDmnY-(cDmnY>>1);
        } else {
          if (!qR.Width()) qR.right++; if (!qR.Height()) qR.bottom++;
        };
        dc->SetBkMode( TRANSPARENT );
        dc->Rectangle( qR ); }; // if BPM's marker is small
      if (GSCR_GetCg( cRd, m_Rf,BPM_XY(), &qM0 )) { const int cR = 5; // BPM's origin
        qDC.SetPen( PS_SOLID, 1, cBPM_ColorMark ); 
        dc->MoveTo( qM0.x-cR, qM0.y-cR ); dc->LineTo( qM0.x+cR, qM0.y+cR );
        dc->MoveTo( qM0.x+cR, qM0.y-cR ); dc->LineTo( qM0.x-cR, qM0.y+cR ); }; // if _GetCg
    }; // if _MarkGet
  }; // BPM-mark
  //if (m_DPzoo==1) qMd = m_DPlst-tl; // Zdrg=1:change_gray_gamut;
  if (m_DPzoo>1) {
    qDC.SetPen( PS_SOLID, 1, cUt02_TxtColor0 );
    dc->MoveTo( qMd ); // Zdrg>1:pan_shift  
    qMd = m_DPfst-tl; dc->LineTo( qMd ); };
  // leave
  qDC.Blit( &qPaintDC, tl ); qDC.Delete(); }

BOOL CViewDlg::PreTranslateMessage( MSG *pMsg ) {
  switch (pMsg->message) {
    case WM_KEYDOWN:
      if (GetKeyState(VK_CONTROL)&0x8000) switch (pMsg->wParam) {
        case 'S': case 's': PostMessage( WM_COMMAND, cMenu0+0 ); break; // [Settings]
        case 'B': case 'b': PostMessage( WM_COMMAND, cMenu0+2 ); break; // [BPM]  
      } else if (GetKeyState(VK_SHIFT)&0x8000) switch (pMsg->wParam) {
        case VK_F2: PostMessage( WM_COMMAND, cMenu0+8 ); break; // [Explore]
      } else switch (pMsg->wParam) {
        case VK_F2: PostMessage( WM_COMMAND, cMenu0+1 ); break; // [Files]
        case VK_F3: PostMessage( WM_COMMAND, MAKEWPARAM(IDC_BUTTON3,BN_CLICKED) ); break; // [Load]
        case VK_F5: PostMessage( WM_COMMAND, MAKEWPARAM(IDC_BUTTON2,BN_CLICKED) ); break; // [Center]
        case VK_F9: iDisplayLocalMenu( 0 ); return TRUE;
      }; break; // switch wParam
    case WM_MOUSEWHEEL: { 
      int qRedirToZoom = m_DPzoo>=0 || pMsg->hwnd==GetSafeHwnd() ? 1 : 0; // redirect _MOUSEWHEEL to [Zoom]
      if (!qRedirToZoom) for (int qi=IDC_COMBO1;qi<=IDC_COMBO4;qi++) {
        HWND qHWnd = GetDlgItem(qi)->GetSafeHwnd(); if (qHWnd==pMsg->hwnd || ::IsChild(qHWnd,pMsg->hwnd)) {
        if (::SendMessage( qHWnd, CB_GETDROPPEDSTATE, 0, 0 )) qRedirToZoom = -1; break; }; }; // for i
      if (qRedirToZoom>0) { // redirect to [Zoom]; do it ~manually without a refocus
        int qMn, qMx; SendDlgItemMessage( IDC_SPIN3, UDM_GETRANGE32, (WPARAM)&qMn, (LPARAM)&qMx );
        int qD = GET_WHEEL_DELTA_WPARAM( pMsg->wParam ), qZ = m_Z+mcRng( qD, -1, 1 ); 
        if (qZ>=qMn && qZ<=qMx) { m_bInitialized = false; SetDlgItemInt( IDC_EDIT3, qZ ); m_bInitialized = true;
          PostMessage( WM_COMMAND, MAKEWPARAM(IDC_EDIT3,EN_CHANGE) ); }; // if Z in the range
        return TRUE; }; // if RedirectToZoom>0
    }; break;
  }; // switch message
  return CDialog::PreTranslateMessage( pMsg ); }

void CViewDlg::OnLButtonDown( UINT nFlags, CPoint point ) {
  if (m_Rd.PtInRect( point )) { // within RECTclient
    CPoint qMf; if (GSCR_GetCf( m_Rd, m_Rf, point, &qMf )) { // within Rd
      // update [marker's location] in frame's coordinates
      m_bInitialized = false; SetDlgItemInt( IDC_EDIT4, qMf.x );
      m_bInitialized = true;  SetDlgItemInt( IDC_EDIT5, qMf.y ); };
    m_DPzoo = m_Z; if (m_Z>1) {
      SetCursor( LoadCursor(NULL,IDC_SIZEALL) );	
      SetCapture(); m_DPfst = m_DPlst = point; };
  } else
    m_DPzoo = -1;
  CDialog::OnLButtonDown( nFlags, point ); } // leave

void CViewDlg::OnLButtonUp( UINT nFlags, CPoint point ) {
  if (m_DPzoo>0) { if (m_DPzoo>1) {
    SetCursor( NULL ); ReleaseCapture(); theD_Repaint(); }; m_DPzoo = 0; };
  CDialog::OnLButtonUp( nFlags, point ); }

void CViewDlg::OnMouseMove( UINT nFlags, CPoint point ) {
  if (m_DPlst!=point) switch (m_DPzoo) { // dragging=zoom
    case 0: case -1: break; 
    case 1: break; // change [Gamut] perhaps ...
    default: { // >1: change Rf
      // note for Cfo,Cfn here aren't correct in absolute; the delta Cfd=Cfo-Cfn is correct
      CPoint qCfo; GSCR_GetCf( m_Rd, m_Rf, m_DPlst, &qCfo ); // frame's coordinates old
      CPoint qCfn; GSCR_GetCf( m_Rd, m_Rf, point, &qCfn ); // frame's coordinates new
      CPoint qCfd = qCfo-qCfn, // pan's shift in frame's coordinates
             qSPo( m_SBX.GetScrollPos(), m_SBY.GetScrollPos() ), // scroll_pos old 
             qSPn = qSPo+qCfd; // scroll_pos new 
      // check scroll-range limits initially
      CPoint cSR( m_SBX.GetScrollLimit()-1, m_SBY.GetScrollLimit()-1 );
      #ifdef _DEBUG1
        CPoint cSR0( GSCR_GetScrollRng(m_W,m_H,m_Rf) );
        if (cSR!=cSR0) { TCHAR qS[80];
          _stprintf( qS, _T("ERR CViewDlg::OnMouseMove SR{%d:%d}<>SR0{%d:%d}"),
          cSR.x, cSR.y, cSR0.x, cSR0.y ); WriteLn( qS ); cSR = cSR0; };
      #endif    
      if (qSPn.x<0) qSPn.x = 0; else if (qSPn.x>cSR.x) qSPn.x = cSR.x;
      if (qSPn.y<0) qSPn.y = 0; else if (qSPn.y>cSR.y) qSPn.y = cSR.y;
      // pan's delta in frame's coordinates
      qCfd = qSPn-qSPo;
      if (qCfd.x|qCfd.y) { m_DPlst = point; CPoint cAWH = theA_WH();
        m_Rf.OffsetRect( qCfd );
        CSCR_SentinelRf( cAWH.x, cAWH.y, &m_Rf );
        m_SBX.SetScrollPos( m_Rf.left ); m_SBY.SetScrollPos( m_Rf.top );
        jOnChangeRdRf();
      }; // if Cfd.x or Cfd.y
    }; break;
  }; // switch
  CDialog::OnMouseMove( nFlags, point ); }

void CViewDlg::OnSize( UINT nType, int cx, int cy ) {
  CDialog::OnSize( nType, cx, cy );
  // WM_SIZE might be early in creation process (before childs initialize)
  if (!m_bInitialized) return;
  iOnSizeControls( cx, cy );
  int qCurSel = m_List1.GetCurSel(); if (qCurSel>=0) m_List1.SetTopIndex( qCurSel );
  jReCalcRdRf( 1 ); } // bit#0: called from OnSize

void CViewDlg::OnDropFiles( HDROP hDropInfo ) {
  // TODO: Add your message handler code here and/or call default
  TCHAR qS[256];
  int qCnt = ::DragQueryFile( hDropInfo, 0xFFFFFFFF, NULL, 0 );
  bool qFil = false, qDir = false;
  for (int qi=0;qi<qCnt;qi++) {
    ::DragQueryFile( hDropInfo, qi, qS, 192 );
    int qAttr = ::GetFileAttributes( qS ); if (qAttr==-1) continue;
    if (qAttr & FILE_ATTRIBUTE_DIRECTORY) {
      _tcscpy( VaU_AddBackSlash(qS), gWildCardDef ); m_Combo3.SetWindowText( qS );
      qDir = true; break; };
    // treat a file
    TCHAR *qNam = VaU_Sfx( qS, _T('\\') ); if (qNam<=qS) continue;
    if (_tcslen(qNam)>63) continue;
    TCHAR *qExt = VaU_Sfx( qNam, _T('.') ); if (!*qExt) continue;
    if (_tcsicmp( qExt+1, gFilExtDef() )) continue;
    if (!qFil) m_List1.ResetContent(); // 1-st file
    m_List1.AddString( qNam );
    if (!qFil) { qFil = true; _tcscpy( m_Combo3Nam, qNam ); // 1-st file
      _tcscpy( qNam, gWildCardDef ); m_Combo3.SetWindowText( qS ); };
  }; // for
  ::DragFinish( hDropInfo ); // Free memory allocated for drop structure
  CDialog::OnDropFiles( hDropInfo );
  if (!(qDir||qFil)) return;
  m_Combo3Flg |= 2; // as if [wildcard editor changed]
  if (m_List1.GetCount()<2) qDir = true;
  // Msg0+1:keep List1
  PostMessage( qDir ? cMsg0+0 : cMsg0+1 ); } 

/////////////////////////////////////////////////////////////////////////////
// CViewDlg ~API-exports
void CViewDlg::GetSBRects( CRect *ARX, CRect *ARY ) { // -- scrollbars
  m_SBX.GetWindowRect( ARX ); ScreenToClient( ARX ); m_SBY.GetWindowRect( ARY ); ScreenToClient( ARY ); }

void CViewDlg::SetImgDim() { // alloc bufs for new WxH
  // don't rely on ~VaUt_SpinInt(this,IDC_EDIT1) here
  const BITMAPINFOHEADER *cBMI = &m_BMPBuf.mBMPHdr.rBMI;
  const CPoint cOldWH( cBMI->biWidth, cBMI->biHeight ), cAWH = theA_WH(); 
  if (cOldWH.x==cAWH.x && cOldWH.y==cAWH.y) return; // nothing todo
  if (!theApp.m_nSizeDiff && gFlags&1) { m_bInitialized = false; // gFlags#0:adim
    VaUt_SetInt( this, IDC_EDIT1, cAWH.x ); VaUt_SetInt( this, IDC_EDIT2, cAWH.y );  // [Width,Height]
    if (cOldWH.x>4 && cOldWH.y>4) { m_X = m_X*cAWH.x/cOldWH.x; m_Y = m_Y*cAWH.y/cOldWH.y; } // [Marker X,Y]
                             else { m_X = cAWH.x>>1; m_Y = cAWH.y>>1; };
    SetDlgItemInt( IDC_EDIT4, m_X ); SetDlgItemInt( IDC_EDIT5, m_Y );
    m_bInitialized = true; }; // if SizeDiff=0
  m_BMPBuf.Alloc( cAWH.x, cAWH.y );
  jReCalcRdRf( 2 ); } // bit#1: called from SetImgDim (scroll to the center)

void CViewDlg::OnLoadImage( int ATag, const void *AImg ) { // -- called by App.LoadImage
  //WriteLn( _T("-- CViewDlg::OnLoadImage") );
  _tcscpy( m_Combo3Nam, theApp.m_sNam ); // remember selected file-name
  // stats
  TCHAR qS[256], *qCh = _tcsrchr( m_Combo3Nam, _T('.') ); *qCh = 0;
  const TCHAR *cSizeDiff; int qSizeDiff = theApp.m_nSizeDiff;
  if (qSizeDiff==(int)0x80000000) cSizeDiff = _T("?"); else if (qSizeDiff<0) cSizeDiff = _T(">");
  else if (qSizeDiff>0) cSizeDiff = _T("<"); else cSizeDiff = _T("");
  _stprintf( qS, _T("%s%s %05d..%05d =%d ~%0.2f"), cSizeDiff,
    m_Combo3Nam, theApp.m_nMn, theApp.m_nMx, theApp.m_nMed, theApp.m_fAve );
  *qCh = _T('.'); SetWindowText( qS );
  // if [cut Gamut] 
  if (theD_Gamut()) iSetCutGamut();
  // set image data
  CPoint cAWH = theA_WH();
  SendDlgItemMessage( IDC_SPIN4, UDM_SETRANGE32, 0, cAWH.x-1 ); // [marker Locattion X]
  SendDlgItemMessage( IDC_SPIN5, UDM_SETRANGE32, cAWH.y-1, 0 ); // [marker Locattion Y]
  m_BMPBuf.ImageToBMP( cAWH.x, cAWH.y, ATag, AImg );
  if (!(m_Combo3Flg&0xFF00)) { theD_Repaint(); return; };
  if (!m_Combo3DropDown) jCombo3Validate();
  // recalculate "RECTdisplay", "RECTframe"
  jReCalcRdRf(); }

void CViewDlg::OnPresentationUpdate( bool AImmediately ) { // -- stored image to stored DIB-buffer
  if (!AImmediately) { m_bPresentationUpdate = true; return; };
  m_bPresentationUpdate = false;
  m_BMPBuf.ImageToBMP( 0, 0, theA_GrayMn()<0?0:1, NULL );
  theD_Repaint(); }

bool CViewDlg::SuggestWHforNP( int ANP, int *AW, int *AH ) { // -- check stored wildcards
  int cC = m_Combo3.GetCount(); for (int qi=0;qi<cC;qi++) { int qW, qH; iCombo3StrGet( qi, NULL, &qW, &qH );
  if (qW*qH==ANP) { *AW = qW; *AH = qH; return true; }; }; return false; }
