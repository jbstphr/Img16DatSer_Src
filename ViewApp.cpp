// Dataseries-Viewer MFC
// (c) LGPL v3.0; by S. Bochkarev
// ViewApp.cpp : the application-object
#include "StdAfx.h"
#include "math.h"
#include "ViewApp.h"
#include "VD_NMDlg1.h" // dialog [BPM]
#include "VD_UtMnuPUp.h" // CViewDlg local pop-up menu
#include "VD_UtViewDlg.h" // adhoc main dialog controls' access

#ifdef _DEBUG
  #define new DEBUG_NEW
#endif

void WriteLn( const TCHAR *AMsg ) { // -- trace to terminal; declared in StdAfx.h
  TCHAR qMsg[256]; _stprintf( qMsg, _T("%s\n"), AMsg );
  // _tprintf( qMsg ); }
  OutputDebugString( qMsg ); }

CViewApp theApp; // The one and only CViewApp object
CViewDlg theDlg; // the dialog's object in data segment

BEGIN_MESSAGE_MAP(CViewApp, CWinApp)
  ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

CViewApp::CViewApp() {}

void CViewApp::CommandLineCheck() { // -- could assign W,H,Path:=Mask
  m_sPath[0] = 0; if (!m_lpCmdLine || !m_lpCmdLine[0]) return;
  const int cSzPathMx = mcASz(m_sPath)-1;
  if (m_lpCmdLine[0]==_T('"')) { // from ~WindowsExplorer
    _tcsncpy( m_sPath, m_lpCmdLine+1, cSzPathMx );
    TCHAR *qCh = VaU_Sfx( m_sPath, _T('"') ); if (qCh) *qCh = 0; }
  else { TCHAR *qCh; // if CmdLine[0]<>quote
    int qW = _tcstol( m_lpCmdLine, &qCh, 10 ), qH = _tcstol( qCh, &qCh, 10 );
    while (*qCh && *qCh<=_T(' ')) qCh++;
    if (!*qCh || (qCh!=m_lpCmdLine && (qW<2||qW>=1<<15||qH<2||qH>=1<<15))) {
      TCHAR qS[60]; _stprintf( qS, _T("WRN: wrong CmdLine '%.40s'"), m_lpCmdLine );
      WriteLn( qS ); return; }; // if not...
    _tcsncpy( m_sPath, qCh, cSzPathMx ); m_nW = qW; m_nH = qH; }; // if CmdLine[0]<>quote
  m_sPath[cSzPathMx] = 0; m_sNam = VaU_Sfx( m_sPath, _T('\\') ); // if specific file then assign sNam<>nil 
  if (1) { TCHAR *qExt = VaU_Sfx( m_sNam, _T('.') ); // default_extension 
    if (*qExt) _tcsncpy( gFilExtDef(), qExt, 3 ); gFilExtDef()[3] = 0; };
  if (!*m_sNam || VaU_Sfx(m_sNam,_T('*')) || VaU_Sfx(m_sNam,_T('?'))) m_sNam = NULL; }

void CViewApp::PixelTypeTagSet( int ATag ) { switch (ATag) { // -- "=0":signed 16-bit, "=1":unsigned
  case 1: m_nGrayMn = cGrayUnsMn; break; default: m_nGrayMn = cGraySigMn; }; }

void CViewApp::LoadResetFlags() { // -- reset load's flags
  m_nSizeDiff = (int)0x80000000; m_bHistOk = false; }

