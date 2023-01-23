
#include "ic4src_gst_device_provider.h"
#include "gst_tcam_ic4_src.h"

#include <gst/gst.h>


// static gboolean plugin_init(GstPlugin *plugin)
// {
//   gst_device_provider_register(plugin, "tcamic4srcdeviceprovider",
//                                GST_RANK_PRIMARY,
//                                TCAM_TYPE_IC4_SRC_DEVICE_PROVIDER);
//   gst_element_register(plugin, "tcamic4src", GST_RANK_PRIMARY,
//                        GST_TYPE_TCAM_IC4_SRC);

//   GST_DEBUG_CATEGORY_INIT(tcam_ic4_src_debug, "tcamic4src", 0,
//                           "tcam interface");

//   return TRUE;
// }

// #ifndef PACKAGE
// #define PACKAGE "tcam"
// #endif

// GST_PLUGIN_DEFINE(GST_VERSION_MAJOR, GST_VERSION_MINOR, tcamic4src,
//                   "TCam Video Source", plugin_init, "1.0.0",
//                   "Proprietary", "tcamic4src", "theimagingsource.com")
