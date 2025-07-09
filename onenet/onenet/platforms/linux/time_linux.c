/**
 * Copyright (c), 2012~2024 iot.10086.cn All Rights Reserved
 *
 * @file time_linux.c
 * @brief time API for linux
 */

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "plat_osl.h"
#include "plat_time.h"

#include <sys/time.h>
#include <time.h>
#include <unistd.h>

/*****************************************************************************/
/* Local Definitions ( Constant and Macro )                                  */
/*****************************************************************************/

/*****************************************************************************/
/* Structures, Enum and Typedefs                                             */
/*****************************************************************************/
struct countdown_tmr_t {
  uint64_t time_end;
};
/*****************************************************************************/
/* Local Function Prototype                                                  */
/*****************************************************************************/

/*****************************************************************************/
/* Local Variables                                                           */
/*****************************************************************************/

/*****************************************************************************/
/* Global Variables                                                          */
/*****************************************************************************/

/*****************************************************************************/
/* Function Implementation                                                   */
/*****************************************************************************/

uint64_t time_get_date(int* year, int* month, int* day, int* hour, int* min, int* sec, int* ms) {
    struct timeval tv;
    struct tm* tm = NULL;
    gettimeofday(&tv, NULL);
    time_t tt = tv.tv_sec;
    tm        = localtime(&tt);
    *year     = tm->tm_year + 1900;
    *month    = tm->tm_mon + 1;
    *day      = tm->tm_mday;
    *hour     = tm->tm_hour;
    *min      = tm->tm_min;
    *sec      = tm->tm_sec;
    *ms       = tv.tv_usec / 1000;
    return 0;
}

uint64_t time_count_ms(void) {
  uint64_t cnt_ms = 0;
  struct timeval tv;

  gettimeofday(&tv, NULL);
  cnt_ms = ((uint64_t)(tv.tv_sec)) * 1000 + tv.tv_usec / 1000;
  return cnt_ms;
}

uint64_t time_count(void) { return (uint64_t)time(NULL); }

void time_delay_ms(uint32_t m_sec) { usleep(m_sec * 1000); }

void time_delay(uint32_t sec) { sleep(sec); }

handle_t countdown_start(uint32_t ms) {
  struct countdown_tmr_t *tmr = NULL;

  tmr = osl_malloc(sizeof(*tmr));

  if (tmr) {
    countdown_set((handle_t)tmr, ms);
  }

  return (handle_t)tmr;
}

void countdown_set(handle_t handle, uint32_t new_ms) {
  struct countdown_tmr_t *tmr = (struct countdown_tmr_t *)handle;

  if (tmr) {
    tmr->time_end = time_count_ms() + new_ms;
  }
}

uint32_t countdown_left(handle_t handle) {
  struct countdown_tmr_t *tmr = (struct countdown_tmr_t *)handle;
  int64_t left = 0;

  if (tmr) {
    left = tmr->time_end - time_count_ms();
  }

  return ((left > 0) ? left : 0);
}

uint32_t countdown_is_expired(handle_t handle) {
  struct countdown_tmr_t *tmr = (struct countdown_tmr_t *)handle;

  return ((!tmr || (tmr->time_end <= time_count_ms())) ? 1 : 0);
}

void countdown_stop(handle_t handle) {
  if (handle) {
    osl_free((void *)handle);
  }
}
