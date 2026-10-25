// Dataseries-Viewer MFC
// (c) LGPL v3.0; by S. Bochkarev
// file VD_MDlg3.h : dialog [Help]
#pragma once

class CVD_MDlg3 : public CDialog {
  // stds
  DECLARE_DYNAMIC(CVD_MDlg3)
  DECLARE_MESSAGE_MAP()
  virtual void DoDataExchange( CDataExchange *pDX ); // DDX/DDV support
  virtual BOOL OnInitDialog();
  virtual BOOL OnCommand( WPARAM wParam, LPARAM lParam );
  virtual LRESULT WindowProc( UINT message, WPARAM wParam, LPARAM lParam );
  virtual BOOL PreTranslateMessage( MSG* pMsg );
  afx_msg HBRUSH OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor );
  // services
  void iGetCap( int ATopic, TCHAR *AStr ); // get topic's caption (for multitopics only)
  void iGetBody( int ATopic, TCHAR *AStr ); // get topic's body
  CFont  m_Font; // adhoc font
  CBrush m_BkBrush; // Brush used to paint the background
public:
  // stds
  CVD_MDlg3( CWnd *pParent=NULL ); // standard constructor
  virtual ~CVD_MDlg3();
  enum { IDD = IDD_MDLG3 };
  // ~API-exports
  const TCHAR *m_Title, *m_Str[5];
  int n_Str;
};

// CVD_MDlg3 global exports
void HelpDlg( const TCHAR *ATitle, CWnd *AParent, const TCHAR *AStr0, // modal help-dialog
  const TCHAR *AStr1=NULL, const TCHAR *AStr2=NULL, const TCHAR *AStr3=NULL, const TCHAR *AStr4=NULL );