// Dataseries-Viewer MFC
// (c) LGPL v3.0; by S. Bochkarev
// file VD_Ut.cpp : casual capture utils
#include "StdAfx.h"
#include "resource.h"
#include "ViewApp.h" // the application-object
#include "VD_UtViewDlg.h" // adhoc main dialog controls' access
#include "VD_Ut.h"

//// common Windows utilities ////
TCHAR *VaU_AddBackSlash( TCHAR *AStr ) { int qLen = (int)_tcslen( AStr )-1;
  if (qLen>0 && AStr[qLen]!=_T('\\') && AStr[qLen]!=_T('/')) { 
    AStr[++qLen] = _T('\\'); AStr[++qLen] = 0; };
  return AStr+qLen; }

void VaU_RemoveBackSlash( TCHAR *AStr ) { int qLen = (int)_tcslen( AStr )-1;
  if (qLen>0 && (AStr[qLen]==_T('\\') || AStr[qLen]==_T('/'))) AStr[qLen] = 0; }

TCHAR *VaU_Sfx( const TCHAR *APath, TCHAR AChR ) { switch (AChR) {
  case _T('\\'): { const TCHAR *cR0 = _tcsrchr( APath, _T('\\') ),
    *cR1 = _tcsrchr( APath, _T('/') ); if (cR0<cR1) cR0 = cR1;
    if (cR0) cR0++; else cR0 = APath; return (TCHAR*)cR0; }; 
  case _T('.'): { const TCHAR *cR0 = VaU_Sfx( APath, _T('\\') ),
    *cR1 = _tcsrchr( cR0, _T('.') ); if (!cR1) cR1 = cR0+_tcslen(cR0);
    return (TCHAR*)cR1; }; 
  default: return (TCHAR*)_tcsrchr( APath, AChR ); }; } // switch

int VaU_TrimStr( TCHAR *AStr ) { CString qStr( AStr );
  qStr.TrimRight(); qStr.TrimLeft(); _tcscpy( AStr, (LPCTSTR)qStr ); return qStr.GetLength(); }

void VaU_ASCII_TCHAR( const char *ASCIIStr, TCHAR *ATStr, int AMaxLength ) {
  #ifndef UNICODE
    strncpy( ATStr, ASCIIStr, AMaxLength ); ATStr[__min(AMaxLength-1,(int)strlen(ASCIIStr))] = 0;
  #else
    MultiByteToWideChar( CP_ACP, 0, ASCIIStr, -1, ATStr, AMaxLength-1 );
  #endif
  VaU_TrimStr( ATStr ); }

void VaU_TCHAR_ASCII( const TCHAR *ATStr, char *ASCIIStr, int AMaxLength ) {
  #ifndef UNICODE
    strcpy( ASCIIStr, ATStr ); }
  #else
    WideCharToMultiByte( CP_ACP, 0, ATStr, -1, ASCIIStr, AMaxLength-1, NULL, NULL ); }
  #endif

void VaUt_EnableControls( HWND AHWnd, const int *AIDs, int ANum, bool AEnable ) {
  while (ANum-->0) EnableWindow( GetDlgItem(AHWnd,*AIDs++), (BOOL)AEnable ); }

void VaUt_ShowControls( HWND AHWnd, const int *AIDs, int ANum, int ACmdShow ) {
  while (ANum-->0) ShowWindow( GetDlgItem(AHWnd,*AIDs++), ACmdShow ); }

void VaUt_DisplayMenu( CDialog *ADlg, LPARAM APoint, HMENU AHMenu ) { // -- TrackPopupMenu
  POINT qP = { (short)APoint, (short)((APoint>>16)&0xFFFF) };
  if (qP.x<=0 && qP.y<=0) { qP.x = qP.y = 20; ADlg->ClientToScreen( &qP ); };
  TrackPopupMenu( AHMenu, TPM_LEFTALIGN, qP.x, qP.y, 0, ADlg->m_hWnd, NULL ); }

int VaUt_SpinInt( CDialog *ADlg, int AEditCtrlId ) { // -- integer value of a spin-edit
  int qV = ADlg->GetDlgItemInt( AEditCtrlId ), qMn, qMx; // there are continuous IDC_SPIN,IDC_EDIT in resource.h
  ADlg->SendDlgItemMessage( IDC_SPIN1+(AEditCtrlId-IDC_EDIT1), UDM_GETRANGE32, (WPARAM)&qMn, (LPARAM)&qMx );
  return qMx>qMn ? mcRng(qV,qMn,qMx) : mcRng(qV,qMx,qMn); }

void VaUt_SetInt( CDialog *ADlg, int AEditCtrlId, int AV ) { // -- set integer to editbox
  TCHAR qS[32]; ADlg->GetDlgItemText( AEditCtrlId, qS, mcASz(qS)-1 ); VaU_TrimStr( qS );
  if (qS[0]>=_T('0') && qS[0]<=_T('9') && _ttoi(qS)==AV) return; // to avoid ~deselect etc
  ADlg->SetDlgItemInt( AEditCtrlId, AV ); } // only if change is really necessary

