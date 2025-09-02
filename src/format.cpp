
#include "format.h"
#include <ic4/ImageType.h>
#include <iterator>

#include <cstring>
#include <system_error>

namespace {


static const ic4::gst::ic4_gst_table_entry format_list[] = {
    { ic4::PixelFormat::Mono8, "video/x-raw", "GRAY8" },
    { ic4::PixelFormat::Mono10p, "video/x-raw", "GRAY10p" },
    { ic4::PixelFormat::Mono12p, "video/x-raw", "GRAY12p" },
    { ic4::PixelFormat::Mono12Packed, "video/x-raw", "GRAY12sp" },
    { ic4::PixelFormat::Mono16, "video/x-raw", "GRAY16_LE" },
    { ic4::PixelFormat::BayerBG8, "video/x-bayer", "bggr" },
    { ic4::PixelFormat::BayerBG10p, "video/x-bayer", "bggr10p" },
    { ic4::PixelFormat::BayerBG12p, "video/x-bayer", "bggr12p" },
    { ic4::PixelFormat::BayerBG12Packed, "video/x-bayer", "bggr12sp" },
    { ic4::PixelFormat::BayerBG16, "video/x-bayer", "bggr16" },
    { ic4::PixelFormat::BayerGB8, "video/x-bayer", "gbrg" },
    { ic4::PixelFormat::BayerGB10p, "video/x-bayer", "gbrg10p" },
    { ic4::PixelFormat::BayerGB12p, "video/x-bayer", "gbrg12p" },
    { ic4::PixelFormat::BayerGB12Packed, "video/x-bayer", "gbrg12sp" },
    { ic4::PixelFormat::BayerGB16, "video/x-bayer", "gbrg16" },
    { ic4::PixelFormat::BayerGR8, "video/x-bayer", "grbg" },
    { ic4::PixelFormat::BayerGR10p, "video/x-bayer", "grbg10p" },
    { ic4::PixelFormat::BayerGR12p, "video/x-bayer", "grbg12p" },
    { ic4::PixelFormat::BayerGR12Packed, "video/x-bayer", "grbg12sp" },
    { ic4::PixelFormat::BayerGR16, "video/x-bayer", "grbg16" },
    { ic4::PixelFormat::BayerRG8, "video/x-bayer", "rggb" },
    { ic4::PixelFormat::BayerRG10p, "video/x-bayer", "rggb10p" },
    { ic4::PixelFormat::BayerRG12p, "video/x-bayer", "rggb12p" },
    { ic4::PixelFormat::BayerRG12Packed, "video/x-bayer", "rggb12sp" },
    { ic4::PixelFormat::BayerRG16, "video/x-bayer", "rggb16" },

    { ic4::PixelFormat::BGR8, "video/x-raw", "BGR"},
    { ic4::PixelFormat::BGRa8, "video/x-raw", "BGRx" },
    { ic4::PixelFormat::BGRa16, "video/x-raw", "BGRA16_LE" },
    { ic4::PixelFormat::YUV422_8, "video/x-raw", "YUY2"},
    { ic4::PixelFormat::YCbCr411_8, "video/x-raw", "Y41B"},
    // { ic4::PixelFormat::YCbCr422_8,          "YCbCr422_8", "video/x-raw", "YUY2"},
    { ic4::PixelFormat::YCbCr422_8, "video/x-raw",  "UYVY", },
    { ic4::PixelFormat::YCbCr411_8_CbYYCrYY, "video/x-raw", "IYU1", },
    { ic4::PixelFormat::YCbCr411_8_CbYYCrYY, "video/x-raw", "NV12", },

    ////// polarization formats
    { ic4::PixelFormat::PolarizedMono8, "video/x-raw", "polarized-GRAY8-v0"},
    { ic4::PixelFormat::PolarizedMono12p, "video/x-raw", "polarized-GRAY12p-v0"},
    { ic4::PixelFormat::PolarizedMono12Packed, "video/x-raw", "polarized-GRAY12sp-v0"},
    { ic4::PixelFormat::PolarizedMono16,  "video/x-raw", "polarized-GRAY16-v0"},

    { ic4::PixelFormat::PolarizedBayerBG8, "video/x-bayer", "polarized-bggr8-v0"},
    { ic4::PixelFormat::PolarizedBayerBG12p, "video/x-bayer", "polarized-bayer-bggr12p-v0"},
    { ic4::PixelFormat::PolarizedBayerBG12Packed, "video/x-bayer", "polarized-bayer-bggr12sp-v0"},
    { ic4::PixelFormat::PolarizedBayerBG16, "video/x-bayer", "polarized-bayer-bggr16-v0"},
    { ic4::PixelFormat::PolarizedADIMono8, "tis", "polarized-ADI-GRAY8"},
    { ic4::PixelFormat::PolarizedADIMono16, "tis", "polarized-ADI-GRAY16"},
    { ic4::PixelFormat::PolarizedADIRGB8, "tis", "polarized-ADI-RGB8"},
    { ic4::PixelFormat::PolarizedADIRGB16, "tis", "polarized-ADI-RGB16"},
    { ic4::PixelFormat::PolarizedQuadMono8, "tis", "polarized-quad-GRAY8"},
    { ic4::PixelFormat::PolarizedQuadMono16, "tis", "polarized-quad-GREY16"},
    { ic4::PixelFormat::PolarizedQuadBG8, "tis", "polarized-quad-bggr8"},
    { ic4::PixelFormat::PolarizedQuadBG16, "tis", "polarized-quad-bggr16"},
}; // format_list


} // namespace

