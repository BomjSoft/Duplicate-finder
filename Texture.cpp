//---------------------------------------------------------------------------
#include "Texture.h"
//---------------------------------------------------------------------------
void CTexture::Delete(STexture *Texture)
{
   if (Texture != nullptr)
   {
      if (Texture->data != nullptr)
      {
         delete[] (BYTE*)Texture->data;
      }
      delete Texture;
   }
}
//---------------------------------------------------------------------------

STexture *CTexture::Copy(STexture *Texture)
{
   if (Texture != nullptr)
   {
      STexture *answer = new STexture;
      answer->sizeX = Texture->sizeX;
      answer->sizeY = Texture->sizeY;
      answer->channels = Texture->channels;
      answer->bytesPerChannel = Texture->bytesPerChannel;
      if (Texture->data != nullptr)
      {
         answer->data = new BYTE[answer->sizeX * answer->sizeY * answer->channels * answer->bytesPerChannel];
         memcpy(answer->data, Texture->data, answer->sizeX * answer->sizeY * answer->channels * answer->bytesPerChannel);
      }
      else
      {
         answer->data = nullptr;
      }
      return answer;
   }
   return nullptr;
}
//---------------------------------------------------------------------------

STexture *CTexture::LoadData(const TCHAR *FileName)
{
#ifdef UNICODE
   FILE *file = _wfopen(FileName, L"rb");
#else
   FILE *file = fopen(FileName, "rb");
#endif
   if (file)
   {
      STexture *result = LoadData(file);
      fclose(file);
      return result;
   }
   return NULL;
}
//---------------------------------------------------------------------------

STexture *CTexture::LoadData(FILE *File)
{
   switch (DetectFormat(File))
   {
      case tfBMP:
         return LoadBMP(File);

      case tfTGA:
         return LoadTGA(File);

      case tfPNG:
         return LoadPNG(File);

      case tfJPG:
         return LoadJPG(File);

      case tfUnknow:
      default:
         return nullptr;
   }
}
//---------------------------------------------------------------------------

ETextureFormat CTexture::DetectFormat(FILE *File)
{
   BYTE buffer[MAX(MAX(PNG_SIGNATURE_SIZE, JPG_SIGNATURE_SIZE), TGA_SIGNATURE_SIZE)];
   fseek(File, 0, SEEK_SET);
   fread(buffer, 1, MAX(PNG_SIGNATURE_SIZE, JPG_SIGNATURE_SIZE), File);
   if (memcmp(buffer, PNG_SIGNATURE, PNG_SIGNATURE_SIZE) == 0)
   {
      return tfPNG;
   }
   if (memcmp(buffer, JPG_SIGNATURE, JPG_SIGNATURE_SIZE) == 0)
   {
      return tfJPG;
   }
   fseek(File, 0, SEEK_END);
   DWORD fileSize = ftell(File);
   if ((*((WORD *)buffer) == BMP_SIGNATURE)&&(*((DWORD *)(&buffer[2])) == fileSize))
   {
      return tfBMP;
   }
   fseek(File, -TGA_SIGNATURE_SIZE, SEEK_END);
   fread(buffer, 1, TGA_SIGNATURE_SIZE, File);
   if (memcmp(buffer,TGA_SIGNATURE,TGA_SIGNATURE_SIZE) == 0)
   {
      return tfTGA;
   }
   fseek(File, 0, SEEK_SET);
   fread(buffer, 1, 1, File);
   fseek(File, buffer[0], SEEK_CUR);
   fread(buffer, 1, 2, File);
   if ((buffer[1] <= 11)&&((buffer[1] <= 3)||(buffer[1] >= 9)))
   {
      return tfTGA;
   }
   return tfUnknow;
}
//---------------------------------------------------------------------------

