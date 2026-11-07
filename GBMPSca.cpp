// Dataseries-Viewer MFC
// (c) LGPL v3.0; by S. Bochkarev
// BitMap buffer's grayscale conversion
#include "StdAfx.h"
#include <emmintrin.h>
#include "GBMPSca.h"

// service //
static void iComplement255( int AN, unsigned char *ABmp ) { // invert unsigned bytes
  int qi = 0;
  #if 1 // sse-code
    __m128i v_255 = _mm_set1_epi8( (char)255 );
    const int cNsimd = AN&~15; for (;qi<cNsimd;qi+=16) {
      __m128i p = _mm_loadu_si128( (const __m128i*)(ABmp+qi) );
      p = _mm_subs_epu8( v_255, p ); _mm_storeu_si128( (__m128i*)(ABmp+qi), p ); }; // for i
  #endif
  // handle remaining pixels and leave
  for (;qi<AN;qi++) ABmp[qi] = 255-ABmp[qi]; }

static inline unsigned iScale( unsigned AGam ) { // scale (either for sse, normal instructions)
  const int cScale = AGam ? (255<<16)/AGam : 255<<16;
  //static int ixDumGam = 0; if (ixDumGam!=AGam) { ixDumGam = AGam; TCHAR qS[80];
  //  _stprintf( qS, _T("scale(%05d):=%05d %04X"), AGam, cScale, cScale ); WriteLn( qS ); };
  return cScale; }

static inline unsigned iShift( unsigned AScale ) { // to avoid 16-bit overflow
  int qSh = 0; while (AScale>=1<<16) { qSh++; AScale >>= 1; }; return qSh; }
  // when "formal Scale>=1 shl 16" then "shift Scale right", "shift the Input left"

// SSE-scale //
static inline __m128i ii_cut( const __m128i mn, const __m128i mx, const __m128i a ) { // result := rng(a,mn,mx)-mn
  __m128i p = _mm_min_epi16( a, mx ); p = _mm_max_epi16( p, mn ); return p; }

static inline __m128i ii_uns( const __m128i vMinOrg, const __m128i vXor, // unsigned operation's chain
  const __m128i vMin, const __m128i vMax, const __m128i vSca, const unsigned short *AImg, int ASh ) {
  __m128i p = _mm_loadu_si128( (const __m128i*)AImg );
  p = _mm_xor_si128( p, vXor ); p = ii_cut( vMin, vMax, p ); p = _mm_xor_si128( p, vXor );
  p = _mm_sub_epi16( p, vMinOrg ); p = _mm_slli_epi16 ( p, ASh ); 
  p = _mm_mulhi_epu16( p, vSca ); return p; }

static inline __m128i ii_sig( // signed operation's chain
  const __m128i vMin, const __m128i vMax, const __m128i vSca, const short *AImg, int ASh ) {
  __m128i p = _mm_loadu_si128( (const __m128i*)AImg );
  p = ii_cut( vMin, vMax, p );
  p = _mm_sub_epi16( p, vMin ); p = _mm_slli_epi16 ( p, ASh );
  p = _mm_mulhi_epu16( p, vSca ); return p; }
  
void GBMPSca_16to8Uns( int AN, // -- 16bit unsigned => 8bit
  int AMn, int AMx, const unsigned short *AImg, unsigned char *ABmp ) {
  int qGam = AMx-AMn; if (qGam<0) { int qT = AMn; AMn = AMx; AMx = qT; } else if (!qGam) qGam=1;
  unsigned qScale = iScale( abs(qGam) );
  int qi = 0;
  #if 1 // sse-code
    int     qSh = iShift( qScale );
    __m128i v_xor = _mm_set1_epi16( (short)0x8000 ),
            v_minorg = _mm_set1_epi16( (short)AMn ),
            v_min = _mm_xor_si128( v_minorg, v_xor ),
            v_max = _mm_xor_si128( _mm_set1_epi16((short)AMx), v_xor ),
            v_sca = _mm_set1_epi16( (short)(qScale>>qSh) );
    const int cNsimd = AN&~15; for (;qi<cNsimd;qi+=16) { __m128i
      p1 = ii_uns( v_minorg, v_xor, v_min, v_max, v_sca, AImg+qi, qSh ),
      p2 = ii_uns( v_minorg, v_xor, v_min, v_max, v_sca, AImg+qi+8, qSh );
      _mm_storeu_si128( (__m128i*)(ABmp+qi), _mm_packus_epi16(p1,p2) ); }; // for i
  #endif
  // handle remaining pixels and leave
  for (;qi<AN;qi++) { unsigned short qV = AImg[qi];
    if (qV<=AMn) { ABmp[qi] = 0; continue; };
    if (qV>=AMx) { ABmp[qi] = 255; continue; };
    ABmp[qi] = (unsigned)(qV-AMn)*qScale>>16; };
  // complementary unsigned bytes
  if (qGam<0) iComplement255( AN, ABmp ); }

