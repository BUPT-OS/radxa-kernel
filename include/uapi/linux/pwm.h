/* SPDX-License-Identifier: GPL-2.0-only WITH Linux-syscall-note */
/*
 * <linux/pwm.h> - userspace ABI for the GPIO character devices
 *
 * Copyright (C) 2025 Qichen Qiu
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 */
#ifndef _UAPI_PWM_H_
#define _UAPI_PWM_H_

#include <linux/types.h>

/*
*  polarity of pwm wawe
*/
enum uapi_pwm_polarity {
	PWM_UAPI_POLARITY_NORMAL,
	PWM_UAPI_POLARITY_INVERSED,
};

/**
 * struct pwm_state_request - Change state of current pwm.
 * @period: PWM period (in nanoseconds)
 * @duty_cycle: PWM duty cycle (in nanoseconds)
 * @polarity: PWM polarity
 * @enabled: PWM enabled status
 * @oenshot_count: If oneshot_count is non zero and the device support one-shot
 *                 mode, the device will only triger oneshot_count times of 
 *                 PWM wave.
 * @oneshot_repeat: Reserved.
 * @usage_power: If set, the PWM driver is only required to maintain the power
 *               output but has more freedom regarding signal form.
 *               If supported, the signal can be optimized, for example to
 *               improve EMI by phase shifting individual channels.

 */
struct pwm_state_request {
	u64 period;
	u64 duty_cycle;
	enum uapi_pwm_polarity polarity;
	u64 oneshot_count;
	u32 oneshot_repeat;
	bool enabled;
	bool usage_power;
};

/*
 * v1 ioctl()s
 *
 */
#define PWM_GET_STATE_IOCTL _IOR(0xBE, 0x01, struct pwm_state_request)
#define PWM_SET_STATE_IOCTL _IOW(0xBE, 0x02, struct pwm_state_request)

#endif /* _PWM_GPIO_H */