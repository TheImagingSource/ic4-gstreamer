#pragma once

#include <ic4/ImageType.h>
#include <vector>
#include <gst/gst.h>
#include <expected>

namespace ic4::gst
{

    struct ic4_gst_table_entry
    {
        ic4::PixelFormat ic4_format;

        const char* gst_name;
        const char* gst_format;
    };


    std::expected<ic4_gst_table_entry, std::errc> get_entry_by_pixel_format_name(const std::string&);

    std::vector<ic4_gst_table_entry> get_ic4_gst_table();

    const char* pixel_format_name_to_gst_format(const std::string& name);

    ic4::PixelFormat gst_format_to_pixel_format(const char* format_str);

    ic4::PixelFormat gst_caps_to_pixel_format(const GstCaps& caps);
    GstCaps* pixel_format_to_gst_caps(const char* fmt);

} // namespace ic4::gst
