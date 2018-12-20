#include "main.h"

#pragma pack(push,1)
struct BITMAPFILEHEADER {
    uint16_t bfType;
    uint32_t  bfSize;
    uint16_t   bfReserved1;
    uint16_t   bfReserved2;
    uint32_t   bfOffBits;
};

struct BITMAPINFOHEADER {
    uint32_t  biSize;
    int   biWidth;
    int   biHeight;
    short biPlanes;
    short biBitCount;
    uint32_t  biCompression;
    uint32_t  biSizeImage;
    int   biXPelsPerMeter;
    int   biYPelsPerMeter;
    uint32_t  biClrUsed;
    uint32_t  biClrImportant;
};

struct RGBQUAD {
    uint8_t rgbBlue;
    uint8_t rgbGreen;
    uint8_t rgbRed;
    uint8_t rgbReserved;
};
#pragma pack(pop)

#define BI_RGB 0

void bitsetToBmp(uint8_t  *data, const char* filename, int width, int height){
   //write the bitset to file as 1-bit deep bmp
   //bit order 0...n equals image pixels  top left...bottom right, row by row
   //the bitset must be at least the size of width*height, this is not checked

   int fd = open(filename, O_RDWR | O_TRUNC | O_CREAT);

   // save bitmap file headers
   struct BITMAPFILEHEADER fileHeader;
   struct BITMAPINFOHEADER infoHeader; 

   fileHeader.bfType      = 0x4d42;
   fileHeader.bfSize      = 0;
   fileHeader.bfReserved1 = 0;
   fileHeader.bfReserved2 = 0;
   fileHeader.bfOffBits   = sizeof(struct BITMAPFILEHEADER) + (sizeof(struct BITMAPINFOHEADER)) + 256*sizeof(struct RGBQUAD); 

   infoHeader.biSize          = (sizeof(struct BITMAPINFOHEADER) );
   infoHeader.biWidth         = width/8;    
   infoHeader.biHeight        = height/8;
   infoHeader.biPlanes        = 1;
   infoHeader.biBitCount      = 8;
   infoHeader.biCompression   = BI_RGB; //no compression needed
   infoHeader.biSizeImage     = 0;
   infoHeader.biXPelsPerMeter = 0;
   infoHeader.biYPelsPerMeter = 0;
   infoHeader.biClrUsed       = 256;
   infoHeader.biClrImportant  = 256;

   write(fd,&fileHeader, sizeof(fileHeader)); //write bitmapfileheader
   write(fd,&infoHeader, sizeof(struct BITMAPINFOHEADER)); //write bitmapinfoheader

   for(int i=0; i < 256; i++){
      struct RGBQUAD bl = {(uint8_t)i,(uint8_t)i,(uint8_t)i,0};  //black color
      write(fd,&bl,sizeof(bl)); //write RGBQUAD for black
   }
// convert the bits into bytes and write the file
   write(fd,data,width*height/64);
   close(fd);
}

