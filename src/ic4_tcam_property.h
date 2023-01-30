

#pragma once

#include "Tcam-1.0.h"
#include "ic4/Properties.h"
#include "tcamprop1.0_base/tcamprop_base.h"
#include "tcamprop1.0_base/tcamprop_property_interface.h"

#include <ic4/ic4.h>
#include <tcam-property-1.0.h>

namespace ic4::gst
{

auto make_wrapper_instance(ic4::Property& prop)
    -> std::unique_ptr<tcamprop1::property_interface>;
void ic4_tcam_property_init(
        TcamPropertyProviderInterface* iface);
}