STexture *CTexture::LoadBMP(FILE *File)
{
   fseek(File, 0, SEEK_END);
   DWORD fileSize = ftell(File);
   BITMAPFILEHEADER fileHeader;
   fseek(File, 0, SEEK_SET);
   fread(&fileHeader, 1, sizeof(fileHeader), File);
   if ((fileHeader.bfType != BMP_SIGNATURE)||(fileSize != fileHeader.bfSize))
   {
      return nullptr;
   }
   BITMAPV5HEADER header;
   fread(&header, 1, sizeof(header), File);
   if (header.bV5Size == sizeof(BITMAPCOREHEADER))
   {
      BITMAPCOREHEADER temp;
      memcpy(&temp, &header, sizeof(temp));
      header.bV5Size = temp.bcSize;
      header.bV5Width = temp.bcWidth;
      header.bV5Height = temp.bcHeight;
      header.bV5Planes = temp.bcPlanes;
      header.bV5BitCount = temp.bcBitCount;
   }
   memset(((char*)&header) + header.bV5Size, 0, sizeof(header) - header.bV5Size);
   if ((header.bV5Width == 0)||(header.bV5Height == 0)||(header.bV5Planes != 1)||(header.bV5BitCount == 0))
   {
      return nullptr;
   }
   if (header.bV5Size < sizeof(BITMAPV4HEADER))
   {
      if (header.bV5Size + sizeof(BITMAPFILEHEADER) < fileHeader.bfOffBits)
      {
         fread(&header.bV5RedMask, 1, sizeof(DWORD), File);
         fread(&header.bV5GreenMask, 1, sizeof(DWORD), File);
         fread(&header.bV5BlueMask, 1, sizeof(DWORD), File);
         fread(&header.bV5AlphaMask, 1, sizeof(DWORD), File);
      }
      else if (header.bV5BitCount == 16)
      {
         header.bV5RedMask = 0x7C00;
         header.bV5GreenMask = 0x03E0;
         header.bV5BlueMask = 0x001F;
         header.bV5AlphaMask = 0x0000;
      }
      else if (header.bV5BitCount == 32)
      {
         header.bV5RedMask = 0x00FF0000;
         header.bV5GreenMask = 0x0000FF00;
         header.bV5BlueMask = 0x000000FF;
         header.bV5AlphaMask = 0x00000000;
      }
   }

   RGBQUAD colorTable[256];
   if (header.bV5BitCount <= 8)
   {
      fseek(File, sizeof(fileHeader) + header.bV5Size, SEEK_SET);
      if (header.bV5Size == sizeof(BITMAPCOREHEADER))
      {
         BYTE temp[256 * 3];
         fread(temp, 1, sizeof(temp), File);
         for (int i = 0; i < 256; i++)
         {
            colorTable[i].rgbRed = temp[i * 3 + 0];
            colorTable[i].rgbGreen = temp[i * 3 + 1];
            colorTable[i].rgbBlue = temp[i * 3 + 2];
            colorTable[i].rgbReserved = 0;
         }
      }
      else
      {
         fread(colorTable, 1, sizeof(colorTable), File);
      }
   }

   bool usedAlpha = ((((header.bV5BitCount == 16)||(header.bV5BitCount == 32))&&(header.bV5AlphaMask != 0))||(header.bV5BitCount == 64));

   STexture *answer = new STexture;
   if (answer == nullptr)
   {
      return nullptr;
   }
   answer->sizeX = ABS(header.bV5Width);
   answer->sizeY = ABS(header.bV5Height);
   answer->channels = (usedAlpha?4:3);
   answer->bytesPerChannel = ((header.bV5BitCount <= 32)?1:2);
   answer->data = new BYTE[answer->sizeX * answer->sizeY * answer->channels * answer->bytesPerChannel];
   if (answer->data == nullptr)
   {
      delete answer;
      return nullptr;
   }

   fseek(File, fileHeader.bfOffBits, SEEK_SET);
   switch (header.bV5Compression)
   {
      case BI_RGB:
         {
            DWORD lineLength = answer->sizeX * header.bV5BitCount;
            if (lineLength & 7)
            {
               lineLength = (lineLength & (DWORD(-1) - 7)) + 8;
            }
            lineLength /= 8;
            DWORD lineAlign = 0;
            if(lineLength & 3)
            {
               lineAlign = 4 - (lineLength & 3);
            }
            BYTE *buffer = new BYTE[(lineLength + lineAlign) * answer->sizeY];
            BYTE *p = buffer;
            fread(buffer, 1, (lineLength + lineAlign) * answer->sizeY, File);
            BYTE *texture = (BYTE*)answer->data;

            switch (header.bV5BitCount)
            {
               case 1:
                  for (DWORD i = 0; i < answer->sizeY; i++)
                  {
                     for (DWORD o = 0; o < (answer->sizeX & (DWORD(-1) - 7)); o += 8)
                     {
                        BYTE temp = *p++;
                        memcpy(texture, &colorTable[(temp >> 7) & 1], answer->channels);
                        texture += answer->channels;
                        memcpy(texture, &colorTable[(temp >> 6) & 1], answer->channels);
                        texture += answer->channels;
                        memcpy(texture, &colorTable[(temp >> 5) & 1], answer->channels);
                        texture += answer->channels;
                        memcpy(texture, &colorTable[(temp >> 4) & 1], answer->channels);
                        texture += answer->channels;
                        memcpy(texture, &colorTable[(temp >> 3) & 1], answer->channels);
                        texture += answer->channels;
                        memcpy(texture, &colorTable[(temp >> 2) & 1], answer->channels);
                        texture += answer->channels;
                        memcpy(texture, &colorTable[(temp >> 1) & 1], answer->channels);
                        texture += answer->channels;
                        memcpy(texture, &colorTable[(temp >> 0) & 1], answer->channels);
                        texture += answer->channels;
                     }
                     if (answer->sizeX & 7)
                     {
                        for (DWORD o = answer->sizeX & 7; o != 0; o--)
                        {
                           memcpy(texture, &colorTable[(*p >> (8 - o)) & 1], answer->channels);
                           texture += answer->channels;
                        }
                        p++;
                     }
                     p += lineAlign;
                  }
                  break;

               case 4:
                  for (DWORD i = 0; i < answer->sizeY; i++)
                  {
                     for (DWORD o = 0; o < (answer->sizeX & (DWORD(-1) - 1)); o += 2)
                     {
                        BYTE temp = *p++;
                        memcpy(texture, &colorTable[temp >> 4], answer->channels);
                        texture += answer->channels;
                        memcpy(texture, &colorTable[temp & 0x0F], answer->channels);
                        texture += answer->channels;
                     }
                     if (answer->sizeX & 1)
                     {
                        memcpy(texture, &colorTable[*p >> 4], answer->channels);
                        texture += answer->channels;
                        p++;
                     }
                     p += lineAlign;
                  }
                  break;

               case 8:
                  for (DWORD i = 0; i < answer->sizeY; i++)
                  {
                     for (DWORD o = 0; o < answer->sizeX; o++)
                     {
                        BYTE temp = *p++;
                        memcpy(texture, &colorTable[temp], answer->channels);
                        texture += answer->channels;
                     }
                     p += lineAlign;
                  }
                  break;

               case 24:
                  for (DWORD i = 0; i < answer->sizeY; i++)
                  {
                     memcpy(texture, p, answer->sizeX * 3);
                     p += answer->sizeX * 3 + lineAlign;
                     texture += answer->sizeX * 3;
                  }
                  break;

               default:
                  Delete(answer);
                  delete[] buffer;
                  return nullptr;
            }
            delete[] buffer;
         }
         break;

      case BI_RLE8:
         {
            BYTE command[2];
            BYTE *texture = (BYTE*)answer->data;
            DWORD x = 0;
            DWORD y = 0;
            while (fread(command, 1, 2, File) != 0)
            {
               if (command[0] == 0)
               {
                  if (command[1] < 3)
                  {
                     if (command[1] == 0)
                     {
                        x = 0;
                        y++;
                     }
                     else if (command[1] == 1)
                     {
                        break;
                     }
                     else
                     {
                        BYTE delta[2];
                        fread(delta, 1, 2, File);
                        x += delta[0];
                        y += delta[1];
                     }
                  }
                  else if (y < answer->sizeY)
                  {
                     BYTE tempCommand = command[1];
                     while (command[1] != 0)
                     {
                        command[1]--;
                        BYTE temp;
                        fread(&temp, 1, 1, File);
                        if (x < answer->sizeX)
                        {
                           memcpy(&texture[(y * answer->sizeX + x) * 3], &colorTable[temp], answer->channels);
                           x++;
                        }
                     }
                     if (tempCommand & 1)
                     {
                        fseek(File, 1, SEEK_CUR);
                     }
                  }
               }
               else if (y < answer->sizeY)
               {
                  while (command[0] != 0)
                  {
                     command[0]--;
                     if (x >= answer->sizeX)
                     {
                        break;
                     }
                     memcpy(&texture[(y * answer->sizeX + x) * 3], &colorTable[command[1]], answer->channels);
                     x++;
                  }
               }
            }
         }
         break;

      case BI_RLE4:
         {
            BYTE command[2];
            BYTE *texture = (BYTE*)answer->data;
            DWORD x = 0;
            DWORD y = 0;
            while (fread(command, 1, 2, File) != 0)
            {
               if (command[0] == 0)
               {
                  if (command[1] < 3)
                  {
                     if (command[1] == 0)
                     {
                        x = 0;
                        y++;
                     }
                     else if (command[1] == 1)
                     {
                        break;
                     }
                     else
                     {
                        BYTE delta[2];
                        fread(delta, 1, 2, File);
                        x += delta[0];
                        y += delta[1];
                     }
                  }
                  else if (y < answer->sizeY)
                  {
                     BYTE tempCommand = command[1];
                     while (command[1] != 0)
                     {
                        command[1]--;
                        BYTE temp;
                        fread(&temp, 1, 1, File);
                        if (x < answer->sizeX)
                        {
                           memcpy(&texture[(y * answer->sizeX + x) * 3], &colorTable[temp >> 4], answer->channels);
                           x++;
                        }
                        if (command[1] != 0)
                        {
                           command[1]--;
                           if (x < answer->sizeX)
                           {
                              memcpy(&texture[(y * answer->sizeX + x) * 3], &colorTable[temp & 0x0F], answer->channels);
                              x++;
                           }
                        }
                     }
                     tempCommand &= 3;
                     if ((tempCommand == 1)||(tempCommand == 2))
                     {
                        fseek(File, 1, SEEK_CUR);
                     }
                  }
               }
               else if (y < answer->sizeY)
               {
                  while (command[0] != 0)
                  {
                     command[0]--;
                     if (x >= answer->sizeX)
                     {
                        break;
                     }
                     memcpy(&texture[(y * answer->sizeX + x) * 3], &colorTable[command[1] >> 4], answer->channels);
                     x++;
                     if (command[0] != 0)
                     {
                        command[0]--;
                        if (x >= answer->sizeX)
                        {
                           break;
                        }
                        memcpy(&texture[(y * answer->sizeX + x) * 3], &colorTable[command[1] & 0x0F], answer->channels);
                        x++;
                     }
                  }
               }
            }
         }
         break;

      case BI_BITFIELDS:
      case BI_ALPHABITFIELDS:
         {
            DWORD lineLength = answer->sizeX * header.bV5BitCount;
            if (lineLength & 7)
            {
               lineLength = (lineLength & (DWORD(-1) - 7)) + 8;
            }
            lineLength /= 8;
            DWORD lineAlign = 0;
            if(lineLength & 3)
            {
               lineAlign = 4 - (lineLength & 3);
            }
            BYTE *buffer = new BYTE[(lineLength + lineAlign) * answer->sizeY];
            BYTE *p = buffer;
            fread(buffer, 1, (lineLength + lineAlign) * answer->sizeY, File);
            BYTE *texture = answer->data;

            DWORD uniMask = (header.bV5BitCount == 16)?0x8000:0x80000000;
            int delta[4];
            DWORD mask[4];
            DWORD orMask[4];
            mask[0] = header.bV5RedMask;
            mask[1] = header.bV5GreenMask;
            mask[2] = header.bV5BlueMask;
            mask[3] = header.bV5AlphaMask;
            for (int i = 0; i < 4; i++)
            {
               delta[i] = header.bV5BitCount - 8;
               if (mask[i] != 0)
               {
                  while ((mask[i] & uniMask) == 0)
                  {
                     mask[i] <<= 1;
                     delta[i]--;
                  }
                  mask[i] >>= header.bV5BitCount - 8;
               }
               orMask[i] = 0;
               while ((mask[i] | orMask[i]) != 0xFF)
               {
                  orMask[i] = (orMask[i] << 1) | 1;
               }
            }

            switch (header.bV5BitCount)
            {
               case 16:
                  {
                     DWORD t1 = mask[0];
                     mask[0] = mask[2];
                     mask[2] = t1;
                     t1 = orMask[0];
                     orMask[0] = orMask[2];
                     orMask[2] = t1;
                     int t2 = delta[0];
                     delta[0] = delta[2];
                     delta[2] = t2;
                     for (DWORD i = 0; i < answer->sizeY; i++)
                     {
                        for (DWORD o = 0; o < answer->sizeX; o++)
                        {
                           WORD temp = *((WORD*)p);
                           p += 2;
                           for (BYTE t = 0; t < answer->channels; t++)
                           {
                              if (delta[t] > 0)
                              {
                                 texture[t] = (temp >> delta[t]) & mask[t];
                              }
                              else
                              {
                                 texture[t] = (temp << -delta[t]) & mask[t];
                              }
                              if (texture[t] & (orMask[t] + 1))
                              {
                                 texture[t] |= orMask[t];
                              }
                           }
                           texture += answer->channels;
                        }
                        p += lineAlign;
                     }
                  }
                  break;

               case 32:
                  for (DWORD i = 0; i < answer->sizeY; i++)
                  {
                     for (DWORD o = 0; o < answer->sizeX; o++)
                     {
                        DWORD temp = *((DWORD*)p);
                        temp = DWORD_MIX(temp);
                        p += 4;
                        for (BYTE t = 0; t < answer->channels; t++)
                        {
                           if (delta[t] > 0)
                           {
                              texture[t] = (temp >> delta[t]) & mask[t];
                           }
                           else
                           {
                              texture[t] = (temp << -delta[t]) & mask[t];
                           }
                           if (texture[t] & (orMask[t] + 1))
                           {
                              texture[t] |= orMask[t];
                           }
                        }
                        texture += answer->channels;
                     }
                     p += lineAlign;
                  }
                  break;

               default:
                  Delete(answer);
                  delete[] buffer;
                  return nullptr;
            }
            delete[] buffer;
         }
         break;

      case BI_JPEG:
      case BI_PNG:
         Delete(answer);
         return nullptr;

      default:
         Delete(answer);
         return nullptr;
   }
   if (header.bV5Width < 0)
   {
      const DWORD pixelSize = answer->bytesPerChannel * answer->channels;
      const DWORD lineLen = pixelSize * answer->sizeX;
      const DWORD imgSize = lineLen * answer->sizeY;
      BYTE *line = answer->data;
      BYTE *buffer = new BYTE[pixelSize];
      for (DWORD i = 0; i < imgSize; i += lineLen)
      {
         for (DWORD o = 0; o < answer->sizeX / 2; o++)
         {
            memcpy(buffer, &line[o * pixelSize], pixelSize);
            memcpy(&line[o * pixelSize], &line[(answer->sizeX - o - 1) * pixelSize], pixelSize);
            memcpy(&line[(answer->sizeX - o - 1) * pixelSize], buffer, pixelSize);
         }
         line += lineLen;
      }
      delete[] buffer;
   }
   if (header.bV5Height > 0)
   {
      const DWORD lineLen = answer->bytesPerChannel * answer->channels * answer->sizeX;
      BYTE *data = answer->data;
      BYTE *buffer = new BYTE[lineLen];
      for (DWORD i = 0; i < answer->sizeY / 2; i++)
      {
         memcpy(buffer, &data[i * lineLen], lineLen);
         memcpy(&data[i * lineLen], &data[(answer->sizeY - i - 1) * lineLen], lineLen);
         memcpy(&data[(answer->sizeY - i - 1) * lineLen], buffer, lineLen);
      }
      delete[] buffer;
   }
   if (answer->channels >= 3)
   {
      const DWORD pixelSize = answer->bytesPerChannel * answer->channels;
      const DWORD imgSize = pixelSize * answer->sizeX * answer->sizeY;
      BYTE *data = answer->data;
      BYTE buffer[256];
      for (DWORD i = 0; i < imgSize; i += pixelSize)
      {
         memcpy(buffer, data, answer->bytesPerChannel);
         memcpy(data, &data[answer->bytesPerChannel * 2], answer->bytesPerChannel);
         memcpy(&data[answer->bytesPerChannel * 2], buffer, answer->bytesPerChannel);
         data += pixelSize;
      }
   }
   return answer;
}
//---------------------------------------------------------------------------

