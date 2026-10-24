// Dataseries-Viewer MFC
// (c) LGPL v3.0; by S. Bochkarev
// StdAfx.h : application-wide precompiled headers
#pragma once

#define VC_EXTRALEAN // Exclude rarely-used stuff from Windows headers
#include "targetver.h"

#define _AFX_ALL_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_NON_CONFORMING_SWPRINTFS
#pragma conform(forScope, off)
#pragma warning(disable: 4288 4430 4311 4313 4996)
#define NO_WARN_MBCS_MFC_DEPRECATION

#if _MSC_VER >= 1600  // for VS>=2010
  #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS
  #define USE_DIRECT2D // ~exclusively used inside GBMPBuf.cpp,.h
#endif

#include <afxwin.h>   // MFC core and standard components
#include <afxdtctl.h> // MFC support for Internet Explorer 4 Common Controls

typedef __int64 tLL;

#define mcASz(AnArray) ((int)(sizeof(AnArray)/sizeof(AnArray[0]))) // array's size
#define mcRng(V,Min,Max) (((V)<(Min))?(Min):(((V)>(Max))?(Max):(V))) // a gamut
#define mcSwap(tN,A,B) { tN T=(tN)(A); A=(tN)(B); B=T; } // exchange

void WriteLn( const TCHAR *AMsg ); // trace to terminal