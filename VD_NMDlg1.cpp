// Dataseries-Viewer MFC
// (c) LGPL v3.0; by S. Bochkarev
// file VD_NMDlg1.cpp : dialog [BPM]
#include "StdAfx.h"
#include "ViewApp.h" // the application-object
#include "VD_UtViewDlg.h" // adhoc main dialog controls' access
#include "VD_UtMnuPUp.h" // CViewDlg local pop-up menu
#include "VD_MDlg3.h" // dialog [Help]
#include "VD_NMDlg1.h"

static CVD_NMDlg1 *ixBPMDlg = NULL;     // The one and only CVD_NMDlg1 object
static int ixBPMrcDim[2] = {0,0};       // ~passive variables in DSeg to 
static TCHAR ixBPMrcPath[192] = _T(""); // remember recently loaded BPM

IMPLEMENT_DYNAMIC(CVD_NMDlg1, CDialog)

CVD_NMDlg1::CVD_NMDlg1( CWnd* pParent):CDialog(CVD_NMDlg1::IDD, pParent) {
  m_bInitialized = false; }

CVD_NMDlg1::~CVD_NMDlg1() { 
  if (m_BPM) delete m_BPM;
  ixBPMDlg = NULL;
  theD_OnPresentationUpdate(); }

void CVD_NMDlg1::DoDataExchange( CDataExchange* pDX ) {
  CDialog::DoDataExchange( pDX );
}

BEGIN_MESSAGE_MAP(CVD_NMDlg1, CDialog)
END_MESSAGE_MAP()

//// CVD_NMDlg1 services ////
static void iHelp( CVD_NMDlg1 *ADlg ) {
  HelpDlg( _T("BPMM editor"), ADlg, _T("\
Shows overlay \"BadPixels' Map <automatic>\"\n\
Shows & edits overlay \"BadPixels' Map Manual\"\n\
At open this dialog:\n\
- <by current loaded image> assigns overlay's dimension\n\
- <tries to> loads the overlays from SDK-specific location\n\n\
Editing area:\n\
- BPMM_org\ttwo spin_edits (x & y) specify 2D-point\n\
- BPMM_cnt\tcombo_box & spin_edit specify direction & count\n\n\
The actions:\n\
[Browse]      \tselects location of loading overlays\n\
[:= XY]       \tBPMM_org := Marker_XY\n\
[XY :=]       \tMarker_XY := BPMM_org\n\
[count :=]    \tassigns BPMM_cnt by Marker_XY\n\
[:= whole Row]\tassigns BPMM_org, BPMM_cnt by Marker_XY\n\
[:= whole Col]\t-\"-\n\
[Load]        \tloads the overlays (abandons changes)\n\
[Save]        \tsaves BPMM_file\n\
[Erase]       \tfills BPMM_frame by 0 ([Erase]+[Save] delete BPMM_file)\n\
[Mark]        \tfills editing area by 1\n\
[Clean]       \tfills editing area by 0") ); }

static const int ciMenu0 = 9000;

static void iDisplayLocalMenu( CVD_NMDlg1 *ADlg, LPARAM APoint  ) {
  HMENU qMenu = CreatePopupMenu();
  AppendMenu( qMenu, MF_STRING, ciMenu0+0, _T("Help") );
  // display menu
  VaUt_DisplayMenu( ADlg, APoint, qMenu ); }

static void iOnCombo1_Internal( CVD_NMDlg1 *ADlg ) {
  int qDir = (int)ADlg->SendDlgItemMessage( IDC_COMBO1, CB_GETCURSEL ),
      qRng = qDir==6 ? 7 : 99999, // [Count]
      qC = ADlg->GetDlgItemInt( IDC_EDIT6 ), qC1 = mcRng( qC, 1, qRng );
  ADlg->SendDlgItemMessage( IDC_SPIN6, UDM_SETRANGE32, 1, qRng );
  if (qC1!=qC) { const bool cInitialized = ADlg->m_bInitialized;
    ADlg->m_bInitialized = false; ADlg->SetDlgItemInt( IDC_EDIT6, qC1 );
    ADlg->m_bInitialized = cInitialized; }; }

