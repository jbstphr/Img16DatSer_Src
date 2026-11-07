// Dataseries-Viewer MFC
// (c) LGPL v3.0; by S. Bochkarev
// file VD_MDlg3.cpp : dialog [Help]
#include "StdAfx.h"
#include "VD_Ut.h"
#include "ViewApp.h" // the application-object
#include "VD_UtViewDlg.h" // adhoc main dialog controls' access
#include "VD_MDlg3.h"

IMPLEMENT_DYNAMIC(CVD_MDlg3, CDialog)

CVD_MDlg3::CVD_MDlg3( CWnd* pParent) : CDialog(CVD_MDlg3::IDD, pParent) {}

CVD_MDlg3::~CVD_MDlg3() {}

void CVD_MDlg3::DoDataExchange( CDataExchange* pDX ) {
  CDialog::DoDataExchange( pDX );
}

BEGIN_MESSAGE_MAP(CVD_MDlg3, CDialog)
  ON_WM_CTLCOLOR()
END_MESSAGE_MAP()

// CVD_MDlg3 services

void CVD_MDlg3::iGetCap( int ATopic, TCHAR *AStr ) { // -- get topic's caption (for multitopics only)
  const TCHAR *cS = m_Str[ATopic]; int qL = (int)(_tcschr(cS,_T('\n'))-cS);
  _tcsncpy( AStr, cS, qL ); AStr[qL] = 0; }

void CVD_MDlg3::iGetBody( int ATopic, TCHAR *AStr ) { // -- get topic's body
  const TCHAR *cS = m_Str[ATopic];
  if (n_Str>1) { cS = _tcschr(cS,_T('\n'));
    if (!cS) { AStr[0] = 0; return; }; cS++; };
  while (1) {
    const TCHAR *cE = _tcschr( cS, _T('\n') );
    int qL = cE? (int)(cE-cS) : (int)_tcslen( cS );
    _tcsncpy( AStr, cS, qL ); AStr += qL;
    if (!cE) { *AStr = 0; break; }; cS = cE+1;
    *AStr++ = _T('\r'); *AStr++ = _T('\n'); }; }

// CVD_MDlg3 message handlers

static const int cTxtBuf = 2048; // reserve text buffer in steck

BOOL CVD_MDlg3::OnInitDialog() {
  CDialog::OnInitDialog();
  m_BkBrush.CreateSolidBrush( cUt02_BkColor0 );
  LOGFONT   qLogFont; memset( &qLogFont, 0, sizeof(LOGFONT) );
  qLogFont.lfHeight = 14; qLogFont.lfWeight = 400;
  _tcscpy( qLogFont.lfFaceName, theApp.StandardMonoFontName() );
  m_Font.CreateFontIndirect( &qLogFont );
  GetDlgItem(IDC_EDIT1)->SetFont( &m_Font );
  CWnd *qParent = GetParent();
  TCHAR qS0[60]; if (m_Title) _tcsncpy( qS0, m_Title, mcASz(qS0)-1 );
                         else qParent->GetWindowText( qS0, mcASz(qS0)-1 );
  qS0[mcASz(qS0)-1] = 0;
  TCHAR qS[cTxtBuf];
  if (n_Str==1) {
    SetWindowText( _T("Help") ); const int cIDs[] = {IDC_COMBO1};
    VaUt_EnableControls( m_hWnd, cIDs, mcASz(cIDs), false );
    SendDlgItemMessage( IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)qS0 );
  } else {
    _stprintf( qS, _T("Help [%s]"), qS0 ); SetWindowText( qS );
    for (int qi=0;qi<n_Str;qi++) { iGetCap( qi, qS );
      SendDlgItemMessage( IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)qS ); }
  };
  iGetBody( 0, qS );
  SetDlgItemText( IDC_EDIT1, qS );
  SendDlgItemMessage( IDC_COMBO1, CB_SETCURSEL, 0 );
  // initially locate help-window
  CRect qR; GetWindowRect ( &qR ); // this window's rect 
  int cW = qR.Width(), cH = qR.Height();
  SystemParametersInfo( SPI_GETWORKAREA, 0, &qR, 0 ); // display's work area
  int qXmx = qR.right-cW, qYmx = qR.bottom-cH;
  qParent->GetWindowRect( &qR ); // parent's rect
  int qX = qR.right-33; if (qX>qXmx) qX = qXmx;
  int qY = qR.bottom-33; if (qY>qYmx) qY = qYmx;
  MoveWindow( qX, qY, cW, cH ); 
  // leave
  return TRUE; } // return TRUE unless you set the focus to a control

BOOL CVD_MDlg3::OnCommand( WPARAM wParam, LPARAM lParam ) {
  TCHAR qS[cTxtBuf];
  WORD qNC = HIWORD(wParam), qID = LOWORD(wParam);
  switch (qID) {
    case IDC_COMBO1: if (qNC==CBN_SELCHANGE) {
      iGetBody( (int)SendDlgItemMessage(IDC_COMBO1,CB_GETCURSEL), qS );
      SetDlgItemText( IDC_EDIT1, qS );
    }; break;
  };
  // leave	
  return CDialog::OnCommand( wParam, lParam ); }

LRESULT CVD_MDlg3::WindowProc( UINT message, WPARAM wParam, LPARAM lParam ) {
  switch (message) {
    case WM_HELP: return 1;
    case WM_DESTROY:
      m_Font.DeleteObject();
      m_BkBrush.DeleteObject();
      break;
  };
  // leave	
  return CDialog::WindowProc( message, wParam, lParam ); }

BOOL CVD_MDlg3::PreTranslateMessage( MSG *pMsg ) {
  // leave	
  return CDialog::PreTranslateMessage( pMsg ); }

HBRUSH CVD_MDlg3::OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor ) {
  HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
  switch (nCtlColor) {
    case CTLCOLOR_STATIC:
      if (pWnd->GetDlgCtrlID()!=IDC_STATIC1) break;
      pDC->SetBkMode( TRANSPARENT );
    case CTLCOLOR_DLG:
      pDC->SetBkColor( cUt02_BkColor0 ); 
      return (HBRUSH)m_BkBrush.GetSafeHandle();
  }; return hbr; }

//// CVD_MDlg3 global exports ////

void HelpDlg( const TCHAR *ATitle, CWnd *AParent, const TCHAR *AStr0, // -- modal help-dialog
  const TCHAR *AStr1, const TCHAR *AStr2, const TCHAR *AStr3, const TCHAR *AStr4 ) {
  if (!IsWindow(AParent->m_hWnd)) return;
  CVD_MDlg3 qDlg( AParent );
  qDlg.m_Title = ATitle; qDlg.n_Str = 0; 
  if (AStr0) qDlg.m_Str[qDlg.n_Str++] = AStr0;
  if (AStr1) qDlg.m_Str[qDlg.n_Str++] = AStr1;
  if (AStr2) qDlg.m_Str[qDlg.n_Str++] = AStr2;
  if (AStr3) qDlg.m_Str[qDlg.n_Str++] = AStr3;
  if (AStr4) qDlg.m_Str[qDlg.n_Str++] = AStr4;
  qDlg.DoModal(); }

