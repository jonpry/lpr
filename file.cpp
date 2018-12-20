#include "main.h"

static void* thread_start(void *arg){
    ((FileLoader*)arg)->thread_start();
    return 0;
}

FileLoader::FileLoader(const char* path){
   m_file = fopen(path,"r");   
   assert(m_file);
   fseek(m_file, 0L, SEEK_END);
   m_size = ftell(m_file);
   fseek(m_file, 0L, SEEK_SET);

   fread(&m_layers,4,1,m_file);
   fread(&m_xres,4,1,m_file);
   fread(&m_yres,4,1,m_file);

   m_need = 0;
   m_done = -1;
   m_waitFor=-1;
   m_result=0;
   pthread_cond_init(&m_condNeed,0);
   pthread_cond_init(&m_condDone,0);
   pthread_mutex_init(&m_mutexNeed,0);
   pthread_mutex_init(&m_mutexDone,0);
   m_quit = false;

   pthread_attr_t attr;
   pthread_attr_init(&attr);
   pthread_create(&m_thread, &attr, ::thread_start, this);
}

FileLoader::~FileLoader(){
   m_quit=true;
   m_need=-1;
   pthread_cond_signal(&m_condNeed);
   pthread_join(m_thread,0);
}

void FileLoader::thread_start(){
   int inProgress=0;
   while(!m_quit){
       //Wait for m_need to change
       pthread_mutex_lock(&m_mutexNeed);
       while(inProgress==m_need)
          pthread_cond_wait(&m_condNeed, &m_mutexNeed);
       inProgress=m_need;
       pthread_mutex_unlock(&m_mutexNeed);

       if(inProgress==-1 || m_quit)
          return;

       int csize=0,height=0;
       fread(&height,4,1,m_file);
       fread(&csize,4,1,m_file);

       uint8_t *cdata = (uint8_t*)malloc(csize);
       fread(cdata,1,csize,m_file);

       m_usize = ZSTD_getDecompressedSize(cdata, csize);

       printf("Decompressing %lu\n", m_usize);
       m_result = (volatile uint8_t*)malloc(m_usize);
       size_t ret = ZSTD_decompress((uint8_t*)m_result,m_usize,cdata,csize);

       free(cdata);
      
       //Notify other thread
       pthread_mutex_lock(&m_mutexDone);
       m_done=inProgress;
       pthread_cond_signal(&m_condDone);
       pthread_mutex_unlock(&m_mutexDone);
   }
}

void FileLoader::begin(int index){
   m_waitFor=index;
   assert(index < m_layers);
   pthread_mutex_lock(&m_mutexNeed);
   m_need = index;
   pthread_cond_signal(&m_condNeed);   
   pthread_mutex_unlock(&m_mutexNeed);
}

void FileLoader::get(uint8_t* ret){
   pthread_mutex_lock(&m_mutexDone);
   while(m_done != m_waitFor)
      pthread_cond_wait(&m_condDone,&m_mutexDone);
   pthread_mutex_unlock(&m_mutexDone);
   assert(m_result);
   memcpy(ret,(void*)m_result,m_usize);
   free((void*)m_result);
   m_result=0;
}


