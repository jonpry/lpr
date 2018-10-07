#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <pru_cfg.h>
#include <pru_intc.h>
#include <rsc_types.h>
#include <pru_rpmsg.h>
#include <pru_uart.h>
#include "edma.h"

volatile register uint32_t __R31;

void config_dma(uint32_t chan, uint32_t src, uint32_t dst, uint32_t len){
   hostBuffer hostData;
   uint32_t channelMask;
   uint16_t paramOffset;
   edmaParam params;
   volatile uint32_t *ptr;
   volatile uint32_t *ptr_cm;
   volatile edmaParam *pParams;

   ptr = EDMA0_CC_BASE;
   ptr_cm = CM_PER_BASE;
   ptr_cm[TPTC2_CLKCTRL] = ON;
   ptr_cm[TPCC_CLKCTRL] = ON;

   /* Load channel parameters from DRAM - loaded by host */
   hostData.src = src;	//PRU Shared memory
   hostData.dst = dst;	//PRU Shared memory
   hostData.chan = /*buf.chan*/ chan;

   channelMask = (1 << hostData.chan);
   ptr[DCHMAP_0+chan] = (hostData.chan << 5);
   ptr[DRAE1] |= channelMask;
   ptr[SECR] |= channelMask;
   ptr[ICR] |= channelMask;
   ptr[IESR] |= channelMask;
   ptr[EESR] |= channelMask;
   ptr[EMCR] |= channelMask;
   ptr[DMAQNUM0] = (ptr[DMAQNUM0] & 0xFFFFFF00) | 0x22;

   /* Setup and store PaRAM set for transfer */
   paramOffset = PARAM_OFFSET;
   /* channel * 0x20, word address */
   paramOffset += ((hostData.chan << 5) / 4);

   params.lnkrld.link = 0xFFFF;
   params.lnkrld.bcntrld = 0x0000;
   params.opt.tcc = hostData.chan;
   params.opt.tcinten = 1;
   params.opt.itcchen = 1;
   params.opt.static_set = 1;

   params.ccnt.ccnt = CCNT;
   params.abcnt.acnt = len;
   params.abcnt.bcnt = BCNT;
   params.bidx.srcbidx = 0x1;
   params.bidx.dstbidx = 0x1;
   params.src = hostData.src;
   params.dst = hostData.dst;

   pParams = (volatile edmaParam *)(ptr + paramOffset);
   *pParams = params;
}

void config_dma2(uint32_t chan, uint32_t src, uint32_t dst, uint32_t len){
   hostBuffer hostData;
   uint32_t channelMask;
   uint16_t paramOffset;
   edmaParam params;
   volatile uint32_t *ptr;
   volatile edmaParam *pParams;

   ptr = EDMA0_CC_BASE;

   /* Load channel parameters from DRAM - loaded by host */
   hostData.src = src;  //PRU Shared memory
   hostData.dst = dst;  //PRU Shared memory
   hostData.chan = /*buf.chan*/ chan;

   channelMask = (1 << hostData.chan);
/*
   ptr[DRAE1] |= channelMask;
   ptr[SECR] |= channelMask;
   ptr[ICR] |= channelMask;
   ptr[IESR] |= channelMask;
   ptr[EESR] |= channelMask;
   ptr[EMCR] |= channelMask;
*/
   /* Setup and store PaRAM set for transfer */
   paramOffset = PARAM_OFFSET;
   /* channel * 0x20, word address */
   paramOffset += ((hostData.chan << 5) / 4);

   params.lnkrld.link = 0xFFFF;
   params.lnkrld.bcntrld = 0x0000;
   params.opt.tcc = hostData.chan;
   params.opt.tcinten = 1;
   params.opt.itcchen = 1;
   params.opt.static_set = 1;

   params.ccnt.ccnt = CCNT;
   params.abcnt.acnt = len;
   params.abcnt.bcnt = BCNT;
   params.bidx.srcbidx = 0x1;
   params.bidx.dstbidx = 0x1;
   params.src = hostData.src;
   params.dst = hostData.dst;

   pParams = (volatile edmaParam *)(ptr + paramOffset);
   *pParams = params;
}


void change_dma(uint32_t chan, uint32_t src){
   uint16_t paramOffset;
   volatile uint32_t *ptr;
   volatile edmaParam *pParams;

   ptr = EDMA0_CC_BASE;

   uint32_t channelMask = (1 <<chan);

//   ptr[DRAE1] |= channelMask;
//   ptr[SECR] |= channelMask;
//   ptr[ICR] |= channelMask;
//   ptr[IESR] |= channelMask;
//   ptr[EESR] |= channelMask;
//   ptr[EMCR] |= channelMask;



   // Setup and store PaRAM set for transfer 
   paramOffset = PARAM_OFFSET;
   // channel * 0x20, word address 
   paramOffset += (chan << 5) / 4;

   pParams = (volatile edmaParam *)(ptr + paramOffset);
   pParams->src = src;

}