std::expected<ic4::gst::ic4_gst_table_entry, std::errc> ic4::gst::get_entry_by_pixel_format_name(const std::string& name)
{
    for (const auto &entry : format_list)
    {
        if (ic4::to_string(entry.ic4_format) == name)
        {
            return entry;
        }
    }
    return std::unexpected(std::errc::invalid_argument);
}


std::vector<ic4::gst::ic4_gst_table_entry> ic4::gst::get_ic4_gst_table()

{
    auto size = std::size(format_list);
    std::vector<ic4::gst::ic4_gst_table_entry> ret;

    ret.reserve(size);

    for (const auto& f : format_list)
    {
        ret.push_back(f);
    }

    return ret;
}

const char* ic4::gst::pixel_format_name_to_gst_format(const std::string& name)
{

    for (const auto& f : format_list)
    {
        if (ic4::to_string((f.ic4_format)) == name)
        {
            return f.gst_format;
        }
    }


    return nullptr;

}


ic4::PixelFormat ic4::gst::gst_format_to_pixel_format(const char* format_str)
{

    for (const auto& e : format_list)
    {
        if (strcmp(format_str, e.gst_format) == 0)
        {
            return e.ic4_format;
        }
    }


    return ic4::PixelFormat::Invalid;
}


ic4::PixelFormat ic4::gst::gst_caps_to_pixel_format(const GstCaps& caps)
{

    if (!gst_caps_is_fixed(&caps))
    {
        return ic4::PixelFormat::Invalid;
    }

    GstStructure* struc = gst_caps_get_structure(&caps, 0);

    const char* name = gst_structure_get_name(struc);
    const char* format = gst_structure_get_string(struc, "format");

    for (const auto& e : format_list)
    {
        if (strcmp(name, e.gst_name) == 0
            && strcmp(format, e.gst_format) == 0)
        {
            return e.ic4_format;
        }
    }

    return ic4::PixelFormat::Invalid;
}

GstCaps *ic4::gst::pixel_format_to_gst_caps(const char* fmt) {

    for (const auto& e : format_list)
    {
        if (strcmp(fmt, ic4::to_string(e.ic4_format).c_str()) == 0)
        {
            std::string s = e.gst_name;
            s += ",format=";
            s += e.gst_format;
            GstCaps* caps = gst_caps_from_string(s.c_str());
            return caps;
        }
    }

    return nullptr;
}