void iEdit1WndTxtSet( CVD_NMDlg1 *ADlg, const TCHAR *APath ) { // path > UIstring
  ADlg->SetDlgItemText( IDC_EDIT1, VaUt_PathToUIStr(APath) );
  ADlg->SendDlgItemMessage( IDC_EDIT1, WM_KEYDOWN, VK_END, 1 ); }
  
void iEdit1WndTxtGet( CVD_NMDlg1 *ADlg, TCHAR *APath ) { // UIstring > path
  ADlg->GetDlgItemText( IDC_EDIT1, APath, 192 ); VaU_TrimStr( APath );
  _tcscpy( APath, VaUt_UIStrToPath(APath) ); }

static TCHAR ixBPMRaw[16], ixBPMMRaw[16]; // ~BPM.raw,BPMM.raw

//// CVD_NMDlg1 message handlers ////

BOOL CVD_NMDlg1::OnInitDialog() {
  CDialog::OnInitDialog();
  _stprintf( ixBPMRaw, _T("BPM%.5s"), gFilExtDef() );
  _stprintf( ixBPMMRaw, _T("BPMM%.5s"), gFilExtDef() );
  TCHAR qS[256];
  GetDlgItem(IDC_STATIC1)->SetFont( &theDlg.m_FontSmall );
  // initally locate BPM-editor on the topleft_corner; unless 
  // '[ClientCorner]=LftBtm' locates the PixelInspector there
  CRect qRp; GetParent()->GetWindowRect( &qRp );
  CRect qRc; GetWindowRect( &qRc );
  SetWindowPos( NULL, qRp.right-qRc.Width(), 
    xMenuCCo!=1 ? qRp.top+33 : qRp.bottom-qRc.Height(), 
    0, 0, SWP_NOZORDER|SWP_NOSIZE );
  // [marker's area]
  GetDlgItem(IDC_COMBO1)->SetFont( &theDlg.m_Font );
  SendDlgItemMessage( IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)_T("0:<  lft") );
  SendDlgItemMessage( IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)_T("1:>  rgt") );
  SendDlgItemMessage( IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)_T("2:>< cen_hrz") );
  SendDlgItemMessage( IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)_T("3:^  top") );
  SendDlgItemMessage( IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)_T("4:v  btm") );
  SendDlgItemMessage( IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)_T("5:v^ cen_vrt") );
  SendDlgItemMessage( IDC_COMBO1, CB_ADDSTRING, 0, (LPARAM)_T("6:   cen 2D") );
  SendDlgItemMessage( IDC_COMBO1, CB_SETCURSEL, 1 );
  // frames' dimension
  m_nW = theA_W(); m_nH = theA_H(); m_nN = m_nW*m_nH;
  m_BPM = new char[m_nN]; memset( m_BPM, 0, m_nN );
  _stprintf( qS, _T("BPMM editor %04dx%04d"), m_nW, m_nH ); SetWindowText( qS );
  // marker's location
  SendDlgItemMessage( IDC_SPIN4, UDM_SETRANGE32, 0, m_nW-1 ); // [X]
  SendDlgItemMessage( IDC_SPIN5, UDM_SETRANGE32, m_nH-1, 0 ); // [Y]
  SetDlgItemInt( IDC_EDIT4, theD_X() ); SetDlgItemInt( IDC_EDIT5, theD_Y() );
  SetDlgItemInt( IDC_EDIT6, 1 );
  iOnCombo1_Internal( this );
  // the directory
  qS[0] = 0; if (theApp.m_bUnderSDK) { 
    // if we work with an SDK then assume that:
    // - SDK's IniFile contains "[Settings] PresetIni=ConfigDir/preset.ini"
    // - the ConfigDir is relative to this_exe_directory
    // - the BPM_files exist in ConfigDir
    theApp.VDI_SetSectName( _T("Settings") );
    const TCHAR *cS = theApp.VDI_Str( _T("PresetIni"), NULL );
    if (cS) { GetModuleFileName( NULL, qS, 192 );
      _tcscpy( VaU_Sfx(qS,_T('\\')), cS ) ; };
  };
  if (!qS[0] && ixBPMrcDim[0]==m_nW && ixBPMrcDim[1]==m_nH) _tcscpy( qS, ixBPMrcPath );
  if (!qS[0] && theApp.m_sNam) _tcscpy( qS, theApp.m_sPath );
  if (!qS[0]) GetModuleFileName( NULL, qS, 192 );
  int qL = (int)_tcslen( qS ); for (int qi=0;qi<qL;qi++) if (qS[qi]==_T('/')) qS[qi] = _T('\\');
  _tcscpy( VaU_Sfx(qS,_T('\\')), ixBPMMRaw ); iEdit1WndTxtSet( this, qS );
  // load BPM.raw,BPMM.raw
  Load();
  // leave
  m_bInitialized = true; return TRUE; }

