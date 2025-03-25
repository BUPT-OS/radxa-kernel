/* SPDX-License-Identifier: GPL-2.0 */

#ifndef PWMLIB_CDEV_H
#define PWMLIB_CDEV_H

#include <linux/types.h>

struct pwm_device;

int pwmlib_cdev_register(struct pwm_device *pdev);
void pwmlib_cdev_unregister(struct pwm_device *pdev);

#endif /* PWMLIB_CDEV_H */
