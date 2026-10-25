// Dataseries-Viewer MFC
// (c) LGPL v3.0; by S. Bochkarev
// file VD_MDlg2.cpp : dialog [Files]
#include "StdAfx.h"
#include "VD_Ut.h"
#include "ViewApp.h" // the application-object
#include "VD_UtViewDlg.h" // adhoc main dialog controls' access
#include "VD_UtTransf.h" // [crop,rotate,flip]
#include "VD_MDlg3.h" // dialog [Help]
#include "VD_MDlg2.h"

IMPLEMENT_DYNAMIC(CVD_MDlg2, CDialog)

CVD_MDlg2::CVD_MDlg2( CWnd *pParent, CListBox *ALB ) : CDialog( CVD_MDlg2::IDD, pParent ) {
  m_LB = ALB; m_bModifHave = false; m_bModifFile = true;
  m_bInitialized = false; }

CVD_MDlg2::~CVD_MDlg2() {}

void CVD_MDlg2::DoDataExchange( CDataExchange* pDX ) {
  CDialog::DoDataExchange( pDX );
}

BEGIN_MESSAGE_MAP(CVD_MDlg2, CDialog)
END_MESSAGE_MAP()

// CVD_MDlg2 services
static void iHelp( CVD_MDlg2 *ADlg ) {
  HelpDlg( NULL, ADlg, _T("\
\tTerminology:\n\
 - 'Frame'\t- inp image 'as is'\n\
 - 'Image'\t- transformed 'Frame'.\n\n\
\tApply to:\n\
= current selection in files'list\n\
= whole files'list\n\n\
\tThe operations:\n\
= 0:Frame => Image (in the following order):\n\
  - Crop margins\n\
  - Stretch to specified WxH\n\
  - Rotate\n\
  - Flip\n\
  - Write \"./SubDir/image_file\" or overwrite existing \"image_file\"\n\n\
= 1:Write simple average frame _ave\n\n\
= 2:Frame => BMP (cut grayscale image_file<s> to 8-bit-gray *.BMP)\n\n\
= 3:Delete image_file<s>\n\n\
= 4:OS-specific 'open'\n\
  - Invokes ~ShellExecute(\"open\",image_file).\n\n\
= 5,6:Hide <un>selected file_name<s> in the files'list\n\
  This is to compare histograms, profiles, etc of two image_files,\n\
  when those aren't neighbors in files'list initially.\n\
  - a) <by this operation> make them neighbors in the list,\n\
  - b) <by arrow_keys up|down> move files'list_caret and watch\n\
    quantitative changes between two charts or between two images.") ); }

static const int ciMenu0 = 9000;

static void iDisplayLocalMenu( CVD_MDlg2 *ADlg, LPARAM APoint ) {
  HMENU qMenu = CreatePopupMenu();
  AppendMenu( qMenu, MF_STRING, ciMenu0+0, _T("Help") );
  // display menu
  VaUt_DisplayMenu( ADlg, APoint, qMenu ); }

int CVD_MDlg2::iGetSels( int ATag, short *ASelIdxs ) { // -- get selections
  int             qCnt = m_LB->GetCount();
  if (qCnt<=0) return 0; if (qCnt>128) qCnt = 128;
  if (ATag==1) { // Tag=1:all in the list
    if (ASelIdxs) for (int qi=0;qi<qCnt;qi++) ASelIdxs[qi] = qi;
    return qCnt; };
  // Tag=0:selected
  int qIdxs[128]; qCnt = m_LB->GetSelItems( 128, qIdxs ); if (qCnt>0) {
    if (ASelIdxs) for (int qi=0;qi<qCnt;qi++) ASelIdxs[qi] = qIdxs[qi]; return qCnt; };
  qCnt = m_LB->GetCaretIndex();
  if (qCnt>=0) { if (ASelIdxs) ASelIdxs[0] = qCnt; return 1; };
  return 0; }

static int ixSels[] = { // recently selected items in combo-boxes
  0, // #0: COMBO1 [apply to]
  0, // #1: COMBO2 [the operation]
  0, // #2: Cut Top
  0, // #3: Cut Lft
  0, // #4: Cut Rgt
  0, // #5: Cut Btm
  0, // #6: COMBO4 [Rotation]
  0, // #7: flip
  0, // #8: COMBO5 [overwrite]
  0, // #9 [stretch W]
  0  // #10: [stretch H]
};  

