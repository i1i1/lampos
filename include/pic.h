#ifndef _PIC_H_
#define _PIC_H_

// Map IRQ indexes for master and slave PIC
#define _PIC_INT_OFF1 0x20
#define _PIC_INT_OFF2 0x28

#define IRQ_TIMER _PIC_INT_OFF1
#define IRQ_KBD (_PIC_INT_OFF1 + 1)

enum pic_port {
	PIC_MASTER_CMD	= 0x20,
	PIC_MASTER_DT	= 0x21,
	PIC_SLAVE_CMD	= 0xA0,
	PIC_SLAVE_DT	= 0xA1,
};

void pic_init(void);

void pic_eoi(enum pic_port p);

#endif /* _PIC_H_ */

