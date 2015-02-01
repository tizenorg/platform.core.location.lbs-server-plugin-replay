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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <errno.h>
#include <sys/time.h>

#include <geoclue_plugin_intf.h>

#include "geoclue_plugin_debug.h"


int geoclue_plugin_test_load(void);
int geoclue_plugin_test_unload(void);
int geoclue_plugin_test_location(unsigned long period, LocationCallback cb, void *arg, void **handle);
int geoclue_plugin_test_cancel(void *handle, CancelCallback cb, void *arg);
void geoclue_plugin_test_get_offline_token(const unsigned char *key,
			unsigned int keyLengh,
			OfflineTokenCallback cb,
			void *arg);
int geoclue_plugin_test_offline_location(const unsigned char *key,
			unsigned int keyLength,
			const unsigned char *token,
			unsigned int tokenSize,
			LocationCallback cb,
			void *arg);

static const geoclue_plugin_interface g_geoclue_plugin_interface_test_interface = {
	geoclue_plugin_test_load,
	geoclue_plugin_test_unload,
	geoclue_plugin_test_location,
	geoclue_plugin_test_cancel,
	geoclue_plugin_test_get_offline_token,
	geoclue_plugin_test_offline_location
};

typedef struct {
	int index;						// used for handle
	plugin_LocationInfo *location;
	unsigned long period;
	LocationCallback location_cb;	// save from location
	void *arg;						// save from location
} GeoclueXpsPluginTest;

static GeoclueXpsPluginTest *xps_plugint_test = NULL;

static gboolean update_fake_position(gpointer data)
{
	GeoclueXpsPluginTest *xps_plugin = data;

	if (xps_plugin) {
		if (xps_plugin->location) {
			if (xps_plugin->location->latitude < 90) {
				xps_plugin->location->latitude++;
			} else {
				xps_plugin->location->latitude = 0;
			}
			if (xps_plugin->location->longitude< 180) {
				xps_plugin->location->longitude++;
			} else {
				xps_plugin->location->longitude = 0;
			}
			if (xps_plugin->location->age < 10000) {
				xps_plugin->location->age++;
			} else {
				xps_plugin->location->age = 0;
			}
			if (xps_plugin->location->altitude < 5000) {
				xps_plugin->location->altitude++;
			} else {
				xps_plugin->location->altitude = 0;
			}
			if (xps_plugin->location->bearing < 90) {
				xps_plugin->location->bearing++;
			} else {
				xps_plugin->location->bearing = 0;
			}
			if (xps_plugin->location->hpe < 100) {
				xps_plugin->location->hpe++;
			} else {
				xps_plugin->location->hpe = 0;
			}
			if (xps_plugin->location->speed < 250) {
				xps_plugin->location->speed++;
			} else {
				xps_plugin->location->speed = 0;
			}
		}

		// called intervals
		if (xps_plugin->location_cb) {
			xps_plugin->location_cb(xps_plugin->arg, xps_plugin->location, NULL);
		}
	}

	return TRUE;
}

