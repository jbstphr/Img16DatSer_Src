// Dataseries-Viewer MFC
// (c) LGPL v3.0; by S. Bochkarev
// file VD_NMDlg2.cpp : dialog [reserved]
#include "StdAfx.h"
#include "ViewApp.h" // the application-object
#include "VD_MDlg3.h" // dialog [Help]
#include "VD_NMDlg2.h"

static CVD_NMDlg2 *ixFCRDlg = NULL; // The one and only CVD_NMDlg2 object

IMPLEMENT_DYNAMIC(CVD_NMDlg2, CDialog)

CVD_NMDlg2::CVD_NMDlg2( CWnd* pParent):CDialog(CVD_NMDlg2::IDD, pParent) {
  m_bInitialized = false; }

CVD_NMDlg2::~CVD_NMDlg2() {
  ixFCRDlg = NULL; }

void CVD_NMDlg2::DoDataExchange( CDataExchange* pDX ) {
  CDialog::DoDataExchange( pDX );
}

BEGIN_MESSAGE_MAP(CVD_NMDlg2, CDialog)
END_MESSAGE_MAP()

//// CVD_NMDlg2 services ////
static void iHelp( CVD_NMDlg2 *ADlg ) {
  HelpDlg( _T("BPMM editor"), ADlg, _T("") ); }

static const int ciMenu0 = 9000;

static void iDisplayLocalMenu( CVD_NMDlg2 *ADlg, LPARAM APoint  ) {
  HMENU qMenu = CreatePopupMenu();
  AppendMenu( qMenu, MF_STRING, ciMenu0+0, _T("Help") );
  // display menu
  VaUt_DisplayMenu( ADlg, APoint, qMenu ); }

//// CVD_NMDlg2 message handlers ////

BOOL CVD_NMDlg2::OnInitDialog() {
  CDialog::OnInitDialog();
  // leave
  m_bInitialized = true; return TRUE; }

BOOL CVD_NMDlg2::OnCommand( WPARAM wParam, LPARAM lParam ) {
  WORD  qNC = HIWORD(wParam), qID = LOWORD(wParam);
  switch (qID) {
    case IDOK: case IDCANCEL:
      DestroyWindow(); return TRUE; // Triggers cleanup sequence
    case ciMenu0:
      iHelp( this ); return TRUE;
  };
  // leave	
  return CDialog::OnCommand( wParam, lParam ); }

LRESULT CVD_NMDlg2::WindowProc( UINT message, WPARAM wParam, LPARAM lParam ) {
  switch (message) {
    case WM_CONTEXTMENU: iDisplayLocalMenu( this, lParam ); return 1;
    case WM_HELP: iHelp( this ); return 1;
    case WM_DESTROY:
      break;
  };
  // leave	
  return CDialog::WindowProc( message, wParam, lParam ); }

BOOL CVD_NMDlg2::PreTranslateMessage( MSG *pMsg ) {
  switch (pMsg->message) {
    case WM_KEYDOWN:
      switch (pMsg->wParam) {
        case VK_F9: iDisplayLocalMenu( this, 0 ); return TRUE;
      };
  };
  // leave	
  return CDialog::PreTranslateMessage( pMsg ); }

void CVD_NMDlg2::PostNcDestroy() {
  CDialog::PostNcDestroy();
  delete this; }

//// CVD_NMDlg2 global exports ////
