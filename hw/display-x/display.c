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
#include <X11/Xlib.h>
#include <X11/extensions/dpms.h>

#include <hw/display.h>
#include "../shared.h"

#ifndef BACKLIGHT_PATH
#define BACKLIGHT_PATH  "/sys/class/backlight/s6e3ha2"
#endif

static int get_max_brightness(int *val)
{
	static int max = -1;
	int r;

	if (!val)
		return -EINVAL;

	if (max < 0) {
		r = sys_get_int(BACKLIGHT_PATH"/max_brightness", &max);
		if (r < 0)
			return r;
	}

	*val = max;
	return 0;
}

static int display_get_brightness(int *brightness)
{
	int r, v;

	if (!brightness) {
		_E("wrong parameter");
		return -EINVAL;
	}

	r = sys_get_int(BACKLIGHT_PATH"/brightness", &v);
	if (r < 0) {
		_E("fail to get brightness : %s", strerror(r));
		return r;
	}

	*brightness = v;
	return 0;
}

static int display_set_brightness(int brightness)
{
	int r, v, max;

	if (brightness < 0 || brightness > 100) {
		_E("wrong parameter");
		return -EINVAL;
	}

	r = get_max_brightness(&max);
	if (r < 0) {
		_E("fail to get max brightness : %s", strerror(r));
		return r;
	}

	v = brightness/100.f*max;
	r = sys_set_int(BACKLIGHT_PATH"/brightness", v);
	if (r < 0) {
		_E("fail to set brightness : %s", strerror(r));
		return r;
	}

	return 0;
}

static int display_get_power_state(enum display_state *state)
{
	Display *dpy;
	int dummy;
	CARD16 dpms_state = DPMSModeOff;
	BOOL onoff;

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		_E("fail to open display");
		return -EPERM;
	}

	if (DPMSQueryExtension(dpy, &dummy, &dummy)) {
		if (DPMSCapable(dpy)) {
			DPMSInfo(dpy, &dpms_state, &onoff);
		}
	}

	XCloseDisplay(dpy);

	if (state)
		*state = dpms_state;

	return 0;
}

static int display_set_power_state(enum display_state state)
{
	int type;
	Display *dpy;

	if (state == DISPLAY_ON)
		type = DPMSModeOn;
	else if (state == DISPLAY_STANDBY)
		type = DPMSModeStandby;
	else if (state == DISPLAY_SUSPEND)
		type = DPMSModeSuspend;
	else if (state == DISPLAY_OFF)
		type = DPMSModeOff;
	else
		return -EINVAL;

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		_E("fail to open display");
		return -EPERM;
	}

	DPMSEnable(dpy);
	DPMSForceLevel(dpy, type);

	XCloseDisplay(dpy);

	return 0;
}

static int display_open(struct hw_info *info,
		const char *id, struct hw_common **common)
{
	struct display_device *display_dev;

	if (!info || !common)
		return -EINVAL;

	display_dev = calloc(1, sizeof(struct display_device));
	if (!display_dev)
		return -ENOMEM;

	display_dev->common.info = info;
	display_dev->get_brightness = display_get_brightness;
	display_dev->set_brightness = display_set_brightness;
	display_dev->get_state = display_get_power_state;
	display_dev->set_state = display_set_power_state;

	*common = (struct hw_common *)display_dev;
	return 0;
}

static int display_close(struct hw_common *common)
{
	if (!common)
		return -EINVAL;

	free(common);
	return 0;
}

HARDWARE_MODULE_STRUCTURE = {
	.magic = HARDWARE_INFO_TAG,
	.hal_version = HARDWARE_INFO_VERSION,
	.device_version = DISPLAY_HARDWARE_DEVICE_VERSION,
	.id = DISPLAY_HARDWARE_DEVICE_ID,
	.name = "Display",
	.open = display_open,
	.close = display_close,
};