STexture *CTexture::LoadTGA(FILE *File)
{
   fseek(File, 0, SEEK_SET);

   BYTE idLength;
   BYTE usedColorMap;
   BYTE imgType;

   fread(&idLength, 1, sizeof(BYTE), File);
   fread(&usedColorMap, 1, sizeof(BYTE), File);
   fread(&imgType, 1, sizeof(BYTE), File);

   WORD colorMapStart = 0;
   WORD colorMapLen = 0;
   BYTE colorMapBits = 0;

   fread(&colorMapStart, 1, sizeof(WORD), File);
   fread(&colorMapLen, 1, sizeof(WORD), File);
   fread(&colorMapBits, 1, sizeof(BYTE), File);

   WORD width;
   WORD height;
   BYTE bitsPerPixel;
   BYTE description;
   fseek(File, 4, SEEK_CUR);
   fread(&width, 1, sizeof(WORD), File);
   fread(&height, 1, sizeof(WORD), File);
   fread(&bitsPerPixel, 1, sizeof(BYTE), File);
   fread(&description, 1, sizeof(BYTE), File);

   if ((imgType == TGA_NONE)||((width | height) == 0))
   {
      return nullptr;
   }
   fseek(File, idLength, SEEK_CUR);
   STexture *answer = nullptr;
   switch (imgType)
   {
      case TGA_PALETTE:
         {
            answer = new STexture;
            if (answer == nullptr)
            {
               return nullptr;
            }
            answer->sizeX = width;
            answer->sizeY = height;
            answer->bytesPerChannel = 1;
            switch (colorMapBits)
            {
               case 8:
                  answer->channels = 1;
                  break;

               case 16:
               case 24:
                  answer->channels = 3;
                  break;

               case 32:
                  answer->channels = 3 + (((description & TGA_TYPE_ALPHA) != 0)?1:0);
                  break;

               default:
                  delete answer;
                  return nullptr;
            }
            const DWORD imgSize = answer->sizeX * answer->sizeY * answer->bytesPerChannel * answer->channels;
            answer->data = new BYTE[imgSize];
            if (answer->data == nullptr)
            {
               delete answer;
               return nullptr;
            }
            if (bitsPerPixel == 8)
            {
               const DWORD pixelSize = answer->channels * answer->bytesPerChannel;
               BYTE *colorMap = new BYTE[pixelSize * colorMapLen];
               fread(colorMap, 1, pixelSize * colorMapLen, File);
               for (DWORD i = 0; i < imgSize; i += pixelSize)
               {
                  BYTE temp;
                  fread(&temp, 1, sizeof(BYTE), File);
                  temp -= colorMapStart;
                  if (temp < colorMapLen)
                  {
                     memcpy(&((BYTE*)answer->data)[i], &colorMap[pixelSize * temp], pixelSize);
                  }
               }
               delete[] colorMap;
            }
            else
            {
               Delete(answer);
               return nullptr;
            }
         }
         break;

      case TGA_RGB:
         {
            answer = new STexture;
            if (answer == nullptr)
            {
               return nullptr;
            }
            answer->sizeX = width;
            answer->sizeY = height;
            answer->bytesPerChannel = 1;
            switch (bitsPerPixel)
            {
               case 8:
                  answer->channels = 1;
                  break;

               case 16:
               case 24:
                  answer->channels = 3;
                  break;

               case 32:
                  answer->channels = 3 + (((description & TGA_TYPE_ALPHA) != 0)?1:0);
                  break;

               default:
                  delete answer;
                  return nullptr;
            }
            answer->data = new BYTE[answer->sizeX * answer->sizeY * answer->bytesPerChannel * answer->channels];
            if (answer->data == nullptr)
            {
               delete answer;
               return nullptr;
            }
            if (bitsPerPixel != 16)
            {
               fread(answer->data, 1, answer->sizeX * answer->sizeY * answer->bytesPerChannel * answer->channels, File);
            }
            else
            {
               const DWORD imgSize = answer->sizeX * answer->sizeY * answer->bytesPerChannel;
               BYTE *pixel = (BYTE*)answer->data;
               for (DWORD i = 0; i < imgSize; i++)
               {
                  WORD temp;
                  fread(&temp, 1, sizeof(WORD), File);
                  pixel[0] = (temp << 3) & 0xF8;
                  pixel[1] = (temp >> 2) & 0xF8;
                  pixel[2] = (temp >> 7) & 0xF8;
                  pixel += 3;
               }
            }
         }
         break;

      case TGA_A:
         {
            answer = new STexture;
            if (answer == nullptr)
            {
               return nullptr;
            }
            answer->sizeX = width;
            answer->sizeY = height;
            answer->bytesPerChannel = bitsPerPixel / 8;
            if (answer->bytesPerChannel == 0)
            {
               answer->bytesPerChannel = 1;
            }
            answer->channels = 1;
            answer->data = new BYTE[answer->sizeX * answer->sizeY * answer->bytesPerChannel * answer->channels];
            if (answer->data == nullptr)
            {
               delete answer;
               return nullptr;
            }
            if (bitsPerPixel >= 8)
            {
               fread(answer->data, 1, answer->sizeX * answer->sizeY * answer->bytesPerChannel * answer->channels, File);
            }
            else
            {
               Delete(answer);
               return nullptr;
            }
         }
         break;

      case TGA_RLE_PALETTE:
         {
            answer = new STexture;
            if (answer == nullptr)
            {
               return nullptr;
            }
            answer->sizeX = width;
            answer->sizeY = height;
            answer->bytesPerChannel = 1;
            switch (colorMapBits)
            {
               case 8:
                  answer->channels = 1;
                  break;

               case 16:
               case 24:
                  answer->channels = 3;
                  break;

               case 32:
                  answer->channels = 3 + (((description & TGA_TYPE_ALPHA) != 0)?1:0);
                  break;

               default:
                  delete answer;
                  return nullptr;
            }
            const DWORD imgSize = answer->sizeX * answer->sizeY * answer->bytesPerChannel * answer->channels;
            answer->data = new BYTE[imgSize];
            if (answer->data == nullptr)
            {
               delete answer;
               return nullptr;
            }
            if (bitsPerPixel == 8)
            {
               const DWORD pixelSize = answer->channels * answer->bytesPerChannel;
               BYTE *colorMap = new BYTE[pixelSize * colorMapLen];
               fread(colorMap, 1, pixelSize * colorMapLen, File);
               const DWORD imgSize = answer->sizeX * answer->sizeY;
               BYTE *pixel = (BYTE*)answer->data;
               DWORD position = 0;
               while (position < imgSize)
               {
                  BYTE rleId;
                  fread(&rleId, 1, sizeof(BYTE), File);
                  if (rleId < 128)
                  {
                     rleId++;
                     if (position + rleId > imgSize)
                     {
                        rleId = imgSize - position;
                     }
                     for (DWORD i = 0; i < rleId; i++)
                     {
                        BYTE temp;
                        fread(&temp, 1, sizeof(BYTE), File);
                        temp -= colorMapStart;
                        if (temp < colorMapLen)
                        {
                           memcpy(&pixel[i], &colorMap[pixelSize * temp], pixelSize);
                        }
                     }
                     pixel += rleId * pixelSize;
                     position += rleId;
                  }
                  else
                  {
                     rleId -= 127;
                     if (position + rleId > imgSize)
                     {
                        rleId = imgSize - position;
                     }
                     position += rleId;
                     BYTE temp;
                     fread(&temp, 1, sizeof(BYTE), File);
                     temp -= colorMapStart;
                     if (temp < colorMapLen)
                     {
                        memcpy(pixel, &colorMap[pixelSize * temp], pixelSize);
                     }
                     BYTE *buffer = pixel;
                     pixel += answer->bytesPerChannel * answer->channels;
                     for (BYTE i = 1; i < rleId; i++)
                     {
                        memcpy(pixel, buffer, answer->bytesPerChannel * answer->channels);
                        pixel += answer->bytesPerChannel * answer->channels;
                     }
                  }
               }
            }
            else
            {
               Delete(answer);
               return nullptr;
            }
         }
         break;

      case TGA_RLE_RGB:
         {
            answer = new STexture;
            if (answer == nullptr)
            {
               return nullptr;
            }
            answer->sizeX = width;
            answer->sizeY = height;
            answer->bytesPerChannel = 1;
            switch (bitsPerPixel)
            {
               case 8:
                  answer->channels = 1;
                  break;

               case 16:
               case 24:
                  answer->channels = 3;
                  break;

               case 32:
                  answer->channels = 3 + (((description & TGA_TYPE_ALPHA) != 0)?1:0);
                  break;

               default:
                  delete answer;
                  return nullptr;
            }
            answer->data = new BYTE[answer->sizeX * answer->sizeY * answer->bytesPerChannel * answer->channels];
            if (answer->data == nullptr)
            {
               delete answer;
               return nullptr;
            }
            const DWORD imgSize = answer->sizeX * answer->sizeY;
            BYTE *pixel = (BYTE*)answer->data;
            DWORD position = 0;
            while (position < imgSize)
            {
               BYTE rleId;
               fread(&rleId, 1, sizeof(BYTE), File);
               if (rleId < 128)
               {
                  rleId++;
                  if (position + rleId > imgSize)
                  {
                     rleId = imgSize - position;
                  }
                  if (bitsPerPixel != 16)
                  {
                     fread(pixel, 1, rleId * answer->bytesPerChannel * answer->channels, File);
                  }
                  else
                  {
                     BYTE *buffer = pixel;
                     for (DWORD i = 0; i < rleId; i++)
                     {
                        WORD temp;
                        fread(&temp, 1, sizeof(WORD), File);
                        buffer[0] = (temp << 3) & 0xF8;
                        buffer[1] = (temp >> 2) & 0xF8;
                        buffer[2] = (temp >> 7) & 0xF8;
                        buffer += 3;
                     }
                  }
                  pixel += rleId * answer->bytesPerChannel * answer->channels;
                  position += rleId;
               }
               else
               {
                  rleId -= 127;
                  if (position + rleId > imgSize)
                  {
                     rleId = imgSize - position;
                  }
                  position += rleId;
                  if (bitsPerPixel != 16)
                  {
                     fread(pixel, 1, answer->bytesPerChannel * answer->channels, File);
                  }
                  else
                  {
                     WORD temp;
                     fread(&temp, 1, sizeof(WORD), File);
                     pixel[0] = (temp << 3) & 0xF8;
                     pixel[1] = (temp >> 2) & 0xF8;
                     pixel[2] = (temp >> 7) & 0xF8;
                  }
                  BYTE *buffer = pixel;
                  pixel += answer->bytesPerChannel * answer->channels;
                  for (BYTE i = 1; i < rleId; i++)
                  {
                     memcpy(pixel, buffer, answer->bytesPerChannel * answer->channels);
                     pixel += answer->bytesPerChannel * answer->channels;
                  }
               }
            }
         }
         break;

      case TGA_RLE_A:
         {
            answer = new STexture;
            if (answer == nullptr)
            {
               return nullptr;
            }
            answer->sizeX = width;
            answer->sizeY = height;
            answer->bytesPerChannel = bitsPerPixel / 8;
            if (answer->bytesPerChannel == 0)
            {
               answer->bytesPerChannel = 1;
            }
            answer->channels = 1;
            answer->data = new BYTE[answer->sizeX * answer->sizeY * answer->bytesPerChannel * answer->channels];
            if (answer->data == nullptr)
            {
               delete answer;
               return nullptr;
            }
            if (bitsPerPixel >= 8)
            {
               const DWORD imgSize = answer->sizeX * answer->sizeY;
               BYTE *pixel = (BYTE*)answer->data;
               DWORD position = 0;
               while (position < imgSize)
               {
                  BYTE rleId;
                  fread(&rleId, 1, sizeof(BYTE), File);
                  if (rleId < 128)
                  {
                     rleId++;
                     if (position + rleId > imgSize)
                     {
                        rleId = imgSize - position;
                     }
                     fread(pixel, 1, rleId * answer->bytesPerChannel, File);
                     pixel += rleId * answer->bytesPerChannel;
                     position += rleId;
                  }
                  else
                  {
                     rleId -= 127;
                     if (position + rleId > imgSize)
                     {
                        rleId = imgSize - position;
                     }
                     position += rleId;
                     fread(pixel, 1, answer->bytesPerChannel, File);
                     BYTE *buffer = pixel;
                     pixel += answer->bytesPerChannel * answer->channels;
                     for (BYTE i = 1; i < rleId; i++)
                     {
                        memcpy(pixel, buffer, answer->bytesPerChannel);
                        pixel += answer->bytesPerChannel;
                     }
                  }
               }
            }
            else
            {
               Delete(answer);
               return nullptr;
            }
         }
         break;

      default:
         return nullptr;
   }

   if (description & TGA_TYPE_HORIZONTAL)
   {
      const DWORD pixelSize = answer->bytesPerChannel * answer->channels;
      const DWORD lineLen = pixelSize * answer->sizeX;
      const DWORD imgSize = lineLen * answer->sizeY;
      BYTE *line = (BYTE*)answer->data;
      BYTE *buffer = new BYTE[pixelSize];
      for (DWORD i = 0; i < imgSize; i += lineLen)
      {
         for (DWORD o = 0; o < answer->sizeX / 2; o++)
         {
            memcpy(buffer, &line[o * pixelSize], pixelSize);
            memcpy(&line[o * pixelSize], &line[(answer->sizeX - o - 1) * pixelSize], pixelSize);
            memcpy(&line[(answer->sizeX - o - 1) * pixelSize], buffer, pixelSize);
         }
         line += lineLen;
      }
      delete[] buffer;
   }
   if (!(description & TGA_TYPE_VERTICAL))
   {
      const DWORD lineLen = answer->bytesPerChannel * answer->channels * answer->sizeX;
      BYTE *data = (BYTE*)answer->data;
      BYTE *buffer = new BYTE[lineLen];
      for (DWORD i = 0; i < answer->sizeY / 2; i++)
      {
         memcpy(buffer, &data[i * lineLen], lineLen);
         memcpy(&data[i * lineLen], &data[(answer->sizeY - i - 1) * lineLen], lineLen);
         memcpy(&data[(answer->sizeY - i - 1) * lineLen], buffer, lineLen);
      }
      delete[] buffer;
   }
   if (answer->channels >= 3)
   {
      const DWORD pixelSize = answer->bytesPerChannel * answer->channels;
      const DWORD imgSize = pixelSize * answer->sizeX * answer->sizeY;
      BYTE *data = answer->data;
      BYTE buffer[256];
      for (DWORD i = 0; i < imgSize; i += pixelSize)
      {
         memcpy(buffer, data, answer->bytesPerChannel);
         memcpy(data, &data[answer->bytesPerChannel * 2], answer->bytesPerChannel);
         memcpy(&data[answer->bytesPerChannel * 2], buffer, answer->bytesPerChannel);
         data += pixelSize;
      }
   }
   return answer;
}
//---------------------------------------------------------------------------

