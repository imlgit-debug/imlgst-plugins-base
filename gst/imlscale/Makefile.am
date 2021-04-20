plugin_LTLIBRARIES = libgstimlscale.la

libgstvideoscale_la_SOURCES = gstimlscale.c 

libgstvideoscale_la_CFLAGS = $(GST_PLUGINS_BASE_CFLAGS) $(GST_BASE_CFLAGS) $(GST_CFLAGS)
libgstvideoscale_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstvideoscale_la_LIBADD = \
	$(top_builddir)/gst-libs/gst/video/libgstimlscale-$(GST_API_VERSION).la \
	$(GST_BASE_LIBS) $(GST_LIBS) $(LIBM)

noinst_HEADERS = \
	gstimlscale.h