BOOL CVD_NMDlg1::OnCommand( WPARAM wParam, LPARAM lParam ) {
  WORD  qNC = HIWORD(wParam), qID = LOWORD(wParam);
  TCHAR qS[256];
  switch (qID) {
    case IDOK: case IDCANCEL:
      DestroyWindow(); return TRUE; // Triggers cleanup sequence
    case IDC_BUTTON1: { iEdit1WndTxtGet( this, qS ); // [Browse]
      TCHAR qFilter[256]; int cL = _stprintf( qFilter,
        _T("Standard BPMM.%s|BPMM.%s|All frames|*.%s|"), gFilExtDef(), gFilExtDef(), gFilExtDef() );
      for (int qi=0;qi<cL;qi++) if (qFilter[qi]==_T('|')) qFilter[qi] = 0;
      if (VaU_SaveFileDlg( this, qS, _T("Browse manual BPM"), qFilter, NULL ))
        iEdit1WndTxtSet( this, qS );
    }; break;
    case IDC_BUTTON2: { CPoint cXY = theD_XY(); // [:=XY]
      m_bInitialized = false; SetDlgItemInt( IDC_EDIT4, cXY.x ); 
      m_bInitialized = true;  SetDlgItemInt( IDC_EDIT5, cXY.y ); 
    }; break;
    case IDC_BUTTON3: { CPoint cXY = theD_XY(); // [count]
      int qDX = cXY.x-VaUt_SpinInt(this,IDC_EDIT4),
          qDY = cXY.y-VaUt_SpinInt(this,IDC_EDIT5),
          qTo = (int)SendDlgItemMessage( IDC_COMBO1, CB_GETCURSEL );
      if (abs(qDX)>abs(qDY)*2/3) qDY = 0; else
      if (abs(qDY)>abs(qDX)*2/3) qDX = 0; else 
      if (qTo<3) qDY = 0; else qDX = 0; 
      if ((qDX==0)^(qDY==0)) {
        // _stprintf( qS, _T("%d:%d"), qDX, qDY ); SetDlgItemText( IDC_STATIC1, qS );
        int qC = qDX ? abs(qDX) : abs(qDY); if (qC) {
          if (qDX<-5) qTo = 0; else
          if (qDX> 5) qTo = 1; else
          if (qDY<-5) qTo = 3; else
          if (qDY> 5) qTo = 4;
          m_bInitialized = false; SendDlgItemMessage( IDC_COMBO1, CB_SETCURSEL, qTo );
          m_bInitialized = true;  SetDlgItemInt( IDC_EDIT6, qC );
        };
      };
    }; break;
    case IDC_BUTTON4: { // [XY:=]
      CPoint cBPM_XY = BPM_XY();
      theDlg.m_bInitialized = false; theDlg.SetDlgItemInt( IDC_EDIT4, cBPM_XY.x );
      theDlg.m_bInitialized = true;  theDlg.SetDlgItemInt( IDC_EDIT5, cBPM_XY.y );
    }; break;
    case IDC_BUTTON6: Load(); theD_OnPresentationUpdate(); break; // [Load]
    case IDC_BUTTON7: Save(); break; // [Save]
    case IDC_BUTTON8: Erase(); theD_OnPresentationUpdate(); break;// [Erase]
    case IDC_BUTTON10:   // [Mark]
    case IDC_BUTTON11: { // [Clean]
      CRect qM; if (!BPM_MarkGet( &qM )) break; int cWstride = theA_W();
      char *qBPM = m_BPM+cWstride*qM.top+qM.left;
      int qw = qM.Width(), qh = qM.Height(), qN = 0; do {
        if (qID==IDC_BUTTON11)
          for (int qx=0;qx<qw;qx++) { int qV = qBPM[qx]; if (qV&2) { qBPM[qx] = qV &~2; qN++; }; }
        else
          for (int qx=0;qx<qw;qx++) { int qV = qBPM[qx]; if (!(qV&2)) { qBPM[qx] = qV|2; qN++; }; };
        qBPM += cWstride; } while (--qh);
      _stprintf( qS, _T("%d BPMM-pixel<s> updated"), qN ); SetDlgItemText( IDC_STATIC1, qS );
      theD_OnPresentationUpdate();
    }; break;
    case IDC_BUTTON15:   // [whole row]
    case IDC_BUTTON16: { // [whole col]
      int   qC = qID==IDC_BUTTON16 ? theA_H() : theA_W(), qC0, qX, qY,
            qTo = ((int)SendDlgItemMessage(IDC_COMBO1,CB_GETCURSEL));
      if (qTo==6) qTo = 2; // treat here 'cen 2D' as 'cen hrz'
      qTo = qTo%3; switch (qTo) {
        case  1: qC0 = 0; break;
        case  2: qC0 = qC>>1; qC = qC0+(qC&1); break;
        default: qC0 = qC-1; };
      if (qID==IDC_BUTTON16) { qX = theD_X(); qY = qC0; qTo += 3; }
                        else { qY = theD_Y(); qX = qC0; };
      m_bInitialized = false;
      SendDlgItemMessage( IDC_COMBO1, CB_SETCURSEL, qTo ); iOnCombo1_Internal( this );
      SetDlgItemInt( IDC_EDIT4, qX ); SetDlgItemInt( IDC_EDIT5, qY );
      m_bInitialized = true; SetDlgItemInt( IDC_EDIT6, qC );
      theD_OnPresentationUpdate();
    }; break;

    case IDC_EDIT4: // [X]
    case IDC_EDIT5: // [Y]
    case IDC_EDIT6: // [Count]
      if (m_bInitialized && qNC==EN_CHANGE)
        theD_OnPresentationUpdate();
      break;
    
    case IDC_COMBO1: // [to]
      if (m_bInitialized && qNC==CBN_SELCHANGE) {
        iOnCombo1_Internal( this ); theD_OnPresentationUpdate(); };
      break;
      
    case ciMenu0: iHelp( this ); return TRUE;
  };
  // leave	
  return CDialog::OnCommand( wParam, lParam ); }

