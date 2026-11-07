// Dataseries-Viewer MFC
// (c) LGPL v3.0; by S. Bochkarev
// Declaration of the GIniFile class
#pragma once

class GIniFile { public:
  TCHAR rFName[192], rSName[48]; GIniFile(); ~GIniFile();
  void SetFileName( const TCHAR *AFileName ); // sets name of ini file
  void SetSectName( const TCHAR *ASectName ); // sets name of section
  int  GetIniStr( const TCHAR *AKeyName, TCHAR *AResult ); // returns ini-value and its length
  void SetIniStr( const TCHAR *AKeyName, const TCHAR *AStr ); // writes ini-value
};

