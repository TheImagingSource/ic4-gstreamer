

#pragma once

#include "ic4/Properties.h"
#include <gst/gst.h>
#include <ic4/ic4.h>

namespace ic4::gst
{

GstCaps* create_caps(ic4::PropertyMap&);

} //namespace ic4::gst
