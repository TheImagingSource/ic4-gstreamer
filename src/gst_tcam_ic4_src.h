

#pragma once

#include <gst/base/gstpushsrc.h>
#include <gst/gst.h>

G_BEGIN_DECLS

#define GST_TYPE_TCAM_IC4_SRC (gst_tcam_ic4_src_get_type())
#define GST_TCAM_IC4_SRC(obj)                                                  \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_TCAM_IC4_SRC, GstTcamIC4Src))
#define GST_TCAM_IC4_SRC_CLASS(klass)                                          \
  (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_TCAM_IC4_SRC, GstTcamIC4Src))
#define GST_IS_TCAM_IC4_SRC(obj)                                               \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_TCAM_IC4_SRC))
#define GST_IS_TCAM_IC4_SRC_CLASS(obj)                                         \
  (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_TCAM_IC4_SRC))

typedef struct _GstTcamIC4Src GstTcamIC4Src;
typedef struct _GstTcamIC4SrcClass GstTcamIC4SrcClass;
struct ic4_device_state;

struct _GstTcamIC4Src {
  GstPushSrc element;

  GstBufferPool *pool;
  // device_state *device;
  struct ic4_device_state *device;
  gdouble fps;
};

struct _GstTcamIC4SrcClass {
  GstPushSrcClass parent_class;
};

GST_DEBUG_CATEGORY_EXTERN(tcam_ic4_src_debug);

GType gst_tcam_ic4_src_get_type(void);

G_END_DECLS
