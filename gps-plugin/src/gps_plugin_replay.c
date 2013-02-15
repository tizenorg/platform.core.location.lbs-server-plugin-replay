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

#include <gps_manager_plugin_intf.h>

#include "gps_plugin_debug.h"
#include "nmea_parser.h"
#include "setting.h"

#define REPLAY_NMEA_SET_SIZE		4096
#define REPLAY_NMEA_SENTENCE_SIZE	128

gps_event_cb g_gps_event_cb = NULL;

typedef struct {
	FILE *fd;
	int interval;
	int replay_mode;

	pos_data_t *pos_data;
	sv_data_t *sv_data;
	nmea_data_t *nmea_data;

	GSource *timeout_src;
	GMainContext *default_context;
} replay_timeout;

replay_timeout *g_replay_timer = NULL;

int gps_plugin_replay_gps_init(gps_event_cb gps_event_cb, gps_server_param_t * gps_params);
int gps_plugin_replay_gps_deinit(gps_failure_reason_t * reason_code);
int gps_plugin_replay_gps_request(gps_action_t gps_action, void *data, gps_failure_reason_t * reason_code);

static const gps_plugin_interface g_gps_plugin_replay_interface = {
	gps_plugin_replay_gps_init,
	gps_plugin_replay_gps_deinit,
	gps_plugin_replay_gps_request
};

void gps_plugin_replay_pos_event(pos_data_t * data)
{
	gps_event_info_t gps_event;
	time_t timestamp;

	memset(&gps_event, 0, sizeof(gps_event_info_t));
	time(&timestamp);

	gps_event.event_id = GPS_EVENT_REPORT_POSITION;

	if (data == NULL) {
		LOG_PLUGIN(DBG_ERR, "NULL POS data.");
		gps_event.event_data.pos_ind.error = GPS_ERR_COMMUNICATION;
	} else {
		gps_event.event_data.pos_ind.error = GPS_ERR_NONE;
		gps_event.event_data.pos_ind.pos.timestamp = timestamp;
		gps_event.event_data.pos_ind.pos.latitude = data->latitude;
		gps_event.event_data.pos_ind.pos.longitude = data->longitude;
		gps_event.event_data.pos_ind.pos.altitude = data->altitude;
		gps_event.event_data.pos_ind.pos.speed = data->speed;
		gps_event.event_data.pos_ind.pos.bearing = data->bearing;
		gps_event.event_data.pos_ind.pos.hor_accuracy = data->hor_accuracy;
		gps_event.event_data.pos_ind.pos.ver_accuracy = data->ver_accuracy;
	}

	//LOG_PLUGIN(DBG_LOW, "%d / %f / %f / %f", gps_event.event_data.pos_ind.pos.timestamp, gps_event.event_data.pos_ind.pos.latitude, gps_event.event_data.pos_ind.pos.longitude, gps_event.event_data.pos_ind.pos.altitude);

	if (g_gps_event_cb != NULL) {
		g_gps_event_cb(&gps_event);
	}
}

void gps_plugin_replay_sv_event(sv_data_t * data)
{
	int i;
	gps_event_info_t gps_event;
	time_t timestamp;

	memset(&gps_event, 0, sizeof(gps_event_info_t));
	time(&timestamp);
	gps_event.event_id = GPS_EVENT_REPORT_SATELLITE;

	if (data == NULL) {
		LOG_PLUGIN(DBG_ERR, "NULL SV data.");
		gps_event.event_data.sv_ind.error = GPS_ERR_COMMUNICATION;
	} else {
		gps_event.event_data.sv_ind.error = GPS_ERR_NONE;
		gps_event.event_data.sv_ind.sv.timestamp = timestamp;
		gps_event.event_data.sv_ind.sv.pos_valid = data->pos_valid;
		gps_event.event_data.sv_ind.sv.num_of_sat = data->num_of_sat;
		for (i = 0; i < data->num_of_sat; i++) {
			gps_event.event_data.sv_ind.sv.sat[i].used = data->sat[i].used;
			gps_event.event_data.sv_ind.sv.sat[i].prn = data->sat[i].prn;
			gps_event.event_data.sv_ind.sv.sat[i].snr = data->sat[i].snr;
			gps_event.event_data.sv_ind.sv.sat[i].elevation = data->sat[i].elevation;
			gps_event.event_data.sv_ind.sv.sat[i].azimuth = data->sat[i].azimuth;
		}
	}

	if (g_gps_event_cb != NULL) {
		g_gps_event_cb(&gps_event);
	}
}