int geoclue_plugin_test_load(void)
{
	LOG_PLUGIN(LOG_DEBUG, "geoclue_plugin_test_load called");


	// create plugin_LocationInfo *location
	if (NULL == xps_plugint_test) {
		xps_plugint_test = g_new0(GeoclueXpsPluginTest, 1);
		if (NULL == xps_plugint_test) {
			LOG_PLUGIN(LOG_ERROR, "[ERROR] GeoclueXpsPluginTest create fail");
			return FALSE;
		} else {
			xps_plugint_test->index = 0;
			xps_plugint_test->period = 5000;	// 5s
			xps_plugint_test->location = g_new0(plugin_LocationInfo, 1);
			if (NULL == xps_plugint_test->location) {
				LOG_PLUGIN(LOG_ERROR, "[ERROR] plugin_LocationInfo create fail");
				g_free(xps_plugint_test);
				return FALSE;
			}
			xps_plugint_test->location->latitude = 10;
			xps_plugint_test->location->longitude = 10;
			xps_plugint_test->location->hpe = 10;
			xps_plugint_test->location->altitude= 10;
			xps_plugint_test->location->age = 10;
			xps_plugint_test->location->speed = 10;
			xps_plugint_test->location->bearing = 10;
		}
	}

	// create the timer
	//g_timeout_add (xps_plugint_test->period, update_fake_position, xps_plugint_test);
	g_timeout_add (5000, update_fake_position, xps_plugint_test);

	return TRUE;
}
int geoclue_plugin_test_unload(void)
{
	LOG_PLUGIN(LOG_DEBUG, "geoclue_plugin_test_unload called");

	// free plugin_LocationInfo *location
	if (xps_plugint_test) {
		if (xps_plugint_test->location) {
			g_free(xps_plugint_test->location);
			xps_plugint_test->location = NULL;
		}
		g_free(xps_plugint_test);
		xps_plugint_test = NULL;
	}

	// kill the timer
	return TRUE;
}
int geoclue_plugin_test_location(unsigned long period, LocationCallback cb, void *arg, void **handle)
{
	LOG_PLUGIN(LOG_DEBUG, "geoclue_plugin_test_location called");

	// update the plugin_LocationInfo *location in the timer

	// update handle
	if (xps_plugint_test) {
		LOG_PLUGIN(LOG_DEBUG, "geoclue_plugin_test_location: before set handle");
		xps_plugint_test->index++;
		gchar *tmp = g_strdup_printf("%d", xps_plugint_test->index);
		*handle = (void *)tmp;
		LOG_PLUGIN(LOG_DEBUG, "geoclue_plugin_test_location: after set handle, set[%s], handle[%s]", tmp, *handle);

		// call LocationCallback
		if (cb) {
			cb(arg, xps_plugint_test->location, NULL);
			xps_plugint_test->location_cb = cb;
			xps_plugint_test->arg = arg;
		}

		LOG_PLUGIN(LOG_DEBUG, "geoclue_plugin_test_location after call callback");
	}

	return TRUE;		// to test online
	//return FALSE;		// to test the offline
}
int geoclue_plugin_test_cancel(void *handle, CancelCallback cb, void *arg)
{
	LOG_PLUGIN(LOG_DEBUG, "geoclue_plugin_test_cancel called");
	// check handle
	if (handle) {
		LOG_PLUGIN(LOG_DEBUG, "canel handle %s", handle);
		g_free(handle);
		handle = NULL;
	}

	// call CancelCallback
	if (cb) {
		cb(arg);
	}
	return TRUE;
}

void geoclue_plugin_test_get_offline_token(const unsigned char *key,
			unsigned int keyLengh,
			OfflineTokenCallback cb,
			void *arg)
{
	LOG_PLUGIN(LOG_DEBUG, "geoclue_plugin_test_get_offline_token called");

	unsigned char *key_copied = NULL;
	if (key && keyLengh > 0) {
		key_copied = g_memdup(key, keyLengh);
		key_copied[keyLengh - 1] = '\0';
		if (key_copied) {
			LOG_PLUGIN(LOG_DEBUG, "key_copied [%s]", key_copied);

			// call OfflineTokenCallback
			if (cb) {
				char *token = g_strdup("samsung_token");
				LOG_PLUGIN(LOG_DEBUG, "geoclue_plugin_test_get_offline_token: before callback");
				cb(arg, token, strlen(token));
				LOG_PLUGIN(LOG_DEBUG, "geoclue_plugin_test_get_offline_token: after callback");
			}
		} else {
			LOG_PLUGIN(LOG_ERROR, "[ERROR] key copy fail");
		}
	} else {
		LOG_PLUGIN(LOG_ERROR, "[ERROR] key or keyLengh parameter error");
	}

}
int geoclue_plugin_test_offline_location(const unsigned char *key,
			unsigned int keyLength,
			const unsigned char *token,
			unsigned int tokenSize,
			LocationCallback cb,
			void *arg)
{
	LOG_PLUGIN(LOG_DEBUG, "geoclue_plugin_test_offline_location called");

	if (cb) {
		if (xps_plugint_test) {
			LOG_PLUGIN(LOG_DEBUG, "geoclue_plugin_test_offline_location: before callback");
			cb(arg, xps_plugint_test->location, NULL);
			LOG_PLUGIN(LOG_DEBUG, "geoclue_plugin_test_offline_location: before callback");
			xps_plugint_test->location_cb = cb;
			xps_plugint_test->arg = arg;
		}
	}
	return TRUE;
}

EXPORT_API const geoclue_plugin_interface *get_geoclue_plugin_interface()
{
	LOG_PLUGIN(LOG_DEBUG, "get_geoclue_plugin_interface called");
	return &g_geoclue_plugin_interface_test_interface;
}
