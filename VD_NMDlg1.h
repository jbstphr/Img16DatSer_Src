// Dataseries-Viewer MFC
// (c) LGPL v3.0; by S. Bochkarev
// file VD_NMDlg1.h : dialog [BPM]
#pragma once

class CVD_NMDlg1 : public CDialog {
  DECLARE_DYNAMIC(CVD_NMDlg1)
  DECLARE_MESSAGE_MAP()
  virtual void DoDataExchange( CDataExchange *pDX ); // DDX/DDV support
  virtual BOOL OnInitDialog();
  virtual BOOL OnCommand( WPARAM wParam, LPARAM lParam );
  virtual LRESULT WindowProc( UINT message, WPARAM wParam, LPARAM lParam );
  virtual BOOL PreTranslateMessage( MSG* pMsg );
  virtual void PostNcDestroy();
public:
  // stds
  CVD_NMDlg1( CWnd*pParent=NULL ); // standard constructor
  virtual ~CVD_NMDlg1();
  enum { IDD = IDD_NMDLG1 };
  // ~API-exports
  int m_nW, m_nH, m_nN;
  char *m_BPM;
  bool m_bInitialized; // ad-hoc helper tag for MFC-controls
  void Load(); // load BPM.raw,BPMM.raw
  void Save(); // save BPMM.raw
  void Erase(); // ...
  void MarkGet( CRect *AM ); // get marker
};

// CVD_NMDlg1 global exports
//const int cBPM_PalEntries = 4; // concerns _DIB8Update
void BPMDlg_Show(); // show BPM-dialog
bool BPMDlg_Active(); // 
void BPM_DIB8Update( BITMAPINFOHEADER *ABMI, RGBQUAD *APal, BYTE *ABMPBuf ); // called by BMPBuf.ImageToBMP
  // changes 1st #_PalEntries in Pal^; nrm_pix:=mx(nrm_pix,_PalEntries); BPM_pix:=0.._PalEntries-1
CPoint BPM_XY(); // returns [X],[Y] in frame's cordinates
bool BPM_MarkGet( CRect *AM ); // called by ViewDlg.OnPaint
 // the TopBottom is incremented to use IntersectRect for frame's coordinates
#define cBPM_ColorMark RGB(0,255,0) // 
#define cBPM_ColorMarkPale RGB(0,177,0) // 
COLORREF BPM_PixelBkColor( CPoint AXY ); // called by ~_PixelInspectorDraw
void BPM_IniProfile( bool AWrite ); // read/write initialization profile