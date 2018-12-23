#!/bin/sh
g++ main.cpp bmp.cpp -I /usr/include/pango-1.0/ -I /usr/include/glib-2.0/ -I /usr/lib/x86_64-linux-gnu//glib-2.0/include/ -I /usr/include/cairo -I /usr/include/librsvg-2.0/ -I /usr/include/gdk-pixbuf-2.0/ -lcairo -lglib-2.0 -lpango-1.0 -lgobject-2.0 -lpangocairo-1.0 -o render -lrsvg-2 -g -lzstd -lgio-2.0

