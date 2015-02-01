/*
 * gps replay plugin
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <gps_plugin_intf.h>

#include "gps_plugin_debug.h"
#include "nmea_parser.h"
#include "setting.h"

#define MAX_NMEA_SENTENCES	32
#define MAX_TOEKNS			25

#define DECIMAL_TO_DEGREE	60.0
#define METER_TO_FEET		3.2808399
#define KNOT_TO_MPS			0.5144444
#define KMPH_TO_MPS			0.2777778

#define NORTH		 1
#define SOUTH		-1
#define EAST		 1
#define WEST		-1

int used_sat[MAX_GPS_NUM_SAT_USED] = { 0, };

static char nmea_parser_c2n(char ch)
{
	if (ch <= '9') {
		return ch - '0';
	} else {
		return (ch - 'A') + 10;
	}
}

int nmea_parser_verify_checksum(char *nmea_sen)
{
	int ret = -1;
	int i;
	int checksum = 0;
	int sum = 0;

	for (i = 0; i < strlen(nmea_sen) && (nmea_sen[i] != '*'); i++) {
		checksum ^= nmea_sen[i];
	}

	if (++i + 1 < strlen(nmea_sen)) {
		sum = (nmea_parser_c2n(nmea_sen[i]) << 4) + nmea_parser_c2n(nmea_sen[i + 1]);
	}

	if (sum == checksum) {
		ret = 0;
	} else {
		LOG_PLUGIN(DBG_LOW, "NMEA checksum is INVALID");
		ret = -1;
	}
	return ret;
}

int nmea_parser_tokenize(char input[], char *token[])
{
	char *s = input;
	int num_tokens = 0;
	int state;

	token[num_tokens] = s;
	num_tokens++;
	state = 0;
	LOG_PLUGIN(DBG_LOW, "input:%s \n", input);

	while ((*s != 0) && (num_tokens < MAX_TOEKNS)) {
		switch (state) {
		case 0:
			if (*s == ',') {
				*s = 0;
				state = 1;
			}
			break;
		case 1:
			token[num_tokens++] = s;
			if (*s == ',') {
				*s = 0;
			} else {
				state = 0;
			}
			break;
		}
		s++;
	}
	return num_tokens;
}

static double nmea_parser_get_latitude(const char *lat, const char *bearing)
{
	double latitude = 0.0;
	int ns;
	int deg;
	double remainder;

	if ((*lat == 0) || (*bearing == 0)) {
		return latitude;
	}

	ns = (*bearing == 'N') ? NORTH : SOUTH;

	latitude = atof(lat);
	deg = (int)(latitude / 100.0);
	remainder = latitude - (deg * 100.0);
	latitude = (deg + (remainder / DECIMAL_TO_DEGREE)) * ns;

	return latitude;
}

static double nmea_parser_get_longitude(const char *lon, const char *bearing)
{
	double longitude = 0.0;
	int ew;
	int deg;
	double remainder;

	if (*lon == 0 || (*bearing == 0)) {
		return longitude;
	}

	ew = (*bearing == 'E') ? EAST : WEST;

	longitude = atof(lon);
	deg = (int)(longitude / 100.0);
	remainder = longitude - (deg * 100.0);
	longitude = (deg + (remainder / DECIMAL_TO_DEGREE)) * ew;

	return longitude;
}

static double nmea_parser_get_altitude(const char *alt, const char *unit)
{
	double altitude;

	if (*alt == 0) {
		return 0.0;
	}

	altitude = atof(alt);
	altitude = (*unit == 'M') ? altitude : altitude * METER_TO_FEET;

	return altitude;
}

static int nmea_parser_gpgga(char *token[], pos_data_t * pos, sv_data_t * sv)
{
	double latitude, longitude, altitude;
	int quality;

	quality = atoi(token[6]);

	if (quality == 0) {
		LOG_PLUGIN(DBG_LOW, "Not fixed");
		sv->pos_valid = FALSE;
		return READ_NOT_FIXED;
	}

//	utctime = atoi(token[1]);
	latitude = nmea_parser_get_latitude(token[2], token[3]);
	longitude = nmea_parser_get_longitude(token[4], token[5]);
	altitude = nmea_parser_get_altitude(token[9], token[10]);
//	num_of_sat_used = atoi(token[7]);
//	eph = atof(token[8]);
//	geoid = nmea_parser_get_altitude(token[11], token[12]);

	pos->latitude = latitude;
	pos->longitude = longitude;
	pos->altitude = altitude;

	sv->pos_valid = TRUE;

	return READ_SUCCESS;
}

static int nmea_parser_gprmc(char *token[], pos_data_t * pos)
{
	char *status;
	double latitude, longitude, speed, bearing;

	status = token[2];	//warn = *token[2];
	if (strcmp(status, "V") == 0) {
		LOG_PLUGIN(DBG_LOW, "Not fixed");
		return READ_NOT_FIXED;
	}

//	utctime = atoi(token[1]);
	latitude = nmea_parser_get_latitude(token[3], token[4]);
	longitude = nmea_parser_get_longitude(token[5], token[6]);
	speed = atof(token[7]);
	bearing = atof(token[8]);
//	date = atoi(token[9]);
//	magvar = atof(token[10]);

	pos->latitude = latitude;
	pos->longitude = longitude;
	pos->speed = speed * KNOT_TO_MPS;
	pos->bearing = bearing;

	return READ_SUCCESS;
}

static int nmea_parser_gpgll(char *token[], pos_data_t * pos)
{
	char *status;
	double latitude, longitude;

	status = token[6];	//warn = *token[2];
	if (strcmp(status, "V") == 0) {
		LOG_PLUGIN(DBG_LOW, "Not fixed");
		return READ_NOT_FIXED;
	}

		latitude = nmea_parser_get_latitude(token[1], token[2]);
		longitude = nmea_parser_get_longitude(token[3], token[4]);

		pos->latitude = latitude;
		pos->longitude = longitude;

		return READ_SUCCESS;
}

static int nmea_parser_gpgsa(char *token[], pos_data_t * pos)
{
	int i, fix_type;

	fix_type = atoi(token[2]);
	if (fix_type == 1) {
		LOG_PLUGIN(DBG_LOW, "Not fixed");
		return READ_NOT_FIXED;
	}

//	selection_type = *token[1];

	memset(used_sat, 0, sizeof(used_sat));
	for (i = 0; i < MAX_GPS_NUM_SAT_USED; i++) {
		used_sat[i] = atoi(token[i + 3]);
	}

//	pdop = atof(token[15]);
//	hdop = atof(token[16]);
//	vdop = atof(token[17]);

	return READ_SUCCESS;
}

static int nmea_parser_gpvtg(char *token[], pos_data_t * pos)
{
	double true_course, kmh_speed;

	true_course = atof(token[1]);
//	magnetic_course = atof(token[3]);
//	knot_speed = atof(token[5]);
	kmh_speed = atof(token[7]);

	pos->speed = kmh_speed * KMPH_TO_MPS;
	pos->bearing = true_course;

	return READ_SUCCESS;
}

static int nmea_parser_gpgsv(char *token[], sv_data_t * sv)
{
	int i, j;
	int p, q, iter;
	int msg_num, num_sv;

//	num_sen = atoi(token[1]);
	msg_num = atoi(token[2]);
	if (msg_num < 1) {
		LOG_PLUGIN(DBG_LOW, "There is not GSV message");
		return READ_ERROR;
	}

	num_sv = atoi(token[3]);
	sv->num_of_sat = num_sv;
	iter = ((num_sv < (msg_num * 4)) ? (num_sv - ((msg_num - 1) * 4)) : 4);
	for (i = 0; i < iter; i++) {
		q = (i + 1) * 4;
		p = i + 4 * (msg_num - 1);
		sv->sat[p].prn = atoi(token[q]);
		for (j = 0; j < MAX_GPS_NUM_SAT_USED; j++) {
			if (sv->sat[p].prn == used_sat[j]) {
				sv->sat[p].used = 1;
				break;
			} else {
				sv->sat[p].used = 0;
			}
		}
		sv->sat[p].elevation = atoi(token[q + 1]);
		sv->sat[p].azimuth = atoi(token[q + 2]);
		sv->sat[p].snr = atoi(token[q + 3]);
	}
	return READ_SUCCESS;
}

int nmea_parser_sentence(char *sentence, char *token[], pos_data_t * pos, sv_data_t * sv)
{
	int ret = READ_SUCCESS;
	if (strcmp(sentence, "GPGGA") == 0) {
		ret = nmea_parser_gpgga(token, pos, sv);
	} else if (strcmp(sentence, "GPRMC") == 0) {
		ret = nmea_parser_gprmc(token, pos);
	} else if (strcmp(sentence, "GPGLL") == 0) {
		ret = nmea_parser_gpgll(token, pos);
	} else if (strcmp(sentence, "GPGSA") == 0) {
		ret = nmea_parser_gpgsa(token, pos);
	} else if (strcmp(sentence, "GPVTG") == 0) {
		ret = nmea_parser_gpvtg(token, pos);
	} else if (strcmp(sentence, "GPGSV") == 0) {
		ret = nmea_parser_gpgsv(token, sv);
	} else {
		LOG_PLUGIN(DBG_LOW, "Unsupported sentence : [%s]\n", sentence);
	}

	return ret;
}

int nmea_parser(char *data, pos_data_t * pos, sv_data_t * sv)
{
	int ret = READ_SUCCESS;
	read_error_t err;
	int num_sen = 0;
	int count = 0;
	char *last = NULL;
	char *nmea_sen[MAX_NMEA_SENTENCES] = { 0, };
	char *token[MAX_TOEKNS] = { 0, };

	nmea_sen[num_sen] = (char *)strtok_r((char *)data, "$", &last);
	while (nmea_sen[num_sen] != NULL) {
		num_sen++;
		nmea_sen[num_sen] = (char *)strtok_r(NULL, "$", &last);
	}
	LOG_PLUGIN(DBG_LOW, "Number of NMEA sentences:%d \n", num_sen);

	while (num_sen > 0) {
		if (nmea_parser_verify_checksum(nmea_sen[count]) == 0) {
			nmea_parser_tokenize(nmea_sen[count], token);
			err = nmea_parser_sentence(token[0], token, pos, sv);
			if (err == READ_NOT_FIXED) {
				LOG_PLUGIN(DBG_LOW, "NOT Fixed");
				ret = err;
			} else if (err == READ_ERROR) {
				ret = err;
			}
		} else {
			LOG_PLUGIN(DBG_ERR, "[NMEA Parser] %dth sentense : Invalid Checksum\n", count);
		}
		count++;
		num_sen--;
	}

	return ret;
}
