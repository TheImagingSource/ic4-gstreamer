

#pragma once

#include <gst/base/gstpushsrc.h>
#include <gst/gst.h>

G_BEGIN_DECLS

#define GST_TYPE_IC4_SRC (gst_ic4_src_get_type())
#define GST_IC4_SRC(obj)                                                  \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), GST_TYPE_IC4_SRC, GstIC4Src))
#define GST_IC4_SRC_CLASS(klass)                                          \
  (G_TYPE_CHECK_CLASS_CAST((klass), GST_TYPE_IC4_SRC, GstIC4Src))
#define GST_IS_IC4_SRC(obj)                                               \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj), GST_TYPE_IC4_SRC))
#define GST_IS_IC4_SRC_CLASS(obj)                                         \
  (G_TYPE_CHECK_CLASS_TYPE((klass), GST_TYPE_IC4_SRC))

typedef struct _GstIC4Src GstIC4Src;
typedef struct _GstIC4SrcClass GstIC4SrcClass;
struct ic4_device_state;

struct _GstIC4Src {
  GstPushSrc element;

  GstBufferPool *pool;

  struct ic4_device_state *device;
  gdouble fps;
};

struct _GstIC4SrcClass {
  GstPushSrcClass parent_class;
};

GST_DEBUG_CATEGORY_EXTERN(ic4_src_debug);

GType gst_ic4_src_get_type(void);

G_END_DECLS
