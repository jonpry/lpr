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

#define MAX_BUFFER_SIZE		512
char readBuf[MAX_BUFFER_SIZE];

#define NUM_MESSAGES		400
#define DEVICE_NAME		"/dev/rpmsg_pru31"

#define Z_OFST          -1.0f
#define LIFT_DISTANCE   3.5f


enum { CMD_ST=1, CMD_STAT, CMD_RUN };

#define PIX_BYTES (8192*2400*10/8)

#define WRITE(fd,str) mwrite(fd,str,strlen(str))

class Machine {
 public:
   Machine() {}
   virtual ~Machine() {}

   virtual void onST(char *str){
     printf("Message received from PRU:%s\n",  str);
     if(strstr(str,"Stopped"))
        onStop();
   }
   virtual void onStop() {}
   virtual void onGrbl(char *str){
      printf("Mach grbl: %s\n", str);
      if(strstr(str,"unlock")){
         printf("On unlock\n");
         onGrblReady();
      }else if(strstr(str,"ok")){
         printf("On ok\n");
         onOk();
      }
   }
   virtual void onGrblReady(){}
   virtual void onOk(){printf("Def ok\n");}
};


int rpmsgfd, grblfd;

void mwrite(int fd, const void* d, size_t l){
   assert(write(fd,d,l)==l);
   if(fd == grblfd)
      printf("GRBL wr: %s", d);
}

uint8_t* load_file() {
   FILE *f = fopen("out.raw.zst","r");

   fseek(f, 0L, SEEK_END);
   size_t csize = ftell(f);
   fseek(f, 0L, SEEK_SET);

   uint8_t *cdata = (uint8_t*)malloc(csize);
   fread(cdata,1,csize,f);
   fclose(f);


   size_t usize = ZSTD_getDecompressedSize(cdata, csize);

   printf("%lu\n", usize);
   uint8_t* udata = (uint8_t*)malloc(usize);
   size_t ret = ZSTD_decompress(udata,usize,cdata,csize);

   free(cdata);

   printf("%lu\n", ret);   
   return udata;
}

int fd_set_blocking(int fd, int blocking) {
    /* Save the current flags */
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        return 0;

    if (blocking)
        flags &= ~O_NONBLOCK;
    else
        flags |= O_NONBLOCK;
    return fcntl(fd, F_SETFL, flags) != -1;
}

Machine *gMachine=0;

void procFdMsg(uint8_t* readBuf, int len){
   printf("Got Msg %x\n", readBuf[0]);
   if(readBuf[0] == CMD_ST){
      if(gMachine)
         gMachine->onST((char*)readBuf+1);
      else
         printf("Received ST: %s\n", readBuf+1);
   }else if(readBuf[0] == CMD_STAT){
      uint32_t loops=0, cycles=0, dmas=0, fails=0, txs=0, failidx=0;
      memcpy(&loops, readBuf+1, 4);
      memcpy(&cycles, readBuf+5, 4);
      memcpy(&dmas, readBuf+9, 4);
      memcpy(&fails, readBuf+13, 4);
      memcpy(&txs, readBuf+17, 4);
      memcpy(&failidx, readBuf+21, 4);
      printf("LCs 0x%8.8X 0x%8.8X 0x%8.8X 0x%8.8X 0x%8.8X 0x%8.8X\n", loops, cycles, dmas, fails, txs, failidx);
   }
}

void procFd(uint8_t c){
   static bool hasLen=false;
   static int len=0, pos=0;
   static uint8_t msg[512];

   if(!hasLen){
      hasLen=true;
      len = c;
      pos = 0;
      memset(msg,0,512);
      return;
   }
   msg[pos++] = c;
   if(pos == len){
      hasLen=false;
      procFdMsg(msg,len);
   }   
}

