#ifndef _ASM_IA64_SPINLOCK_H
#define _ASM_IA64_SPINLOCK_H

/*
 * Copyright (C) 1998-2003 Hewlett-Packard Co
 *	David Mosberger-Tang <davidm@hpl.hp.com>
 * Copyright (C) 1999 Walt Drummond <drummond@valinux.com>
 *
 * This file is used for SMP configurations only.
 */

#include <linux/compiler.h>
#include <linux/kernel.h>

#include <asm/atomic.h>
#include <asm/bitops.h>
#include <asm/intrinsics.h>
#include <asm/system.h>

#define DEBUG_SPINLOCK

typedef struct {
	volatile unsigned int lock;
} raw_spinlock_t;

#define _RAW_SPIN_LOCK_UNLOCKED	/*(raw_spinlock_t)*/ { 0 }

#define _raw_spin_is_locked(x)	((x)->lock != 0)
#define _raw_spin_unlock(x)	do { barrier(); (x)->lock = 0; } while (0)
#define _raw_spin_trylock(x)	(cmpxchg_acq(&(x)->lock, 0, 1) == 0)

typedef struct {
	volatile unsigned int read_counter	: 31;
	volatile unsigned int write_lock	:  1;
} raw_rwlock_t;
#define _RAW_RW_LOCK_UNLOCKED /*(raw_rwlock_t)*/ { 0, 0 }

#define _raw_read_unlock(rw)					\
do {								\
	raw_rwlock_t *__read_lock_ptr = (rw);			\
	ia64_fetchadd(-1, (int *) __read_lock_ptr, rel);	\
} while (0)

#ifdef ASM_SUPPORTED

#define _raw_write_trylock(rw)							\
({										\
	register long result;							\
										\
	__asm__ __volatile__ (							\
		"mov ar.ccv = r0\n"						\
		"dep r29 = -1, r0, 31, 1;;\n"					\
		"cmpxchg4.acq %0 = [%1], r29, ar.ccv\n"				\
		: "=r"(result) : "r"(rw) : "ar.ccv", "r29", "memory");		\
	(result == 0);								\
})

#else /* !ASM_SUPPORTED */


#define _raw_write_trylock(rw)						\
({									\
	__u64 ia64_val;							\
	__u64 ia64_set_val = ia64_dep_mi(-1, 0, 31,1);			\
	ia64_val = ia64_cmpxchg4_acq((__u32 *)(rw), ia64_set_val, 0);	\
	(ia64_val == 0);						\
})

#endif /* !ASM_SUPPORTED */

#define _raw_read_trylock(lock) generic_raw_read_trylock(lock)

#define _raw_write_unlock(x)								\
({											\
	smp_mb__before_clear_bit();	/* need barrier before releasing lock... */	\
	clear_bit(31, (x));								\
})

#define _raw_rw_is_locked(x) (*(int *)(x) != 0)
#define _raw_rw_is_write_locked(x) (test_bit(31, (x)))

#endif /*  _ASM_IA64_SPINLOCK_H */
