// Dataseries-Viewer MFC
// (c) LGPL v3.0; by S. Bochkarev
// file VD_NMDlg2.h : dialog [reserved]
#pragma once

class CVD_NMDlg2 : public CDialog {
  DECLARE_DYNAMIC(CVD_NMDlg2)
  DECLARE_MESSAGE_MAP()
  virtual void DoDataExchange( CDataExchange *pDX ); // DDX/DDV support
  virtual BOOL OnInitDialog();
  virtual BOOL OnCommand( WPARAM wParam, LPARAM lParam );
  virtual LRESULT WindowProc( UINT message, WPARAM wParam, LPARAM lParam );
  virtual BOOL PreTranslateMessage( MSG* pMsg );
  virtual void PostNcDestroy();
public:
  // stds
  CVD_NMDlg2( CWnd*pParent=NULL ); // standard constructor
  virtual ~CVD_NMDlg2();
  enum { IDD = IDD_NMDLG2 };
  // ~API-exports
  bool m_bInitialized; // ad-hoc helper tag for MFC-controls
};

// CVD_NMDlg2 global exports
