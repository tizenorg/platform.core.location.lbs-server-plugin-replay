/*
 * gps-manager replay plugin
 *
 * Copyright (c) 2011-2013 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Youngae Kang <youngae.kang@samsung.com>, Minjune Kim <sena06.kim@samsung.com>
 *			Genie Kim <daejins.kim@samsung.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _SETTING_H_
#define _SETTING_H_

#include <vconf.h>
#include <vconf-internal-location-keys.h>

#define NMEA_FILE_PATH "/opt/usr/media/lbs-server/replay/"
#define DEFAULT_NMEA_LOG "/etc/lbs-server/replay/nmea_replay.log"

typedef enum {
	REPLAY_OFF = 0,
	REPLAY_NMEA,
	REPLAY_MANUAL,
	REPLAY_MODE_MAX
} replay_mode_t;

typedef enum {
	BATCH_MODE_OFF = 0,
	BATCH_MODE_ON
} batch_mode_t;

typedef enum {
	READ_ERROR		= -1,
	READ_SUCCESS	= 0,
	READ_NOT_FIXED	= 1,
} read_error_t;

int setting_get_int(const char *path, int *val);
int setting_get_double(const char *path, double *val);
char *setting_get_string(const char *path);

typedef void (*key_changed_cb)(keynode_t *key, void *data);

int setting_notify_key_changed(const char *path, void *key_changed_cb);
int setting_ignore_key_changed(const char *path, void *key_changed_cb);
#endif				/* _SETTING_H_ */
