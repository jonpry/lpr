#ifndef _RSC_TABLE_PRU_H_
#define _RSC_TABLE_PRU_H_

#include <stddef.h>
#include <rsc_types.h>

struct my_resource_table {
	struct resource_table base;

	uint32_t offset[1]; /* Should match 'num' in actual definition */
        struct fw_rsc_custom pru_ints;
};

struct ch_map pru_intc_map[] = { {16, 0},
};

#define HOST_UNUSED             255


#pragma DATA_SECTION(pru_remoteproc_ResourceTable, ".resource_table")
#pragma RETAIN(pru_remoteproc_ResourceTable)
struct my_resource_table pru_remoteproc_ResourceTable = {
	1,	/* we're the first version that implements this */
	1,	/* number of entries in the table */
	0, 0,	/* reserved, must be zero */
        {
                offsetof(struct my_resource_table, pru_ints),
        },
        {
                TYPE_POSTLOAD_VENDOR, PRU_INTS_VER0 | TYPE_PRU_INTS,
                sizeof(struct fw_rsc_custom_ints),
                {
                        0x0000,
                        /* Channel-to-host mapping, 255 for unused */
                        0, HOST_UNUSED, HOST_UNUSED, HOST_UNUSED, HOST_UNUSED,
                        HOST_UNUSED, HOST_UNUSED, HOST_UNUSED, HOST_UNUSED, HOST_UNUSED,
                        /* Number of evts being mapped to channels */
                        (sizeof(pru_intc_map) / sizeof(struct ch_map)),
                        /* Pointer to the structure containing mapped events */
                        pru_intc_map,
                },
        },

};

#endif /* _RSC_TABLE_PRU_H_ */

