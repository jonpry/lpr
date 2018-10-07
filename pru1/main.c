#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <pru_cfg.h>
#include <pru_intc.h>
#include <rsc_types.h>
#include <pru_rpmsg.h>
#include <pru_uart.h>
#include "resource_table_1.h"
#include "edma.h"

volatile register uint32_t __R31;

/* Host-1 Interrupt sets bit 31 in register R31 */
#define HOST_INT			((uint32_t) 1 << 31)

/* The PRU-ICSS system events used for RPMsg are defined in the Linux device tree
 * PRU0 uses system event 16 (From PRU1), 17 (To PRU1, From PRU0)
 * PRU1 uses system event 18 (To ARM) and 19 (From ARM)
 */
#define TO_PRU0 			16
#define FROM_PRU0			17
#define TO_ARM_HOST			18
#define FROM_ARM_HOST			19

/*
 * Using the name 'rpmsg-pru' will probe the rpmsg_pru driver found
 * at linux-x.y.z/drivers/rpmsg/rpmsg_pru.c
 */
#define CHAN_NAME			"rpmsg-pru"
#define CHAN_DESC			"Channel 31"
#define CHAN_PORT			31

/*
 * Used to make sure the Linux drivers are ready for RPMsg communication
 * Found at linux-x.y.z/include/uapi/linux/virtio_config.h
 */
#define VIRTIO_CONFIG_S_DRIVER_OK	4


uint8_t payload[RPMSG_MESSAGE_SIZE];
uint8_t rxbuffer[68];
uint8_t txbuffer[64];

void config_dma(uint32_t chan, uint32_t src, uint32_t dst, uint32_t len);
void config_dma2(uint32_t chan, uint32_t src, uint32_t dst, uint32_t len);
void change_dma(uint32_t chan, uint32_t src);

enum {CMD_ST=1, CMD_STAT, CMD_RUN};

