// Dataseries-Viewer MFC
// (c) LGPL v3.0; by S. Bochkarev
// file VD_MDlg1.cpp : dialog [Settings]
#include "StdAfx.h"
#include "VD_Ut.h"
#include "ViewApp.h" // the application-object
#include "VD_UtViewDlg.h" // adhoc main dialog controls' access
#include "VD_UtMnuPUp.h" // CViewDlg local pop-up menu
#include "VD_MDlg1.h"
#include "VD_MDlg3.h" // dialog [Help]

IMPLEMENT_DYNAMIC(CVD_MDlg1, CDialog)

CVD_MDlg1::CVD_MDlg1( CWnd* pParent) : CDialog(CVD_MDlg1::IDD, pParent) {}

CVD_MDlg1::~CVD_MDlg1() {}

void CVD_MDlg1::DoDataExchange( CDataExchange* pDX ) {
  CDialog::DoDataExchange( pDX );
  DDX_Control( pDX, IDC_LIST1, m_List1 );
}

BEGIN_MESSAGE_MAP(CVD_MDlg1, CDialog)
END_MESSAGE_MAP()

// CVD_MDlg1 services
static void iHelp( CVD_MDlg1 *ADlg ) {
  HelpDlg( NULL, ADlg,
    _T("Commons\n\
= Markers' gutter\t~50..200\n\
  <in pixels> width of markers 2:Profile, 3:Histogram\n\n\
= Base bk-color\t~50..200\n\
  <in LSB> brightness of drawing accessories\n\n\
= Bit_flags\n\
  - 0:\"detect w,h by image_file's size\"\t- affects button [>>]; the order of checks:\n\
     the same FileSize in stored wildcards\n\
     if FS mod sqrt(FS)=0 then w:=h:=sqrt(FS)\n\
     if FS mod w=0 then h:=FS div w\n\
     if FS mod h=0 then w:=FS div h\n\
  -  :\"plain txt ... := ...\"  \t- affect drawing accessories\n\
  - 3:\"files' sort := WINDOWS\"\t- default is alphabet_order\n\
  - 4:\"statiscics := details\" \t- affects markers 3:Histogram, 4:Rectangle\n\
  - 5:\"WinE DOS-paths := On\"  \t- show DOS-paths, force WinExplorer's stub\n\
     (affects under WinE)" ),
    _T("Application domain\n\
= Filenames' extension\n\
  - *.raw\t~ ImageJ, AdobePhotoShop\n\
  - *.bin\t~ GEV cameras\n\n\
= Stretch copying BMP ~>d*d\n\
  stretch clipboard CF_DIB[h][w] as \"d*d/w*h\" if \"w*h>d*d\"\n\
  (this was imposed ~for WinE's clipboard limitation)"),
    _T("Marker 3:Histogram\n\
= Visible axes\n\
  - abscissa\t- by \"GrayScale_mn..mx\"\n\
  - ordinate\t- by \"0..Occurencies_grid\"") ); }

static const int ciMenu0 = 9000;

static void iDisplayLocalMenu( CVD_MDlg1 *ADlg, LPARAM APoint ) {
  HMENU qMenu = CreatePopupMenu();
  AppendMenu( qMenu, MF_STRING, ciMenu0+0, _T("Help") );
  // display menu
  VaUt_DisplayMenu( ADlg, APoint, qMenu ); }

