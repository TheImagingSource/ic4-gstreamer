
#include <Tcam-1.0.h>
#include <cstddef>
#include <cstdint>
#include <doctest/doctest.h>
#include <fmt/format.h>

#include <iostream>

#include <tcam-property-1.0.h>

#include <thread>
#include <gst/gst.h>
#include "glib-object.h"
#include "test_helper.h"


TEST_CASE("device-listed")
{

    // The device monitor listens to device activities for us
    GstDeviceMonitor* monitor = gst_device_monitor_new();
    // We are only interested in devices that are in the categories
    // Video and Source && tcam
    gst_device_monitor_add_filter(monitor, "Video/Source/tcam", NULL);

    std::string serial_to_test = test_helper::get_test_serial();

    GList* devices = gst_device_monitor_get_devices(monitor);
    bool dev_found = false;

    for (GList* elem = devices; elem; elem = elem->next)
    {
        GstDevice* device = (GstDevice*) elem->data;

        GstStructure* struc = gst_device_get_properties(device);
        std::string dev_serial = gst_structure_get_string(struc, "serial");

        if (dev_serial == serial_to_test)
        {
            dev_found = true;
        }
        
        gst_structure_free(struc);
    }

    CHECK(dev_found);

    g_list_free_full(devices, gst_object_unref);
}


struct dev_monitor_helper
{
    std::string serial;
    bool dev_lost = false;
    bool dev_new = false;
};


gboolean bus_function(GstBus* bus __attribute__((unused)), GstMessage* message, gpointer user_data __attribute__((unused)))
{
    GstDevice* device;
    auto* values = (dev_monitor_helper*)user_data;
    std::string serial = values->serial;
    auto check_if_correct_dev = [serial](GstDevice* dev)
    {
        GstStructure* struc = gst_device_get_properties(dev);
        std::string dev_serial = gst_structure_get_string(struc, "serial");
        //std::cout << "NEW " << dev_serial  << std::endl;

        bool res = false;
        if (dev_serial == serial)
        {
            res = true;
        }

        gst_structure_free(struc);
        return res;
    };
    
    switch (GST_MESSAGE_TYPE(message))
    {
        case GST_MESSAGE_DEVICE_ADDED:
        {
            gst_message_parse_device_added(message, &device);

            if (check_if_correct_dev(device))
            {
                values->dev_new = true;
            }

            gst_object_unref(device);
            break;
        }
        case GST_MESSAGE_DEVICE_REMOVED:
        {
            // this can also be used as an alternative to device-lost signals
            gst_message_parse_device_removed(message, &device);
            
            if (check_if_correct_dev(device))
            {
                values->dev_lost = true;
            }
            gst_object_unref(device);
            break;
        }
        default:
        {
            break;
        }
    }

    // this means we want to continue
    // to listen to device events
    // to stop listening return G_SOURCE_REMOVE;
    return G_SOURCE_CONTINUE;
}


TEST_CASE("device-add-remove")
{

    // The device monitor listens to device activities for us
    GstDeviceMonitor* monitor = gst_device_monitor_new();
    // We are only interested in devices that are in the categories
    // Video and Source && tcam
    gst_device_monitor_add_filter(monitor, "Video/Source/tcam", NULL);

    struct dev_monitor_helper bus_values;
    
    std::string serial = test_helper::get_test_serial();
    bus_values.serial = serial;

    GstBus* bus = gst_device_monitor_get_bus(monitor);
    gst_bus_add_watch(bus, bus_function, &bus_values);
    gst_object_unref(bus);

    // actually start the dynamic monitoring
    gst_device_monitor_start(monitor);

    
    std::string pipeline_desc = fmt::format("ic4src serial={} name=source ! appsink ", serial);
    
    GError* err = nullptr;
    GstElement* pipeline = gst_parse_launch(pipeline_desc.c_str(), &err);

    if (pipeline == nullptr)
    {
        CHECK(false);
    }


    gst_element_set_state(pipeline, GST_STATE_READY);
    GstElement* source = gst_bin_get_by_name(GST_BIN(pipeline), "source");

    
    TcamPropertyCommand* reset = TCAM_PROPERTY_COMMAND(tcam_property_provider_get_tcam_property(TCAM_PROPERTY_PROVIDER(source), "DeviceReset", &err));
    tcam_property_command_set_command(reset, &err);

    GMainLoop* loop = g_main_loop_new(NULL, FALSE);

    std::jthread helper_thread = std::jthread([loop]()
    {
        sleep(10);
        g_main_loop_quit(loop);
    });


    // This is simply used to wait for events or the user to end this script
    g_main_loop_run(loop);
    g_main_loop_unref(loop);


    CHECK(bus_values.dev_lost);
    CHECK(bus_values.dev_new);
    

    gst_element_set_state(pipeline, GST_STATE_NULL);


    gst_object_unref(source);
    gst_object_unref(pipeline);

    
    
    // has to be called when gst_device_monitor_start has been called
    gst_device_monitor_stop(monitor);

    // cleanup
    gst_object_unref(monitor);

}
