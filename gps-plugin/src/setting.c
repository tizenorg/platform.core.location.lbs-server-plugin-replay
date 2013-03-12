/*
 * gps-manager replay plugin
 *
 * Copyright (c) 2011-2013 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Youngae Kang <youngae.kang@samsung.com>, Minjune Kim <sena06.kim@samsung.com>
 *          Genie Kim <daejins.kim@samsung.com>
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

#include <string.h>
#include <glib.h>
#include "setting.h"
#include "gps_plugin_debug.h"

int setting_get_int(const char *path, int *val)
{
	int ret = vconf_get_int(path, val);
	if (ret == 0) {
		ret = TRUE;
	} else {
		LOG_PLUGIN(DBG_ERR, "vconf_get_int failed, [%s]", path);
		ret = FALSE;
	}
	return ret;
}

int setting_get_double(const char *path, double *val)
{
	int ret = vconf_get_dbl(path, val);
	if (ret == 0) {
		ret = TRUE;
	} else {
		LOG_PLUGIN(DBG_ERR, "vconf_get_int failed, [%s]", path);
		ret = FALSE;
	}
	return ret;
}

char *setting_get_string(const char *path)
{
	return vconf_get_str(path);
}

int setting_notify_key_changed(const char *path, void *key_changed_cb)
{
	int ret = TRUE;
	if (vconf_notify_key_changed(path, key_changed_cb, NULL) != 0) {
		LOG_PLUGIN(DBG_ERR, "Fail to vconf_notify_key_changed [%s]", path);
		ret = FALSE;
	}
	return ret;
}

int setting_ignore_key_changed(const char *path, void *key_changed_cb)
{
	int ret = TRUE;
	if (vconf_ignore_key_changed(path, key_changed_cb) != 0) {
		LOG_PLUGIN(DBG_ERR, "Fail to vconf_ignore_key_changed [%s]", path);
		ret = FALSE;
	}
	return ret;
}
