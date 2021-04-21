plugin_LTLIBRARIES = libgstimlscale.la

libgstimlscale_la_SOURCES = gstimlscale.c 

libgstimlscale_la_CFLAGS = $(GST_PLUGINS_BASE_CFLAGS) $(GST_BASE_CFLAGS) $(GST_CFLAGS)
libgstimlscale_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstimlscale_la_LIBADD = \
	$(top_builddir)/gst-libs/gst/video/libgstvideo-$(GST_API_VERSION).la \
	$(GST_BASE_LIBS) $(GST_LIBS) $(LIBM)

noinst_HEADERS = \
	gstimlscale.h
