

#pragma once

#include "ic4/Properties.h"
#include <gst/gst.h>
#include <ic4/ic4.h>

namespace ic4::gst {

const char* caps_to_PixelFormat(const GstCaps&);
GstCaps *PixelFormat_to_GstCaps(const char *fmt);

GstCaps* create_caps(ic4::PropertyMap&);

} //namespace ic4::gst