void CViewApp::LoadImage() { // -- load *.raw
  LoadResetFlags();
  FILE *qF = _tfopen( m_sPath, _T("rb") ); if (!qF) return;
  if (fseek( qF, 0, SEEK_END )) { fclose( qF ); return; }; int qNP = ftell(qF)/sizeof(short);
  if (fseek( qF, 0, SEEK_SET )) { fclose( qF ); return; };
  int qW = VaUt_SpinInt( &theDlg, IDC_EDIT1 ), qH = VaUt_SpinInt( &theDlg, IDC_EDIT2 );
  if (!(gFlags&1) || (qNP==qW*qH)) goto lab1; // if "[#0:adim]=1"
  const int  cLastDlgCtrlId = theDlg.m_LastDlgCtrlId; // tag of the last changed control
  const bool cLastW = cLastDlgCtrlId==IDC_EDIT1, cLastH = cLastDlgCtrlId==IDC_EDIT2;
  if (!(cLastW||cLastH) && theDlg.SuggestWHforNP( qNP, &qW, &qH )) goto lab1;
  { int qD =(int)(sqrt((float)(qNP))+0.49999f); if (qNP==qD*qD) { qW = qH = qD; goto lab1; }; };
  if (!cLastH && (qW>qNP>>14) && !(qNP%qW)) { qH = qNP/qW; goto lab1; };
  if (!cLastW && (qH>qNP>>14) && !(qNP%qH)) { qW = qNP/qH; goto lab1; };
  lab1: theDlg.m_LastDlgCtrlId = 0; // reset tag of the last changed control
  qW = mcRng( qW, 4, (1<<15)-1 ); qH = mcRng( qH, 4, (1<<15)-1 );
  const int cNP = qW*qH; m_nSizeDiff = qNP-cNP;
  if (cNP<128) { fclose( qF ); return; };
  m_nW = qW; m_nH = qH; theDlg.SetImgDim();
  short *qBuf = (short*)malloc( cNP*sizeof(short) ); memset( qBuf, 0, cNP*sizeof(short) );
  fread( qBuf, sizeof(short), cNP, qF ); fclose( qF );
  ViD_Hist16BaseBuild( qBuf ); // build histogram & calc basic stats
  m_bHistOk = true; 
  theDlg.OnLoadImage( theA_GrayMn()<0?0:1, qBuf ); // BMPBuf.ImageToBMP( Tag=0:signed, =1:unsigned )
  free( qBuf ); }

DWORD CViewApp::MetaInfoWxH() { // -- try read metainformation WxH ~"alongside current Path"
  if (!m_sPath[0]) return 0; _tcscpy( theApp.m_sNam, _T("$RecPar.txt") );
  FILE *qF = _tfopen( m_sPath, _T("rt") ); if (!qF)  return 0;
  int qW = 0, qH = 0; TCHAR qS[82]; if (_fgetts( qS, 81, qF )) _stscanf( qS, _T("%d %d"), &qW, &qH ); 
  fclose( qF ); return (qW<4 || qW>=1<<15 || qH<4 || qH>=1<<15) ? 0 : MAKELONG(qW,qH); }

const TCHAR *CViewApp::StandardMonoFontName() { // -- standard mono-font
  return m_bUnderWinE?_T("LIBERATION MONO"):_T("COURIER NEW"); }
 
/////////////////////////////////////////////////////////////////////////////
// CViewApp INI-profile services

void CViewApp::VDI_SetSectName( const TCHAR *ASectName ) { m_IF.SetSectName( ASectName ); } // -- sets section name in library's ini-profile
void CViewApp::VDI_SetStr( const TCHAR *AKeyName, const TCHAR *AStr ) { m_IF.SetIniStr( AKeyName, AStr ); } // -- set string key to library's ini-profile
const TCHAR *CViewApp::VDI_Str( const TCHAR *AKeyName, const TCHAR *ADefault ) { // -- get string key from library's ini-profile
  _declspec(thread) static TCHAR ixLastIniFileStr[256]; ixLastIniFileStr[0] = 0;
  if (m_IF.GetIniStr( AKeyName, ixLastIniFileStr )>=0) return ixLastIniFileStr;
  if (ADefault) VDI_SetStr( AKeyName, ADefault ); return ADefault; }
__declspec(thread) static float ixVDIRes[16]; // -- scanf's buffer in DSeg
int *CViewApp::VDI_Ints() { return (int*)ixVDIRes; } float *CViewApp::VDI_Floats() { return (float*)ixVDIRes; } 
#define mcscanf(S,F,R) _stscanf( S, F F F F F F F F F F F F F F F F,\
  R+0, R+1, R+2, R+3, R+4, R+5, R+6, R+7, R+8, R+9, R+10, R+11, R+12, R+13, R+14, R+15 )
int CViewApp::VDI_Ints( const TCHAR *AKeyName, const TCHAR *ADefault ) { // -- get small array of ints
  const TCHAR *cStr = VDI_Str( AKeyName, ADefault ); return cStr ? mcscanf(cStr,_T("%d"),VDI_Ints()) : -1; }
int CViewApp::VDI_Hexs( const TCHAR *AKeyName, const TCHAR *ADefault ) { // -- get small array of ints
  const TCHAR *cStr = VDI_Str( AKeyName, ADefault ); return cStr ? mcscanf(cStr,_T("%X"),VDI_Ints()) : -1; }
int CViewApp::VDI_Floats( const TCHAR *AKeyName, const TCHAR *ADefault ) { // -- get small array of floats
  const TCHAR *cStr = VDI_Str( AKeyName, ADefault ); return cStr ? mcscanf(cStr,_T("%f"),VDI_Floats()) : -1; }

