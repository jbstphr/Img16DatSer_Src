// Dataseries-Viewer MFC
// (c) LGPL v3.0; by S. Bochkarev
// file VD_Ut.h : casual capture utils
#pragma once

//// common Windows utilities ////
TCHAR *VaU_AddBackSlash( TCHAR *AStr );
void VaU_RemoveBackSlash( TCHAR *AStr );
TCHAR *VaU_Sfx( const TCHAR *APath, TCHAR AChR );
  // it reflects usage for path and extension and respectivelly returns
  // - for back_slash: 'max(back_slash_pos,slash_pos)+1' or 'Path';
  // - for dot: 'last dot' after _Sfx(back_slash)
  // - for another char: 'last_char_pos' as-is.
int VaU_TrimStr( TCHAR *AStr );
void VaU_ASCII_TCHAR( const char *ASCIIStr, TCHAR *ATStr, int AMaxLength );
void VaU_TCHAR_ASCII( const TCHAR *ATStr, char *ASCIIStr, int AMaxLength );
void VaUt_EnableControls( HWND AHWnd, const int *AIDs, int ANum, bool AEnable );
void VaUt_ShowControls( HWND AHWnd, const int *AIDs, int ANum, int ACmdShow );
void VaUt_DisplayMenu( CDialog *ADlg, LPARAM APoint, HMENU AMenu ); // TrackPopupMenu
int VaUt_SpinInt( CDialog *ADlg, int AEditCtrlId ); // integer value of a spin-edit
void VaUt_SetInt( CDialog *ADlg, int AEditCtrlId, int AV ); // set integer to editbox

const TCHAR *VaUt_PathToUIStr( const TCHAR *APath ); // invoke wine_get_unix_file_name
const TCHAR *VaUt_UIStrToPath( const TCHAR *AStr ); // invoke wine_get_dos_file_name

///// file/directory open/save/select dialogs //////
bool VaU_OpenFileDlg( CWnd *ADlg, TCHAR *AFName, const TCHAR *ACaption, const TCHAR *AFilter, const TCHAR *ADefExt );
bool VaU_SaveFileDlg( CWnd *ADlg, TCHAR *AFName, const TCHAR *ACaption, const TCHAR *AFilter, const TCHAR *ADefExt );

///// temp display context for ~double buffering //////
typedef struct tagCOMPATDCREC {
  CDC      rDC;
  CBitmap  rBMP,   *rOldBMP;
  CFont    rFont,  *rOldFont;
  CPen     rPen,   *rOldPen;
  CBrush   rBrush, *rOldBrush;
  int      rW, rH;
  void Create( int AWidth, int AHeight, COLORREF AColor, CDC *APaintDC=NULL ); void Delete(); // create,delete
  CPoint GetDim(); void Blit( CDC *ADC, const CPoint ATgtXY, int AWidth=0, int AHeight=0 ); // dim,bitblit
  void SetFont( LOGFONT *ALogFont ); void SetFont( int AHeigt ); void ReleaseFont(); // font
  void SetPen( int AStyle, int AWidth, COLORREF AColor ); void ReleasePen(); // pen
  void SetBrush( int AStyle, int AHatch, COLORREF AColor ); void ReleaseBrush(); // brush
  void SolidRect( const CRect ARect, COLORREF AColor ); // fill solid rectangle
} tDC;

///// 1D-grid //////
typedef struct tagGrid1D {
  int     rMin,    // (out) min(First,Last)
          rMax,    //       max(First,Last)
          rGridFirst, //       suggested grid first value
          rGridLast,  //       suggested grid last value
          rGridSnap,  //       suggested grid step
          rNumMarks;  //       number of mark-lines (it is "number_of_inner_grid_intervals+1")
  void Init( int ANumMarks );
} tGrid1D;

//  void DrawChart2DGrid( tDC *ADC, const CRect AR, tUt02_1DGrid *AGridX, tUt02_1DGrid *AGridY,
//    const TCHAR *AFmtX, const TCHAR *AFmtY );
