//---------------------------------------------------------------------------
#include "PerceptiveHash.h"
//---------------------------------------------------------------------------
float DCTFators[SMALL_PICTURE_SIZE][DCT_SIZE];
float ReversDCTFators[SMALL_PICTURE_SIZE][DCT_SIZE];
//---------------------------------------------------------------------------
float ToLight(BYTE Red, BYTE Green, BYTE Blue)
{
   return Red * 0.299 + Green * 0.587 + Blue * 0.114;
}
//---------------------------------------------------------------------------

void ScaleImage(STexture *Image, float Result[SMALL_PICTURE_SIZE][SMALL_PICTURE_SIZE])
{
   memset(Result, 0, sizeof(float) * SMALL_PICTURE_SIZE * SMALL_PICTURE_SIZE);
   float sizeX = float(Image->sizeX) / float(SMALL_PICTURE_SIZE);
   float sizeY = float(Image->sizeY) / float(SMALL_PICTURE_SIZE);
   for (int i = 0; i < SMALL_PICTURE_SIZE; i++)
   {
      for (int o = 0; o < SMALL_PICTURE_SIZE; o++)
      {
         DWORD startX = i * sizeX;
         DWORD startY = o * sizeY;
         float startXWeight = 1 - (i * sizeX - startX);
         float startYWeight = 1 - (o * sizeY - startY);

         DWORD endX = (i + 1) * sizeX;
         DWORD endY = (o + 1) * sizeY;
         float endXWeight = (i + 1) * sizeX - endX;
         float endYWeight = (o + 1) * sizeY - endY;
         endX--;
         endY--;

         double temp = 0;
         if (Image->channels >= 3)
         {
            BYTE *pos = &Image->data[(startX + startY * Image->sizeX) * Image->bytesPerChannel * Image->channels];
            temp += ToLight(pos[0], pos[Image->bytesPerChannel], pos[Image->bytesPerChannel * 2]) * startXWeight * startYWeight;

            pos = &Image->data[(endX + startY * Image->sizeX) * Image->bytesPerChannel * Image->channels];
            temp += ToLight(pos[0], pos[Image->bytesPerChannel], pos[Image->bytesPerChannel * 2]) * endXWeight * startYWeight;

            pos = &Image->data[(startX + endY * Image->sizeX) * Image->bytesPerChannel * Image->channels];
            temp += ToLight(pos[0], pos[Image->bytesPerChannel], pos[Image->bytesPerChannel * 2]) * startXWeight * endYWeight;

            pos = &Image->data[(endX + endY * Image->sizeX) * Image->bytesPerChannel * Image->channels];
            temp += ToLight(pos[0], pos[Image->bytesPerChannel], pos[Image->bytesPerChannel * 2]) * endXWeight * endYWeight;

            for (DWORD tx = startX + 1; tx < endX; tx++)
            {
               pos = &Image->data[(tx + startY * Image->sizeX) * Image->bytesPerChannel * Image->channels];
               temp += ToLight(pos[0], pos[Image->bytesPerChannel], pos[Image->bytesPerChannel * 2]) * startYWeight;

               pos = &Image->data[(tx + endY * Image->sizeX) * Image->bytesPerChannel * Image->channels];
               temp += ToLight(pos[0], pos[Image->bytesPerChannel], pos[Image->bytesPerChannel * 2]) * endYWeight;
            }

            for (DWORD ty = startY + 1; ty < endY; ty++)
            {
               pos = &Image->data[(startX + ty * Image->sizeX) * Image->bytesPerChannel * Image->channels];
               temp += ToLight(pos[0], pos[Image->bytesPerChannel], pos[Image->bytesPerChannel * 2]) * startXWeight;

               pos = &Image->data[(endX + ty * Image->sizeX) * Image->bytesPerChannel * Image->channels];
               temp += ToLight(pos[0], pos[Image->bytesPerChannel], pos[Image->bytesPerChannel * 2]) * endXWeight;
            }

            for (DWORD tx = startX + 1; tx < endX; tx++)
            {
               for (DWORD ty = startY + 1; ty < endY; ty++)
               {
                  pos = &Image->data[(tx + ty * Image->sizeX) * Image->bytesPerChannel * Image->channels];
                  temp += ToLight(pos[0], pos[Image->bytesPerChannel], pos[Image->bytesPerChannel * 2]);
               }
            }
         }
         else
         {
            BYTE *pos = &Image->data[(startX + startY * Image->sizeX) * Image->bytesPerChannel * Image->channels];
            temp += pos[0] * startXWeight * startYWeight;

            pos = &Image->data[(endX + startY * Image->sizeX) * Image->bytesPerChannel * Image->channels];
            temp += pos[0] * endXWeight * startYWeight;

            pos = &Image->data[(startX + endY * Image->sizeX) * Image->bytesPerChannel * Image->channels];
            temp += pos[0] * startXWeight * endYWeight;

            pos = &Image->data[(endX + endY * Image->sizeX) * Image->bytesPerChannel * Image->channels];
            temp += pos[0] * endXWeight * endYWeight;

            for (DWORD tx = startX + 1; tx < endX; tx++)
            {
               pos = &Image->data[(tx + startY * Image->sizeX) * Image->bytesPerChannel * Image->channels];
               temp += pos[0] * startYWeight;

               pos = &Image->data[(tx + endY * Image->sizeX) * Image->bytesPerChannel * Image->channels];
               temp += pos[0] * endYWeight;
            }

            for (DWORD ty = startY + 1; ty < endY; ty++)
            {
               pos = &Image->data[(startX + ty * Image->sizeX) * Image->bytesPerChannel * Image->channels];
               temp += pos[0] * startXWeight;

               pos = &Image->data[(endX + ty * Image->sizeX) * Image->bytesPerChannel * Image->channels];
               temp += pos[0] * endXWeight;
            }

            for (DWORD tx = startX + 1; tx < endX; tx++)
            {
               for (DWORD ty = startY + 1; ty < endY; ty++)
               {
                  pos = &Image->data[(tx + ty * Image->sizeX) * Image->bytesPerChannel * Image->channels];
                  temp += pos[0];
               }
            }
         }
         Result[o][i] = temp / (sizeX * sizeY);
      }
   }
}
//---------------------------------------------------------------------------

