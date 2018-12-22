#include "main.h"

#include <string>
#include <fstream>
#include <streambuf>
#include <iostream>
#include <list>
#include <rapidxml/rapidxml.hpp>

#include <zstd.h>

using namespace rapidxml;
using namespace std;

#define X_DPI 1500
#define Y_DPI 2000

#define XRES 8192
#define YRES (2000*10)

#define FILENAME "temp.svg"

#define BASE_LAYERS 4
#define BASE_MULT 5

#pragma pack(push,4)
typedef struct {
  uint32_t xres,yres,layers;
} header_t;

typedef struct {
  uint32_t bytes;
  float zpos;
} layer_header_t;
#pragma pack(pop)

string read_file(){
   ifstream t(FILENAME);
   string str((std::istreambuf_iterator<char>(t)),
                 std::istreambuf_iterator<char>());
   return str;
}

void write_header(FILE *f, int layers){
   header_t header = {XRES, YRES, (uint32_t)layers};
   fwrite(&header,1,sizeof(header),f);
}

int main() {
   int bytes=XRES*YRES/8;
   printf("Pixels: 0x%X\n", bytes);
   uint8_t *cdata = (uint8_t*)malloc(bytes);

   FILE *f = fopen("out.lpr","w+");

   string xml = read_file();
   xml_document<> doc;
   doc.parse<0>((char*)xml.c_str());
   xml_node<> *svg = doc.first_node("svg");
   list<string> layers;
   list<float> zs;
   for(xml_node<> *node = svg->first_node("g"); node; node = node->next_sibling("g")){
       zs.push_back(atof(node->first_attribute("slic3r:z")->value())); //slic3r output seems off by factor of 1000
       layers.push_back(node->first_attribute("id")->value());
   }

   write_header(f, layers.size() + BASE_LAYERS * (BASE_MULT-1));


   GError* e = NULL;
   RsvgHandle* handle = rsvg_handle_new_from_file(FILENAME, &e);
   if(e != NULL){
      printf("Could not open svg\n");
      return 1;
   }

   int i=0;
   auto it2=zs.begin();
   for(auto it=layers.begin(); it!=layers.end(); it++,it2++,i++){
      string layer = "#" + *it;
      cout << layer << ", @" << *it2 << endl;

      cairo_surface_t *s = cairo_image_surface_create (CAIRO_FORMAT_A1, XRES, YRES);
      cairo_t *cr = cairo_create (s);

      cairo_surface_destroy (s);

      cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
      cairo_set_source_rgba (cr, 1, 1, 1, 0);
      cairo_paint (cr);

      cairo_set_source_rgba (cr, 1, 1, 1, 1);
      cairo_scale (cr, 
             (double)XRES/6/25.4, 
             (double)YRES/9/25.4);

      cairo_translate(cr,70,100);
             
      rsvg_handle_render_cairo_sub(handle, cr, layer.c_str());

      //cairo_surface_write_to_png (cairo_get_target(cr), "out.png");
      //exit(0);
      uint32_t *data = (uint32_t*)cairo_image_surface_get_data(cairo_get_target(cr));
      
      size_t cbytes = ZSTD_compress( cdata, bytes,
                data, bytes, 5);
      cout << cbytes << endl;

      layer_header_t hdr = {(uint32_t)cbytes,*it2};
      for(int j=0; j < ((i<BASE_LAYERS)?BASE_MULT:1); j++){
         fwrite(&hdr,1,sizeof(hdr),f);
         fwrite(cdata,1,cbytes,f);
      }

      cairo_destroy (cr);
   }


#if 0


   uint32_t *data = (uint32_t*)cairo_image_surface_get_data(cairo_get_target(cr));
   printf("Stride: %d\n", cairo_image_surface_get_stride(cairo_get_target(cr)));


#if 0
   for(int i=0; i < bytes/4; i++){
     data[i] = __builtin_bswap32(data[i]);
   }
#endif



   int len = 8192*2400*10/64;
   printf("Len: %d\n", len);
   uint8_t *gdata=(uint8_t*)malloc(len);
   memset(gdata,0,len);
#if 1
   for(int i=0; i < 2400*10; i++){
      for(int j=0; j < 8192; j++){
         int px = i*8192+j;
         bool v = (data[px/32]>>(px%32))&1;
         if(v && gdata[(i/8)*8192/8 + j/8] < 0xF8){
            gdata[(i/8)*8192/8 + j/8]+=4;
         }
      }
   } 
#endif

bitsetToBmp(gdata,"out.bmp",8192,2400*10);

   FILE *f = fopen("out.raw","w+");
   fwrite(data,1,bytes,f);
   fclose(f);
#endif
   free(cdata);
   fclose(f);
   return 0;
}
