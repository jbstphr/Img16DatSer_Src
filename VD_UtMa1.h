// Dataseries-Viewer MFC
// (c) LGPL v3.0; by S. Bochkarev
// file VD_UtMa1.h : marker's drawing service
#pragma once

//// pixel-inspector ////
typedef struct {
  RECT   rR; // drawing rectangle
  int    rW, rH; // cell's width,height
  bool   rHex; // display pixels in hex-format
} tMa1_PixelInspector;

void Ma1_PixelInspectorSz( tMa1_PixelInspector *API ); // assign drawing's width,height
void Ma1_PixelInspectorDraw( tDC *ADC, tMa1_PixelInspector *API, CRect AROI ); // draw pixel inspector
 