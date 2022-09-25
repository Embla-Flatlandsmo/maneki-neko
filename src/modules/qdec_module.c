/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */


#include <zephyr/kernel.h>
#include <app_event_manager.h>
#include <zephyr/settings/settings.h>
#include <drivers/sensor.h>
#include "modules_common.h"
#include "events/qdec_module_event.h"

#define MODULE qdec_module
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(MODULE, CONFIG_QDEC_MODULE_LOG_LEVEL);

static struct device *qdeca_dev;
static struct device *qdecb_dev;

static struct sensor_trigger trig_a;
static struct sensor_trigger trig_b;

static void trigger_handler(const struct device *dev,
			    const struct sensor_trigger *trig)
{
	struct sensor_value temp;
}

static struct module_data self = {
	.name = "qdec",
	// .msg_q = &msgq_data,
	// .supports_shutdown = true,
};

static bool app_event_handler(const struct app_event_header *aeh)
{
	/* If event is unhandled, unsubscribe. */
	__ASSERT_NO_MSG(false);

	return false;
}

// static int set_qdec_attr(struct device* dev)
// {
// 	int t = SENSOR_ATTR_
// 	int rc = sensor_attr_set(dev, SENSOR_CHAN_ROTATION, )
	// int rc = sensor_attr_set(dev, SENSOR_CHAN_AMBIENT_TEMP,
	// 						 SENSOR_ATTR_LOWER_THRESH, &val);
// }

static void trigger_a_handler(const struct device *dev, const struct sensor_trigger trig)
{
	struct sensor_value rot;
	int err;
	err = sensor_sample_fetch(dev);
	if (err != 0)
	{
		printf("sensor_sample_fetch error: %d\n", err);
		return;
	}
	err = sensor_channel_get(dev, SENSOR_CHAN_ROTATION, &rot);
	if (err != 0)
	{
		printf("sensor_channel_get error: %d\n", err);
		return;
	}
	struct qdec_module_event *qdec_module_event = new_qdec_module_event();
	qdec_module_event->type = QDEC_EVT_DATA_SEND;
	qdec_module_event->data.rot_speed_val = sensor_value_to_double(&rot);
	APP_EVENT_SUBMIT(qdec_module_event);
}

static void trigger_b_handler(const struct device *dev, const struct sensor_trigger trig)
{
}

static int setup(void)
{
	qdeca_dev = device_get_binding(DT_LABEL(DT_NODELABEL(qdeca)));
	qdecb_dev = device_get_binding(DT_LABEL(DT_NODELABEL(qdecb)));
	if (qdeca_dev == NULL || qdecb_dev == NULL)
	{
		LOG_ERR("Failed to get bindings for qdec devices");
		return -ENODEV;
	}

	trig_a.type = SENSOR_TRIG_DATA_READY;
	trig_a.chan = SENSOR_CHAN_ROTATION;
	trig_b.type = SENSOR_TRIG_DATA_READY;
	trig_b.chan = SENSOR_CHAN_ROTATION;
	int err;
	err = sensor_trigger_set(qdeca_dev, trig_a, trigger_a_handler);
	if (err)
	{
		LOG_ERR("sensor_trigger_set for qdecb error: %d", err);
		return err;
	}

	err = sensor_trigger_set(qdecb_dev, trig_b, trigger_b_handler);
	{
		LOG_ERR("sensor_trigger_set for qdecb error: %d", err);
		return err;
	}

	return 0;
}

static void module_thread_fn(void)
{
	int err;

	self.thread_id = k_current_get();

	err = module_start(&self);
	if (err) {
		LOG_ERR("Failed starting module, error: %d", err);
		SEND_ERROR(qdec, QDEC_EVT_ERROR, err);
	}

	err = setup();
	if (err) {
		LOG_ERR("setup, error: %d", err);
		SEND_ERROR(qdec, QDEC_EVT_ERROR, err);
	}
    uint8_t simulated_val = 0;
	while (true) {
        simulated_val += 10;
		struct qdec_module_event *qdec_module_event = new_qdec_module_event();
		qdec_module_event->type = QDEC_EVT_DATA_SEND;
		qdec_module_event->data.rot_speed_val = simulated_val;
        APP_EVENT_SUBMIT(qdec_module_event);
        k_sleep(K_MSEC(500));
	}
}

K_THREAD_DEFINE(qdec_module_thread, CONFIG_QDEC_THREAD_STACK_SIZE,
		module_thread_fn, NULL, NULL, NULL,
		K_LOWEST_APPLICATION_THREAD_PRIO, 0, 0);

APP_EVENT_LISTENER(MODULE, app_event_handler);