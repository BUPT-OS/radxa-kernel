#ifndef _EVL_UAPI_DEVICES_PWM_H
#define _EVL_UAPI_DEVICES_PWM_H

#include <linux/types.h>


enum pwm_uapi_polarity{
	UAPI_PWM_POLARITY_NORMAL,
	UAPI_PWM_POLARITY_INVERSED,
};


struct pwm_uapi_state {
	u64 period;
	u64 duty_cycle;
	enum pwm_uapi_polarity polarity;
#ifdef __KERNEL__
#ifdef CONFIG_PWM_ROCKCHIP_ONESHOT
	u64 oneshot_count;
	u32 oneshot_repeat;
	u64 duty_offset;
#endif /* CONFIG_PWM_ROCKCHIP_ONESHOT */
#endif
    bool enabled;
};

#endif