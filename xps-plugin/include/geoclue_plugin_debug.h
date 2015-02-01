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

#ifndef __GPS_PLUGIN_DEBUG_H__
#define __GPS_PLUGIN_DEBUG_H__

#ifdef __cplusplus
extern "C" {
#endif

#define GPS_DLOG
#define GPS_DLOG_DEBUG		// filename and line number will be shown

#ifdef GPS_DLOG
#include <dlog.h>
#define TAG_GPS_PLUGIN		"xps-plugin"

#define DBG_LOW		LOG_DEBUG
#define DBG_INFO	LOG_INFO
#define DBG_WARN	LOG_WARN
#define DBG_ERR		LOG_ERROR

#ifdef GPS_DLOG_DEBUG		// Debug mode
#define LOG_PLUGIN(dbg_lvl,fmt,args...)	SLOG(dbg_lvl, TAG_GPS_PLUGIN, "[%-40s: %-4d] "fmt, __FILE__, __LINE__, ##args)
#else				// Release(commercial) mode
#define LOG_PLUGIN(dbg_lvl,fmt,args...)	SLOG(dbg_lvl, TAG_GPS_PLUGIN, fmt, ##args)
#endif
#endif

#ifdef __cplusplus
}
#endif
#endif
