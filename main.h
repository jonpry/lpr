#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/mman.h>
#include <stdbool.h>
#include <pthread.h>
#include <zstd.h>
#include <termios.h>
#include <errno.h>
#include <poll.h>
#include <assert.h>

#include <list>
#include <algorithm>
using namespace std;

#pragma pack(push,4)
typedef struct {
  uint32_t xres,yres,layers;
} header_t;

typedef struct {
  uint32_t bytes;
  float zpos;
} layer_header_t;
#pragma pack(pop)


class FileLoader{
 public:
   FileLoader(const char* path);
   ~FileLoader();

   void begin(int index);
   float get(uint8_t*);
   void thread_start();

   FILE* m_file;
   size_t m_size;
   pthread_t m_thread;
   pthread_cond_t m_condNeed, m_condDone;
   pthread_mutex_t m_mutexNeed, m_mutexDone;
   volatile int m_need, m_done;
   volatile bool m_quit;
   volatile float m_z;
   header_t m_header;
   int m_waitFor;
   volatile uint8_t *m_result;
   size_t m_usize;
};

#endif