void main(void) {
   struct pru_rpmsg_transport transport;
   uint16_t src, dst=0, len;
   volatile uint8_t *status;
   bool could_rx=false;

   /* Allow OCP master port access by the PRU so the PRU can read external memories */
   CT_CFG.SYSCFG_bit.STANDBY_INIT = 0;

   /* Clear the status of the PRU-ICSS system event that the ARM will use to 'kick' us */
   CT_INTC.SICR_bit.STS_CLR_IDX = FROM_ARM_HOST;

   __R31 = (1<<5) | (TO_PRU0 - 16); //16;//TO_PRU0; //Notify PRU0 we are awake
   while(__R31&(1<<30)){} //Wait for pru0 to clear the interrupt   

   /* Make sure the Linux drivers are ready for RPMsg communication */
   status = &resourceTable.rpmsg_vdev.status;
   while (!(*status & VIRTIO_CONFIG_S_DRIVER_OK));

   /* Initialize the RPMsg transport structure */
   pru_rpmsg_init(&transport, &resourceTable.rpmsg_vring0, &resourceTable.rpmsg_vring1, TO_ARM_HOST, FROM_ARM_HOST);

   /* Create the RPMsg channel between the PRU and ARM user space using the transport structure. */
   while (pru_rpmsg_channel(RPMSG_NS_CREATE, &transport, CHAN_NAME, CHAN_DESC, CHAN_PORT) != PRU_RPMSG_SUCCESS);


   /*** INITIALIZATION ***/
   config_dma(0, 0x9e000000, 0x4A310000, 0x200);
   config_dma(1, 0x9e000200, 0x4A310200, 0x200);

   /* Setup edma pointer */
   volatile uint32_t *ptr;
   ptr = EDMA0_CC_BASE;

   /* Set up UART to function at 115200 baud - DLL divisor is 104 at 16x oversample
   * 192MHz / 104 / 16 = ~115200 */
   CT_UART.DLL = 104;
   CT_UART.DLH = 0;
   CT_UART.MDR = 0x0;
   CT_UART.IER = 0x7;
   CT_UART.FCR = (0x8) | (0x4) | (0x2) | (0x1);
   CT_UART.LCR = 3;
   CT_UART.MCR = 0x0; //0x10
   CT_UART.PWREMU_MGMT = 0x6001;

   int txhead=0,txtail=0;
   uint16_t tdst;
   int i;
   bool has_dst=false;
   bool has_len=false;
   bool running=false;
   int rxlen,rxpos;
   uint32_t fails=0;
   uint32_t txs=0;
   uint32_t failidx=0xAAAAAAA;
   while (1) {
      if(__R31 & HOST_INT){
         int int_val = CT_INTC.HIPIR1;
         if(int_val == FROM_PRU0){
 	    CT_INTC.SICR_bit.STS_CLR_IDX = FROM_PRU0;
	    if(!running)
		continue;

            uint32_t loops = *(volatile uint32_t*)0x00012000;
            if(loops==6144000){
               running = false;
            }else{
               uint32_t channel=1;
               if(loops & 0x80){
                  channel=0;
               }
               txs++;

               uint32_t completes = ptr[IPR];
               //Both transfers should always be complete or something is wrong
               if(!(completes & 3)){
                  fails++;
                  failidx=(txs-1) | (completes << 8) | (channel << 16);
               }
               //Make sure transfer complete on this channel or we can't reissue
               if(completes & (1<<channel)) {
                  //ptr[SECR] |= 1<<channel;
                  do{
                     ptr[ICR] = 1<<channel;
                  } while(ptr[IPR] & (1<<channel));
                  uint32_t nsrc = 0x9e000000 + ((loops<<2) & (0x02000000-1));
                  change_dma(channel,nsrc);
                  ptr[ESR] = (1<<channel);
               }

               //Send unsolicited status update
               if((txs & 0xFF) == 0){
                  if(has_dst){
                     uint8_t buf[25] = {CMD_STAT};
                     memcpy(buf+1,(uint32_t*)0x00012000,8);
                     memcpy(buf+9,&completes, 4);
                     memcpy(buf+13,&fails, 4);
                     memcpy(buf+17,&txs, 4);
                     memcpy(buf+21,&failidx, 4);
                     pru_rpmsg_send(&transport, tdst, src, buf, 25);
                  }
               }
            }
         }else if(int_val == FROM_ARM_HOST){
            CT_INTC.SICR_bit.STS_CLR_IDX = FROM_ARM_HOST;
            could_rx = true;
         }
      } else if ((CT_UART.LSR & 1) == 1) {
         uint8_t t = CT_UART.RBR;
         if(!has_len){
           rxlen = t;
           if(rxlen <= 64 && rxlen > 0){
             rxpos = 1;
             rxbuffer[0] = 1;
             has_len = true;
           }
         }else{
           rxbuffer[rxpos++] = t;
           if(rxpos == rxlen+1){
             has_len = false;
             if(has_dst){
               pru_rpmsg_send(&transport, tdst, src, rxbuffer, rxpos);
/*
                  uint8_t buf[17] = {CMD_STAT};
                  memcpy(buf+1,(uint32_t*)0x00012000,8);
                  memcpy(buf+9,ptr + IPR, 4);
                  memcpy(buf+13,&fails,4);
                  pru_rpmsg_send(&transport, tdst, src, buf, 17);
*/
             }
           }
         }
      } else if(txhead != txtail && ((CT_UART.LSR & 0x20) == 0x20)) {
        uint8_t byte=txbuffer[txhead++];
        if(txhead >= 64)
           txhead = 0;
        CT_UART.THR = byte;
      } else if (could_rx) {
         //TODO: get interrupt number
         /* Check bit 31 of register R31 to see if the ARM has kicked us */
         /* Clear the event status */
	 if(pru_rpmsg_receive(&transport, &src, &dst, payload, &len) == PRU_RPMSG_SUCCESS){
            could_rx=true;
            has_dst = true;
            tdst = dst;
            if(payload[0] == CMD_ST){ //This is a uart message
    	       /* Echo the message back to the same address from which we just received */
               txbuffer[txtail++] = len-1;
               if(txtail >= 64){
                  txtail = 0;
               }
               if(txtail == txhead) //uhoh
                  txhead++;
               if(txhead >= 64)
                  txhead=0;

               for(i=1; i < len; i++){
                  txbuffer[txtail++] = payload[i];
                  if(txtail >= 64){
                     txtail = 0;
                  }
                  if(txtail == txhead) //uhoh
                     txhead++;
                  if(txhead >= 64)
                     txhead=0;
               }
            }else if(payload[0] == CMD_RUN && !running){
               running=true;
//               ptr[SECR] = 3;
               ptr[ICR] = 3;
/*
               uint32_t nsrc = 0x9e000000 + ((txs++<<2) & (0x01000000-1));
	       change_dma(0,nsrc);
               nsrc = 0x9e000000 + ((txs++<<2) & (0x01000000-1));
               change_dma(1,nsrc);
*/
               //Trigger both transfers
               ptr[ESR] = (1<<1);
               while(!(ptr[IPR] & 2)){}
               ptr[ESR] = (1<<0);
               while(!(ptr[IPR] & 1)){}


               //Both DMA's complete
//               ptr[SECR] = 3;
//	       do{
//                 ptr[ICR] = 3;
//               } while(ptr[IPR]);
               __R31 = (1<<5) | (TO_PRU0 - 16); //16;//TO_PRU0; //Notify PRU0 we are ready 
               while(__R31&(1<<30)){} //Wait for pru0 to clear the interrupt   

            }
         }else{
           could_rx=false;
         }
      } 
   }
}
