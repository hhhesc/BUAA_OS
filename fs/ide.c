/*
 * operations on IDE disk.
 */

#include "serv.h"
#include <drivers/dev_disk.h>
#include <lib.h>
#include <mmu.h>

// Overview:
//  read data from IDE disk. First issue a read request through
//  disk register and then copy data from disk buffer
//  (512 bytes, a sector) to destination array.
//
// Parameters:
//  diskno: disk number.
//  secno: start sector number.
//  dst: destination for data read from IDE disk.
//  nsecs: the number of sectors to read.
//
// Post-Condition:
//  Panic if any error occurs. (you may want to use 'panic_on')
//
// Hint: Use syscalls to access device registers and buffers.
// Hint: Use the physical address and offsets defined in 'include/drivers/dev_disk.h':
//  'DEV_DISK_ADDRESS', 'DEV_DISK_ID', 'DEV_DISK_OFFSET', 'DEV_DISK_OPERATION_READ',
//  'DEV_DISK_START_OPERATION', 'DEV_DISK_STATUS', 'DEV_DISK_BUFFER'
void ide_read(u_int diskno, u_int secno, void *dst, u_int nsecs) {
	u_int begin = secno * BY2SECT;
	u_int end = begin + nsecs * BY2SECT;
	u_int read_flag = 0;
	u_int ret = 0;

	for (u_int off = 0; begin + off < end; off += BY2SECT) {
		uint32_t temp = diskno;
		u_int offset = begin+off;
		/* Exercise 5.3: Your code here. (1/2) */
		panic_on(syscall_write_dev((void*)&diskno,DEV_DISK_ADDRESS+0x10,4));
		panic_on(syscall_write_dev((void*)&offset,DEV_DISK_ADDRESS+0x0,4));
		panic_on(syscall_write_dev((void*)&read_flag,DEV_DISK_ADDRESS+0x20,1));
		panic_on(syscall_read_dev((void*)&ret,DEV_DISK_ADDRESS+0x30,1));
		if(ret==0){
			user_panic("ide_read fail.");
		}
		panic_on(syscall_read_dev((void*)(dst+off),DEV_DISK_ADDRESS+0x4000,BY2SECT));
	}
}

// Overview:
//  write data to IDE disk.
//
// Parameters:
//  diskno: disk number.
//  secno: start sector number.
//  src: the source data to write into IDE disk.
//  nsecs: the number of sectors to write.
//
// Post-Condition:
//  Panic if any error occurs.
//
// Hint: Use syscalls to access device registers and buffers.
// Hint: Use the physical address and offsets defined in 'include/drivers/dev_disk.h':
//  'DEV_DISK_ADDRESS', 'DEV_DISK_ID', 'DEV_DISK_OFFSET', 'DEV_DISK_BUFFER',
//  'DEV_DISK_OPERATION_WRITE', 'DEV_DISK_START_OPERATION', 'DEV_DISK_STATUS'
void ide_write(u_int diskno, u_int secno, void *src, u_int nsecs) {
	u_int begin = secno * BY2SECT;
	u_int end = begin + nsecs * BY2SECT;
	u_int write_flag = 1;
	u_int ret = 0;

	for (u_int off = 0; begin + off < end; off += BY2SECT) {
		uint32_t temp = diskno;
		u_int offset = begin+off;
		/* Exercise 5.3: Your code here. (2/2) */
		panic_on(syscall_write_dev((void*)(src+off),DEV_DISK_ADDRESS+0x4000,BY2SECT));
		panic_on(syscall_write_dev((void*)&diskno,DEV_DISK_ADDRESS+0x10,4));
		panic_on(syscall_write_dev((void*)&offset,DEV_DISK_ADDRESS+0x0,4));
		panic_on(syscall_write_dev((void*)&write_flag,DEV_DISK_ADDRESS+0x20,1));
		panic_on(syscall_read_dev((void*)&ret,DEV_DISK_ADDRESS+0x30,1));
	}
}