void procGrbl(uint8_t c){
   static int pos=0;
   static uint8_t msg[512]={};
   msg[pos++] = c;
   //printf("Grbl: %x\n", c);
   if(c=='\n'){
      printf("RX'd GRBL: %s", msg);
      if(gMachine)
        gMachine->onGrbl((char*)msg);
      memset(msg,0,512);
      pos=0;
   }
}

#define EMR             (0x0300 / 4)

#define SER             (0x2038 / 4)
#define ESR             (0x2210 / 4)
#define ESRH            (0x2214 / 4)
#define EESR            (0x2230 / 4)
#define EECR            (0x2228 / 4)
#define EECRH           (0x222C / 4)
#define SECR            (0x2240 / 4)
#define SECRH           (0x2244 / 4)
#define IPR             (0x2268 / 4)
#define IPRH            (0x226C / 4)
#define ICR             (0x2270 / 4)
#define ICRH            (0x2274 / 4)
#define IESR            (0x2260 / 4)
#define IESRH           (0x2264 / 4)
#define IEVAL           (0x2278 / 4)
#define IECR            (0x2258 / 4)
#define IECRH           (0x225C / 4)


void dma_regs(volatile uint32_t *edma){
   printf("IPR: 0x%X\n", edma[IPR]);

   printf("ESR: 0x%X\n", edma[ESR]);
   printf("EMR: 0x%X\n", edma[EMR]);
   printf("SER: 0x%X\n", edma[SER]);
   printf("IESR: 0x%X\n", edma[IECR]);
   printf("SECR: 0x%X\n", edma[SECR]);
}

void dma(volatile uint32_t *edma, volatile uint32_t *pru){
   printf("PRU 0x%X\n", pru[0x10000/4]);

   dma_regs(edma);

   printf("Clearing ICR\n");
//  edma[ICR] = 3;

   dma_regs(edma);
}

void move(int distance, int pre, int flags){
   int omsgidx = 0;//TODO:msgidx;
   char s[64];
   sprintf(s,"\x01move%dp%df%de", distance, pre, flags);
   printf("%s\n", s);
   WRITE(rpmsgfd,s);
}

#define FAST_HOME 6400
#define SLOW_HOME 1000
#define SLOW_MOVE 6400
#define FAST_MOVE 80000
#define EXPOSE (6400*6)
#define EXPOSE_RET (6400*12)

class PingMachine : public Machine {
 public:
   PingMachine(){
      /* The RPMsg channel exists and the character device is opened */
      printf("Opened %s, sending %d messages\n\n", DEVICE_NAME, NUM_MESSAGES);

      WRITE(rpmsgfd, "\x01hello world!");
      mState=0;
   }

   void onST(char *str){
      if(mState < NUM_MESSAGES){
         WRITE(rpmsgfd, "\x01hello world!");
      }else if(mState == NUM_MESSAGES){
	 /* Received all the messages the example is complete */
	 printf("Received %d messages, closing %s\n", NUM_MESSAGES, DEVICE_NAME);
         gMachine=0;
         delete this;
      }
   }

   int mState;
};

class HomeMachine : public Machine {
 public:
   HomeMachine(){
      move(6400,FAST_HOME,1);
      mState=0;
   }

   void onStop(){
      switch(mState++){
         case 0: move(-1000000,FAST_HOME,0); break;
         case 1: move(3200,FAST_HOME,1); break;
         case 2: move(-1000000,SLOW_HOME,0); break;
         case 3: move(6400,FAST_HOME,1); break;
         case 4: WRITE(grblfd,"$H\n"); break;
      }
   }

   void onOk() override {
      switch(mState++){
         case 5: WRITE(grblfd, "G4 P0.1\n"); break;
         case 6: gMachine=0; delete this;
      }
   }

   int mState;
};