void PNGReadFunction(png_structp png_ptr, png_bytep data, png_size_t length)
{
   FILE *file = (FILE*)png_get_io_ptr(png_ptr);
   fread(data, 1, length, file);
}
//---------------------------------------------------------------------------

STexture *CTexture::LoadPNG(FILE *File)
{
   fseek(File, 0, SEEK_SET);
   png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
   if (png_ptr == nullptr)
   {
      return nullptr;
   }
   png_infop info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == nullptr)
   {
      png_destroy_read_struct(&png_ptr, nullptr, nullptr);
      return nullptr;
   }

   STexture *answer = nullptr;
   png_byte **rows = nullptr;
   if (setjmp(png_jmpbuf(png_ptr)))
   {
      png_destroy_read_struct(&png_ptr, &info_ptr, 0);
      CTexture::Delete(answer);
      if (rows != nullptr)
      {
         delete[] rows;
      }
      return nullptr;
   }

   png_set_read_fn(png_ptr, File, PNGReadFunction);
   png_set_sig_bytes(png_ptr, 0);
   png_read_info(png_ptr, info_ptr);

   png_uint_32 width = 0;
   png_uint_32 height = 0;
   int bit_depth = 0;
   int color_type = 0;
   png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, 0, 0, 0);
   if (color_type == PNG_COLOR_TYPE_PALETTE)
   {
      png_set_palette_to_rgb(png_ptr);
   }
   if ((color_type == PNG_COLOR_TYPE_GRAY)&&(bit_depth < 8))
   {
      png_set_expand_gray_1_2_4_to_8(png_ptr);
   }
   if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
   {
      png_set_tRNS_to_alpha(png_ptr);
   }

   int intent;
   if (png_get_sRGB(png_ptr, info_ptr, &intent))
   {
      png_set_gamma(png_ptr, PNG_DEFAULT_sRGB, PNG_DEFAULT_sRGB);
   }
   else
   {
      double image_gamma;
      if (png_get_gAMA(png_ptr, info_ptr, &image_gamma))
      {
         png_set_gamma(png_ptr, PNG_DEFAULT_sRGB, image_gamma);
      }
      else
      {
         png_set_gamma(png_ptr, PNG_DEFAULT_sRGB, 0.45455);
      }
   }
   png_set_interlace_handling(png_ptr);

   png_read_update_info(png_ptr, info_ptr);
   png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, 0, 0, 0);

   answer = new STexture;
   answer->sizeX = width;
   answer->sizeY = height;
   switch (color_type)
   {
      case PNG_COLOR_TYPE_GRAY:
         answer->channels = 1;
         break;

      case PNG_COLOR_TYPE_GRAY_ALPHA:
         answer->channels = 2;
         break;

      case PNG_COLOR_TYPE_RGB:
         answer->channels = 3;
         break;

      case PNG_COLOR_TYPE_RGB_ALPHA:
         answer->channels = 4;
         break;

      default:
         png_destroy_read_struct(&png_ptr, &info_ptr, 0);
         delete answer;
         return nullptr;
   }
   answer->bytesPerChannel = bit_depth / 8;
   answer->data = new BYTE[answer->sizeX * answer->sizeY * answer->channels * answer->bytesPerChannel];

   rows = new png_byte*[height];
   for (png_uint_32 i = 0; i < height; i++)
   {
      rows[i] = &answer->data[i * answer->sizeX * answer->channels * answer->bytesPerChannel];
   }
   png_read_image(png_ptr, rows);
   png_destroy_read_struct(&png_ptr, &info_ptr, 0);
   delete[] rows;
   return answer;
}
//---------------------------------------------------------------------------

