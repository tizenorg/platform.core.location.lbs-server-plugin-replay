################# Build library ################
lib_LTLIBRARIES = libSLP-xps-plugin-replay.la

libSLP_xps_plugin_replay_la_SOURCES = \
						$(srcdir)/src/geoclue_xps_plugin_replay.c

libSLP_xps_plugin_replay_la_CFLAGS = -Wall -fPIC -fvisibility=hidden -DEXPORT_API="__attribute__((visibility(\"default\")))" \
						-I$(srcdir)/include \
						$(GEOCLUE_XPS_PLUGIN_CFLAGS) \
						$(VCONF_CFLAGS) \
						$(DLOG_CFLAGS)

libSLP_xps_plugin_replay_la_LDFLAGS = -Wl,--hash-style=both -Wl,--as-needed \
						$(VCONF_LIBS) \
						$(DLOG_LIBS)