#define FAST_FEED 100.0f
#define SLOW_FEED 40.0f
#define STAB_SEC  1.0f
class RunMachine : public Machine {
 public:
   RunMachine(float z, bool movez){
      char buf[64];
      sprintf(buf, "G1 Y%f F%f\n", LIFT_DISTANCE+Z_OFST+z, FAST_FEED);
      if(movez)
         WRITE(grblfd, buf); 
      else
         issueRun();
      mState=0;
      mZ = z;
      mMoveZ = movez;
      mGrblState=0;
      mStopped=false;
      mGrblStopped=!movez;
   }

   void issueRun(){
      uint32_t max_loops=(int)(256*2000*9.5);
      uint8_t tbuf[5] = {3};
      memcpy(tbuf+1,&max_loops, 4);
      mwrite(rpmsgfd, tbuf, 5); //Issue the run command
      mDistance = (int)(6400*9*25.4/4);

      move(mDistance, EXPOSE, 3); 
      printf("Issue run\n");
   }

   void checkStop(){
      if(mStopped && mGrblStopped){
         printf("Complete\n");
         gMachine=0;
         delete this;
      }
   }

   void onStop(){
      char buf[64];
      switch(mState++){
         case 0: 
            move(-mDistance, EXPOSE_RET, 1); 
            sprintf(buf, "G1 Y%f F%f\n", LIFT_DISTANCE+Z_OFST+mZ, SLOW_FEED);
            if(mMoveZ);
            WRITE(grblfd, buf); 
            break;
         case 1: 
            mStopped = true;
            checkStop();
      }
   }

   void onOk() override{
      printf("OK %d\n", mState);
      char buf[64];
      switch(mGrblState++){
         case 0: 
            sprintf(buf, "G1 Y%f F%f\n", Z_OFST+mZ, SLOW_FEED);
            WRITE(grblfd, buf); 
            break;
         case 1: 
            sprintf(buf, "G4 P%f\n", STAB_SEC);
            WRITE(grblfd, buf); 
            break;
         case 2:  
            issueRun(); 
            break;
         case 3:
            WRITE(grblfd, "G4 P0.1\n");
            break;             
         case 4:
            mGrblStopped=true;
            checkStop();
      }
   }

   int mState,mGrblState,mDistance;
   bool mStopped, mGrblStopped;
   float mZ;
   bool mMoveZ;
};

bool quit=false;

void procIn(uint8_t c){
   switch(c){
      case 'd':
	 //dma(edma_map, pru_map);
	 break;
      case 'r': 
         gMachine = new RunMachine(10,false);
         break;
      case 'p':
	 gMachine = new PingMachine();
	 break;
      case 'l':
         WRITE(rpmsgfd, "\x01lon");
       	 break;
      case 'f':
	 WRITE(rpmsgfd, "\x01loff");
	 break;
      case 'm':
	 WRITE(rpmsgfd, "\x01mon");
	 break;
      case 's':
	 WRITE(rpmsgfd, "\x01moff");
	 break;
      case '2':
	 WRITE(rpmsgfd, "\x01toff");
	 break;
      case 't':
	 WRITE(rpmsgfd, "\x01ton");
	 break;
      case 'h':
         WRITE(rpmsgfd, "\x01tr");
         break;
      case 'i':
	 WRITE(rpmsgfd, "\x01tir");
	 break;
      case 'g':
         WRITE(rpmsgfd, "\x01gotime");
         break;
      case '+':
         move(6400*10*6, EXPOSE, 1);
	 break;
      case '-':
         move(-6400*10*6, SLOW_MOVE, 0);
         break;
      case 'a':
	 gMachine = new HomeMachine();
	 break;
      case 'o':
	 WRITE(rpmsgfd,"\x01stop");
	 break;
      case 'x':
	 quit=true;
	 break;
   }
}