LRESULT CVD_NMDlg1::WindowProc( UINT message, WPARAM wParam, LPARAM lParam ) {
  switch (message) {
    case WM_CONTEXTMENU: iDisplayLocalMenu( this, lParam ); return 1;
    case WM_HELP: iHelp( this ); return 1;
   };
  // leave	
  return CDialog::WindowProc( message, wParam, lParam ); }

BOOL CVD_NMDlg1::PreTranslateMessage( MSG *pMsg ) {
  switch (pMsg->message) {
    case WM_KEYDOWN:
      switch (pMsg->wParam) {
        //case VK_F9: iDisplayLocalMenu( 0 ); return TRUE;
        case VK_F6:  PostMessage( WM_COMMAND, MAKEWPARAM(IDC_BUTTON6,BN_CLICKED) ); break; // [Load]
        case VK_F7:  PostMessage( WM_COMMAND, MAKEWPARAM(IDC_BUTTON7,BN_CLICKED) ); break; // [Save]
        case VK_F8:  PostMessage( WM_COMMAND, MAKEWPARAM(IDC_BUTTON8,BN_CLICKED) ); break; // [Erase]
      };
  };
  // leave	
  return CDialog::PreTranslateMessage( pMsg ); }

void CVD_NMDlg1::PostNcDestroy() {
  CDialog::PostNcDestroy();
  delete this; }

