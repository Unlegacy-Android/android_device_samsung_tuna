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
/*******************************************************************************
 *
 * $Id: mlos_linux.c 5629 2011-06-11 03:13:08Z mcaramello $
 *
 *******************************************************************************/

/**
 *  @defgroup MLOS
 *  @brief OS Interface.
 *
 *  @{
 *      @file mlos.c
 *      @brief OS Interface.
**/

/* ------------- */
/* - Includes. - */
/* ------------- */

#include <sys/time.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#include "stdint_invensense.h"

#include "mlos.h"
#include <errno.h>


/* -------------- */
/* - Functions. - */
/* -------------- */

/**
 *  @brief  Sleep function.
 */
void inv_sleep(int mSecs)
{
    usleep(mSecs*1000);
}


/**
 *  @brief  get system's internal tick count.
 *          Used for time reference.
 *  @return current tick count.
 */
unsigned long inv_get_tick_count()
{
    struct timeval tv;

    if (gettimeofday(&tv, NULL) !=0)
        return 0;

    return (long)((tv.tv_sec * 1000000LL + tv.tv_usec) / 1000LL);
}

  /**********************/
 /** @} */ /* defgroup */
/**********************/