/////////////////////////////////////////////////////////////////////////////
// WinE APIs
static bool iwine_get_version( TCHAR *AWineVer ) {
  HMODULE qHDll = GetModuleHandleA( "ntdll.dll" );
  if (!qHDll) qHDll = LoadLibraryA( "ntdll.dll" ); if (!qHDll) return false;
  typedef const char*(CDECL *twine_get_version)(void);
  twine_get_version qFunc = (twine_get_version)GetProcAddress( qHDll, "wine_get_version" );
  if (!qFunc) return false;
  VaU_ASCII_TCHAR( qFunc(), AWineVer, 80 ); return true; }

static bool iwine_get_APIs( twine_get_unix_file_name *ADosToUnix, twine_get_dos_file_name *AUnixToDOS ) {
  HMODULE qHDll = GetModuleHandleA( "kernel32.dll" );
  if (!qHDll) qHDll = LoadLibraryA( "kernel32.dll" ); if (!qHDll) return false;
  *ADosToUnix = (twine_get_unix_file_name)GetProcAddress( qHDll, "wine_get_unix_file_name" );  
  *AUnixToDOS = (twine_get_dos_file_name)GetProcAddress( qHDll, "wine_get_dos_file_name" );
  if (*ADosToUnix!=NULL && *AUnixToDOS!=NULL) return true;
  *ADosToUnix = NULL; *AUnixToDOS = NULL; return false; }

/////////////////////////////////////////////////////////////////////////////
// CViewApp message handlers

BOOL CViewApp::InitInstance() {
  CWinApp::InitInstance();
  TCHAR qS[256];
  // comprehend about WinE
  rwine_get_unix_file_name = NULL; rwine_get_dos_file_name = NULL;
  m_bUnderWinE = iwine_get_version( qS+200 );
  if (m_bUnderWinE) { _stprintf( qS, _T("InInst: under WinE %s"), qS+200 ); WriteLn( qS ); };
  // If we work with an SDK then assume that:
  //  - SDK's IniFile locates in this_exe_directory,
  //  - it <might> contains metainfo about BPM_files etc.
  // By default:
  //  - INI_profile is 'this_exefullname.ini'
  GetFullPathName( _T("SDK.ini"), 192, qS, NULL );
  m_bUnderSDK = GetFileAttributes(qS)!=-1;
  if (m_bUnderSDK) {
    m_IF.SetFileName( qS ); WriteLn( _T("InInst: under the SDK") );
  } else {
    GetModuleFileName( NULL, qS, 192 ); _tcscpy( VaU_Sfx(qS,_T('.')), _T(".ini") ) ;
    m_IF.SetFileName( qS ); };
  // 
  free( (void*)m_pszAppName ); m_pszAppName = _tcsdup( _T("Dataseries viewer") );
  m_nW = m_nH = 0;
  // histogram for 16-bit diapason
  m_Hist = (int*)malloc( (1<<16)*sizeof(int) );
  // grayscale
  GetModuleFileName( NULL, qS, 192 );
  switch (VaU_Sfx(qS,_T('.'))[-1]) {
    case _T('U'): case _T('u'): PixelTypeTagSet( 1 ); break;
    default: PixelTypeTagSet( 0 ); };
  LoadResetFlags(); // reset load's flags    
  m_sPath[0] = 0; m_sNam = m_sPath;
  // read initialization profiles
  theApp.VDI_SetSectName( _T("View") );
  UtMnu_IniProfile( false ); // VD_UtMnuPUp
  ViD_IniProfile( false ); // VD_UtViewDlg
  BPM_IniProfile( false ); // VD_NMDlg1
  if (m_bUnderWinE) {
    m_bWinEstrAPI = iwine_get_APIs( &rwine_get_unix_file_name, &rwine_get_dos_file_name );
    WriteLn( m_bWinEstrAPI ?
      _T("InInst: winestrAPI is available") : _T("ERR InInst: winestrAPI isn't available") );
    if (m_bWinEstrAPI && gFlags&(1<<5)) {
      m_bWinEstrAPI = false; WriteLn( _T("InInst: [WinE DOSpaths]=On; winestrAPI is off") ); };
  }; // if m_bUnderWinE
  // modal dialog
  m_pMainWnd = &theDlg; 
  theDlg.DoModal();
  m_pMainWnd = NULL;
  free( m_Hist );
  // write initialization profiles
  // theApp.VDI_SetSectName( _T("View") ); // set in OnDestroy
  UtMnu_IniProfile( true );
  ViD_IniProfile( true );
  BPM_IniProfile( true );
  return FALSE; }

int CViewApp::ExitInstance() {
  return CWinApp::ExitInstance(); }
