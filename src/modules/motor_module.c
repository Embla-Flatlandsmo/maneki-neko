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

static int module_init(void)
{
    return 0;
}

void set_motor_speed(uint speed, bool dir)
{
    return;
}

uint calculate_motor_speed(const struct qdec_module_event *event)
{
    return 0;
}

static bool app_event_handler(const struct app_event_header *aeh)
{
    if (is_qdec_module_event(aeh))
    {
        uint speed = calculate_motor_speed(cast_qdec_module_event(aeh));
        set_motor_speed(speed, 1);
        return false;
    }

    if (is_module_state_event(aeh))
    {
        struct module_state_event *event = cast_module_state_event(aeh);

        if (check_state(event, MODULE_ID(ble_state),
                        MODULE_STATE_READY))
        {
            static bool initialized;

            __ASSERT_NO_MSG(!initialized);
            initialized = true;

            // if (CONFIG_DESKTOP_HIDS_FIRST_REPORT_DELAY > 0)
            // {
            //     k_work_init_delayable(&notify_secured,
            //                           notify_secured_fn);
            // }

            if (module_init())
            {
                LOG_ERR("Service init failed");

                return false;
            }
            LOG_INF("Service initialized");

            module_set_state(MODULE_STATE_READY);
        }
        return false;
    }

    /* If event is unhandled, unsubscribe. */
    __ASSERT_NO_MSG(false);

    return false;
}
APP_EVENT_LISTENER(MODULE, app_event_handler);
APP_EVENT_SUBSCRIBE(MODULE, qdec_module_event);
APP_EVENT_SUBSCRIBE(MODULE, hid_notification_event);
APP_EVENT_SUBSCRIBE(MODULE, module_state_event);
APP_EVENT_SUBSCRIBE_EARLY(MODULE, ble_peer_event);