void gps_plugin_replay_nmea_event(nmea_data_t * data)
{
	gps_event_info_t gps_event;
	time_t timestamp;

	memset(&gps_event, 0, sizeof(gps_event_info_t));
	time(&timestamp);

	gps_event.event_id = GPS_EVENT_REPORT_NMEA;

	if (data == NULL) {
		LOG_PLUGIN(DBG_ERR, "NULL NMEA data.");
		gps_event.event_data.nmea_ind.error = GPS_ERR_COMMUNICATION;
	} else {
		if (data->len > REPLAY_NMEA_SENTENCE_SIZE) {
			LOG_PLUGIN(DBG_WARN, "The Size of NMEA[ %d ] is larger then max ", data->len);
			data->len = REPLAY_NMEA_SENTENCE_SIZE;
			gps_event.event_data.nmea_ind.error = GPS_ERR_COMMUNICATION;
		} else {
			gps_event.event_data.nmea_ind.error = GPS_ERR_NONE;
		}
		gps_event.event_data.nmea_ind.nmea.timestamp = timestamp;
		gps_event.event_data.nmea_ind.nmea.len = data->len;
		gps_event.event_data.nmea_ind.nmea.data = (char *)malloc(data->len);
		memset(gps_event.event_data.nmea_ind.nmea.data, 0x00, data->len);
		memcpy(gps_event.event_data.nmea_ind.nmea.data, data->data, data->len);
		//LOG_PLUGIN(DBG_LOW, "NMEA[%d] : %s", gps_event.event_data.nmea_ind.nmea.len, gps_event.event_data.nmea_ind.nmea.data);
	}

	if (g_gps_event_cb != NULL) {
		g_gps_event_cb(&gps_event);
	}

	if (gps_event.event_data.nmea_ind.nmea.data != NULL) {
		free(gps_event.event_data.nmea_ind.nmea.data);
		gps_event.event_data.nmea_ind.nmea.data = NULL;
	}
}

void gps_plugin_respond_start_session(gboolean ret)
{
	gps_event_info_t gps_event;
	gps_event.event_id = GPS_EVENT_START_SESSION;

	if (ret == TRUE) {
		gps_event.event_data.start_session_rsp.error = GPS_ERR_NONE;
	} else {
		gps_event.event_data.start_session_rsp.error = GPS_ERR_COMMUNICATION;
	}

	if (g_gps_event_cb != NULL) {
		g_gps_event_cb(&gps_event);
	}
}

void gps_plugin_respond_stop_session(void)
{
	gps_event_info_t gps_event;

	gps_event.event_id = GPS_EVENT_STOP_SESSION;
	gps_event.event_data.stop_session_rsp.error = GPS_ERR_NONE;

	if (g_gps_event_cb != NULL) {
		g_gps_event_cb(&gps_event);
	}
}

