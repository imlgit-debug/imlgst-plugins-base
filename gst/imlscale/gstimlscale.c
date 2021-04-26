/* GStreamer
 * Copyright (C) 2021 FIXME <fixme@example.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Suite 500,
 * Boston, MA 02110-1335, USA.
 */
/**
 * SECTION:element-gstimlscale
 *
 * The imlscale element does FIXME stuff.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 -v fakesrc ! imlscale ! FIXME ! fakesink
 * ]|
 * FIXME Describe what the pipeline does.
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gst/video/gstvideometa.h>
#include <gst/video/gstvideopool.h>

#include "gstimlscale.h"

GST_DEBUG_CATEGORY_STATIC (gst_imlscale_debug_category);
#define GST_CAT_DEFAULT gst_imlscale_debug_category

/* prototypes */


static void gst_imlscale_set_property (GObject * object,
    guint property_id, const GValue * value, GParamSpec * pspec);
static void gst_imlscale_get_property (GObject * object,
    guint property_id, GValue * value, GParamSpec * pspec);
static void gst_imlscale_dispose (GObject * object);
static void gst_imlscale_finalize (GObject * object);

static gboolean gst_imlscale_start (GstBaseTransform * trans);
static gboolean gst_imlscale_stop (GstBaseTransform * trans);
static gboolean gst_imlscale_set_info (GstVideoFilter * filter, GstCaps * incaps,
    GstVideoInfo * in_info, GstCaps * outcaps, GstVideoInfo * out_info);
static GstFlowReturn gst_imlscale_transform_frame (GstVideoFilter * filter,
    GstVideoFrame * inframe, GstVideoFrame * outframe);
static GstFlowReturn gst_imlscale_transform_frame_ip (GstVideoFilter * filter,
    GstVideoFrame * frame);

enum
{
  PROP_0
};

/* pad templates */

/* FIXME: add/remove formats you can handle */

#define PROCESSING_CAPS \
  "{ AYUV, ARGB, BGRA, ABGR, RGBA, Y444, xRGB, RGBx, " \
  "xBGR, BGRx, RGB, BGR, Y42B, YUY2, UYVY, YVYU, " \
  "I420, YV12, IYUV, Y41B, NV12, NV21 }"

static GstStaticPadTemplate gst_imlscale_src_template =
    GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE (PROCESSING_CAPS) ";"
        "video/x-raw(ANY)")
    );

static GstStaticPadTemplate gst_imlscale_sink_template =
    GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE (PROCESSING_CAPS) ";"
        "video/x-raw(ANY)")
    );


/* class initialization */
#define GST_VIDEO_FORMATS GST_VIDEO_FORMATS_ALL

G_DEFINE_TYPE_WITH_CODE (GstImlscale, gst_imlscale, GST_TYPE_VIDEO_FILTER,
  GST_DEBUG_CATEGORY_INIT (gst_imlscale_debug_category, "imlscale", 0,
  "debug category for imlscale element"));

static GstStaticCaps gst_iml_scale_format_caps =
    GST_STATIC_CAPS (GST_VIDEO_CAPS_MAKE (GST_VIDEO_FORMATS) ";"
    GST_VIDEO_CAPS_MAKE_WITH_FEATURES ("ANY", GST_VIDEO_FORMATS));


static GstCaps *
gst_iml_scale_get_capslist (void)
{
  static GstCaps *caps = NULL;
  static volatile gsize inited = 0;

  if (g_once_init_enter (&inited)) {
    caps = gst_static_caps_get (&gst_iml_scale_format_caps);
    g_once_init_leave (&inited, 1);
  }
  return caps;
}


static GstPadTemplate *
gst_iml_scale_src_template_factory (void)
{
  return gst_pad_template_new ("src", GST_PAD_SRC, GST_PAD_ALWAYS,
      gst_iml_scale_get_capslist ());
}

static GstPadTemplate *
gst_iml_scale_sink_template_factory (void)
{
  return gst_pad_template_new ("sink", GST_PAD_SINK, GST_PAD_ALWAYS,
      gst_iml_scale_get_capslist ());
}

