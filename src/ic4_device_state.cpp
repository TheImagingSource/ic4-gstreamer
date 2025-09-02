
#include "ic4_device_state.h"
#include "gst/gstinfo.h"
#include "ic4/Properties.h"
#include "ic4_tcam_property.h"
#include <algorithm>
#include <ic4/Error.h>
#include <ic4/Grabber.h>
#include <memory>

#include "gst_tcam_ic4_src.h"

#define GST_CAT_DEFAULT ic4_src_debug


namespace
{

struct blacklist_entry
{
    std::string property_name;
};

static const std::string property_blacklist[] = {
    "Width",
    "Height",
    // "BinningHorizontal",
    // "BinningVertical",
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

#ifdef ENABLE_TCAM_PROP

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
            auto prop = ic4::gst::make_wrapper_instance(child, category.name());

            if (prop)
            {
                // GST_DEBUG("new prop: %s", std::string(prop->get_property_name()).c_str());
                interface.tcamprop_properties.push_back(std::move(prop));
            }
        }
    }
}
#endif /* ENABLE_TCAM_PROP */

void ic4_device_state::populate_tcamprop_interface()
{
#ifdef ENABLE_TCAM_PROP

    auto properties = grabber->devicePropertyMap();

    auto root  = properties.findCategory("Root");

    iterate_node_children(root, tcamprop_interface_);

    tcamprop_container_.create_list(&tcamprop_interface_);

#endif /* ENABLE_TCAM_PROP */
}


bool ic4_device_state::open_device()
{

    grabber = std::make_shared<ic4::Grabber>();

    auto dev_list = ic4::DeviceEnum::enumDevices();

    if (dev_list.empty())
    {
        GST_ERROR("No devices available");
        return false;
    }
    // use first device
    if (identifier_.empty())
    {

        if (!grabber->deviceOpen(dev_list.at(0)))
        {
            GST_ERROR("Unable to open device");
            return false;
        }
        identifier_ = dev_list.at(0).serial();
    }
    else
    {
        ic4::Error err;
        grabber = std::make_shared<ic4::Grabber>(identifier_, err);

        if (err)
        {

            GST_ERROR("Unable to open the wanted device. %s", err.message().c_str());
            return false;
        }
    }
    listener = std::make_shared<sink_listener>();
    listener->state = this;

    populate_tcamprop_interface();

    GST_INFO("Opened device with identifier: %s", identifier_.c_str());

    if (!set_property_cache_.empty())
    {
        if (!set_properties_from_string(set_property_cache_))
        {
            GST_ERROR("Setting properties caused an error. Some properties may not be set.");
        }
        else
        {
            GST_INFO("Applied properties");
        }
    }
    else
    {
        GST_INFO("No user properties to apply.");

    }

    return true;

}


bool ic4_device_state::set_properties_from_string(const std::string &str)
{
    if (!grabber)
    {
        GST_INFO("storing string for when camera is open");
        set_property_cache_ = str;
        return true;
    }

    auto split = [](const std::string s, std::string delimiter) -> std::vector<std::string>
        {
            size_t pos_start = 0, pos_end, delim_len = delimiter.length();
            std::string token;
            std::vector<std::string> res;

            while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos)
            {
                token = s.substr (pos_start, pos_end - pos_start);
                pos_start = pos_end + delim_len;
                res.push_back (token);
            }

            res.push_back (s.substr (pos_start));
            return res;
        };

    auto properties_to_set = split(str, " ");
    auto props = grabber->devicePropertyMap();

    for (const auto& p : properties_to_set)
    {
        auto property_and_value = split(p, "=");

        if (property_and_value.size() != 2)
        {
            GST_ERROR("Can not determine value for \"%s\". Use <Name>=<Value>", p.c_str());
            return false;
        }

        auto property_name = property_and_value.at(0);
        auto property_value_str = property_and_value.at(1);

        ic4::Error err;
        GST_DEBUG("Setting %s to %s", property_name.c_str(), property_value_str.c_str());;

        if (!props.setValue(property_name, property_value_str, err))
        {
            GST_ERROR("Error while setting %s to %s: %s", property_name.c_str(), property_value_str.c_str(), err.message().c_str());
            return false;
        }
    }

    return true;
}
