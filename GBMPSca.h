// Dataseries-Viewer MFC
// (c) LGPL v3.0; by S. Bochkarev
// BitMap buffer's grayscale conversion
#pragma once

void GBMPSca_16to8Uns( int AN, // 16bit unsigned => 8bit
  int AMn, int AMx, const unsigned short *AImg, unsigned char *ABmp ); 

void GBMPSca_16to8Sig( int AN, // 16bit signed => 8bit
  int AMn, int AMx, const short *AImg, unsigned char *ABmp );

void GBMPSca_32to8( int AN, // 32bit signed => 8bit
  int AMn, int AMx, const int *AImg, unsigned char *ABmp );
