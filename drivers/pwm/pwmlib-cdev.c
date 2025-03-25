#include <linux/export.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/pwm.h>
#include <uapi/linux/pwm.h>
#include <evl/file.h>
#include "pwmlib-cdev.h"

#define PWM_DEVT_CHIP_BASE_OFFSET 5
#define PWM_CHIP_BASE(dev) ((unsigned int)((dev) << PWM_DEVT_CHIP_BASE_OFFSET))
#define PWMCHIP_NAME "pwmchip"
#define PWM_DEV_MAX 256

static dev_t pwm_devt;
static bool pwmlib_initialized = true;

struct pwmlib_context {
	struct pwm_device *pwm;
};

static long pwmlib_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct pwmlib_context *ctx = filp->private_data;
	struct pwm_state state;
	struct pwm_state_request ustate;
	int ret;
	void __user *uarg = (void __user *)arg;
	switch (cmd) {
	case PWM_GET_STATE_IOCTL:
		pwm_get_state(ctx->pwm, &state);
		return copy_to_user(uarg, &state, sizeof(ustate)) ? -EFAULT : 0;

	case PWM_SET_STATE_IOCTL:
		ret = copy_from_user(&ustate, uarg, sizeof(ustate));
		if (ret) {
			return -EFAULT;
		}
		pwm_get_state(ctx->pwm, &state);
		state.period = ustate.period;
		state.duty_cycle = ustate.duty_cycle;
		state.polarity = ustate.polarity == PWM_UAPI_POLARITY_NORMAL ?
					 PWM_POLARITY_NORMAL :
					 PWM_POLARITY_INVERSED;
		state.enabled = ustate.enabled;
		ret = pwm_apply_state(ctx->pwm, &state);
		return ret;
	default:
		return -EINVAL;
	}

	return 0;
}

static int pwmlib_open(struct inode *inode, struct file *filp)
{
	struct cdev *cdev = inode->i_cdev;
	struct pwm_device *pwm;
	struct pwmlib_context *ctx;
	int ret = 0;

	pwm = container_of(cdev, struct pwm_device, cdev);
	if (!pwm->chip) {
		ret = -ENODEV;
		return ret;
	}
	pwm = pwm_request_from_chip(pwm->chip, pwm->hwpwm, "cdev");
	if (IS_ERR(pwm)) {
		return PTR_ERR(pwm);
	}

	ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
	if (!ctx) {
		goto alloc_fail;
	}
	ctx->pwm = pwm;
	filp->private_data = ctx;
	nonseekable_open(inode, filp);

	return 0;

alloc_fail:
	pwm_put(pwm);
	return ret;
}

static int pwmlib_release(struct inode *inode, struct file *filp)
{
	struct cdev *cdev = inode->i_cdev;
	struct pwm_device *pwm = container_of(cdev, struct pwm_device, cdev);
	struct pwmlib_context *ctx = filp->private_data;

	kfree(ctx);
	pwm_put(pwm);

	return 0;
}

static struct class * pwm_cdev_class;
static const struct file_operations pwm_fops = {
	.unlocked_ioctl = pwmlib_ioctl,
	.release = pwmlib_release,
	.open = pwmlib_open,
	.owner = THIS_MODULE,
	.llseek = no_llseek,
};

static int __init pwm_cdev_init(void)
{
	int ret;
	pwm_cdev_class = class_create(THIS_MODULE, "pwm_cdev");
	if (IS_ERR(pwm_cdev_class)){
		pr_err("failed to create class\n");
		return PTR_ERR(pwm_cdev_class);
	}

	ret = alloc_chrdev_region(&pwm_devt, 0, PWM_DEV_MAX, PWMCHIP_NAME);
	if (ret < 0) {
		pr_err("pwmlib: failed to allocate char dev region\n");
		return ret;
	}
	pwmlib_initialized = true;

	return 0;
}
core_initcall(pwm_cdev_init);

int pwmlib_cdev_register(struct pwm_device *pdev)
{
	int ret;
	dev_t devt;
	cdev_init(&pdev->cdev, &pwm_fops);
	pdev->cdev.owner = THIS_MODULE;
	devt = MKDEV(MAJOR(pwm_devt),
		     PWM_CHIP_BASE(pdev->chip->base) | pdev->hwpwm);
	device_initialize(&pdev->dev);
	pdev->dev.devt = devt;
	pdev->dev.class = pwm_cdev_class;
	pdev->dev.parent = pdev->chip->dev;
	dev_set_name(&pdev->dev, "pwm%d", pdev->chip->base);

	ret = cdev_device_add(&pdev->cdev, &pdev->dev);
	if (ret)
		return ret;

	dev_dbg(pdev->chip->dev, "added PWM chardev (%d:%d)\n", MAJOR(devt),
		MINOR(devt));
	return 0;
}
EXPORT_SYMBOL(pwmlib_cdev_register);

void pwmlib_cdev_unregister(struct pwm_device *pdev)
{
	cdev_device_del(&pdev->cdev, &pdev->dev);
}
EXPORT_SYMBOL(pwmlib_cdev_unregister);