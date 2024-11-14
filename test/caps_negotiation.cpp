
#include <doctest/doctest.h>
#include <fmt/format.h>

#include <gst/gst.h>
#include <gst/video/video.h>

#include <iostream>
#include <cassert>

std::string get_test_serial()
{
    char* value = getenv("TEST_SERIAL_U3V");

    if (value)
    {
        return std::string(value);
    }

    return {};

}


std::string get_highest_bayer_format(const std::string& serial)
{
    GstElement* source = gst_element_factory_make("tcamic4src", "test-source");

    GValue val = {};
    g_value_init(&val, G_TYPE_STRING);
    g_value_set_static_string(&val, serial.c_str());

    g_object_set_property(G_OBJECT(source), "serial", &val);
    

    /* Setting the state to ready ensures that all resources
       are initialized and that we really get all format capabilities */
    gst_element_set_state(source, GST_STATE_READY);

    GstPad* pad = gst_element_get_static_pad(source, "src");

    GstCaps* caps = gst_pad_query_caps(pad, NULL);

    std::string caps_str = gst_caps_to_string(caps);

    gst_caps_unref(caps);
    gst_element_set_state(source, GST_STATE_NULL);
    gst_object_unref(source);

    auto format_exists = [&caps_str](const std::string& format)
    {
        auto found = caps_str.find(format);
        return found != std::string::npos;
    };

    static const char* formats[] = {
        "rggb16",
        "bggr16",
        "grbg16",
        "gbrg16",
        "rggb12",
        "bggr12",
        "grbg12",
        "gbrg12",
        "rggb10",
        "bggr10",
        "grbg10",
        "gbrg10",
        "rggb",
        "bggr",
        "grbg",
        "gbrg",
    };

    for (const auto& f : formats)
    {
        if (format_exists(f))
        {
            return f;
        }
    }

    return {};
}


struct cb_data
{
    unsigned int frame_counter = 0;
    GstCaps* caps = nullptr;
    bool caps_are_equal = true;
};

static GstFlowReturn callback(GstElement* sink, void* user_data)
{

    GstSample* sample = nullptr;
    cb_data* data = (cb_data*) user_data;

    g_signal_emit_by_name(sink, "pull-sample", &sample, nullptr);

    if (sample)
    {
        GstBuffer* buffer = gst_sample_get_buffer(sample);
        GstMapInfo info; // contains the actual image
        if (gst_buffer_map(buffer, &info, GST_MAP_READ))
        {
            GstVideoInfo* video_info = gst_video_info_new();
            if (!gst_video_info_from_caps(video_info, gst_sample_get_caps(sample)))
            {
                // Could not parse video info (should not happen)
                g_warning("Failed to parse video info");
                return GST_FLOW_ERROR;
            }

            data->frame_counter++;
            GstCaps* caps = gst_sample_get_caps(sample);

            if (data->caps)
            {
                bool caps_equal = gst_caps_is_equal(caps, data->caps);
                if (!caps_equal)
                {

                    fmt::print("{} != {}\n", gst_caps_to_string(caps), gst_caps_to_string(data->caps));
                    data->caps_are_equal = false;
                }
            }
            gst_buffer_unmap(buffer, &info);
            gst_video_info_free(video_info);
        }
        gst_sample_unref(sample);
    }
    return GST_FLOW_OK;
}


TEST_CASE("device-caps")
{

    auto serial = get_test_serial();
    auto bayer_fmt = get_highest_bayer_format(serial);

    std::string caps_str = fmt::format("video/x-bayer,format={},width=640,height=480,framerate=30/1", bayer_fmt);

    std::string pipe_str = fmt::format("tcamic4src serial={} "
    " ! {}"
    " ! appsink name=sink emit-signals=true", serial, caps_str);

    GstElement* pipeline = gst_parse_launch(pipe_str.c_str(), nullptr);

    gst_element_set_state(pipeline, GST_STATE_READY);

    cb_data data;

    data.caps = gst_caps_from_string(caps_str.c_str());

    /* retrieve the appsink from the pipeline */
    GstElement* sink = gst_bin_get_by_name(GST_BIN(pipeline), "sink");

    // tell appsink to notify us when it receives an image
    g_object_set(G_OBJECT(sink), "emit-signals", TRUE, NULL);

    // tell appsink what function to call when it notifies us
    g_signal_connect(sink, "new-sample", G_CALLBACK(callback), &data);

    gst_object_unref(sink);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    sleep(5);

    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    pipeline = nullptr;

    CHECK(data.frame_counter >= 140);
    CHECK(data.caps_are_equal);

    gst_caps_unref(data.caps);

}

TEST_CASE("ic4-format-transformation")
{

    auto serial = get_test_serial();
    auto bayer_fmt = get_highest_bayer_format(serial);

    std::string caps_str = fmt::format("video/x-raw,format=BGRx,device-format={},width=640,height=480,framerate=30/1", bayer_fmt);

    std::string pipe_str = fmt::format("tcamic4src serial={} "
    " ! {}"
    " ! appsink name=sink emit-signals=true", serial, caps_str);

    GstElement* pipeline = gst_parse_launch(pipe_str.c_str(), nullptr);

    gst_element_set_state(pipeline, GST_STATE_READY);

    cb_data data;

    data.caps = gst_caps_from_string(caps_str.c_str());

    /* retrieve the appsink from the pipeline */
    GstElement* sink = gst_bin_get_by_name(GST_BIN(pipeline), "sink");

    // tell appsink to notify us when it receives an image
    g_object_set(G_OBJECT(sink), "emit-signals", TRUE, NULL);

    // tell appsink what function to call when it notifies us
    g_signal_connect(sink, "new-sample", G_CALLBACK(callback), &data);

    gst_object_unref(sink);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    sleep(5);

    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    pipeline = nullptr;

    CHECK(data.frame_counter >= 140);
    CHECK(data.caps_are_equal);

    gst_caps_unref(data.caps);
}



TEST_CASE("binning 2x2")
{

    auto serial = get_test_serial();
    auto bayer_fmt = get_highest_bayer_format(serial);

    std::string caps_str = fmt::format("video/x-raw,format=BGRx,binning=2x2,device-format={},width=640,height=480,framerate=30/1", bayer_fmt);

    std::string pipe_str = fmt::format("tcamic4src serial={} "
    " ! {}"
    " ! appsink name=sink emit-signals=true", serial, caps_str);

    GstElement* pipeline = gst_parse_launch(pipe_str.c_str(), nullptr);

    gst_element_set_state(pipeline, GST_STATE_READY);

    cb_data data;

    data.caps = gst_caps_from_string(caps_str.c_str());

    /* retrieve the appsink from the pipeline */
    GstElement* sink = gst_bin_get_by_name(GST_BIN(pipeline), "sink");

    // tell appsink to notify us when it receives an image
    g_object_set(G_OBJECT(sink), "emit-signals", TRUE, NULL);

    // tell appsink what function to call when it notifies us
    g_signal_connect(sink, "new-sample", G_CALLBACK(callback), &data);

    gst_object_unref(sink);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    sleep(5);

    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    pipeline = nullptr;


    CHECK(data.frame_counter >= 140);
    CHECK(data.caps_are_equal);

    gst_caps_unref(data.caps);
}