static GstCaps *
gst_imlscale_transform_caps (GstBaseTransform * trans,
    GstPadDirection direction, GstCaps * caps, GstCaps * filter)
{
    GstImlscale *imlscale = GST_IMLSCALE(trans);
    GstCaps *ret;

    if (filter) 
    {
      ret = gst_caps_intersect_full (filter, caps, GST_CAPS_INTERSECT_FIRST);
    } 
    else 
    {
      ret = gst_caps_ref (caps);
    }
    
    return ret;
}

static void
gst_video_balance_semiplanar_yuv (GstImlscale * scale,
    GstVideoFrame * frame)
{
    gint x, y;
    guint8 *ydata;
    guint8 *uvdata;
    gint ystride, uvstride;
    gint width, height;
    guint8 *tabley = scale->tabley;
    
    width = GST_VIDEO_FRAME_WIDTH (frame);
    height = GST_VIDEO_FRAME_HEIGHT (frame);

    ydata = GST_VIDEO_FRAME_PLANE_DATA (frame, 0);
    ystride = GST_VIDEO_FRAME_PLANE_STRIDE (frame, 0);
    
    for (y = 0; y < height/2; y++) 
    {
        guint8 *yptr;

        yptr = ydata + y * ystride;
        for (x = 0; x < width/2; x++) 
        {
            *yptr = 255;
            yptr++;
        }
    }
}


static void
gst_imlscale_class_init (GstImlscaleClass * klass)
{
  GObjectClass *gobject_class = (GObjectClass *) klass;
  GstElementClass *element_class = (GstElementClass *) klass;
  GstBaseTransformClass *base_transform_class = (GstBaseTransformClass *) klass;
  GstVideoFilterClass *video_filter_class = (GstVideoFilterClass *) klass;

  gobject_class->set_property = gst_imlscale_set_property;
  gobject_class->get_property = gst_imlscale_get_property;
  gobject_class->dispose = gst_imlscale_dispose;
  gobject_class->finalize = gst_imlscale_finalize;
    
  gst_element_class_set_static_metadata (element_class,
     "IML scaler", "Logic/Scaler", "IML Logic Resize video",
     "IML <yielkyu.yang@iml.co.kr>");
   
  /* Setting up pads and setting metadata should be moved to
     base_class_init if you intend to subclass this class. */
    
  gst_element_class_add_static_pad_template (gstelement_class,
      &gst_video_balance_sink_template);
  gst_element_class_add_static_pad_template (gstelement_class,
      &gst_video_balance_src_template);

#if 0
  gst_element_class_add_pad_template (element_class,
      gst_iml_scale_sink_template_factory ());
  gst_element_class_add_pad_template (element_class,
      gst_iml_scale_src_template_factory ());
#endif
    
  //base_transform_class->start = GST_DEBUG_FUNCPTR (gst_imlscale_start);
  //base_transform_class->stop = GST_DEBUG_FUNCPTR (gst_imlscale_stop);
    
  base_transform_class->transform_ip_on_passthrough = FALSE;
  base_transform_class->transform_caps =
      GST_DEBUG_FUNCPTR (gst_imlscale_transform_caps);
    
  video_filter_class->set_info = GST_DEBUG_FUNCPTR (gst_imlscale_set_info);
  //video_filter_class->transform_frame = GST_DEBUG_FUNCPTR (gst_imlscale_transform_frame);
  video_filter_class->transform_frame_ip = GST_DEBUG_FUNCPTR (gst_imlscale_transform_frame_ip);

}

static void
gst_imlscale_init (GstImlscale *imlscale)
{
     
  gint i;
    
  imlscale->tableu[0] = g_new (guint8, 256 * 256 * 2);
  
  for (i = 0; i < 256; i++) 
  {
    imlscale->tableu[i] =
        imlscale->tableu[0] + i * 256 * sizeof (guint8);
    imlscale->tablev[i] =
        imlscale->tableu[0] + 256 * 256 * sizeof (guint8) +
        i * 256 * sizeof (guint8);
  }
}

