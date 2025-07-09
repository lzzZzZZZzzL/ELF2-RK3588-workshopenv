/**
 * Copyright (c), 2012~2024 iot.10086.cn All Rights Reserved
 *
 * @file plat_time.h
 * @brief Time interface:Date, countdown, start time, delay
 */

#ifndef __PLAT_TIME_H__
#define __PLAT_TIME_H__

/*****************************************************************************/
/* Includes                                                                  */
/*****************************************************************************/
#include "data_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*****************************************************************************/
/* External Definition ( Constant and Macro )                                */
/*****************************************************************************/

/*****************************************************************************/
/* External Structures, Enum and Typedefs                                    */
/*****************************************************************************/

/*****************************************************************************/
/* External Variables and Functions                                          */
/*****************************************************************************/
/**
 * @brief Return the current year, month, day information
 *
 * @return 0 - Succeed
 */
uint64_t time_get_date(int* year, int* month, int* day, int* hour, int* min, int* sec, int* ms);

/**
 * @brief Time timing，Unit is millisecond
 *
 * @return uint64_t Returns the current millisecond count
 */
uint64_t time_count_ms(void);

/**
 * @brief Time timing，Unit is second
 *
 * @return uint64_t Returns the current second count
 */
uint64_t time_count(void);

/**
 * @brief Delay a specified time，Unit is millisecond
 *
 * @param m_sec
 */
void time_delay_ms(uint32_t m_sec);

/**
 * @brief Delay a specified time，Unit is second
 *
 * @param sec
 */
void time_delay(uint32_t sec);

/**
 * @brief Start countdown
 *
 * @param ms Set the countdown time，Unit is ms
 * @retval  0 - Failed
 * @retval Other - Succeed，Return to the countdown timer operation handle
 */
handle_t countdown_start(uint32_t ms);

/**
 * @brief Reset the countdown timeout
 *
 * @param handle Countdown timer operation handle
 * @param ms New countdown timer timeout
 */
void countdown_set(handle_t handle, uint32_t new_ms);

/**
 * @brief Return Countdown Timer Time Remaining
 *
 * @param handle Countdown timer operation handle
 * @return  Countdown Timer Time Remaining
 */
uint32_t countdown_left(handle_t handle);

/**
 * @brief Determine whether the countdown timer has timed out
 *
 * @param handle Countdown timer operation handle
 * @return  0 - Not Timed Out
 *          1 - Timed out
 */
uint32_t countdown_is_expired(handle_t handle);

/**
 * @brief Stop the countdown timer，Destroy Resources
 *
 * @param handle Countdown timer operation handle
 */
void countdown_stop(handle_t handle);

#ifdef __cplusplus
}
#endif

#endif
