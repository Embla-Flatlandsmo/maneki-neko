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

#include "events/qdec_module_event.h"

#define MODULE motor_module

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(MODULE, CONFIG_MOTOR_MODULE_LOG_LEVEL);

uint8_t motor_amplitude = 0;
uint8_t motor_time_period_msec = 200;


// static struct module_data self = {
// 	.name = "motor",
// 	// .msg_q = &msgq_data,
// 	.supports_shutdown = true,
// };


static int setup(void)
{
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

static bool app_event_handler(const struct app_event_header *aeh)
{
    if (is_qdec_module_event(aeh))
    {
        struct qdec_module_event *event = cast_qdec_module_event(aeh);

        if (event->type == QDEC_A_EVT_DATA_SEND)
        {
            motor_amplitude = calculate_motor_amplitude(event);
        }
        if (event->type == QDEC_B_EVT_DATA_SEND)
        {
            motor_time_period_msec = calculate_motor_time_period(event);
        }
        return false;
    }

	// if (is_module_state_event(aeh)) {
	// 	const struct module_state_event *event = cast_module_state_event(aeh);

	// 	if (check_state(event, MODULE_ID(main), MODULE_STATE_READY)) {
	// 		module_init();
	// 	}

	// 	return false;
	// }
}

static void module_thread_fn(void)
{
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

	while (true) {

		k_sleep(K_MSEC(motor_time_period_msec));
	}
}

K_THREAD_DEFINE(motor_module_thread, CONFIG_MOTOR_THREAD_STACK_SIZE,
		module_thread_fn, NULL, NULL, NULL,
		K_LOWEST_APPLICATION_THREAD_PRIO, 0, 0);
APP_EVENT_LISTENER(MODULE, app_event_handler);
APP_EVENT_SUBSCRIBE(MODULE, qdec_module_event);
APP_EVENT_SUBSCRIBE(MODULE, module_state_event);