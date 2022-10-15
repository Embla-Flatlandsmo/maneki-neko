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
#include <caf/events/button_event.h>
#include <caf/events/click_event.h>
#include <caf/events/module_state_event.h>

#define MODULE motor_module

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(MODULE, CONFIG_MOTOR_MODULE_LOG_LEVEL);

static const struct pwm_dt_spec servo = PWM_DT_SPEC_GET(DT_NODELABEL(servo));
static const uint32_t min_pulse = DT_PROP(DT_NODELABEL(servo), min_pulse);
static const uint32_t max_pulse = DT_PROP(DT_NODELABEL(servo), max_pulse);

static const uint32_t pulse_range = (max_pulse-min_pulse)/2;

#define BTN_INCREASE_PERIOD 0
#define BTN_DECREASE_PERIOD 2
#define BTN_INCREASE_PW 1
#define BTN_DECREASE_PW 3

#define AMPLITUDE_STEP 1
#define PERIOD_STEP 10

#define MAX_PERIOD_MSEC 3000
#define MIN_PERIOD_MSEC 200

// uint32_t pulse_width = min_pulse;
uint32_t amplitude = 0;
uint32_t motor_time_period_msec = MIN_PERIOD_MSEC;

uint32_t map_range(uint32_t input, uint32_t input_start, uint32_t input_end, uint32_t output_start, uint32_t output_end)
{
    return output_start + ((output_end - output_start) / (input_end - input_start)) * (input - input_start);
}


static int setup(void)
{
	int ret;

	if (!device_is_ready(servo.dev)) {
		LOG_ERR("Error: PWM device %s is not ready\n", servo.dev->name);
		return 0;
	}
    return 0;
}

uint8_t calculate_motor_amplitude(const struct qdec_module_event *event)
{
    return 0;
}

uint8_t calculate_motor_time_period(const struct qdec_module_event *event)
{
    return 0;
}

uint8_t set_motor_time_period(uint8_t time_period_ms)
{
    return 0;
}

static void handle_period_event(float value)
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

static void handle_amplitude_event(float value) 
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

static bool handle_qdec_module_event(const struct qdec_module_event *evt)
{
    switch (evt->type)
    {
        case QDEC_A_EVT_DATA_SEND:
            handle_period_event(evt->data.rot_val);
            break;
        case QDEC_B_EVT_DATA_SEND:
            handle_amplitude_event(evt->data.rot_val);
            break;
        default:
            break;
    }
    return false;
}

static bool handle_button_event(const struct button_event *evt)
{
	if (evt->pressed) {
		switch (evt->key_id) {
		case BTN_INCREASE_PERIOD:
            if (motor_time_period_msec >= MAX_PERIOD_MSEC)
            {
                motor_time_period_msec = MAX_PERIOD_MSEC;
                LOG_DBG("Max period width reached.");
                break;
            }
            motor_time_period_msec += PERIOD_STEP;
            LOG_DBG("increasing period");
			break;
		case BTN_DECREASE_PERIOD:        
            if (motor_time_period_msec <= MIN_PERIOD_MSEC)
            {
                motor_time_period_msec = MIN_PERIOD_MSEC;
                LOG_DBG("Min period width reached.");
                break;
            }
            motor_time_period_msec -= PERIOD_STEP;
            LOG_DBG("Decreasing period");
			break;
        case BTN_INCREASE_PW:
            if (amplitude >= 100)
            {
                amplitude = 100;
                LOG_DBG("Max amplitude reached.");
                break;
            }
            // if (pulse_width >= max_pulse)
            // {
            //     pulse_width = max_pulse;
            //     LOG_DBG("Max pulse width reached.");
            //     break;
            // }
            amplitude += AMPLITUDE_STEP;
            LOG_DBG("Increasing amplitude");
            break;
        case BTN_DECREASE_PW:
            if (amplitude <= 0)
            {
                amplitude = 0;
                LOG_DBG("Min amplitude reached.");
                break;
            }
            amplitude -= AMPLITUDE_STEP;
            LOG_DBG("Decreasing Amplitude");
            break;
		default:
			break;
		}
	}

	return false;
}

static bool app_event_handler(const struct app_event_header *aeh)
{
    if (is_qdec_module_event(aeh))
    {
        return handle_qdec_module_event(cast_qdec_module_event(aeh));
        // struct qdec_module_event *event = cast_qdec_module_event(aeh);

        // if (event->type == QDEC_A_EVT_DATA_SEND)
        // {
        //     motor_amplitude = calculate_motor_amplitude(event);
        // }
        // if (event->type == QDEC_B_EVT_DATA_SEND)
        // {
        //     motor_time_period_msec = calculate_motor_time_period(event);
        // }
        // return false;
    }
	if (is_button_event(aeh)) {
		return handle_button_event(cast_button_event(aeh));
	}

	if (is_module_state_event(aeh)) {
		// const struct module_state_event *event = cast_module_state_event(aeh);

		// if (check_state(event, MODULE_ID(main), MODULE_STATE_READY)) {
		// 	module_init();
		// }

		return false;
	}
	/* Event not handled but subscribed. */
	__ASSERT_NO_MSG(false);
    return false;

}

static void module_thread_fn(void)
{
    LOG_DBG("Initializing motor module");
	int err;
	// self.thread_id = k_current_get();

	// err = module_start(&self);
	// if (err) {
	// 	LOG_ERR("Failed starting module, error: %d", err);
	// 	// SEND_ERROR(data, DATA_EVT_ERROR, err);
	// }

	err = setup();
	if (err) {
		LOG_ERR("setup, error: %d", err);
		// SEND_ERROR(data, DATA_EVT_ERROR, err);
	}
    int ret;
    static int up_or_down = 1;

    while(true)
    {   
        uint32_t pulse_width_offset = map_range(amplitude, 0, 100, 0, pulse_range);
        uint32_t pulse_width = min_pulse+pulse_range+up_or_down*pulse_width_offset;
        up_or_down *= -1;
        ret = pwm_set_pulse_dt(&servo, pulse_width);
		// LOG_DBG("Setting servo %d\n", pulse_width);
		if (ret < 0) {
			LOG_ERR("Error %d: failed to set pulse width\n", ret);
			return;
		}
        k_sleep(K_MSEC(motor_time_period_msec));
    }
	// while (true) {
    //     LOG_DBG("Setting motor (lol)");
	// 	k_sleep(K_MSEC(motor_time_period_msec));
	// }
}

K_THREAD_DEFINE(motor_module_thread, CONFIG_MOTOR_THREAD_STACK_SIZE,
		module_thread_fn, NULL, NULL, NULL,
		K_HIGHEST_APPLICATION_THREAD_PRIO, 0, 0);
APP_EVENT_LISTENER(MODULE, app_event_handler);
APP_EVENT_SUBSCRIBE(MODULE, qdec_module_event);
APP_EVENT_SUBSCRIBE(MODULE, module_state_event);
APP_EVENT_SUBSCRIBE(MODULE, button_event);
// APP_EVENT_SUBSCRIBE(MODULE, click_event);