void GBMPSca_16to8Sig( int AN, // -- 16bit signed => 8bit
  int AMn, int AMx, const short *AImg, unsigned char *ABmp ) {
  int qGam = AMx-AMn; if (qGam<0) { int qT = AMn; AMn = AMx; AMx = qT; } else if (!qGam) qGam=1;
  unsigned qScale = iScale( abs(qGam) );
  int qi = 0;
  #if 1 // sse-code
    int     qSh = iShift( qScale );
    __m128i v_min = _mm_set1_epi16( (short)AMn ),
            v_max = _mm_set1_epi16( (short)AMx ),
            v_sca = _mm_set1_epi16( (short)(qScale>>qSh) );
    const int cNsimd = AN&~15; for (;qi<cNsimd;qi+=16) { __m128i
      p1 = ii_sig( v_min, v_max, v_sca, AImg+qi, qSh ),
      p2 = ii_sig( v_min, v_max, v_sca, AImg+qi+8, qSh );
      _mm_storeu_si128( (__m128i*)(ABmp+qi), _mm_packus_epi16(p1,p2) ); }; // for i
  #endif
  // handle remaining pixels and leave
  for (;qi<AN;qi++) { short qV = AImg[qi];
    if (qV<=AMn) { ABmp[qi] = 0; continue; };
    if (qV>=AMx) { ABmp[qi] = 255; continue; };
    ABmp[qi] = (unsigned)(qV-AMn)*qScale>>16; };
  // complementary unsigned bytes
  if (qGam<0) iComplement255( AN, ABmp ); }
  
///////////////////////////////////

static inline __m128i ii_32( // signed operation's chain
  const __m128i vMin, const __m128i vGam, const __m128 vSca, const int *AImg ) {
  __m128i i = _mm_loadu_si128( (const __m128i*)AImg ); // load
  i = _mm_sub_epi32( i, vMin );
  // clamp (blend chooses mx if mask is true, else keeps val)
  __m128i mask = _mm_cmpgt_epi32( i, _mm_setzero_si128() ); // FFFFFFFF for e>=0, 00000000 for e<0
  i = _mm_and_si128( i, mask ); // if (e<=0) then e:=0
  mask = _mm_cmpgt_epi32( i, vGam ); // if i>mx then mask:=FFFFFFFF
  i = _mm_or_si128( _mm_and_si128(mask,vGam), _mm_andnot_si128(mask,i) ); // Blend
  // convert
  __m128 f = _mm_cvtepi32_ps( i ); f = _mm_mul_ps( f, vSca );
  return _mm_cvtps_epi32( f ); }

void GBMPSca_32to8( int AN, // -- 32bit signed => 8bit
  int AMn, int AMx, const int *AImg, unsigned char *ABmp ) {
  int qGam = AMx-AMn; if (qGam<0) { AMn = AMx; qGam *= -1; } else if (!qGam) qGam = 1;
  float qScale = 255.0f/qGam;
  int qi = 0;
  #if 1 // sse-code
    __m128i v_min = _mm_set1_epi32( AMn ),
            v_gam = _mm_set1_epi32( qGam );
    __m128  v_scale = _mm_set1_ps( qScale );
    const int cNsimd = AN&~15; for (;qi<cNsimd;qi+=16) {
      __m128i r0 = ii_32( v_min, v_gam, v_scale, AImg+qi+0 ),
              r1 = ii_32( v_min, v_gam, v_scale, AImg+qi+4 ),
              r2 = ii_32( v_min, v_gam, v_scale, AImg+qi+8 ),
              r3 = ii_32( v_min, v_gam, v_scale, AImg+qi+12 );
      // Pack signed 32-bit to 16-bit (saturates automatically)
      __m128i pack16_0 = _mm_packs_epi32( r0, r1 ), // Holds 8 ints
              pack16_1 = _mm_packs_epi32( r2, r3 ); // Holds 8 ints
      // Pack signed 16-bit to unsigned 8-bit (saturates to 0-255)
      __m128i pack8 = _mm_packus_epi16( pack16_0, pack16_1 ); // Holds all 16 bytes
      _mm_storeu_si128( (__m128i*)(ABmp+qi), pack8 ); // Store the 16 bytes into the destination array
    }; // for i
  #endif
  // handle remaining pixels and leave
  for (;qi<AN;qi++) { int qV = AImg[qi]-AMn;
    if (qV<=0) { ABmp[qi] = 0; continue; };
    if (qV>=qGam) { ABmp[qi] = 255; continue; };
    ABmp[qi] = (unsigned char)(qV*qScale+0.4999f); };
  // complementary unsigned bytes
  if (AMn==AMx) iComplement255( AN, ABmp ); }