static void iOnCombo2SelChange( CVD_MDlg2 *ADlg ) { // [operation]
  bool cTransfEn = !ixSels[1];
  const int cIDsTransf[] = {IDC_EDIT1,IDC_SPIN1,IDC_EDIT2,IDC_SPIN2,
    IDC_EDIT3,IDC_SPIN3,IDC_EDIT4,IDC_SPIN4,IDC_EDIT6,IDC_SPIN6,IDC_EDIT7,IDC_SPIN7,
    IDC_COMBO4, IDC_CHECK1, IDC_CHECK2, IDC_COMBO5, IDC_EDIT5 };
  VaUt_EnableControls( ADlg->m_hWnd, cIDsTransf, mcASz(cIDsTransf), cTransfEn );
  bool cOverSelShow = cTransfEn; const int cIDsOverSel[] = {IDC_COMBO5};
  VaUt_ShowControls( ADlg->m_hWnd, cIDsOverSel, mcASz(cIDsOverSel), cOverSelShow?SW_SHOW:SW_HIDE );
  bool cOverInfShow = ixSels[1]<3; const int cIDsOverInf[] = {IDC_EDIT5};
  VaUt_ShowControls( ADlg->m_hWnd, cIDsOverInf, mcASz(cIDsOverInf), cOverInfShow?SW_SHOW:SW_HIDE ); }

static const TCHAR
  *cOpsAll[] = { _T("0:Frame => Image"), _T("1:Average frame"),
    _T("2:Frame => BMP"), _T("3:Delete files"), _T("4:OS-specific 'open'") },
  *cOpsSel[] = { _T("5:Hide unselected item<s>"), _T("6:Hide selected item<s>") };

static void iOnCombo1SelChange( CVD_MDlg2 *ADlg ) { // [apply to]
  // update the list of operations
  CComboBox *qCB = (CComboBox*)(ADlg->GetDlgItem( IDC_COMBO2 ));
  int qCntNew = mcASz(cOpsAll); if(!ixSels[0]) qCntNew += mcASz(cOpsSel);
  int qCntOld = qCB->GetCount(); if (qCntOld==qCntNew) return;
  qCB->SetRedraw( FALSE );
  if (qCntNew<qCntOld) {
    for (int qi=qCntOld-1;qi>=qCntNew;qi--) qCB->DeleteString( qi );
    if (ixSels[1]>=qCntNew) { ixSels[1] = 0; qCB->SetCurSel( 0 ); iOnCombo2SelChange( ADlg ); };
  } else {
    for (int qi=0;qi<mcASz(cOpsSel);qi++) qCB->AddString( cOpsSel[qi] );
    qCB->SetCurSel( ixSels[1] );
  };
  qCB->SetRedraw( TRUE ); qCB->Invalidate(); qCB->UpdateWindow(); }

static void iOnCombo5SelChange( CVD_MDlg2 *ADlg ) { // [overwrite]
  bool cOverwrite = ixSels[8]>0;
  ADlg->SendDlgItemMessage( IDC_EDIT5, EM_SETREADONLY, (WPARAM)cOverwrite, 0 ); }

static void itUtTransf( CVD_MDlg2 *ADlg, tUtTransf *ARec ) { // get 'tUtTransf'
  memset( ARec, 0, sizeof(tUtTransf) );
  ARec->rLft = ixSels[3]; ARec->rTop = ixSels[2];
  ARec->rRgt = ixSels[4]; ARec->rBtm = ixSels[5];
  ARec->rSttW = ixSels[9]; ARec->rSttH = ixSels[10]; 
  int qROF = ixSels[7]&3; // rot_CCW=10,_CW=20 flp_h=1,_v=2
  switch (ixSels[6]) { // COMBO4 [Rotation]
    case 1: qROF |= 0x10; break;
    case 2: qROF |= 0x20; break; };
  ARec->rROF = qROF; }

static void iOnTransformChange( CVD_MDlg2 *ADlg ) { // a transform_control changes
  tUtTransf qRec; itUtTransf( ADlg, &qRec );
  int qW, qH; UtTransf_ImgDim( &qRec, &qW, &qH );
  TCHAR qS[88];
  if (qW==qH) _stprintf( qS, _T("x%04d"), qW );
         else _stprintf( qS, _T("x%04dx%04d"), qW, qH );
  ADlg->SetDlgItemText( IDC_EDIT5, qS ); }

// CVD_MDlg2 message handlers