//// CVD_NMDlg1 ~API-exports ////

bool iLoadFileOfShorts( const TCHAR *AFPath, int AN, short *ABuf, int ABitFlag, char *ABPM, TCHAR *AMsg ) {
  // if "raw[i]<>0" then "BPM[i]:=BitFlag"; also assign status-message
  FILE *qF = _tfopen( AFPath, _T("rb") ); if (!qF) {
    _tcscpy( AMsg, _T("doesn't exist") ); return false; };
  fseek( qF, 0, SEEK_END ); if (ftell(qF)/sizeof(short)!=AN) {
    _tcscpy( AMsg, _T("wrong size") ); fclose( qF ); return false; };
  fseek( qF, 0, SEEK_SET ); fread( ABuf, sizeof(short), AN, qF );
  int qNP = 0; for (int qi=0;qi<AN;qi++) if (ABuf[qi]) { qNP++; ABPM[qi] |= ABitFlag; };
  _stprintf( AMsg, _T("%d pixels"), qNP ); return true; }

void CVD_NMDlg1::Load() { // -- load BPM.raw,BPMM.raw
  const int cN = m_nN; memset( m_BPM, 0, cN ); short *qBuf = new short[cN];
  TCHAR qS[192]; iEdit1WndTxtGet( this, qS );
  TCHAR qMsg2[88]; _tcscpy( qMsg2, _T("BPMM ") );
  if (iLoadFileOfShorts( qS, cN, qBuf, 2, m_BPM, qMsg2+_tcslen(qMsg2) )) {
    ixBPMrcDim[0] = m_nW; ixBPMrcDim[1] = m_nH; _tcscpy( ixBPMrcPath, qS ); };
  TCHAR qMsg1[88]; _tcscpy( qMsg1, _T("BPM ") ); _tcscpy( VaU_Sfx(qS,_T('\\')), ixBPMRaw ); 
  iLoadFileOfShorts( qS, cN, qBuf, 1, m_BPM, qMsg1+_tcslen(qMsg1) );
  _stprintf( qS, _T("%s; %s"), qMsg1, qMsg2 ); SetDlgItemText( IDC_STATIC1, qS );
  delete qBuf; theD_OnPresentationUpdate(); }

void CVD_NMDlg1::Save() { // -- save BPMM.raw
  TCHAR qS[256]; iEdit1WndTxtGet( this, qS );
  const int cN = m_nN; for (int qi0=0;qi0<cN;qi0++) if (m_BPM[qi0]&2) goto labSave;
  SetDlgItemText( IDC_STATIC1, DeleteFile(qS) ?
    _T("deleted empty BPMM") : _T("can't delete empty BPMM") ); return;
  labSave: FILE *qF = _tfopen( qS, _T("wb") ); if (!qF) {
    _tcscpy( qS, _T("can't save BPMM") ); WriteLn( qS ); 
    AfxMessageBox( qS, MB_ICONSTOP|MB_OK ); SetDlgItemText( IDC_STATIC1, qS ); return; };
  short *qBuf = new short[cN]; memset( qBuf, 0, cN*sizeof(short) );
  int qN = 0; for (int qi2=0;qi2<cN;qi2++) if (m_BPM[qi2]&2) { qN++; qBuf[qi2] = 1; };
  fwrite( qBuf, sizeof(short), cN, qF ); fclose( qF ); delete qBuf;
  _stprintf( qS, _T("saved BPMM %d pixels"), qN ); SetDlgItemText( IDC_STATIC1, qS ); }