void JPEGErrorExit(j_common_ptr cinfo)
{
   SJPEGErrorMessage *myerr = (SJPEGErrorMessage*)cinfo->err;
   (*cinfo->err->output_message) (cinfo);
   longjmp(myerr->setjmp_buffer, 1);
}
//---------------------------------------------------------------------------

STexture *CTexture::LoadJPG(FILE *File)
{
   fseek(File, 0, SEEK_SET);
   jpeg_decompress_struct cinfo;
   SJPEGErrorMessage jerr;

   cinfo.err = jpeg_std_error(&jerr.pub);
   jerr.pub.error_exit = JPEGErrorExit;
   if (setjmp(jerr.setjmp_buffer))
   {
      jpeg_destroy_decompress(&cinfo);
      return nullptr;
   }
   jpeg_create_decompress(&cinfo);
   jpeg_stdio_src(&cinfo, File);
   jpeg_read_header(&cinfo, TRUE);
   jpeg_start_decompress(&cinfo);

   int row_stride = cinfo.output_width * cinfo.output_components;
   STexture *result = new STexture;
   result->sizeX = cinfo.output_width;
   result->sizeY = cinfo.output_height;
   result->channels = cinfo.output_components;
   result->bytesPerChannel = 1;
   result->data = new BYTE[result->sizeX * result->sizeY * result->channels];
   JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);

   int row = 0;
   while (cinfo.output_scanline < cinfo.output_height)
   {
      jpeg_read_scanlines(&cinfo, buffer, 1);
      memcpy(&result->data[row * row_stride], buffer[0], row_stride);
      row++;
   }

   jpeg_finish_decompress(&cinfo);
   jpeg_destroy_decompress(&cinfo);
   return result;
}
//---------------------------------------------------------------------------