BOOL CVD_MDlg2::OnInitDialog() {
  CDialog::OnInitDialog();
  // fixed controls' items
  TCHAR qS[256];
  _stprintf( qS, _T("selected %d item<s>"), iGetSels(0,NULL) ); // [apply to]
  SendDlgItemMessage( IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)qS );
  _stprintf( qS, _T("whole list (%d item<s>)"), iGetSels(1,NULL) );
  SendDlgItemMessage( IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)qS );
  for (int qi=0;qi<mcASz(cOpsAll);qi++)  // [operation]
    SendDlgItemMessage( IDC_COMBO2, CB_ADDSTRING, 0, (LPARAM)cOpsAll[qi] );
  SendDlgItemMessage( IDC_COMBO4, CB_ADDSTRING, 0, (LPARAM)_T("none") ); // [Rotation]
  SendDlgItemMessage( IDC_COMBO4, CB_ADDSTRING, 0, (LPARAM)_T("90\"CCW") );
  SendDlgItemMessage( IDC_COMBO4, CB_ADDSTRING, 0, (LPARAM)_T("90\"CW") );
  SendDlgItemMessage( IDC_COMBO5, CB_ADDSTRING, 0, (LPARAM)_T("write to subdir") );
  SendDlgItemMessage( IDC_COMBO5, CB_ADDSTRING, 0, (LPARAM)_T("overwrite") ); // [overwrite]
  // setcursel
  SendDlgItemMessage( IDC_COMBO1, CB_SETCURSEL, ixSels[0] ); // [apply to]
  SendDlgItemMessage( IDC_COMBO2, CB_SETCURSEL, ixSels[1] ); // [operation]
  SendDlgItemMessage( IDC_SPIN1, UDM_SETRANGE32, 999, 0 ); SetDlgItemInt( IDC_EDIT1, ixSels[2] ); // [top]
  SendDlgItemMessage( IDC_SPIN2, UDM_SETRANGE32, 0, 999 ); SetDlgItemInt( IDC_EDIT2, ixSels[3] ); // [lft]
  SendDlgItemMessage( IDC_SPIN3, UDM_SETRANGE32, 0, 999 ); SetDlgItemInt( IDC_EDIT3, ixSels[4] ); // [rgt]
  SendDlgItemMessage( IDC_SPIN4, UDM_SETRANGE32, 0, 999 ); SetDlgItemInt( IDC_EDIT4, ixSels[5] ); // [btm]
  SendDlgItemMessage( IDC_SPIN6, UDM_SETRANGE32, 0, 1<<15 ); SetDlgItemInt( IDC_EDIT6, ixSels[9] );  // [stretch W]
  SendDlgItemMessage( IDC_SPIN7, UDM_SETRANGE32, 0, 1<<15 ); SetDlgItemInt( IDC_EDIT7, ixSels[10] ); // [stretch H]
  SendDlgItemMessage( IDC_COMBO4, CB_SETCURSEL, ixSels[6] ); // [rotation]
  if (ixSels[7]&1) CheckDlgButton( IDC_CHECK1, TRUE ); // [flip hrz]
  if (ixSels[7]&2) CheckDlgButton( IDC_CHECK2, TRUE ); // [flip vrt]
  SendDlgItemMessage( IDC_COMBO5, CB_SETCURSEL, ixSels[8] ); // [overwrite]
  // selchange
  iOnCombo1SelChange( this );
  iOnCombo2SelChange( this );
  iOnCombo5SelChange( this ); 
  iOnTransformChange( this );
  m_bInitialized = true;
  return TRUE; } // return TRUE unless you set the focus to a control