void CVD_NMDlg1::Erase() { // -- ...
  if (AfxMessageBox( _T("Erase all BPMM-pixels ?"),
    MB_ICONQUESTION|MB_YESNO|MB_DEFBUTTON2)!=IDYES) return;
  const int cN = m_nN; int qN = 0; for (int qi=0;qi<cN;qi++) {
    if (m_BPM[qi]&2) { m_BPM[qi] &= ~2; qN++; }; };
  TCHAR qS[256]; _stprintf( qS, _T("%d BPMM-pixels erased; [Save] now will delete empty BPMM"), qN );
  SetDlgItemText( IDC_STATIC1, qS ); }

void CVD_NMDlg1::MarkGet( CRect *AM ) { // -- get marker
  int qC = VaUt_SpinInt( this, IDC_EDIT6 )-1; // [count]
  CPoint qP0, qP1, qXY = BPM_XY(); // [X], [Y]
  switch ((int)SendDlgItemMessage( IDC_COMBO1, CB_GETCURSEL )) { // [count dir]
    case  1: qP0.SetPoint( qXY.x, qXY.y );       qP1.SetPoint( qXY.x+qC, qXY.y ); break; // > rgt
    case  2: qP0.SetPoint( qXY.x-qC, qXY.y );    qP1.SetPoint( qXY.x+qC, qXY.y ); break; // >< h_cen
    case  3: qP0.SetPoint( qXY.x, qXY.y-qC );    qP1.SetPoint( qXY.x, qXY.y );    break; // ^ top
    case  4: qP0.SetPoint( qXY.x, qXY.y );       qP1.SetPoint( qXY.x, qXY.y+qC ); break; // v btm
    case  5: qP0.SetPoint( qXY.x, qXY.y-qC );    qP1.SetPoint( qXY.x, qXY.y+qC ); break; // v^ v_cen
    case  6: qP0.SetPoint( qXY.x-qC, qXY.y-qC ); qP1.SetPoint( qXY.x+qC, qXY.y+qC ); break; // cen 21D
    default: qP0.SetPoint( qXY.x-qC, qXY.y );    qP1.SetPoint( qXY.x, qXY.y ); }; // < lft
  AM->SetRect( qP0, qP1 ); }

//// CVD_NMDlg1 global exports ////

bool BPMDlg_Active() { return ixBPMDlg!=NULL; }

void BPMDlg_Show() { // -- show BPM-dialog
  if (BPMDlg_Active()) 
    ixBPMDlg->SetFocus();
  else {
    ixBPMDlg = new CVD_NMDlg1( &theDlg );
    if (ixBPMDlg->Create( CVD_NMDlg1::IDD, &theDlg ))
      ixBPMDlg->ShowWindow( SW_SHOW );
    else {
      delete ixBPMDlg; ixBPMDlg = NULL; };
  };
}

static char inline *iBPM_Get() { // whether BPM-data-buffer exists?
  return BPMDlg_Active() ? ixBPMDlg->m_BPM : NULL; }

static const COLORREF cPal[4] = {
  RGB( 99,  0, 99),   // #0: in BPM
  RGB(155,  0,155),   // #1: in BPMM
  RGB(  0,155,  0),   // #2: in _MarkGet
  RGB( 99,155, 99) }; // #3: in _MarkGet and BPM or BPMM
  
static const COLORREF cBkCur[2] = { // distingush BPM_XY within _MarkGet
  RGB(  0,177,  0),   // #0: 'is BPM_XY'
  RGB(111,199,111) }; // #1: 'is BPM_XY' and BPM or BPMM

