// Dataseries-Viewer MFC
// (c) LGPL v3.0; by S. Bochkarev
// Implementation of the GIniFile class
#include "StdAfx.h" // app-wide precompiled header
#include "GIniFile.h"

GIniFile::GIniFile() { rFName[0] = rSName[0] = 0; } GIniFile::~GIniFile() {}

void GIniFile::SetFileName( const TCHAR *AFileName ) { _tcscpy( rFName, AFileName ); }
void GIniFile::SetSectName( const TCHAR *ASectName ) { _tcscpy( rSName, ASectName ); }

int GIniFile::GetIniStr( const TCHAR *AKeyName, TCHAR *AResult ) {
  int qOk = GetPrivateProfileString( rSName, AKeyName, _T(""), AResult, 256, rFName );
  if (!qOk) return -1;
  if (qOk && AResult[0]==_T('\\') && AResult[1]<=_T(' ')) { // it allows to specify empty lines using back slash
    _tcscpy( AResult, &(AResult[1]) ); qOk--; }; return --qOk; }

void GIniFile::SetIniStr( const TCHAR *AKeyName, const TCHAR *AStr ) {
  if (!AStr || !AStr[0]) AStr = _T("\\");
  WritePrivateProfileString( rSName, AKeyName, AStr, rFName ); }
