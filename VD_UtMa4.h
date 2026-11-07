// Dataseries-Viewer MFC
// (c) LGPL v3.0; by S. Bochkarev
// file VD_UtMa4.h : marker's drawing service
#pragma once

//// rectangle marker ////
typedef struct {
  RECT  rR; // rectangle's statistics
} tMa4_RectStt;

extern POINT xMa4_MarkerArea; // recent rectangle ROI
void Ma4_RectSttSz( tMa4_RectStt *ARS ); // assign drawing's width,height
void Ma4_RectSttDraw( tDC *ADC, tMa4_RectStt *ARS, const CRect AROI ); // draw rectangle's statistics

