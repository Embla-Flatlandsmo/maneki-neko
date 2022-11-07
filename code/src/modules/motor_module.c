/*
 * Copyright (c) 2018 - 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <assert.h>
#include <limits.h>

#include <zephyr/kernel.h>
#include <zephyr/types.h>

#include <zephyr/sys/util.h>
#include <device.h>
#include <drivers/pwm.h>

#include "events/qdec_module_event.h"
#include <caf/events/module_state_event.h>

#define MODULE motor_module

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(MODULE, CONFIG_MOTOR_MODULE_LOG_LEVEL);

static const struct pwm_dt_spec servo = PWM_DT_SPEC_GET(DT_NODELABEL(servo));
static const uint32_t min_pulse = DT_PROP(DT_NODELABEL(servo), min_pulse);
static const uint32_t max_pulse = DT_PROP(DT_NODELABEL(servo), max_pulse);

static const uint32_t pulse_range = (max_pulse-min_pulse)/2;

#if IS_ENABLED(CONFIG_MOTOR_MODULE_USE_BTN_INPUT)
#include <caf/events/button_event.h>
#define BTN_INCREASE_PERIOD 0
#define BTN_DECREASE_PERIOD 2
#define BTN_INCREASE_AMPLITUDE 1
#define BTN_DECREASE_AMPLITUDE 3
#endif /* CONFIG_MOTOR_MODULE_USE_BTN_INPUT */

#define AMPLITUDE_STEP CONFIG_MOTOR_MODULE_AMPLITUDE_STEP
#define PERIOD_STEP CONFIG_MOTOR_MODULE_PERIOD_STEP

#define MAX_PERIOD_MSEC CONFIG_MOTOR_MODULE_MAX_PERIOD_MSEC
#define MIN_PERIOD_MSEC CONFIG_MOTOR_MODULE_MIN_PERIOD_MSEC

uint32_t amplitude = 100;
uint32_t motor_time_period_msec = MIN_PERIOD_MSEC;

/*================= UTILITY FUNCTIONS =================*/

/**
 * @brief Maps a value in range [input_start, input_end] to the range [output_start, output_end]
 * 
 * @param input value to map
 * @param input_start Start of value range (minimum)
 * @param input_end End of value range (maximum)
 * @param output_start Start of new range (minimum)
 * @param output_end End of new range (maximum)
 * @return uint32_t Value, in new range
 */
uint32_t map_range(uint32_t input, uint32_t input_start, uint32_t input_end, uint32_t output_start, uint32_t output_end)
{
    return output_start + ((output_end - output_start) / (input_end - input_start)) * (input - input_start);
}

static void update_period(float value)
{
    if (value > 0) 
    {
        if (motor_time_period_msec >= MAX_PERIOD_MSEC)
        {
            motor_time_period_msec = MAX_PERIOD_MSEC;
            LOG_DBG("Max period width reached.");
            return;
        }
        motor_time_period_msec += PERIOD_STEP;
        LOG_DBG("increasing period");
        return;
    }

    if (value < 0) 
    {
        if (motor_time_period_msec <= MIN_PERIOD_MSEC)
        {
            motor_time_period_msec = MIN_PERIOD_MSEC;
            LOG_DBG("Min period width reached.");
            return;
        }
        motor_time_period_msec -= PERIOD_STEP;
        LOG_DBG("Decreasing period");
        return;
    }
    return;
}

static void update_amplitude(float value) 
{
    if (value > 0) {
        if (amplitude >= 100)
        {
            amplitude = 100;
            LOG_DBG("Max amplitude reached.");
            return;
        }
        amplitude += AMPLITUDE_STEP;
        LOG_DBG("Increasing amplitude");
        return;
    }

    if (value < 0) {
        if (amplitude <= 0)
        {
            amplitude = 0;
            LOG_DBG("Min amplitude reached.");
            return;
        }
        amplitude -= AMPLITUDE_STEP;
        LOG_DBG("Decreasing Amplitude");
        return;
    }

    return;
}


