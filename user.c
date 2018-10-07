#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/mman.h>
#include <stdbool.h>

#include <zstd.h>

#define MAX_BUFFER_SIZE		512
char readBuf[MAX_BUFFER_SIZE];

#define NUM_MESSAGES		400
#define DEVICE_NAME		"/dev/rpmsg_pru31"

enum { CMD_ST=1, CMD_STAT, CMD_RUN };

#define PIX_BYTES (8192*2400*10/8)

uint8_t* load_file() {
   FILE *f = fopen("out.raw.zst","r");

   fseek(f, 0L, SEEK_END);
   size_t csize = ftell(f);
   fseek(f, 0L, SEEK_SET);

   uint8_t *cdata = malloc(csize);
   fread(cdata,1,csize,f);
   fclose(f);


   size_t usize = ZSTD_getDecompressedSize(cdata, csize);

   printf("%lu\n", usize);
   uint8_t* udata = malloc(usize);
   size_t ret = ZSTD_decompress(udata,usize,cdata,csize);

   free(cdata);

   printf("%lu\n", ret);   
   return udata;
}

void ping(int fd){
	int result,i;
	/* The RPMsg channel exists and the character device is opened */
	printf("Opened %s, sending %d messages\n\n", DEVICE_NAME, NUM_MESSAGES);

	for (i = 0; i < NUM_MESSAGES; i++) {
		/* Send 'hello world!' to the PRU through the RPMsg channel */
		result = write(fd, "\x01hello world!", 13);
		if (result > 0)
			printf("Message %d: Sent to PRU\n", i);

		/* Poll until we receive a message from the PRU and then print it */
                while(1){
                	memset(readBuf,0,512);
		        result = read(fd, readBuf, MAX_BUFFER_SIZE);
                        printf("Poll ret\n");
		        if (result > 0){
		        	if(readBuf[0] == CMD_ST){
		        		printf("Message %d received from PRU:%s\n\n", i, readBuf+1);
		        		break;
			        }
			        if(readBuf[0] == CMD_STAT){
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
                }
	}

	/* Received all the messages the example is complete */
	printf("Received %d messages, closing %s\n", NUM_MESSAGES, DEVICE_NAME);

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
//        edma[ICR] = 3;

	dma_regs(edma);
}

int main(void) {
	uint32_t i;
	int fd = open(DEVICE_NAME, O_RDWR);
	int memfd = open("/dev/mem", O_RDWR | O_SYNC);

        volatile uint32_t *ddr_map = mmap(0, 0x02000000, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, 0x9e000000);
        volatile uint32_t *edma_map = mmap(0, 0x8000, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, 0x49000000);
        volatile uint32_t *pru_map = mmap(0, 0x20000, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, 0x4a300000);

	if (fd < 0 || memfd < 0 || !pru_map || !edma_map || !ddr_map) {
		printf("Failed to open %s\n", DEVICE_NAME);
		return -1;
	}

        uint8_t *pix = load_file();

        //Prime the reserved memory region
        int b = 0;
        for(i=0; i < PIX_BYTES/4; i++){
                ddr_map[i] =  ((i&0xFF) > b) && ((i&0xFF) < (b+20)) ? 0xAAAAAAAA: 0 ;//pix[i];//0xAAAAAAAA;
        }
        free(pix);
        pix=0;

        printf("(d)ma, (r)un, e(x)it, (p)ing\n");

	while(1){
		char c=0;
		bool quit=false;
		scanf("%c", &c);
		switch(c){
			case 'd':
				dma(edma_map, pru_map);
				break;
			case 'r':
				write(fd, "\x03", 1); //Issue the run command
				break;
			case 'p':
				ping(fd);
				break;
                        case 'l':
                                write(fd, "\x01lon", 4);
       				break;
			case 'f':
				write(fd, "\x01loff", 5);
				break;
			case 'm':
				write(fd, "\x01mon", 4);
				break;
			case 's':
				write(fd, "\x01moff", 5);
				break;
			case '2':
				write(fd, "\x01toff", 5);
				break;
			case 't':
				write(fd, "\x01ton", 4);
				break;
			case 'x':
				quit=true;
				break;
		}
		if(quit)
			break;
	}



	/* Close the rpmsg_pru character device file */
	close(fd);

	return 0;
}