// CVD_MDlg1 message handlers
BOOL CVD_MDlg1::OnInitDialog() {
  CDialog::OnInitDialog();
  // edits
  SendDlgItemMessage( IDC_SPIN1, UDM_SETRANGE32, 55, 222 ); // [Gutter]
  SetDlgItemInt( IDC_EDIT1, gPlot );
  SendDlgItemMessage( IDC_SPIN5, UDM_SETRANGE32, cUt02_BkColorBaseMn, cUt02_BkColorBaseMx ); // [base bk-color]
  SetDlgItemInt( IDC_EDIT5, gBkg );
  SendDlgItemMessage( IDC_SPIN6, UDM_SETRANGE32, 256, (1<<16)-1 ); // [bmp cpy mx dim]
  SetDlgItemInt( IDC_EDIT6, gBmpCpyMxDim );
  SendDlgItemMessage( IDC_SPIN2, UDM_SETRANGE32, 55, 1<<18 ); // [Occurencies max]
  SetDlgItemInt( IDC_EDIT2, gOccMax );
  SendDlgItemMessage( IDC_SPIN3, UDM_SETRANGE32, -(1<<15), (1<<16)-2 ); // [Gray min]
  SetDlgItemInt( IDC_EDIT3, gOccGrayMn );
  SendDlgItemMessage( IDC_SPIN4, UDM_SETRANGE32, -(1<<15)+1, (1<<16)-1 ); // [Gray max]
  SetDlgItemInt( IDC_EDIT4, gOccGrayMx );
  // checks
  // m_List1.SetFont( &theDlg.m_FontSmall );
  m_List1.AddString( _T("0:<try to> detect w,h by file's size") );
  m_List1.AddString( _T("1:plain txt color := darken") );
  m_List1.AddString( _T("2:plain txt font := enlarge") );
  m_List1.AddString( _T("3:files' sort := WINDOWS") );
  m_List1.AddString( _T("4:statistics := details") );
  m_List1.Enable( m_List1.AddString( _T("5:WinE DOSpaths := On (need restart)") ),
    theApp.m_bUnderWinE );
  int cC = m_List1.GetCount(); for (int qi0=0;qi0<cC;qi0++)
    m_List1.SetCheck( qi0, (gFlags&(1<<qi0))!=0 );
  // combo
  int qSel = -1; for (int qi2=0;qi2<cFilExtsN;qi2++) {
    if (!_tcscmp( gFilExtDef(), cFilExts[qi2] )) qSel = qi2;
    SendDlgItemMessage( IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)cFilExts[qi2] ); };
  if (qSel<0) SetDlgItemText( IDC_COMBO1, gFilExtDef() );
         else SendDlgItemMessage( IDC_COMBO1, CB_SETCURSEL, qSel );
  // leave
  return TRUE; } // return TRUE unless you set the focus to a control

BOOL CVD_MDlg1::OnCommand( WPARAM wParam, LPARAM lParam ) {
  WORD  qNC = HIWORD(wParam), qID = LOWORD(wParam);
  switch (qID) {
    case IDOK: {
      gPlot = VaUt_SpinInt( this, IDC_EDIT1 );
      gBkg = VaUt_SpinInt( this, IDC_EDIT5 );
      gBmpCpyMxDim = VaUt_SpinInt( this, IDC_EDIT6);
      gFlags = 0; int cC = m_List1.GetCount(); for (int qi=0;qi<cC;qi++)
        if (m_List1.GetCheck( qi )) gFlags |= 1<<qi;
      gOccMax = VaUt_SpinInt( this, IDC_EDIT2 );
      gOccGrayMn = VaUt_SpinInt( this, IDC_EDIT3 );
      gOccGrayMx = VaUt_SpinInt( this, IDC_EDIT4 );
      GetDlgItemText( IDC_COMBO1, gFilExtDef(), 5 );
    }; break;

    case ciMenu0+0: iHelp( this ); return TRUE;
  };
  // leave	
  return CDialog::OnCommand( wParam, lParam ); }

LRESULT CVD_MDlg1::WindowProc( UINT message, WPARAM wParam, LPARAM lParam ) {
  switch (message) {
    case WM_CONTEXTMENU: iDisplayLocalMenu( this, lParam ); return 1;
    case WM_HELP: iHelp( this ); return 1;
  };
  // leave	
  return CDialog::WindowProc( message, wParam, lParam ); }

BOOL CVD_MDlg1::PreTranslateMessage( MSG *pMsg ) {
  if (pMsg->message==WM_KEYDOWN && pMsg->wParam==VK_F9) {
    iDisplayLocalMenu( this, 0 ); return TRUE; };
  // leave	
  return CDialog::PreTranslateMessage( pMsg ); }
