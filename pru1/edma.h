#ifndef EDMA_H
#define EDMA_H

/* 1D Transfer Parameters */
typedef struct {
        uint32_t src;
        uint32_t dst;
        uint32_t chan;
} hostBuffer;

/* EDMA PARAM registers */
typedef struct {
        uint32_t sam            : 1;
        uint32_t dam            : 1;
        uint32_t syncdim        : 1;
        uint32_t static_set     : 1;
        uint32_t                : 4;
        uint32_t fwid           : 3;
        uint32_t tccmode        : 1;
        uint32_t tcc            : 6;
        uint32_t                : 2;
        uint32_t tcinten        : 1;
        uint32_t itcinten       : 1;
        uint32_t tcchen         : 1;
        uint32_t itcchen        : 1;
        uint32_t privid         : 4;
        uint32_t                : 3;
        uint32_t priv           : 1;
} edmaParamOpt;

/*typedef struct{
        uint32_t src;
} edmaParamSrc;*/

typedef struct {
        uint32_t acnt           : 16;
        uint32_t bcnt           : 16;
} edmaParamABcnt;

/*typedef struct{
        uint32_t dst;
} edmaParamDst;*/

typedef struct {
        uint32_t srcbidx        : 16;
        uint32_t dstbidx        : 16;
} edmaParamBidx;

typedef struct {
        uint32_t link           : 16;
        uint32_t bcntrld        : 16;
} edmaParamLnkRld;

typedef struct {
        uint32_t srccidx        : 16;
        uint32_t dstcidx        : 16;
} edmaParamCidx;

typedef struct {
        uint32_t ccnt           : 16;
        uint32_t                : 16;
} edmaParamCcnt;

typedef struct {
        edmaParamOpt    opt;
        /*edmaParamSrc*/ uint32_t       src;
        edmaParamABcnt  abcnt;
        /*edmaParamDst*/ uint32_t       dst;
        edmaParamBidx   bidx;
        edmaParamLnkRld lnkrld;
        edmaParamCidx   cidx;
        edmaParamCcnt   ccnt;
} edmaParam;

#define CTBIR_0         (*(volatile uint32_t *)(0x22020))
#define CTBIR_1         (*(volatile uint32_t *)(0x22024))

/* EDMA Channel Registers */
#define CM_PER_BASE     ((volatile uint32_t *)(0x44E00000))
#define TPTC0_CLKCTRL (0x24 / 4)
#define TPTC2_CLKCTRL (0x100 / 4)
#define TPCC_CLKCTRL  (0xBC / 4)
#define ON (0x2)

/* EDMA Channel Registers */
#define EDMA0_CC_BASE   ((volatile uint32_t *)(0x49000000))
#define DMAQNUM0        (0x0240 / 4)
#define DMAQNUM1        (0x0244 / 4)
#define DCHMAP_0    	(0x0100 / 4)
#define DCHMAP_10       (0x0128 / 4)
#define QUEPRI          (0x0284 / 4)
#define EMR             (0x0300 / 4)
#define EMCR            (0x0308 / 4)
#define EMCRH           (0x030C / 4)
#define QEMCR           (0x0314 / 4)
#define CCERRCLR        (0x031C / 4)
#define DRAE0           (0x0340 / 4)
#define DRAE1           (0x0348 / 4)
#define DRAE2           (0x0350 / 4)
#define DRAE3           (0x0358 / 4)
#define QWMTHRA         (0x0620 / 4)
#define GLOBAL_ESR      (0x1010 / 4)
#define GLOBAL_ESRH     (0x1014 / 4)
#define GLOBAL_EECR     (0x1028 / 4)
#define GLOBAL_EECRH    (0x102C / 4)
#define GLOBAL_SECR     (0x1040 / 4)
#define GLOBAL_SECRH    (0x1044 / 4)
#define GLOBAL_IESR     (0x1060 / 4)
#define GLOBAL_IESRH    (0x1064 / 4)
#define GLOBAL_ICR      (0x1070 / 4)
#define GLOBAL_ICRH     (0x1074 / 4)

/* EDMA Shadow Region 1 */
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

/* EDMA PARAM registers */
#define PARAM_OFFSET    (0x4000 / 4)
#define OPT             0x00
#define SRC             0x04
#define ACNT            0x100
#define BCNT            0x1
#define DST             0x0C
#define SRC_DST_BIDX    0x10
#define LINK_BCNTRLD    0x14
#define SRC_DST_CIDX    0x18
#define CCNT            0x1

#endif //EDMA_H
