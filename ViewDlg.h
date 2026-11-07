// Dataseries-Viewer MFC
// (c) LGPL v3.0; by S. Bochkarev
// EViewDlg.h : main dialog
#pragma once
#include "GBMPBuf.h"

class CViewDlg : public CDialog {
  // local pop-up menu
  void iDisplayLocalMenu( LPARAM APoint );
  // services
  int        m_Combo3Flg; // change flags
  int        m_WIWmn, m_WIHmn; // mn allowed monitor-window-size
  CScrollBar m_SBX, m_SBY; // the scrollbars; wisible when zoom>1
  CStatic    m_Sta1; // the label of SBX,SBY
  void iOnSizeControls( int ACX, int ACY ); // as if called by OnSize
  void iSetMarkerArea( int ACX, int ACY, bool AEnabled ); // set [marker Area]
  void iSetCutGamut(); // set [cut Gamut] (if one is active)
  CRect iDrawMarkerROI( tDC *ADC, const CRect ARd, bool ASymmROI ); // draw 'theD_ROI'
  void iCombo3DimSet( int AI, int AW, int AH ); // set dim
  void iCombo3StrSet( int AI, const TCHAR *APath, int AW, int AH ); // set path
  void iCombo3StrGet( int AI, TCHAR *APath, int *AW, int *AH, bool AUIStr=false ); // get path
  void iCombo3WndTxtSet( const TCHAR *APath ); // set wildcard's text
  void iCombo3WndTxtGet( TCHAR *APath ); // get wildcard's text
  void iList1Sort( int *ASel, int *AMaxLen ); // sort List1 in Windows Explorer Mode
  // ~API-internals
  CListBox   m_List1; // [files' list]
  bool       m_Combo3DropDown; // dropdown state
  TCHAR      m_Combo3Nam[80]; // recent file-name
  CComboBox  m_Combo3; // [wildcard] combo
  CPoint     m_DPfst, m_DPlst; // drag-points (since OnLButtonDown...)
  int        m_DPzoo; // '=-1' if LButtonDown outside Rd, '=_Z' if within Rd; '=0' if LButtonUp after '_Z'
    // if _DPzoo>0 then WM_MOUSEWHEEL scrolls [Zoom]
  void jOnChangeRdRf( bool AOnSize=false ); // set label<s> for Zoom>1 when change "RECTdisplay"
  void jReCalcRdRf( int ATag=0 ); // recalculate "RECTdisplay", "RECTframe"
    // bit#0: called from OnSize
    //    #1: called from SetImgDim (scroll to the center)
  void jProcessScrollMsg( CScrollBar *ASBar, WPARAM AWParam ); // message from scrollbar
  void jProcessOwnerDrawMessage( UINT message, WPARAM wParam, LPARAM lParam, int ATag ); // owner_draw
  void jCombo3Validate(); // "Combo3DropDown=1":from CBN_CLOSEUP; "=0":from OnLoadImage
  // stds
  virtual void DoDataExchange( CDataExchange *pDX ); // DDX/DDV support
  DECLARE_MESSAGE_MAP()
  virtual BOOL OnInitDialog();
  virtual BOOL OnCommand( WPARAM wParam, LPARAM lParam );
  virtual LRESULT WindowProc( UINT message, WPARAM wParam, LPARAM lParam );
  virtual BOOL PreTranslateMessage( MSG* pMsg );
  afx_msg HCURSOR OnQueryDragIcon();
  afx_msg void OnPaint();
  afx_msg void OnLButtonDown( UINT nFlags, CPoint point );
  afx_msg void OnLButtonUp( UINT nFlags, CPoint point );
  afx_msg void OnMouseMove( UINT nFlags, CPoint point );
  afx_msg void OnSize( UINT nType, int cx, int cy );
  afx_msg void OnDropFiles(HDROP hDropInfo);
public:
  // stds
  CViewDlg( CWnd *pParent=NULL ); // standard constructor
  enum { IDD = IDD_EVIEW_DIALOG };
  // ~API-exports
  bool       m_bInitialized, // ad-hoc helper tag for MFC-controls
             m_bPresentationUpdate; // used by OnPresentationUpdate
  int        m_Z, // [Zoom]; always >=1
             m_X, m_Y, // [Marker X,Y] in frame's coordinates
             m_LastDlgCtrlId; // tag of the last changed control (~"one of the spinedits")
  CRect      m_Rd, m_Rf; // "RECTdisplay", "RECTframe" (refer GScroll.h)
  GBMPBuf    m_BMPBuf; // 16=>8bit converter's buffers
  void GetSBRects( CRect *ARX, CRect *ARY ); // scrollbars
  void SetImgDim(); // alloc bufs for new WxH
  void OnLoadImage( int ATag, const void *AImg ); // called by App.LoadImage
    // BMPBuf.ImageToBMP( Tag=0:signed, =1:unsigned )
  void OnPresentationUpdate( bool AImmediately ); // stored image to stored DIB-buffer
  bool SuggestWHforNP( int ANP, int *AW, int *AH ); // check stored wildcards
  // non-functional GUI-objects
  CFont      m_Font, m_FontSmall; // medium Courier; small Courier
};
  // the messages to update "files' list"="List1"
  //   Msg0+0 : reassign List1
  //   Msg0+1 : keep List1
  // the bits in Combo3Flg
  //   |=1 : as if [wildcard listbox selected]    
  //   |=2 : as if [wildcard editor changed]
  //   |=FF00 : flags that [wildcard was recently changed]
  //      