gboolean gps_plugin_replay_read_nmea(replay_timeout * timer, char *nmea_data)
{
	gboolean ret = FALSE;
	int ref = 0;
	char buf[REPLAY_NMEA_SENTENCE_SIZE] = { 0, };

	if (timer->fd == NULL) {
		LOG_PLUGIN(DBG_ERR, "nmea fd is NULL");
		return FALSE;
	}

	if (nmea_data == NULL) {
		LOG_PLUGIN(DBG_ERR, "nmea_data is NULL");
		fclose(timer->fd);
		timer->fd = NULL;
		return FALSE;
	}

	while (fgets(buf, REPLAY_NMEA_SENTENCE_SIZE, timer->fd) != NULL) {
		if (strncmp(buf, "$GPGGA", 6) == 0) {
			ref++;
			if (ref > 1) {
				fseek(timer->fd, -strlen(buf), SEEK_CUR);
				LOG_PLUGIN(DBG_LOW, "2nd GPGGA : stop to read nmea data");
				ret = TRUE;
				break;
			} else if (ref == 1) {
				LOG_PLUGIN(DBG_LOW, "1st GPGGA : start to read nmea data");
				strncpy(nmea_data, buf, strlen(buf));
			}
		} else {
			if (strlen(nmea_data) + strlen(buf) > REPLAY_NMEA_SET_SIZE) {
				LOG_PLUGIN(DBG_ERR, "read nmea data size is too long");
				break;
			} else {
				strncat(nmea_data, buf, strlen(buf));
			}
		}
		timer->nmea_data->len = strlen(buf);
		timer->nmea_data->data = buf;
		gps_plugin_replay_nmea_event(timer->nmea_data);
	}

	if (feof(timer->fd)) {
		LOG_PLUGIN(DBG_ERR, "end of file");
		rewind(timer->fd);
		ret = TRUE;
	} else {
		LOG_PLUGIN(DBG_LOW, "read nmea data [%s]", nmea_data);
	}
	return ret;
}

gboolean gps_plugin_replay_read_manual(pos_data_t * pos_data)
{
	gboolean ret = TRUE;

	if (setting_get_double(VCONFKEY_LOCATION_MANUAL_LATITUDE, &pos_data->latitude) == FALSE) {
		LOG_PLUGIN(DBG_ERR, "Fail to get latitude");
		ret = FALSE;
	}
	if (setting_get_double(VCONFKEY_LOCATION_MANUAL_LONGITUDE, &pos_data->longitude) == FALSE) {
		LOG_PLUGIN(DBG_ERR, "Fail to get longitude");
		ret = FALSE;
	}
	if (setting_get_double(VCONFKEY_LOCATION_MANUAL_ALTITUDE, &pos_data->altitude) == FALSE) {
		LOG_PLUGIN(DBG_ERR, "Fail to get altitude");
		ret = FALSE;
	}

	return ret;
}

gboolean gps_plugin_replay_timeout_cb(gpointer data)
{
	gboolean ret = FALSE;
	read_error_t err = READ_SUCCESS;
	char nmea_data[REPLAY_NMEA_SET_SIZE] = { 0, };
	replay_timeout *timer = (replay_timeout *) data;

	if (timer == NULL) {
		LOG_PLUGIN(DBG_ERR, "replay handel[timer] is NULL");
		return FALSE;
	}

	memset(timer->pos_data, 0, sizeof(pos_data_t));
	memset(timer->sv_data, 0, sizeof(sv_data_t));

	if (timer->replay_mode == REPLAY_NMEA) {
		if (gps_plugin_replay_read_nmea(timer, nmea_data) == FALSE) {
			LOG_PLUGIN(DBG_ERR, "Fail to read nmea data from file");
			return FALSE;
		} else {
			err = nmea_parser(nmea_data, timer->pos_data, timer->sv_data);
			if (err == READ_ERROR) {
				LOG_PLUGIN(DBG_ERR, "Fail to parser nmea data from file");
				return FALSE;
			} else if (err == READ_NOT_FIXED) {
				LOG_PLUGIN(DBG_LOW, "GPS position is not fixed");
				timer->sv_data->pos_valid = FALSE;
			}
		}
	} else if (timer->replay_mode == REPLAY_MANUAL) {
		if (gps_plugin_replay_read_manual(timer->pos_data) == FALSE) {
			LOG_PLUGIN(DBG_ERR, "Fail to read manual data");
			err = READ_ERROR;
			return FALSE;
		} else {
			timer->sv_data->pos_valid = TRUE;
			err = READ_SUCCESS;
		}
	} else if (timer->replay_mode == REPLAY_OFF) {
		LOG_PLUGIN(DBG_WARN, "replay_mode is OFF");
		err = READ_NOT_FIXED;
		timer->sv_data->pos_valid = FALSE;
	}

	if (g_gps_event_cb != NULL) {
		if (err != READ_NOT_FIXED) {
			gps_plugin_replay_pos_event(timer->pos_data);
		}
		gps_plugin_replay_sv_event(timer->sv_data);
	}
	ret = TRUE;
	return ret;
}

