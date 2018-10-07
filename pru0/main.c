#include <stdint.h>
#include "resource_table_0.h"
#include <pru_intc.h>
#include <pru_cfg.h>
#include <rsc_types.h>

void foo();

volatile register uint32_t __R31;

#define HOST_INT                        ((uint32_t) 1 << 30)

/* The PRU-ICSS system events used for RPMsg are defined in the Linux device tree
 * PRU0 uses system event 16 (From PRU1), 17 (To PRU1, From PRU0)
 * PRU1 uses system event 18 (To ARM) and 19 (From ARM)
 */
#define FROM_PRU1			16
#define TO_PRU1				17

int main(void)
{
   //Wait for PRU1 to come online
   while(1){
      if (__R31 & HOST_INT) {
         CT_INTC.SICR_bit.STS_CLR_IDX = FROM_PRU1;
         break;
      } 
   }
   //Notify PRU1 we are online
   __R31 = (1<<5) | (TO_PRU1 - 16);
   foo();
}