void InitDCT()
{
   for (int i = 0; i < SMALL_PICTURE_SIZE; i++)
   {
      for (int o = 0; o < DCT_SIZE; o++)
      {
         DCTFators[i][o] = cos(((2 * i + 1) * o * M_PI) / double(2 * SMALL_PICTURE_SIZE));
         ReversDCTFators[i][o] = ((o == 0)?(1.0 / sqrt(2)):1) * cos(((2 * i + 1) * o * M_PI) / double(2 * SMALL_PICTURE_SIZE));
      }
   }
}
//---------------------------------------------------------------------------

void DCTImage(float Dst[DCT_SIZE][DCT_SIZE], float Src[SMALL_PICTURE_SIZE][SMALL_PICTURE_SIZE])
{
   for (int tx = 0; tx < DCT_SIZE; tx++)
   {
      for (int ty = 0; ty < DCT_SIZE; ty++)
      {
         double temp = 0;
         for (int tu = 0; tu < SMALL_PICTURE_SIZE; tu++)
         {
            for (int tv = 0; tv < SMALL_PICTURE_SIZE; tv++)
            {
               temp += Src[tu][tv] * DCTFators[tu][tx] * DCTFators[tv][ty];
            }
         }
         Dst[tx][ty] = temp * (((tx == 0)?(1.0 / sqrt(2)):1.0) * ((ty == 0)?(1.0 / sqrt(2)):1.0)) / sqrt(2 * SMALL_PICTURE_SIZE);
      }
   }
}
//---------------------------------------------------------------------------

void ReversDCTImage(float Dst[SMALL_PICTURE_SIZE][SMALL_PICTURE_SIZE], float Src[DCT_SIZE][DCT_SIZE])
{
   for (int tx = 0; tx < SMALL_PICTURE_SIZE; tx++)
   {
      for (int ty = 0; ty < SMALL_PICTURE_SIZE; ty++)
      {
         double temp = 0;
         for (int tu = 0; tu < DCT_SIZE; tu++)
         {
            for (int tv = 0; tv < DCT_SIZE; tv++)
            {
               temp += Src[tu][tv] * ReversDCTFators[tx][tu] * ReversDCTFators[ty][tv];
            }
         }
         Dst[tx][ty] = temp / sqrt(2 * SMALL_PICTURE_SIZE);
      }
   }
}
//---------------------------------------------------------------------------

void ToBinare(float Table[DCT_SIZE][DCT_SIZE], BYTE Result[HASH_LEN])
{
   float middle = 0;
   for (int i = 0; i < DCT_SIZE; i++)
   {
      for (int o = 0; o < DCT_SIZE; o++)
      {
         middle += Table[i][o];
      }
   }
   middle /= DCT_SIZE * DCT_SIZE;
   memset(Result, 0, HASH_LEN);
   BYTE *pos = Result;
   int bitBumber = 0;
   for (int i = 0; i < DCT_SIZE; i++)
   {
      for (int o = 0; o < DCT_SIZE; o++)
      {
         *pos <<= 1;
         if (Table[i][o] > middle)
         {
            *pos |= 1;
         }
         bitBumber++;
         if (bitBumber >= 8)
         {
            bitBumber = 0;
            pos++;
         }
      }
   }
}
//---------------------------------------------------------------------------

bool CalculateHash(const TCHAR *FileName, BYTE Result[HASH_LEN], DWORD &SizeX, DWORD &SizeY)
{
   STexture *data = CTexture::LoadData(FileName);
   if (data != nullptr)
   {
      SizeX = data->sizeX;
      SizeY = data->sizeY;
      float pict[SMALL_PICTURE_SIZE][SMALL_PICTURE_SIZE];
      ScaleImage(data, pict);
      CTexture::Delete(data);

      float dct[DCT_SIZE][DCT_SIZE];
      DCTImage(dct, pict);
      ToBinare(dct, Result);
      return true;
   }
   return false;
}
//---------------------------------------------------------------------------