void gps_plugin_stop_replay_mode(replay_timeout * timer)
{
	if (timer->replay_mode == REPLAY_NMEA && fclose(timer->fd) != 0) {
		LOG_PLUGIN(DBG_ERR, "fclose failed");
	}
	timer->fd = NULL;

	if (timer->timeout_src != NULL && timer->default_context != NULL && !g_source_is_destroyed(timer->timeout_src)) {
		if (timer->default_context == g_source_get_context(timer->timeout_src)) {
			g_source_destroy(timer->timeout_src);
			LOG_PLUGIN(DBG_LOW, "g_source_destroy timeout_src");
		} else {
			LOG_PLUGIN(DBG_WARN, "timer->timeout_src is attatched to 0x%x (actual 0x%x)",
				   g_source_get_context(timer->timeout_src), timer->default_context);
		}
		timer->timeout_src = NULL;
		timer->default_context = NULL;
	} else {
		LOG_PLUGIN(DBG_WARN, "timeout_src or default_context is NULL or timeout_src is already destroyed");
	}
	gps_plugin_respond_stop_session();
}

gboolean gps_plugin_get_nmea_fd(replay_timeout * timer)
{
	char replay_file_path[256];

	snprintf(replay_file_path, sizeof(replay_file_path), NMEA_FILE_PATH"%s", setting_get_string(VCONFKEY_LOCATION_NMEA_FILE_NAME));
	LOG_PLUGIN(DBG_ERR, "replay file name : %s", replay_file_path);

	timer->fd = fopen(replay_file_path, "r");
	if (timer->fd == NULL) {
		LOG_PLUGIN(DBG_ERR, "fopen(%s) failed", replay_file_path);
		timer->fd = fopen(DEFAULT_NMEA_LOG, "r");
		if (timer->fd == NULL) {
			LOG_PLUGIN(DBG_ERR, "fopen(%s) failed", DEFAULT_NMEA_LOG);
			return FALSE;
		}
	}
	return TRUE;
}

gboolean gps_plugin_start_replay_mode(replay_timeout * timer)
{
	gboolean ret = FALSE;

	if (timer->replay_mode == REPLAY_NMEA) {
		if (gps_plugin_get_nmea_fd(timer) == FALSE) {
			return FALSE;
		}
	}

	if (timer->default_context == NULL) {
		timer->default_context = g_main_context_default();
		if (timer->default_context == NULL) {
			return ret;
		}
	}

	if (timer->timeout_src != NULL) {
		LOG_PLUGIN(DBG_ERR, "timeout_src is already existed");
		ret = FALSE;
	} else {
		timer->timeout_src = g_timeout_source_new_seconds(timer->interval);
		if (timer->timeout_src != NULL) {
			g_source_set_callback(timer->timeout_src, &gps_plugin_replay_timeout_cb, timer, NULL);
			if (g_source_attach(timer->timeout_src, timer->default_context) > 0) {
				LOG_PLUGIN(DBG_LOW, "timeout_src(0x%x) is created & attatched to 0x%x", timer->timeout_src,
					   timer->default_context);
				ret = TRUE;
			} else {
				gps_plugin_stop_replay_mode(timer);
				ret = FALSE;
			}
		}
	}
	gps_plugin_respond_start_session(ret);

	return ret;
}

static void replay_mode_changed_cb(keynode_t * key, void *data)
{
	if (setting_get_int(VCONFKEY_LOCATION_REPLAY_MODE, &g_replay_timer->replay_mode) == FALSE) {
		g_replay_timer->replay_mode = REPLAY_OFF;
	}

	if (g_replay_timer->replay_mode == REPLAY_NMEA) {
		if (gps_plugin_get_nmea_fd(g_replay_timer) == FALSE) {
			LOG_PLUGIN(DBG_ERR, "Fail to get nmea fd.");
		}
	} else {
		if (g_replay_timer->fd != NULL) {
			fclose(g_replay_timer->fd);
			g_replay_timer->fd = NULL;
		}
	}
	return;
}