void
gst_imlscale_set_property (GObject * object, guint property_id,
    const GValue * value, GParamSpec * pspec)
{
  GstImlscale *imlscale = GST_IMLSCALE (object);

  GST_DEBUG_OBJECT (imlscale, "set_property");

  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

void
gst_imlscale_get_property (GObject * object, guint property_id,
    GValue * value, GParamSpec * pspec)
{
  GstImlscale *imlscale = GST_IMLSCALE (object);

  GST_DEBUG_OBJECT (imlscale, "get_property");

  switch (property_id) {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  }
}

void
gst_imlscale_dispose (GObject * object)
{
  GstImlscale *imlscale = GST_IMLSCALE (object);

  GST_DEBUG_OBJECT (imlscale, "dispose");

  /* clean up as possible.  may be called multiple times */

  G_OBJECT_CLASS (gst_imlscale_parent_class)->dispose (object);
}

void
gst_imlscale_finalize (GObject * object)
{
  GstImlscale *imlscale = GST_IMLSCALE (object);

  GST_DEBUG_OBJECT (imlscale, "finalize");

  /* clean up object here */
  g_free (imlscale->tableu[0]);
  G_OBJECT_CLASS (gst_imlscale_parent_class)->finalize (object);
}

static gboolean
gst_imlscale_start (GstBaseTransform * trans)
{
  GstImlscale *imlscale = GST_IMLSCALE (trans);

  GST_DEBUG_OBJECT (imlscale, "start");

  return TRUE;
}

static gboolean
gst_imlscale_stop (GstBaseTransform * trans)
{
  GstImlscale *imlscale = GST_IMLSCALE (trans);

  GST_DEBUG_OBJECT (imlscale, "stop");

  return TRUE;
}

static gboolean
gst_imlscale_set_info (GstVideoFilter * filter, GstCaps * incaps,
    GstVideoInfo * in_info, GstCaps * outcaps, GstVideoInfo * out_info)
{
  GstImlscale *imlscale = GST_IMLSCALE (filter);

  GST_DEBUG_OBJECT (imlscale, "set_info");
    
  imlscale->process = NULL;
    
  switch (GST_VIDEO_INFO_FORMAT (in_info)) 
  {
        case GST_VIDEO_FORMAT_NV12:
        case GST_VIDEO_FORMAT_NV21:
             imlscale->process = gst_video_balance_semiplanar_yuv;
        break;   
  }

  return TRUE;
}

/* transform */
static GstFlowReturn
gst_imlscale_transform_frame (GstVideoFilter * filter, GstVideoFrame * inframe,
    GstVideoFrame * outframe)
{
  GstImlscale *imlscale = GST_IMLSCALE (filter);

  GST_DEBUG_OBJECT (imlscale, "transform_frame");

  return GST_FLOW_OK;
}

static GstFlowReturn
gst_imlscale_transform_frame_ip (GstVideoFilter * filter, GstVideoFrame * frame)
{
  gint h,w;
  guint8 *src;
    
  GstImlscale *imlscale = GST_IMLSCALE (filter);

  GST_DEBUG_OBJECT (imlscale, "transform_frame_ip");
    
  GST_OBJECT_LOCK (imlscale);
  imlscale->process (imlscale, frame);
  GST_OBJECT_UNLOCK (imlscale);

  return GST_FLOW_OK;
}

static gboolean
plugin_init (GstPlugin * plugin)
{

  /* FIXME Remember to set the rank if it's an element that is meant
     to be autoplugged by decodebin. */
  return gst_element_register (plugin, "imlscale", GST_RANK_NONE,
      GST_TYPE_IMLSCALE);
}

/* FIXME: these are normally defined by the GStreamer build system.
   If you are creating an element to be included in gst-plugins-*,
   remove these, as they're always defined.  Otherwise, edit as
   appropriate for your external plugin package. */


GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    imlscale,
    "FIXME plugin description",
    plugin_init, VERSION, GST_LICENSE, GST_PACKAGE_NAME, GST_PACKAGE_ORIGIN)

