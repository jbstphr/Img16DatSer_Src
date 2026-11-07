// Dataseries-Viewer MFC
// (c) LGPL v3.0; by S. Bochkarev
// file VD_UtMa2.h : marker's drawing service
#pragma once

//// hrz,vrt-profiles ////
const int cMa2_gN = 5;
typedef struct {
  RECT  rR; // profiles box's rectangle
  tGrid1D rGrayGrid; // grid of gray values
  int   rGrayVal[cMa2_gN],
        rGrayLin[cMa2_gN];
} tMa2_Profile;

void Ma2_ProfileSz( tMa2_Profile *AP ); // assign drawing's width,height
  // Rd:rectangle on DC, Md:marker's location on DC
void Ma2_ProfileDrawBox( tDC *ADC, tMa2_Profile *AP ); // draw profiles box 
void Ma2_ProfileDrawHrz( tDC *ADC, tMa2_Profile *AP ); // draw hrz profile
void Ma2_ProfileDrawVrt( tDC *ADC, tMa2_Profile *AP ); // draw vrt profile