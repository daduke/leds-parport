/*
 * Parallel port leds driver.
 *
 * Author: Christian Herzog <daduke@daduke.org>
 *
 * License: GPL as published by the FSF.
 */

#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/leds.h>
#include <linux/io.h>
#include <linux/module.h>

#define BASE    0x378     
#define DRVNAME "leds-parport"

struct parport_led {
	struct led_classdev cdev;
	u8 reg;
	const char *name;
	unsigned long port;
	u8 mask;
	u8 inv;
};

static struct platform_device *pdev;

static struct parport_led leds[] = {
	{
		.name = "parport:D0",
		.reg  = 0,
		.port = BASE,
		.mask = BIT(0),
		.inv  = 0,
	},
	{
		.name = "parport:D1",
		.reg  = 0,
		.port = BASE,
		.mask = BIT(1),
		.inv  = 0,
	},
	{
		.name = "parport:D2",
		.reg  = 0,
		.port = BASE,
		.mask = BIT(2),
		.inv  = 0,
	},
	{
		.name = "parport:D3",
		.reg  = 0,
		.port = BASE,
		.mask = BIT(3),
		.inv  = 0,
	},
	{
		.name = "parport:D4",
		.reg  = 0,
		.port = BASE,
		.mask = BIT(4),
		.inv  = 0,
	},
	{
		.name = "parport:D5",
		.reg  = 0,
		.port = BASE,
		.mask = BIT(5),
		.inv  = 0,
	},
	{
		.name = "parport:D6",
		.reg  = 0,
		.port = BASE,
		.mask = BIT(6),
		.inv  = 0,
	},
	{
		.name = "parport:D7",
		.reg  = 0,
		.port = BASE,
		.mask = BIT(7),
		.inv  = 0,
	},
	{
		.name = "parport:Strobe",
		.reg  = 1,
		.port = BASE+2,
		.mask = BIT(0),
		.inv  = 1,
	},
	{
		.name = "parport:LF",
		.reg  = 1,
		.port = BASE+2,
		.mask = BIT(1),
		.inv  = 1,
	},
	{
		.name = "parport:Init",
		.reg  = 1,
		.port = BASE+2,
		.mask = BIT(2),
		.inv  = 0,
	},
	{
		.name = "parport:Select",
		.reg  = 1,
		.port = BASE+2,
		.mask = BIT(3),
		.inv  = 1,
	},
};

static DEFINE_SPINLOCK(value_lock);

static u8 leds_status[2];

static void parport_led_brightness_set(struct led_classdev *led_cdev, enum led_brightness value)
{
	struct parport_led *led = container_of(led_cdev, struct parport_led, cdev);
	u8 *val;
	unsigned long flags;

	spin_lock_irqsave(&value_lock, flags);

	val = &leds_status[led->reg];

	if (value == LED_OFF)
		if (led->inv)
			*val |= led->mask;
		else
			*val &= ~led->mask;
	else
		if (led->inv)
			*val &= ~led->mask;
		else
			*val |= led->mask;

	outb(*val, led->port);
	spin_unlock_irqrestore(&value_lock, flags);
}

static int parport_led_probe(struct platform_device *pdev)
{
	int i;
	int ret;

	for (i = 0; i < ARRAY_SIZE(leds); i++) {

		leds[i].cdev.name = leds[i].name;
		leds[i].cdev.brightness_set = parport_led_brightness_set;

		ret = devm_led_classdev_register(&pdev->dev, &leds[i].cdev);
		if (ret < 0)
			return ret;
	}

	leds_status[0] = 0;		/* turn off all data leds */
	outb(leds_status[0], BASE);
	leds_status[1] = 11;		/* turn off all control leds */
	outb(leds_status[1], BASE+2);

	return 0;
}

static int parport_led_remove(struct platform_device *pdev)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(leds); i++) {
		devm_led_classdev_unregister(&pdev->dev, &leds[i].cdev);
	}
	return 0;
}

static struct platform_driver parport_led_driver = {
	.probe		= parport_led_probe,
	.remove		= parport_led_remove,
	.driver		= {
		.name	= DRVNAME,
	},
};

static int __init parport_led_init(void)
{
	int ret;
	ret = platform_driver_register(&parport_led_driver);
	if (ret < 0)
		goto out;

	pdev = platform_device_register_simple(DRVNAME, -1, NULL, 0);
	if (IS_ERR(pdev)) {
		ret = PTR_ERR(pdev);
		platform_driver_unregister(&parport_led_driver);
		goto out;
	}

out:
	return ret;
}

static void __exit parport_led_exit(void)
{
	platform_device_unregister(pdev);
	platform_driver_unregister(&parport_led_driver);
}

module_init(parport_led_init);
module_exit(parport_led_exit);

MODULE_AUTHOR("Christian Herzog <daduke@daduke.org>");
MODULE_DESCRIPTION("Parallel port LED driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:leds-parport");