bool CTexture::SaveBMP(const STexture *Texture, const TCHAR *FileName)
{
#ifdef UNICODE
   FILE *file = _wfopen(FileName, L"wb");
#else
   FILE *file = fopen(FileName, "wb");
#endif
   if (file)
   {
      bool result = SaveBMP(Texture, file);
      fclose(file);
      return result;
   }
   return false;
}
//---------------------------------------------------------------------------

bool CTexture::SaveBMP(const STexture *Texture, FILE *File)
{
   if ((Texture == nullptr)||(Texture->data == nullptr))
   {
      return false;
   }
   DWORD lineSize = Texture->sizeX * Texture->channels * Texture->bytesPerChannel;
   int align = 4 - (lineSize & 3);
   if (align >= 4)
   {
      align = 0;
   }
   DWORD imgDataSize = (lineSize + align) * Texture->sizeY;

   fseek(File, 0, SEEK_SET);
   BITMAPFILEHEADER fileHeader;
   fileHeader.bfType = BMP_SIGNATURE;
   fileHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + imgDataSize;
   fileHeader.bfReserved1 = 0;
   fileHeader.bfReserved2 = 0;
   fileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
   fwrite(&fileHeader, 1, sizeof(BITMAPFILEHEADER), File);

   BITMAPINFOHEADER header;
   memset(&header, 0, sizeof(BITMAPINFOHEADER));
   header.biSize = sizeof(BITMAPINFOHEADER);
   header.biWidth = Texture->sizeX;
   header.biHeight = Texture->sizeY;
   header.biPlanes = 1;
   header.biBitCount = Texture->bytesPerChannel * Texture->channels * 8;
   header.biCompression = ((Texture->channels < 4)?BI_RGB:BI_ALPHABITFIELDS);
   header.biSizeImage = imgDataSize;
   fwrite(&header, 1, sizeof(BITMAPINFOHEADER), File);

   BYTE zero[4];
   memset(zero, 0, 4);
   for (int i = Texture->sizeY - 1; i >= 0; i--)
   {
      BYTE *pos = &Texture->data[i * lineSize];
      if (Texture->channels >= 3)
      {
         for (DWORD o = 0; o < Texture->sizeX; o++)
         {
            fwrite(&pos[Texture->bytesPerChannel * 2], 1, Texture->bytesPerChannel, File);
            fwrite(&pos[Texture->bytesPerChannel], 1, Texture->bytesPerChannel, File);
            fwrite(pos, 1, Texture->bytesPerChannel, File);
            if (Texture->channels > 3)
            {
               fwrite(&pos[Texture->bytesPerChannel * 3], 1, (Texture->channels - 3) * Texture->bytesPerChannel, File);
            }
            pos += Texture->channels * Texture->bytesPerChannel;
         }
      }
      else
      {
         fwrite(pos, 1, lineSize, File);
      }
      fwrite(zero, 1, align, File);
   }
   return true;
}
//---------------------------------------------------------------------------