static __declspec(thread) WCHAR ixTStr[192];
const TCHAR *VaUt_PathToUIStr( const TCHAR *APath ) { // -- invoke wine_get_unix_file_name
  if (!theApp.m_bWinEstrAPI || !theApp.rwine_get_unix_file_name) return APath;
  const TCHAR *cNam = VaU_Sfx( APath, _T('\\') );
  if (cNam==APath) return APath; cNam--; const int cL = (int)(cNam-APath);
  #ifndef _UNICODE
    MultiByteToWideChar( CP_ACP, 0, APath, cL, ixTStr, 192 );
  #else
    memcpy( ixTStr, APath, cL*sizeof(WCHAR) );
  #endif
  ixTStr[cL] = 0;
  char *qUnixPath = theApp.rwine_get_unix_file_name( ixTStr );
  TCHAR *qT = (TCHAR*)ixTStr;
  if (!qUnixPath) { _stprintf( qT, _T("ERR:wine_get_unix_file_name(%.*s;%s):=nil"),
    cL, APath, cNam ); WriteLn( qT ); return APath; };
  #ifndef _UNICODE
    strcpy( qT, qUnixPath );
  #else
    MultiByteToWideChar( CP_ACP, 0, qUnixPath, -1, qT, 192 );
  #endif
  HeapFree( GetProcessHeap(), 0, qUnixPath );
  _stprintf( qT+_tcslen(qT), _T("/%s"), ++cNam );
  return qT; }

const TCHAR *VaUt_UIStrToPath( const TCHAR *AStr ) { // -- invoke wine_get_dos_file_name
  if (!theApp.m_bWinEstrAPI || !theApp.rwine_get_dos_file_name) return AStr;
  const TCHAR *cNam = VaU_Sfx( AStr, _T('\\') );
  if (cNam==AStr) return AStr; cNam--; const int cL = (int)(cNam-AStr); char *qTc = (char*)ixTStr;
  #ifndef _UNICODE
    memcpy( qTc, AStr, cL );
  #else
    WideCharToMultiByte( CP_ACP, 0, AStr, cL, qTc, 192, NULL, NULL );
  #endif
  qTc[cL] = 0; 
  WCHAR *qDOSPath = theApp.rwine_get_dos_file_name( qTc );
  TCHAR *qT = (TCHAR*)ixTStr;
  if (!qDOSPath) { _stprintf( qT, _T("ERR:wine_get_dos_file_name(%.*s;%s):=nil"),
    cL, AStr, cNam ); WriteLn( qT ); return AStr; };
  #ifndef _UNICODE
    WideCharToMultiByte( CP_ACP, 0, qDOSPath, -1, qT, 192, NULL, NULL );
  #else
    wcscpy( qT, qDOSPath );
  #endif
  HeapFree( GetProcessHeap(), 0, qDOSPath );
  _stprintf( qT+_tcslen(qT), _T("\\%s"), cNam+1 );
  return qT; }

///// file/directory open/save/select dialogs //////
static bool iVaU_FileDlg( bool AOpen, CWnd *ADlg, TCHAR *AFPath,
  const TCHAR *ACaption, const TCHAR *AFilter, const TCHAR *ADefExt, DWORD AFlags ) {
  OPENFILENAME qOFN; memset( &qOFN, 0, sizeof(qOFN) );
  qOFN.lStructSize = sizeof( qOFN );
  qOFN.hwndOwner = ADlg->m_hWnd; qOFN.hInstance = theApp.m_hInstance;
  TCHAR qFilter[256]; if (!AFilter) { int cL = _stprintf( qFilter, _T("16-bit Heraderless %s|%s|All files *.*|*.*|"),
      gWildCardDef, gWildCardDef );
    for (int qi=0;qi<cL;qi++) if (qFilter[qi]==_T('|')) qFilter[qi] = 0;    
    AFilter = qFilter; };
  qOFN.lpstrFilter = AFilter;
  qOFN.nFilterIndex = 0; qOFN.nMaxFile = 256;
  qOFN.Flags = AFlags | OFN_HIDEREADONLY | OFN_EXPLORER | OFN_NONETWORKBUTTON;
  TCHAR qFPath[256]; _tcscpy( qFPath, AFPath ); // file-dialog needs back-slashes
  const int cL = VaU_TrimStr( qFPath );for (int qi=0;qi<cL;qi++)
    if (qFPath[qi]==_T('/')) qFPath[qi] = _T('\\');
  qOFN.lpstrFile = qFPath;
  qOFN.lpstrDefExt = ADefExt ? ADefExt : gFilExtDef();
  qOFN.lpstrTitle = ACaption ? ACaption : _T("Select image_file");;
  if (!(AOpen?GetOpenFileName(&qOFN):GetSaveFileName(&qOFN))) return false;
  _tcscpy( AFPath, qFPath ); return true; }

