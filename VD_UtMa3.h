// Dataseries-Viewer MFC
// (c) LGPL v3.0; by S. Bochkarev
// file VD_UtMa3.h : marker's drawing service
#pragma once

//// histogram ////
typedef struct {
  RECT  rR; // histogram's rectangle
} tMa3_Histogram;

void Ma3_HistogramSz( tMa3_Histogram *AHi ); // assign drawing's width,height
void Ma3_HistogramDraw( tDC *ADC, tMa3_Histogram *AHi ); // draw histogram