################# Build library ################
lib_LTLIBRARIES = libSLP-lbs-plugin-replay.la

libSLP_lbs_plugin_replay_la_SOURCES = \
						$(srcdir)/src/nmea_parser.c \
						$(srcdir)/src/gps_plugin_replay.c \
						$(srcdir)/src/setting.c

libSLP_lbs_plugin_replay_la_CFLAGS = -Wall -fPIC -fvisibility=hidden -DEXPORT_API="__attribute__((visibility(\"default\")))" \
						-I$(srcdir)/include \
						$(GPS_MANAGER_PLUGIN_CFLAGS) \
						$(VCONF_CFLAGS) \
						$(DLOG_CFLAGS) \
						$(DEVICED_CFLAGS)

libSLP_lbs_plugin_replay_la_LDFLAGS = -Wl,--hash-style=both -Wl,--as-needed \
						$(VCONF_LIBS) \
						$(DLOG_LIBS) \
						$(DEVICED_LIBS)
