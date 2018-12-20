#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <cairo.h>
#include <pango/pangocairo.h>
#include <librsvg/rsvg.h>

void bitsetToBmp(uint8_t  *data, const char* filename, int width, int height);

#endif
