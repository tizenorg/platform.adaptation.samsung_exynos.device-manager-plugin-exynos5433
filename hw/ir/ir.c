/*
 * device-node
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
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
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <linux/limits.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <fcntl.h>

#include <hw/ir.h>
#include "../shared.h"

#define IRLED_CONTROL_PATH "/dev/lirc0"

#define LIRC_GET_LENGTH		2147772687
#define LIRC_SET_LENGTH		1074030864
#define LIRC_GET_FREQUENCY	2147772708
#define LIRC_SET_FREQUENCY	1074030885
#define LIRC_GET_FEATURES	2147772672

static int fd;

static int ir_is_available(bool *available)
{
	*available = true;
	return 0;
}

static int ir_transmit(int *frequency_pattern, int size)
{
	int i, j, ret;
	uint32_t temp = 0;
	uint32_t len = 0;
	uint32_t freq;
	ssize_t n;

	if (size <= 1)
		return -EINVAL;
	uint8_t pattern[(size - 1) * 4];

	if (!frequency_pattern)
		return -EINVAL;

	fd = open(IRLED_CONTROL_PATH, O_RDWR);
	if (fd < 0) {
		_E("Unable to open the device");
		close(fd);
		return -ENODEV;
	}
	ret = ioctl(fd, LIRC_GET_FREQUENCY, &temp);
	if (ret < 0)
		_E("Failed to get frequency");
	if (temp != frequency_pattern[0]) {
		freq = frequency_pattern[0];
		ret = ioctl(fd, LIRC_SET_FREQUENCY, &freq);
		if (ret < 0) {
			_D("Set frequency failed %d", ret);
			close(fd);
			return ret;
		}
	}

	len = (size - 1) * 4;
	j = 0;
	for (i = 1; i < size; i++) {
		pattern[j] = (uint8_t)(((unsigned int)frequency_pattern[i]) >> 0);
		pattern[++j] = (uint8_t)(((unsigned int)frequency_pattern[i]) >> 8);
		pattern[++j] = (uint8_t)(((unsigned int)frequency_pattern[i]) >> 16);
		pattern[++j] = (uint8_t)(((unsigned int)frequency_pattern[i]) >> 24);
		j++;
	}
	ret = ioctl(fd, LIRC_GET_LENGTH, &temp);
	if (ret < 0)
		_E("Failed to get length");
	if (temp != len)
		ret = ioctl(fd, LIRC_SET_LENGTH, &len);

	n = write(fd, pattern, len);
	ret = ioctl(fd, LIRC_GET_LENGTH, &temp);
	if (n < 0) {
		_E("Unable to write to the device");
		close(fd);
		return ret;
	} else if (n != len) {
		_E("Failed to write everything wrote %d instead", ret);
		close(fd);
		return ret;
	}

	close(fd);
	return 0;
}

static int ir_open(struct hw_info *info,
		const char *id, struct hw_common **common)
{
	struct ir_device *ir_dev;

	if (!info || !common)
		return -EINVAL;

	ir_dev = calloc(1, sizeof(struct ir_device));
	if (!ir_dev)
		return -ENOMEM;

	ir_dev->common.info = info;
	ir_dev->is_available = ir_is_available;
	ir_dev->transmit = ir_transmit;

	*common = (struct hw_common *)ir_dev;

	return 0;
}

static int ir_close(struct hw_common *common)
{
	if (!common)
		return -EINVAL;

	free(common);
	if (fd >= 0)
		close(fd);

	return 0;
}

HARDWARE_MODULE_STRUCTURE = {
	.magic = HARDWARE_INFO_TAG,
	.hal_version = HARDWARE_INFO_VERSION,
	.device_version = IR_HARDWARE_DEVICE_VERSION,
	.id = IR_HARDWARE_DEVICE_ID,
	.name = "ir",
	.open = ir_open,
	.close = ir_close,
};
