// Dataseries-Viewer MFC
// (c) LGPL v3.0; by S. Bochkarev
// file VD_UtTransf.h : [crop,rotate,flip]
#pragma once

// The parameters below:
//   'LB^'    : enlists files' names '*.raw' 
//   'Path'   : shall be buffered copy of 'App.sPath'
//   'Nam'    : points to ~strchr(Path,'\')+1
//   'SelCnt' : #entries in 'SelIdxs'
  
//// frame=>frame transformations ////
int UtTransf_Average( // average of selected *.raw
  CListBox *ALB, TCHAR *APath, TCHAR *ANam, int ASelCnt, short *ASelIdxs, const TCHAR *ADstNam );
  // returns #accumulated
  
int UtTransf_ToBMP( // convert selected *.raw to *.bmp
  CListBox *ALB, TCHAR *APath, TCHAR *ANam, int ASelCnt, short *ASelIdxs );
  // returns '#transformed'
  // it could modify theApp.mHist and respective stats
  
//// frame=>image transformations ////
typedef struct { int rLft,rTop,rRgt,rBtm,rSttW,rSttH,rROF; } tUtTransf;
  // "[CAL0] ROF=...; hex rot_CCW=10,_CW=20 flp_h=1,_v=2"
void UtTransf_ImgDim( // transformed image's dimension
  const tUtTransf *ARec, int *AW, int *AH );
int UtTransf( const tUtTransf *ARec, const TCHAR *ATgtSubDir, // transform selected *.raw
  CListBox *ALB, const TCHAR *APath, TCHAR *ANam, int ASelCnt, const short *ASelIdxs );
  // returns #transformed (the dimension coincides with output of _ImgDim)
  // if 'TgtSubDir=nil' then overwrite existing *.raw
