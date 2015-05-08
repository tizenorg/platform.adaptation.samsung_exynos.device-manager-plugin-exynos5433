/*
 * device-node
 *
 * Copyright (c) 2015 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <linux/limits.h>

#include <hw/led.h>
#include "../shared.h"

#ifndef CAMERA_BACK_PATH
#define CAMERA_BACK_PATH	"/sys/class/leds/ktd2692-flash"
#endif

static int camera_back_set_state(struct led_state *state)
{
	static int max = -1;
	int brt, r;

	if (!state) {
		_E("wrong parameter");
		return -EINVAL;
	}

	if (state->type == LED_TYPE_BLINK) {
		_E("camera back led does not support LED_TYPE_BLINK mode");
		return -ENOTSUP;
	}

	/* if there is a max brightness of led */
	if (max < 0) {
		r = sys_get_int(CAMERA_BACK_PATH"/max_brightness", &max);
		if (r < 0) {
			_E("fail to get max brightness : %d", r);
			return r;
		}
	}

	brt = (state->color >> 24) & 0xFF;
	brt = brt / 255.f * max;

	r = sys_set_int(CAMERA_BACK_PATH"/brightness", brt);
	if (r < 0) {
		_E("fail to set brightness : %d", r);
		return r;
	}

	return 0;
}

static int led_open(struct hw_info *info,
		const char *id, struct hw_common **common)
{
	struct led_device *led_dev;

	if (!info || !id || !common)
		return -EINVAL;

	if (strcmp(id, LED_ID_CAMERA_BACK) != 0)
		return -ENOTSUP;

	led_dev = calloc(1, sizeof(struct led_device));
	if (!led_dev)
		return -ENOMEM;

	led_dev->common.info = info;

	if (!strcmp(id, LED_ID_CAMERA_BACK))
		led_dev->set_state = camera_back_set_state;

	*common = (struct hw_common *)led_dev;
	return 0;
}

static int led_close(struct hw_common *common)
{
	if (!common)
		return -EINVAL;

	free(common);
	return 0;
}

HARDWARE_MODULE_STRUCTURE = {
	.magic = HARDWARE_INFO_TAG,
	.hal_version = HARDWARE_INFO_VERSION,
	.device_version = LED_HARDWARE_DEVICE_VERSION,
	.id = LED_HARDWARE_DEVICE_ID,
	.name = "Default LED",
	.author = "Jiyoung Yun <jy910.yun@samsung.com>",
	.open = led_open,
	.close = led_close,
};
