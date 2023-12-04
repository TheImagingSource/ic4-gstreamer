
#pragma once

#include "../libs/gst-helper/include/gst-helper/gstelement_helper.h"
#include "../libs/gst-helper/include/gst-helper/helper_functions.h"
#include "../libs/gst-helper/include/tcamprop1.0_gobject/tcam_property_provider.h"
#include "ic4_gst_conversions.h"

#include <atomic>
#include <condition_variable>
#include <gst/gst.h>
#include <ic4/ic4.h>
#include <memory>
#include <mutex>
#include <tcamprop1.0_base/tcamprop_property_interface.h>


namespace ic4::gst
{
struct src_interface_list : tcamprop1::property_list_interface
{
    std::vector<std::unique_ptr<tcamprop1::property_interface>> tcamprop_properties;

    auto get_property_list() -> std::vector<std::string_view> final
    {
        std::vector<std::string_view> ret;

        ret.reserve(tcamprop_properties.size());

        for (const auto& v : tcamprop_properties)
        {
            ret.push_back(v->get_property_name());
        }
        return ret;
    }
    auto find_property(std::string_view name) -> tcamprop1::property_interface* final
    {
        for (const auto& v : tcamprop_properties)
        {
            if (name == v->get_property_name())
            {
                return v.get();
            }
        }
        return nullptr;
    }
    void clear() noexcept
    {
        tcamprop_properties.clear();
    }
};

} // namespace ic4::gst

// forward declaration
// found in src.cpp
struct sink_listener;

struct ic4_device_state
{

    ///ic4::VideoCaptureDeviceItem

    std::shared_ptr<ic4::Grabber> grabber;

    std::shared_ptr<ic4::QueueSink> sink;

    std::shared_ptr<sink_listener> listener;
    void* dev_lost_token_ = nullptr;

    std::atomic<bool> streaming_ = false;
    std::mutex stream_mtx_;
    std::condition_variable stream_cv_;

    std::string serial_;

    std::string get_serial()
    {
        return serial_;
    };

    bool set_serial(const std::string s)
    {
        if (is_open())
        {
            return false;
        }
        GST_DEBUG("Device serial is now: %s", s.c_str());
        serial_ = s;
        return true;
    };


    bool is_open()
    {
        if (grabber)
        {
            return grabber->isDeviceOpen();
        }
        return false;
    };

    bool open_device();

    bool is_streaming()
    {
        return streaming_;
    }

    GstCaps* get_caps()
    {
        if (!is_open())
        {
            GST_WARNING("==== Device not open");
            return nullptr;
        }
        auto props = grabber->devicePropertyMap();
        return ic4::gst::create_caps(props);
    }

    auto get_container() -> tcamprop1_gobj::tcam_property_provider&
    {
        return tcamprop_container_;
    }

private:
    ic4::gst::src_interface_list tcamprop_interface_;
    tcamprop1_gobj::tcam_property_provider tcamprop_container_;
    void populate_tcamprop_interface();
};


struct sink_listener : public ic4::QueueSinkListener
{

    bool sinkConnected(ic4::QueueSink& sink, const ic4::ImageType& frameType)
    {
        (void)sink;
        (void)frameType;

        GST_INFO("sinkConnected");
        state->sink->allocAndQueueBuffers(20);
        return true;
    }

    void sinkDisconnected(ic4::QueueSink& /*sink*/)
    {
        //(void)sink;
        GST_INFO("sinkDisconnected");
    }

    void framesQueued(ic4::QueueSink& /*sink*/)
    {
        //GST_ERROR("framesQueued");
        // _queued = true;
        state->streaming_ = true;
        state->stream_cv_.notify_all();
    };

    ic4_device_state* state;
};
