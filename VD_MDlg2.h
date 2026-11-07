// Dataseries-Viewer MFC
// (c) LGPL v3.0; by S. Bochkarev
// file VD_MDlg2.h : dialog [Files]
#pragma once

class CVD_MDlg2 : public CDialog {
  // stds
  DECLARE_DYNAMIC(CVD_MDlg2)
  DECLARE_MESSAGE_MAP()
  virtual void DoDataExchange( CDataExchange *pDX ); // DDX/DDV support
  virtual BOOL OnInitDialog();
  virtual BOOL OnCommand( WPARAM wParam, LPARAM lParam );
  virtual LRESULT WindowProc( UINT message, WPARAM wParam, LPARAM lParam );
  virtual BOOL PreTranslateMessage( MSG* pMsg );
  // services
  CListBox *m_LB; // parent's list-box
  int iGetSels( int ATag, short *ASelIdxs ); // get selections (Tag=0:selected, =1:all in the list)
public:
  // stds
  CVD_MDlg2( CWnd *pParent, CListBox *ALB ); // constructor
  virtual ~CVD_MDlg2();
  enum { IDD = IDD_MDLG2 };
  // ~API-exports
  // these flags have meaning after 'DoModal:=IDOK'
  bool m_bInitialized,
       m_bModifHave, // the files' list was modified
       m_bModifFile; // the disk-files were modified (~deleted)
};