/*================= EVENT HANDLERS =================*/

static bool handle_qdec_module_event(const struct qdec_module_event *evt)
{
    switch (evt->type)
    {
        case QDEC_A_EVT_DATA_SEND:
            update_period(evt->data.rot_val);
            break;
        case QDEC_B_EVT_DATA_SEND:
            update_amplitude(evt->data.rot_val);
            break;
        default:
            break;
    }
    return false;
}


#if IS_ENABLED(CONFIG_MOTOR_MODULE_USE_BTN_INPUT)
static bool handle_button_event(const struct button_event *evt)
{
	if (evt->pressed) {
		switch (evt->key_id) {
		case BTN_INCREASE_PERIOD:
            update_period(1.0);
			break;
		case BTN_DECREASE_PERIOD:
            update_period(-1.0);
			break;
        case BTN_INCREASE_AMPLITUDE:
            update_amplitude(1.0);
            break;
        case BTN_DECREASE_AMPLITUDE:
            update_amplitude(-1.0);
            break;
		default:
			break;
		}
	}
	return false;
}
#endif /* CONFIG_MOTOR_MODULE_USE_BTN_INPUT */

static bool app_event_handler(const struct app_event_header *aeh)
{
    if (is_qdec_module_event(aeh))
    {
        return handle_qdec_module_event(cast_qdec_module_event(aeh));
    }

#if IS_ENABLED(CONFIG_MOTOR_MODULE_USE_BTN_INPUT)
	if (is_button_event(aeh)) {
		return handle_button_event(cast_button_event(aeh));
	}
#endif /* CONFIG_MOTOR_MODULE_USE_BTN_INPUT */

	if (is_module_state_event(aeh)) {
        /* No setup necessary since it is done in module_thread_fn */
		return false;
	}
	/* Event not handled but subscribed. */
	__ASSERT_NO_MSG(false);
    return false;

}

/*================= MODULE THREAD =================*/
static int setup(void)
{
	int ret;

	if (!device_is_ready(servo.dev)) {
		LOG_ERR("Error: PWM device %s is not ready\n", servo.dev->name);
		return 0;
	}
    return 0;
}

static void module_thread_fn(void)
{
    LOG_DBG("Initializing motor module.");
    LOG_DBG("Pulse width: [%d, %d] (ns)", min_pulse, max_pulse);
    LOG_DBG("Arm rotation period range: [%d, %d] (ms)", MIN_PERIOD_MSEC, MAX_PERIOD_MSEC);
	int err;

	err = setup();
	if (err) {
		LOG_ERR("setup, error: %d", err);
	}

    int ret;
    static int up_or_down = 1;

    while(true)
    {   
        uint32_t pulse_width_offset = map_range(amplitude, 0, 100, 0, pulse_range);
        uint32_t pulse_width = min_pulse+pulse_range+up_or_down*pulse_width_offset;
        up_or_down *= -1;
        ret = pwm_set_pulse_dt(&servo, pulse_width);
		if (ret < 0) {
			LOG_ERR("Error %d: failed to set pulse width\n", ret);
			return;
		}
        k_sleep(K_MSEC(motor_time_period_msec));
    }
}

K_THREAD_DEFINE(motor_module_thread, CONFIG_MOTOR_THREAD_STACK_SIZE,
		module_thread_fn, NULL, NULL, NULL,
		K_HIGHEST_APPLICATION_THREAD_PRIO, 0, 0);
APP_EVENT_LISTENER(MODULE, app_event_handler);
APP_EVENT_SUBSCRIBE(MODULE, qdec_module_event);
APP_EVENT_SUBSCRIBE(MODULE, module_state_event);

#if IS_ENABLED(CONFIG_MOTOR_MODULE_USE_BTN_INPUT)
APP_EVENT_SUBSCRIBE(MODULE, button_event);
#endif /* CONFIG_MOTOR_MODULE_USE_BTN_INPUT */