/*
 * Copyright 2020 The Imaging Source Europe GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gst/gst.h>
#include <fmt/core.h>
#include <iostream>

#include "print_caps.h"

#include <CLI/CLI.hpp>


static void print_devices(size_t /*t*/)
{
    auto monitor = gst_device_monitor_new();

    gst_device_monitor_add_filter(monitor, "Video/Source/tcam", NULL);

    GList* devices = gst_device_monitor_get_devices(monitor);

    auto dev_count = g_list_length(devices);

    if (dev_count > 0)
    {
        fmt::print("{:20} {}", "Model", "Serial");

        for (GList* dev = devices; dev; dev = dev->next)
        {
            GstStructure* struc = gst_device_get_properties(GST_DEVICE(dev->data));
            fmt::print("{:20} {}",
                       gst_structure_get_string(struc, "model"),
                       gst_structure_get_string(struc, "serial"));
        }

    }
    {
        std::cout << "No devices found." << std::endl;
    }
    
    g_list_free_full(devices, gst_object_unref);
    gst_object_unref(monitor);
}

int main(int argc, char *argv[])
{

    CLI::App app{ "Simple ic4 camera control utility", "ic4-ctrl"};
    app.set_help_flag();
    app.set_help_all_flag( "-h,--help", "Expand all help" );

    // CLI11 uses "TEXT" as a filler for the option string arguments
    // replace it with "SERIAL" to make the help text more intuitive.
    app.get_formatter()->label("TEXT", "SERIAL");
        app.allow_extras();

    std::string serial;
    auto list_cmd = app.add_subcommand("devices",
                                       "List available devices and interfaces by connection.");
    auto caps_cmd = app.add_subcommand("caps",
                                       "List gstreamer caps of the devices with given serial.");
    caps_cmd->add_option("serial", serial,
                         "Serial of the camera to open. e.g. '12345678'. Use `devices` subcommand to get a list." )->required();

    auto transform_cmd = app.add_subcommand("transform", "List available transformations of a GstElement");
    std::string transform_element = "videoconvert";
    transform_cmd->add_option("-e,--element", transform_element, "Which transform element to use.");

    auto transform_group = transform_cmd->add_option_group("caps");
    auto list_transform_in = transform_group->add_option("--in", "Caps that go into the transform element");
    auto list_transform_out = transform_group->add_option("--out", "Caps to come out of the transform element");
    list_transform_in->excludes(list_transform_out);

    // the help Formatter instance is inherited/shared from the parent (app)
    // We want a separate formatter to have different place holder texts
    // create new Formatter instance as we really only need the text and nothing else
    transform_cmd->formatter(std::make_shared<CLI::Formatter>());
    transform_cmd->get_formatter()->label("TEXT", "GstElement");

    try
    {
        app.parse( argc, argv );
    }
    catch( const CLI::ParseError& e )
    {
        return app.exit( e );
    }

    
    gst_init(&argc, &argv);

    if (list_cmd->parsed())
    {
        print_devices(0);
    }
    else if (caps_cmd->parsed())
    {
        ic4::gst::helper::list_gstreamer_1_0_formats(serial);
    }
    else if (transform_cmd->parsed())
    {
        std::string transform_caps = "";
        ic4::gst::helper::ElementPadDirection transform_direction = ic4::gst::helper::ElementPadDirection::Both;

        if (*list_transform_in)
        {
            list_transform_in->results(transform_caps);
            transform_direction = ic4::gst::helper::ElementPadDirection::In;
        }
        else if (*list_transform_out)
        {
            list_transform_out->results(transform_caps);
            transform_direction = ic4::gst::helper::ElementPadDirection::Out;
        }
        ic4::gst::helper::convert(transform_element, transform_direction, transform_caps);
    }
    else
    {
        std::cout << app.help() << std::endl;
    }
    return 0;
}