replay_timeout *gps_plugin_replay_timer_init()
{
	replay_timeout *timer = NULL;

	timer = (replay_timeout *) malloc(sizeof(replay_timeout));
	if (timer == NULL) {
		LOG_PLUGIN(DBG_ERR, "replay_timeout allocation is failed.");
		return NULL;
	}

	timer->fd = NULL;
	timer->interval = 1;
	if (setting_get_int(VCONFKEY_LOCATION_REPLAY_MODE, &timer->replay_mode) == FALSE) {
		timer->replay_mode = REPLAY_OFF;
	}
	setting_notify_key_changed(VCONFKEY_LOCATION_REPLAY_MODE, replay_mode_changed_cb);

	timer->pos_data = (pos_data_t *) malloc(sizeof(pos_data_t));
	if (timer->pos_data == NULL) {
		LOG_PLUGIN(DBG_ERR, "pos_data allocation is failed.");
		free(timer);
		return NULL;
	}

	timer->sv_data = (sv_data_t *) malloc(sizeof(sv_data_t));
	if (timer->sv_data == NULL) {
		LOG_PLUGIN(DBG_ERR, "sv_data allocation is failed.");
		free(timer->pos_data);
		free(timer);
		return NULL;
	}

	timer->nmea_data = (nmea_data_t *) malloc(sizeof(nmea_data_t));
	if (timer->nmea_data == NULL) {
		LOG_PLUGIN(DBG_ERR, "nmea_data allocation is failed.");
		free(timer->pos_data);
		free(timer->sv_data);
		free(timer);
		return NULL;
	}

	timer->timeout_src = NULL;
	timer->default_context = NULL;

	return timer;
}

void gps_plugin_replay_timer_deinit(replay_timeout * timer)
{
	if (timer == NULL) {
		return;
	}

	if (timer->pos_data != NULL) {
		free(timer->pos_data);
		timer->pos_data = NULL;
	}
	if (timer->sv_data != NULL) {
		free(timer->sv_data);
		timer->sv_data = NULL;
	}
	if (timer->nmea_data != NULL) {
		free(timer->nmea_data);
		timer->nmea_data = NULL;
	}

	setting_ignore_key_changed(VCONFKEY_LOCATION_REPLAY_MODE, replay_mode_changed_cb);

	free(timer);
	timer = NULL;
}

int gps_plugin_replay_gps_init(gps_event_cb gps_event_cb, gps_server_param_t * gps_params)
{
	g_gps_event_cb = gps_event_cb;
	g_replay_timer = gps_plugin_replay_timer_init();

	return TRUE;
}

int gps_plugin_replay_gps_deinit(gps_failure_reason_t * reason_code)
{
	gps_plugin_replay_timer_deinit(g_replay_timer);

	return TRUE;
}

int gps_plugin_replay_gps_request(gps_action_t gps_action, void *data, gps_failure_reason_t * reason_code)
{
	switch (gps_action) {
	case GPS_ACTION_SEND_PARAMS:
		break;
	case GPS_ACTION_START_SESSION:
		gps_plugin_start_replay_mode(g_replay_timer);
		break;
	case GPS_ACTION_STOP_SESSION:
		gps_plugin_stop_replay_mode(g_replay_timer);
		break;
	case GPS_INDI_SUPL_VERIFICATION:
	case GPS_INDI_SUPL_DNSQUERY:
	case GPS_ACTION_START_FACTTEST:
	case GPS_ACTION_STOP_FACTTEST:
	case GPS_ACTION_REQUEST_SUPL_NI:
		LOG_PLUGIN(DBG_LOW, "Don't use action type : [ %d ]", gps_action);
		break;
	default:
		break;
	}

	return TRUE;
}

EXPORT_API const gps_plugin_interface *get_gps_plugin_interface()
{
	return &g_gps_plugin_replay_interface;
}
