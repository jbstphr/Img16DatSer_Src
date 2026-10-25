// Dataseries-Viewer MFC
// (c) LGPL v3.0; by S. Bochkarev
// file VD_MDlg1.h : dialog [Settings]
#pragma once

class CVD_MDlg1 : public CDialog {
  // stds
  DECLARE_DYNAMIC(CVD_MDlg1)
  DECLARE_MESSAGE_MAP()
  virtual void DoDataExchange( CDataExchange *pDX ); // DDX/DDV support
  virtual BOOL OnInitDialog();
  virtual BOOL OnCommand( WPARAM wParam, LPARAM lParam );
  virtual LRESULT WindowProc( UINT message, WPARAM wParam, LPARAM lParam );
  virtual BOOL PreTranslateMessage( MSG* pMsg );
  // services
  CCheckListBox m_List1; // [Flags]
public:
  // stds
  CVD_MDlg1( CWnd *pParent=NULL ); // standard constructor
  virtual ~CVD_MDlg1();
  enum { IDD = IDD_MDLG1 };
};
