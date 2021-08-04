//---------------------------------------------------------------------------
#pragma once
//---------------------------------------------------------------------------
#include <windows.h>
#include <tchar.h>
//---------------------------------------------------------------------------
#define ABS(A)          (((A) > 0)?(A):(-(A)))
#define DWORD_MIX(A)    ((((A) >> 24) & 0xFF) | (((A) >> 8) & 0xFF00) | (((A) << 8) & 0xFF0000) | (((A) << 24) & 0xFF000000))
#define MIN(A,B)  (((A) < (B))?(A):(B))
#define MAX(A,B)  (((A) > (B))?(A):(B))
//---------------------------------------------------------------------------
#define BMP_SIGNATURE      0x4D42
#ifndef BI_ALPHABITFIELDS
#define BI_ALPHABITFIELDS  6
#endif
//---------------------------------------------------------------------------
#define TGA_SIGNATURE         "TRUEVISION-XFILE.\0"
#define TGA_SIGNATURE_SIZE    18
#define TGA_NONE              0
#define TGA_PALETTE           1
#define TGA_RGB               2
#define TGA_A                 3
#define TGA_RLE_PALETTE       9
#define TGA_RLE_RGB           10
#define TGA_RLE_A             11
#define TGA_TYPE_ALPHA        0x0F
#define TGA_TYPE_HORIZONTAL   0x10
#define TGA_TYPE_VERTICAL     0x20
//---------------------------------------------------------------------------
#define PNG_SIGNATURE           "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A"
#define PNG_SIGNATURE_SIZE      8
//---------------------------------------------------------------------------
#include "png.h"
//---------------------------------------------------------------------------
#define JPG_SIGNATURE           "\xFF\xD8"
#define JPG_SIGNATURE_SIZE      2
//---------------------------------------------------------------------------
#include "jpeglib.h"
#include <setjmp.h>
//---------------------------------------------------------------------------
struct SJPEGErrorMessage
{
   jpeg_error_mgr pub;
   jmp_buf setjmp_buffer;
};
//---------------------------------------------------------------------------
enum ETextureFormat {tfUnknow, tfBMP, tfTGA, tfPNG, tfJPG};
//---------------------------------------------------------------------------
struct STexture
{
public:
   DWORD sizeX;
   DWORD sizeY;
   BYTE channels;
   BYTE bytesPerChannel;
   BYTE *data;
};
//---------------------------------------------------------------------------
class CTexture
{
private:
   static ETextureFormat DetectFormat(FILE *File);
   static STexture *LoadBMP(FILE *File);
   static STexture *LoadTGA(FILE *File);
   static STexture *LoadPNG(FILE *File);
   static STexture *LoadJPG(FILE *File);

public:
   static void Delete(STexture *Texture);
   static STexture *Copy(STexture *Texture);

   static STexture *LoadData(const TCHAR *FileName);
   static STexture *LoadData(FILE *File);

   static bool SaveBMP(const STexture *Texture, const TCHAR *FileName);
   static bool SaveTGA(const STexture *Texture, const TCHAR *FileName);
   static bool SavePNG(const STexture *Texture, const TCHAR *FileName);
   static bool SaveJPG(const STexture *Texture, const TCHAR *FileName);
   static bool SaveJPG(const STexture *Texture, const TCHAR *FileName, int Quality);
   static bool SaveBMP(const STexture *Texture, FILE *File);
   static bool SaveTGA(const STexture *Texture, FILE *File);
   static bool SavePNG(const STexture *Texture, FILE *File);
   static bool SaveJPG(const STexture *Texture, FILE *File);
   static bool SaveJPG(const STexture *Texture, FILE *File, int Quality);
};
//---------------------------------------------------------------------------
