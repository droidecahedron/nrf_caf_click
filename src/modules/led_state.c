/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>

#define MODULE led_state
#include <caf/events/module_state_event.h>
#include <caf/events/led_event.h>
#include <caf/events/button_event.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(MODULE, CONFIG_CAF_SAMPLE_LED_STATE_LOG_LEVEL);

#include "led_state_def.h"

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>

#include <caf/events/click_event.h>


#define BLINKYTHREAD_PRIORITY 3
#define STACKSIZE 256

#define BLINKY_SLEEP_DEFAULT 100
#define BLINKY_SLEEP_DOUBLE 25
#define BLINKY_SLEEP_LONG 1000

uint32_t BLINKY_SLEEP = BLINKY_SLEEP_DEFAULT;

volatile bool dir = true; // true = forward, false = reverse
/*
 * The led0 devicetree alias is optional. If present, we'll use it
 * to turn on the LED whenever the button is pressed.
 */
static struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios,
						     {0});
static struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led1), gpios,
						     {0});
static struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led2), gpios,
						     {0});
static struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led3), gpios,
						     {0});                             



enum button_id {
	BUTTON_ID_NEXT_EFFECT,
	BUTTON_ID_NEXT_LED,

	BUTTON_ID_COUNT
};

static bool handle_click_event(const struct click_event *evt) {
	if(evt->key_id == 0)
	{
		switch (evt->click)
		{
			case CLICK_SHORT:
				dir ^= dir;
				break;
			case CLICK_LONG:
				BLINKY_SLEEP = BLINKY_SLEEP_LONG;
				break;
			case CLICK_DOUBLE:
				BLINKY_SLEEP = BLINKY_SLEEP_DOUBLE;
				break;
			default:
				break;
		
		}
	}

	return false;
}

static bool app_event_handler(const struct app_event_header *aeh)
{
	if (is_click_event(aeh)) {
        return handle_click_event(cast_click_event(aeh));
    }

	if (is_module_state_event(aeh)) {
		const struct module_state_event *event = cast_module_state_event(aeh);

		if (check_state(event, MODULE_ID(leds), MODULE_STATE_READY)) {
			printk("ready steady\n");
		}

		return false;
	}

	/* Event not handled but subscribed. */
	__ASSERT_NO_MSG(false);

	return false;
}



void blinkythread(void)
{
	uint8_t cnt = 0;
    /* If we have an LED, match its state to the button's. */
    while(1)
    {
		(dir)? cnt++ : cnt--;
        switch(cnt & 0x3)
        {
            case 0:
                gpio_pin_toggle_dt(&led0);
                break;
            case 1:
                gpio_pin_toggle_dt(&led1);
                break;
            case 2:
                gpio_pin_toggle_dt(&led3);
                break;
            case 3:
                gpio_pin_toggle_dt(&led2);
                break;
        }

        k_msleep(BLINKY_SLEEP);
    }
}


APP_EVENT_LISTENER(MODULE, app_event_handler);
APP_EVENT_SUBSCRIBE(MODULE, module_state_event);
APP_EVENT_SUBSCRIBE(MODULE, click_event);
K_THREAD_DEFINE(blinkythread_id, STACKSIZE, blinkythread, NULL, NULL, NULL, BLINKYTHREAD_PRIORITY, 0, 0);