void BPM_DIB8Update( BITMAPINFOHEADER *ABMI, RGBQUAD *APal, BYTE *ABMPBuf ) { // -- called by BMPBuf.ImageToBMP
  const int cPalEntries = 4; // # used palette-entries (could be 2 or 4)
  char *qBPM = iBPM_Get(); if (!qBPM) { // unconditionally restore default palette 
    for (int qi=0;qi<cPalEntries;qi++) APal[qi].rgbBlue = APal[qi].rgbGreen = APal[qi].rgbRed = qi; return; };
  int qW = ixBPMDlg->m_nW, qH = ixBPMDlg->m_nH;
  if (ABMI->biWidth!=qW || ABMI->biHeight!=qH) return;
  // DIB-buffer
  int qWstride = qW; if (qWstride&3) qWstride += 4-(qWstride&3);
  BYTE *qBB = ABMPBuf+qWstride*(qH-1);
  for (int qr=0;qr<qH;qr++) {
    for (int qc=0;qc<qW;qc++) { char qb = qBPM[qc];
      if (!qb) { qBB[qc] = __max(qBB[qc],cPalEntries); continue; };
      qBB[qc] = qb-1; };
    qBPM += qW; qBB -= qWstride; };
    if (cPalEntries>2) { // if bitblit _GetMark with diferent color<s>
      CRect qM; BPM_MarkGet( &qM );
      int w = qM.Width(), h = qM.Height(); 
      qBB = ABMPBuf+(qH-qM.top-1)*qWstride+qM.left;
      do { for (int c=0;c<w;c++) qBB[c] = qBB[c]<cPalEntries ? cPalEntries-1 : cPalEntries-2;
        qBB -= qWstride; } while (--h); };
  // DIB-header, palette
  memcpy( APal, cPal, cPalEntries*sizeof(COLORREF) ); }

bool BPM_MarkGet( CRect *AM ) { // -- called by ViewDlg.OnPaint.
  if (!BPMDlg_Active()) return false;
  CRect qM; ixBPMDlg->MarkGet( &qM );
  qM.right++; qM.bottom++;
  qM = theA_Clip( qM );
  bool cOk = !qM.IsRectEmpty(); 
  if (cOk) *AM = qM; return cOk; }

CPoint BPM_XY() { // -- returns [X],[Y]
  return BPMDlg_Active() ? CPoint( VaUt_SpinInt(ixBPMDlg,IDC_EDIT4), VaUt_SpinInt(ixBPMDlg,IDC_EDIT5) )
                         : CPoint(-0x7FFF,-0x7FFF); }

COLORREF BPM_PixelBkColor( CPoint AXY ) { // -- called by ~_PixelInspectorDraw
  char *qBPM = iBPM_Get(); if (!qBPM) return 0;
  char  qb = qBPM[AXY.y*ixBPMDlg->m_nW+AXY.x];
  CRect qM; if ( BPM_MarkGet(&qM) && qM.PtInRect(AXY)) {
    int cb2 = qb?1:0; // within _MarkGet indicate only 'whether BPM[this]>0?'
    return AXY==BPM_XY() ? cBkCur[cb2] : cPal[2+cb2]; };
  return qb ? cPal[qb-1] : 0; }

void BPM_IniProfile( bool AWrite ) { // -- read/write initialization profile
  if (AWrite) {
    if (ixBPMrcDim[1]<1 || ixBPMrcDim[1]>=1<<15) return;
    TCHAR qS[80]; _stprintf( qS, _T("%d %d; recent BPM's W H"),
      ixBPMrcDim[0], ixBPMrcDim[1] ); theApp.VDI_SetStr( _T("BPMrcDim"), qS );
    theApp.VDI_SetStr( _T("BPMrcPath"), ixBPMrcPath );
    return; };
  if (theApp.VDI_Ints( _T("BPMrcDim"), NULL )!=2) return;
  int *qI = theApp.VDI_Ints(); ixBPMrcDim[0] = qI[0]; ixBPMrcDim[1] = qI[1];
  _tcscpy( ixBPMrcPath, theApp.VDI_Str(_T("BPMrcPath"),ixBPMMRaw) ); }