BOOL CVD_MDlg2::OnCommand( WPARAM wParam, LPARAM lParam ) {
  TCHAR qS[256];
  WORD  qNC = HIWORD(wParam), qID = LOWORD(wParam);
  switch (qID) {
    case IDC_COMBO1: if (qNC==CBN_SELCHANGE) { // [apply to]
      ixSels[0] = (int)SendDlgItemMessage( IDC_COMBO1, CB_GETCURSEL ); iOnCombo1SelChange( this ); }; break;
    case IDC_COMBO2: if (qNC==CBN_SELCHANGE) { // [operation]
      ixSels[1] = (int)SendDlgItemMessage( IDC_COMBO2, CB_GETCURSEL ); iOnCombo2SelChange( this ); }; break;
    case IDC_COMBO4: if (qNC==CBN_SELCHANGE) { // [rotate]
      ixSels[6] = (int)SendDlgItemMessage( IDC_COMBO4, CB_GETCURSEL ); iOnTransformChange( this ); }; break;
    case IDC_COMBO5: if (qNC==CBN_SELCHANGE) { // [overwrite]
      ixSels[8] = (int)SendDlgItemMessage( IDC_COMBO5, CB_GETCURSEL ); iOnCombo5SelChange( this ); }; break;

    case IDC_EDIT1: case IDC_EDIT2: case IDC_EDIT3: case IDC_EDIT4: if (m_bInitialized && qNC==EN_CHANGE) { // [cut]
      ixSels[2+qID-IDC_EDIT1] = VaUt_SpinInt( this, qID ); iOnTransformChange( this ); }; break;
    case IDC_EDIT6: case IDC_EDIT7: if (m_bInitialized && qNC==EN_CHANGE) { // [stretch]
      ixSels[9+qID-IDC_EDIT6] = VaUt_SpinInt( this, qID ); iOnTransformChange( this ); }; break;
    
    case IDC_CHECK1: case IDC_CHECK2: { int  qFlg = 1<<(qID-IDC_CHECK1); // [flip hrz, vrt]
      if (IsDlgButtonChecked( qID )) ixSels[7] |= qFlg; else ixSels[7] &= ~qFlg; iOnTransformChange( this ); }; break;

    case IDCANCEL: break; // do nothing else if [Cancel]]
    case IDOK: { // proceed if [OK]
      _tcscpy( qS, theApp.m_sPath );
      TCHAR *qNam = qS+(theApp.m_sNam-theApp.m_sPath); *qNam = 0;
      theApp.DoWaitCursor( 1 );
      short qSelIdxs[128];
      int qCnt = iGetSels( ixSels[0], qSelIdxs ), qC = 0;
      switch (ixSels[1]) {
        case 1: // [average]; returns #accumulated
          qC = UtTransf_Average( m_LB, qS, qNam, qCnt, qSelIdxs ); break;
        case 2: // [to BMP]; returns '#transformed_shl_1 or ModifFlag'
          qC = UtTransf_ToBMP( m_LB, qS, qNam, qCnt, qSelIdxs ); break;
        case 3: { // [delete]
          for (int qi=0;qi<qCnt;qi++) { m_LB->GetText( qSelIdxs[qi], qNam );
            if (DeleteFile( qS )) qC++; }; // for i
        }; break;
        case 4: { // [open]
          const int cCntMx = 5; if (qCnt>cCntMx) qCnt = 5;
          for (int qi=0;qi<qCnt;qi++) { m_LB->GetText( qSelIdxs[qi], qNam );
            if ((size_t)ShellExecute( m_hWnd, _T("OPEN"), qS, NULL, NULL, SW_SHOW )<32) break;
            qC++; };
        }; break;
        case 5: case 6: { m_bModifFile = false; // [hide unselected,selected] 
          // the caller anyway will invoke LB^SetRedraw(false)+LB^SetRedraw(true)
          m_LB->SetRedraw( FALSE ); // so here we prevent immediate redraw
          int qGetSelToHide = ixSels[1]-5, cCnt = m_LB->GetCount();
          if (qCnt<cCnt) { for (int qi=cCnt-1;qi>=0;qi--)
            if (m_LB->GetSel(qi)==qGetSelToHide) m_LB->DeleteString( qi );
            qC = qCnt; }; // suppress diagnostic message about count
        }; break;
        default: { // [transform]; returns #transformed 
          tUtTransf qRec; itUtTransf( this, &qRec );
          TCHAR qS1[80]; GetDlgItemText( IDC_EDIT5, qS1, mcASz(qS1) ); 
          qC = UtTransf( &qRec, ixSels[8]?NULL:qS1, m_LB, qS, qNam, qCnt, qSelIdxs );
        };
      }; // switch
      m_bModifHave |= qC>0;
      if (qC!=qCnt) { _stprintf( qS, _T("The operation applied to %d item<s> of %d"),
        qC, qCnt ); AfxMessageBox( qS, MB_ICONEXCLAMATION ); };
      theApp.DoWaitCursor( -1 );
    }; break;

    case ciMenu0+0: iHelp( this ); return TRUE;
  };
  // leave	
  return CDialog::OnCommand( wParam, lParam ); }

LRESULT CVD_MDlg2::WindowProc( UINT message, WPARAM wParam, LPARAM lParam ) {
  switch (message) {
    case WM_CONTEXTMENU: iDisplayLocalMenu( this, lParam ); return 1;
    case WM_HELP: iHelp( this ); return 1;
  };
  // leave	
  return CDialog::WindowProc( message, wParam, lParam ); }

BOOL CVD_MDlg2::PreTranslateMessage( MSG *pMsg ) {
  if (pMsg->message==WM_KEYDOWN && pMsg->wParam==VK_F9) {
    iDisplayLocalMenu( this, 0 ); return TRUE; };
  // leave	
  return CDialog::PreTranslateMessage( pMsg ); }