bool CTexture::SaveTGA(const STexture *Texture, const TCHAR *FileName)
{
#ifdef UNICODE
   FILE *file = _wfopen(FileName, L"wb");
#else
   FILE *file = fopen(FileName, "wb");
#endif
   if (file)
   {
      bool result = SaveTGA(Texture, file);
      fclose(file);
      return result;
   }
   return false;
}
//---------------------------------------------------------------------------

bool CTexture::SaveTGA(const STexture *Texture, FILE *File)
{
   if ((Texture == nullptr)||(Texture->data == nullptr))
   {
      return false;
   }
   fseek(File, 0, SEEK_SET);
   DWORD temp = 0;
   fwrite(&temp, 1, 2, File);
   temp = ((Texture->channels == 1)?TGA_A:(((Texture->channels == 3)||(Texture->channels == 4))?TGA_RGB:TGA_NONE));
   fwrite(&temp, 1, 1, File);
   temp = 0;
   fwrite(&temp, 1, 4, File);
   fwrite(&temp, 1, 1, File);
   fwrite(&temp, 1, 4, File);
   temp = Texture->sizeX;
   fwrite(&temp, 1, 2, File);
   temp = Texture->sizeY;
   fwrite(&temp, 1, 2, File);
   temp = Texture->channels * Texture->bytesPerChannel * 8;
   fwrite(&temp, 1, 1, File);
   temp = ((Texture->channels == 4)?8:0) | TGA_TYPE_VERTICAL;
   fwrite(&temp, 1, 1, File);
   if (Texture->channels >= 3)
   {
      BYTE *pos = Texture->data;
      for (DWORD i = 0; i < Texture->sizeX * Texture->sizeY; i++)
      {
         fwrite(&pos[Texture->bytesPerChannel * 2], 1, Texture->bytesPerChannel, File);
         fwrite(&pos[Texture->bytesPerChannel], 1, Texture->bytesPerChannel, File);
         fwrite(pos, 1, Texture->bytesPerChannel, File);
         if (Texture->channels > 3)
         {
            fwrite(&pos[Texture->bytesPerChannel * 3], 1, (Texture->channels - 3) * Texture->bytesPerChannel, File);
         }
         pos += Texture->channels * Texture->bytesPerChannel;
      }
   }
   else
   {
      fwrite(Texture->data, 1, Texture->sizeX * Texture->sizeY * Texture->channels * Texture->bytesPerChannel, File);
   }
   temp = 0;
   fwrite(&temp, 1, 4, File);
   fwrite(&temp, 1, 4, File);
   fwrite(TGA_SIGNATURE, 1, TGA_SIGNATURE_SIZE, File);
   return true;
}
//---------------------------------------------------------------------------