bool VaU_OpenFileDlg( CWnd *ADlg, TCHAR *AFName, const TCHAR *ACaption, const TCHAR *AFilter, const TCHAR *ADefExt ) {
  return iVaU_FileDlg( TRUE, ADlg, AFName, ACaption, AFilter, ADefExt, OFN_PATHMUSTEXIST|OFN_FILEMUSTEXIST ); }

bool VaU_SaveFileDlg( CWnd *ADlg, TCHAR *AFName, const TCHAR *ACaption, const TCHAR *AFilter, const TCHAR *ADefExt ) {
  return iVaU_FileDlg( FALSE, ADlg, AFName, ACaption, AFilter, ADefExt, OFN_PATHMUSTEXIST ); }

///// temp display context for ~double buffering //////
void tDC::Create( int AWidth, int AHeight, COLORREF AColor, CDC *APaintDC ) { // -- create
  CDC *qTempDC = APaintDC ? APaintDC : AfxGetMainWnd()->GetDC();
  rDC.CreateCompatibleDC( qTempDC );
  rBMP.CreateCompatibleBitmap( qTempDC, AWidth, AHeight );
  if (!APaintDC) AfxGetMainWnd()->ReleaseDC( qTempDC );
  rOldBMP = rDC.SelectObject( &rBMP );
  rDC.FillSolidRect( 0, 0, AWidth, AHeight, AColor );
  rOldFont = NULL; rOldPen = NULL; rOldBrush = NULL; rW = AWidth; rH = AHeight; }
void tDC::Delete() { // -- delete
  ReleaseFont(); ReleasePen(); ReleaseBrush();
  rDC.SelectObject( rOldBMP ); rDC.DeleteDC(); rBMP.DeleteObject(); }
CPoint tDC::GetDim() { // -- dim
  return CPoint(rW,rH); }
void tDC::Blit( CDC *ADC, const CPoint ATgtXY, int AWidth, int AHeight  ) { // -- bitblt
  if (AWidth<1) { CPoint qDim = GetDim(); AWidth = qDim.x; AHeight = qDim.y; };
  ADC->BitBlt( ATgtXY.x, ATgtXY.y, AWidth, AHeight, &rDC, 0, 0, SRCCOPY ); }
void tDC::SetFont( LOGFONT *ALogFont ) { ReleaseFont(); // -- font
  ALogFont->lfCharSet = DEFAULT_CHARSET; rFont.CreateFontIndirect( ALogFont );
  rOldFont = rDC.SelectObject( &rFont ); }
void tDC::SetFont( int AHeight ) { LOGFONT qLF; // -- font
  theDlg.m_Font.GetLogFont( &qLF ); qLF.lfHeight = AHeight; SetFont( &qLF ); }
void tDC::ReleaseFont() { // -- font
  if (rOldFont) { rDC.SelectObject( rOldFont ); rFont.DeleteObject(); rOldFont = NULL; }; }
void tDC::SetPen( int AStyle, int AWidth, COLORREF AColor ) { ReleasePen(); // -- pen
  rPen.CreatePen( AStyle, AWidth, AColor ); rOldPen = rDC.SelectObject( &rPen ); }
void tDC::ReleasePen() { // -- pen
  if (rOldPen) { rDC.SelectObject( rOldPen ); rPen.DeleteObject(); rOldPen = NULL; }; }
void tDC::SetBrush( int AStyle, int AHatch, COLORREF AColor ) { ReleaseBrush(); // -- brush
  LOGBRUSH qLogBrush; qLogBrush.lbStyle = AStyle; qLogBrush.lbHatch = AHatch; qLogBrush.lbColor = AColor;
  rBrush.CreateBrushIndirect( &qLogBrush ); rOldBrush = rDC.SelectObject( &rBrush ); }
void tDC::SolidRect( const CRect ARect, COLORREF AColor ) { // -- fill solid rectangle
  rDC.FillSolidRect( ARect, AColor ); }
  
void tDC::ReleaseBrush() { // -- brush
  if (rOldBrush) { rDC.SelectObject( rOldBrush ); rBrush.DeleteObject(); rOldBrush = NULL; }; }


///// 1D-grid //////
void tGrid1D::Init( int ANumMarks ) {
  if (rMin>rMax) mcSwap( int, rMin, rMax );
  const int  cSnapColF[] = { 2, 4, 5, 8, 10, 20, 25, 40, 50,
                             100, 200, 250, 400, 500,
                             1000, 2000, 2500, 4000, 5000,
                             10000, 20000, 25000, 50000,
                             100000, 200000 };
  int qGamut = rMax-rMin, qSnap;
  for (int qi=0;qi<mcASz(cSnapColF);qi++) {
    qSnap = cSnapColF[qi]; if (qGamut/qSnap<ANumMarks) break; };
  int qMin = (rMin/qSnap)*qSnap; if (qMin<rMin) qMin += qSnap;
  int qMax = (rMax/qSnap)*qSnap; if (qMax>rMax) qMax -= qSnap;
  rGridFirst = qMin; rGridLast = qMax; rGridSnap = qSnap;
  rNumMarks = (qMax-qMin)/qSnap+1; }
