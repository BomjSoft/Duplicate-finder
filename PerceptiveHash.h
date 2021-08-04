//---------------------------------------------------------------------------
#pragma once
//---------------------------------------------------------------------------
#include <math.h>
//---------------------------------------------------------------------------
#include "Texture.h"
//---------------------------------------------------------------------------
#define SMALL_PICTURE_SIZE    32
#define DCT_SIZE     8
#define HASH_LEN     (DCT_SIZE * DCT_SIZE / 8)
//---------------------------------------------------------------------------
float ToLight(BYTE Red, BYTE Green, BYTE Blue);
void ScaleImage(STexture *Image, float Result[SMALL_PICTURE_SIZE][SMALL_PICTURE_SIZE]);
void InitDCT();
void DCTImage(float Dst[DCT_SIZE][DCT_SIZE], float Src[SMALL_PICTURE_SIZE][SMALL_PICTURE_SIZE]);
void ReversDCTImage(float Dst[SMALL_PICTURE_SIZE][SMALL_PICTURE_SIZE], float Src[DCT_SIZE][DCT_SIZE]);
void ToBinare(float Table[DCT_SIZE][DCT_SIZE], BYTE Result[DCT_SIZE * DCT_SIZE / 8]);
bool CalculateHash(const TCHAR *FileName, BYTE Result[HASH_LEN], DWORD &SizeX, DWORD &SizeY);
//---------------------------------------------------------------------------
