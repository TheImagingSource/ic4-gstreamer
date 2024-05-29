
#include "ic4_device_state.h"
#include "gst/gstinfo.h"
#include "ic4/Properties.h"
#include "ic4_tcam_property.h"
#include <algorithm>

namespace
{

struct blacklist_entry
{
    std::string property_name;
};

static const std::string property_blacklist[] = {
    "Width",
    "Height",
    "PixelFormat",
    "AcquisitionFrameRate",
    "PayloadSize",
    "AcquisitionStart",
    "AcquisitionStop",
    "AcquisitionMode",
    "TLParamsLocked",
    };

bool is_blacklist_property(const std::string& property_name)
{
    auto ret = std::find_if(std::begin(property_blacklist),
                            std::end(property_blacklist),
                            [property_name](const std::string& n)
                            {return n == property_name;});
    if (ret == std::end(property_blacklist))
    {
        return false;
    }
    return true;
}

} // namespace


void iterate_node_children(ic4::PropCategory& category, ic4::gst::src_interface_list& interface)
{

    auto children = category.features();

    for (auto& child : children)
    {
        if (child.type() == ic4::PropType::Category)
        {
            auto tmp = child.asCategory();
            iterate_node_children(tmp, interface);
        }
        else
        {
            if (is_blacklist_property(child.name()) || child.visibility() == ic4::PropVisibility::Invisible)
            {
                continue;
            }
            auto prop = ic4::gst::make_wrapper_instance(child);

            if (prop)
            {
                GST_ERROR("new prop: %s", std::string(prop->get_property_name()).c_str());
                interface.tcamprop_properties.push_back(std::move(prop));
            }
        }
    }
}


void ic4_device_state::populate_tcamprop_interface()
{
    auto properties = grabber->devicePropertyMap();

    auto root  = properties.findCategory("Root");

    iterate_node_children(root, tcamprop_interface_);

    tcamprop_container_.create_list(&tcamprop_interface_);
}


bool ic4_device_state::open_device()
{

    grabber = std::make_shared<ic4::Grabber>();

    auto dev_list = ic4::DeviceEnum::enumDevices();
        //getAvailableVideoCaptureDevices();
    if (dev_list.empty())
    {
        GST_ERROR("No devices available");
        return false;
    }
    // use first device
    if (serial_.empty())
    {

        if (!grabber->deviceOpen(dev_list.at(0)))
        {
            GST_ERROR("Unable to open device");
            return false;
        }
        serial_ = dev_list.at(0).serial();
    }
    else
    {
        for (auto& item : dev_list)
        {
            if (item.serial() == serial_)
            {
                if (!grabber->deviceOpen(item))
                {
                    GST_ERROR("Unable to open device");
                    return false;
                }
                break;
            }
        }
    }
    listener = std::make_shared<sink_listener>();
    listener->state = this;

    populate_tcamprop_interface();

    GST_INFO("Opened device with serial: %s", serial_.c_str());

    return true;

}
