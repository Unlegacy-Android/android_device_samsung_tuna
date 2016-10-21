/*
 $License:
   Copyright 2011 InvenSense, Inc.

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
  $
 */

#ifndef _MLOS_H
#define _MLOS_H

#ifndef __KERNEL__
#include <stdio.h>
#endif

#include "mltypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(LINUX) || defined(__KERNEL__)
#include <stdint.h>
typedef uintptr_t HANDLE; // TODO: shouldn't this be pthread_mutex_t* ???
#endif

	/* ------------ */
	/* - Defines. - */
	/* ------------ */

	/* - MLOSCreateFile defines. - */

#define MLOS_GENERIC_READ         ((unsigned int)0x80000000)
#define MLOS_GENERIC_WRITE        ((unsigned int)0x40000000)
#define MLOS_FILE_SHARE_READ      ((unsigned int)0x00000001)
#define MLOS_FILE_SHARE_WRITE     ((unsigned int)0x00000002)
#define MLOS_OPEN_EXISTING        ((unsigned int)0x00000003)

	/* ---------- */
	/* - Enums. - */
	/* ---------- */

	/* --------------- */
	/* - Structures. - */
	/* --------------- */

	/* --------------------- */
	/* - Function p-types. - */
	/* --------------------- */

#ifndef __KERNEL__
#include <stdlib.h>
#include <string.h>
	void inv_sleep(int mSecs);
	unsigned long inv_get_tick_count(void);

	/* Kernel implmentations */
#define GFP_KERNEL (0x70)
	static inline void *kmalloc(size_t size,
				    unsigned int gfp_flags __unused)
	{
		return malloc((unsigned int)size);
	}
	static inline void *kzalloc(size_t size, unsigned int gfp_flags __unused)
	{
		void *tmp = malloc((unsigned int)size);
		if (tmp)
			memset(tmp, 0, size);
		return tmp;
	}
	static inline void kfree(void *ptr)
	{
		free(ptr);
	}
	static inline void msleep(long msecs)
	{
		inv_sleep(msecs);
	}
	static inline void udelay(unsigned long usecs)
	{
		inv_sleep((usecs + 999) / 1000);
	}
#else
#include <linux/delay.h>
#endif

#ifdef __cplusplus
}
#endif
#endif				/* _MLOS_H */