bool CTexture::SavePNG(const STexture *Texture, const TCHAR *FileName)
{
#ifdef UNICODE
   FILE *file = _wfopen(FileName, L"wb");
#else
   FILE *file = fopen(FileName, "wb");
#endif
   if (file)
   {
      bool result = SavePNG(Texture, file);
      fclose(file);
      return result;
   }
   return false;
}
//---------------------------------------------------------------------------

void PNGWriteFunction(png_structp png_ptr, png_bytep data, png_size_t length)
{
   FILE *file = (FILE*)png_get_io_ptr(png_ptr);
   fwrite(data, 1, length, file);
}
//---------------------------------------------------------------------------

void PNGFlushFunction(png_structp png_ptr)
{
}
//---------------------------------------------------------------------------

bool CTexture::SavePNG(const STexture *Texture, FILE *File)
{
   if ((Texture == nullptr)||(Texture->data == nullptr))
   {
      return false;
   }
   fseek(File, 0, SEEK_SET);

   png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
   if (png_ptr == nullptr)
   {
      return false;
   }
   png_infop info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == nullptr)
   {
      png_destroy_write_struct(&png_ptr, nullptr);
      return false;
   }
   if (setjmp(png_jmpbuf(png_ptr)))
   {
      png_destroy_write_struct(&png_ptr, &info_ptr);
      return false;
   }
   png_set_write_fn(png_ptr, File, PNGWriteFunction, PNGFlushFunction);
   int color_type;
   switch (Texture->channels)
   {
      case 1:
         color_type = PNG_COLOR_TYPE_GRAY;
         break;

      case 2:
         color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
         break;

      case 3:
         color_type = PNG_COLOR_TYPE_RGB;
         break;

      case 4:
         color_type = PNG_COLOR_TYPE_RGB_ALPHA;
         break;

      default:
         png_destroy_write_struct(&png_ptr, &info_ptr);
         return false;
   }
   png_set_IHDR(png_ptr, info_ptr, Texture->sizeX, Texture->sizeY, Texture->bytesPerChannel * 8, color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
   png_write_info(png_ptr, info_ptr);

   png_byte **rows = new png_byte*[Texture->sizeY];
   for (DWORD i = 0; i < Texture->sizeY; i++)
   {
      rows[i] = &Texture->data[i * Texture->sizeX * Texture->channels * Texture->bytesPerChannel];
   }
   png_write_image(png_ptr, rows);
   png_destroy_write_struct(&png_ptr, &info_ptr);
   delete[] rows;
   return true;
}
//---------------------------------------------------------------------------

bool CTexture::SaveJPG(const STexture *Texture, const TCHAR *FileName)
{
   return SaveJPG(Texture, FileName, 85);
}
//---------------------------------------------------------------------------

bool CTexture::SaveJPG(const STexture *Texture, const TCHAR *FileName, int Quality)
{
#ifdef UNICODE
   FILE *file = _wfopen(FileName, L"wb");
#else
   FILE *file = fopen(FileName, "wb");
#endif
   if (file)
   {
      bool result = SaveJPG(Texture, file);
      fclose(file);
      return result;
   }
   return false;
}
//---------------------------------------------------------------------------

bool CTexture::SaveJPG(const STexture *Texture, FILE *File)
{
   return SaveJPG(Texture, File, 85);
}
//---------------------------------------------------------------------------

bool CTexture::SaveJPG(const STexture *Texture, FILE *File, int Quality)
{
   if ((Texture == nullptr)||(Texture->data == nullptr))
   {
      return false;
   }
   fseek(File, 0, SEEK_SET);

   jpeg_compress_struct cinfo;
   jpeg_error_mgr jerr;

   cinfo.err = jpeg_std_error(&jerr);
   jpeg_create_compress(&cinfo);

   jpeg_stdio_dest(&cinfo, File);

   cinfo.image_width = Texture->sizeX;
   cinfo.image_height = Texture->sizeY;
   cinfo.input_components = Texture->channels;
   cinfo.in_color_space = JCS_RGB;
   jpeg_set_defaults(&cinfo);
   jpeg_set_quality(&cinfo, Quality, TRUE);

   jpeg_start_compress(&cinfo, TRUE);

   int row_stride = Texture->sizeX * Texture->bytesPerChannel * Texture->channels;
   JSAMPROW row_pointer[1];
   while (cinfo.next_scanline < cinfo.image_height)
   {
      row_pointer[0] = &Texture->data[cinfo.next_scanline * row_stride];
      jpeg_write_scanlines(&cinfo, row_pointer, 1);
   }
   jpeg_finish_compress(&cinfo);
   jpeg_destroy_compress(&cinfo);
   return true;
}
//---------------------------------------------------------------------------