void setup(int fd){
   struct termios tio;
   tcgetattr(fd, &tio);
   cfmakeraw(&tio);

   cfsetispeed(&tio, B115200);
   cfsetospeed(&tio, B115200);
#if 1
   tio.c_cflag |= CREAD|CLOCAL;
   tio.c_lflag &= (~(ICANON|ECHO|ECHOE|ECHOK|ECHONL|ISIG));
   tio.c_iflag &= (~(INPCK|IGNPAR|PARMRK|ISTRIP|ICRNL|IXANY));
   tio.c_oflag &= (~OPOST);
   tio.c_cc[VMIN] = 0;
#endif
   errno = 0;
   tcsetattr(fd, TCSANOW, &tio);
   if (errno != 0){
      printf("Error setting TCSANOW flag on at_modem %d %s\n", fd, strerror(errno));
   }
   errno = 0;
   tcflush(fd, TCIOFLUSH);
   if (errno != 0){
      printf("Error flushing: %d %s\n",fd, strerror(errno));
   }
   
}

int main(void) {
   uint32_t i;
   rpmsgfd = open(DEVICE_NAME, O_RDWR);
   int memfd = open("/dev/mem", O_RDWR | O_SYNC);
   grblfd = open("/dev/ttyUSB0", O_RDWR | O_NOCTTY | O_NDELAY);
   setup(grblfd);
   fd_set_blocking(rpmsgfd,false);
   fd_set_blocking(grblfd,false);
   fd_set_blocking(STDIN_FILENO,false);

   volatile uint32_t *ddr_map = (volatile uint32_t *)mmap(0, 0x02000000, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, 0x9e000000);
   volatile uint32_t *edma_map = (volatile uint32_t *)mmap(0, 0x8000, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, 0x49000000);
   volatile uint32_t *pru_map = (volatile uint32_t *)mmap(0, 0x20000, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, 0x4a300000);

   if (rpmsgfd < 0 || grblfd < 0 || memfd < 0 || !pru_map || !edma_map || !ddr_map) {
      printf("Failed to open %s\n", DEVICE_NAME);
      return -1;
   }

   struct pollfd pollfds[3] = {{rpmsgfd,POLLIN},{grblfd,POLLIN},{STDIN_FILENO,POLLIN}};


   uint8_t *pix = load_file();

   //Prime the reserved memory region
   int b = 0xF0;
   int w = 1;
#if 1
   for(i=0; i < PIX_BYTES/4; i++){
//    ddr_map[i] = ((uint32_t*)pix)[i];//((i&0xFF) >= b) && ((i&0xFF) < (b+w)) ? 0xAAAAAAAA: 0 ;//pix[i];//0xAAAAAAAA;
      int mod = i %256;
      if(mod < 250)
         ddr_map[i] = 0xFFFFFFFF;
      else
         ddr_map[i] = 0;
   }
#else
   int j;
   for(i=0; i < 2400*10; i++){
      for(j=0; j < 256; j++){
	 uint32_t v=0;
	 if(j == (i/10)%256){
	    v=0xFFFFFFFF;
	 }
         if(255-j == (i/10)%256){
            v=0xFFFFFFFF;
         }
         if(j==128){
	    v = 1<<(j/8);
	 }
	 ddr_map[i*256+j] = v;
      }
   }
#endif

   free(pix);
   pix=0;

   printf("(d)ma, (r)un, e(x)it, (p)ing (h)laser, (s)top motor, run (m)otor\n");

   while(!quit){
      //printf("Poll\n");
      if(poll(pollfds,3,1000)>0){
         //printf("Poll ret\n");
         for(int i=0; i < 3; i++){
            if(pollfds[i].revents==0)
               continue;
            //printf("Read on: %d\n", i);
            memset(readBuf,0,512);
            int result = read(pollfds[i].fd, readBuf, MAX_BUFFER_SIZE);
            //printf("Read: %d\n", result);
            for(int j=0; j < result; j++){
                switch(i){
                    case 0: procFd(readBuf[j]); break;
                    case 1: procGrbl(readBuf[j]); break;
                    case 2: procIn(readBuf[j]); break;
                }
            }
         }   
      }
   }


   /* Close the rpmsg_pru character device file */
   close(rpmsgfd);
   close(grblfd);
   return 0;
}

