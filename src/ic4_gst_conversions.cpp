
#include "ic4_gst_conversions.h"
#include "gst/gstcaps.h"
#include "gst/gststructure.h"
#include "gst/gstutils.h"
#include "gst/gstvalue.h"
#include "ic4/Properties.h"
#include <algorithm>
#include <cstdint>
#include <string>
#include <cstring>
#include <vector>

namespace {

struct gst_pfnc {
    const char* pfnc_str;
    const char* gst_name;
    const char* gst_format;
};

static const gst_pfnc format_list[] = {
    { "Mono1p", "video/x-raw", "Mono1p" },
    { "Mono2p", "video/x-raw", "Mono2p" },
    { "Mono4p", "video/x-raw", "Mono4p" },
    { "Mono8", "video/x-raw", "GRAY8" },
    { "Mono8s", "video/x-raw", "Mono8s" },
    { "Mono10", "video/x-raw", "Mono10" },
    { "Mono10p", "video/x-raw", "Mono10p" },
    { "Mono12", "video/x-raw", "Mono12" },
    { "Mono12p", "video/x-raw", "Mono12p" },
    { "Mono14", "video/x-raw", "Mono14" },
    { "Mono14p", "video/x-raw", "Mono14p" },
    { "Mono16", "video/x-raw", "GRAY16_LE" },
    { "BayerBG4p", "video/x-bayer", "BayerBG4p" },
    { "BayerBG8", "video/x-bayer", "bggr" },
    { "BayerBG10", "video/x-bayer", "BayerBG10" },
    { "BayerBG10p", "video/x-bayer", "BayerBG10p" },
    { "BayerBG12", "video/x-bayer", "BayerBG12" },
    { "BayerBG12p", "video/x-bayer", "BayerBG12p" },
    { "BayerBG14", "video/x-bayer", "BayerBG14" },
    { "BayerBG14p", "video/x-bayer", "BayerBG14p" },
    { "BayerBG16", "video/x-bayer", "BayerBG16" },
    { "BayerGB4p", "video/x-bayer", "BayerGB4p" },
    { "BayerGB8", "video/x-bayer", "gbrg" },
    { "BayerGB10", "video/x-bayer", "BayerGB10" },
    { "BayerGB10p", "video/x-bayer", "BayerGB10p" },
    { "BayerGB12", "video/x-bayer", "BayerGB12" },
    { "BayerGB12p", "video/x-bayer", "BayerGB12p" },
    { "BayerGB14", "video/x-bayer", "BayerGB14" },
    { "BayerGB14p", "video/x-bayer", "BayerGB14p" },
    { "BayerGB16", "video/x-bayer", "BayerGB16" },
    { "BayerGR4p", "video/x-bayer", "BayerGR4p" },
    { "BayerGR8", "video/x-bayer", "grbg" },
    { "BayerGR10", "video/x-bayer", "BayerGR10" },
    { "BayerGR10p", "video/x-bayer", "BayerGR10p" },
    { "BayerGR12", "video/x-bayer", "BayerGR12" },
    { "BayerGR12p", "video/x-bayer", "BayerGR12p" },
    { "BayerGR14", "video/x-bayer", "BayerGR14" },
    { "BayerGR14p", "video/x-bayer", "BayerGR14p" },
    { "BayerGR16", "video/x-bayer", "BayerGR16" },
    { "BayerRG4p", "video/x-bayer", "BayerRG4p" },
    { "BayerRG8", "video/x-bayer", "rggb" },
    { "BayerRG10", "video/x-bayer", "BayerRG10" },
    { "BayerRG10p", "video/x-bayer", "BayerRG10p" },
    { "BayerRG12", "video/x-bayer", "BayerRG12" },
    { "BayerRG12p", "video/x-bayer", "BayerRG12p" },
    { "BayerRG14", "video/x-bayer", "BayerRG14" },
    { "BayerRG14p", "video/x-bayer", "BayerRG14p" },
    { "BayerRG16", "video/x-bayer", "BayerRG16" },
    { "RGBa8", "video/x-raw", "RGBa8" },
    { "RGBa10", "video/x-raw", "RGBa10" },
    { "RGBa10p", "video/x-raw", "RGBa10p" },
    { "RGBa12", "video/x-raw", "RGBa12" },
    { "RGBa12p", "video/x-raw", "RGBa12p" },
    { "RGBa14", "video/x-raw", "RGBa14" },
    { "RGBa16", "video/x-raw", "RGBa16" },
    { "RGB8", "video/x-raw", "RGB8" },
    { "RGB8_Planar", "video/x-raw", "RGB8_Planar" },
    { "RGB10", "video/x-raw", "RGB10" },
    { "RGB10_Planar", "video/x-raw", "RGB10_Planar" },
    { "RGB10p", "video/x-raw", "RGB10p" },
    { "RGB10p32", "video/x-raw", "RGB10p32" },
    { "RGB12", "video/x-raw", "RGB12" },
    { "RGB12_Planar", "video/x-raw", "RGB12_Planar" },
    { "RGB12p", "video/x-raw", "RGB12p" },
    { "RGB14", "video/x-raw", "RGB14" },
    { "RGB16", "video/x-raw", "RGB16" },
    { "RGB16_Planar", "video/x-raw", "RGB16_Planar" },
    { "RGB565p", "video/x-raw", "RGB565p" },
    { "BGRa8", "video/x-raw", "BGRa8" },
    { "BGRa10", "video/x-raw", "BGRa10" },
    { "BGRa10p", "video/x-raw", "BGRa10p" },
    { "BGRa12", "video/x-raw", "BGRa12" },
    { "BGRa12p", "video/x-raw", "BGRa12p" },
    { "BGRa14", "video/x-raw", "BGRa14" },
    { "BGRa16", "video/x-raw", "BGRa16" },
    { "BGR8", "video/x-raw", "BGR8" },
    { "BGR10", "video/x-raw", "BGR10" },
    { "BGR10p", "video/x-raw", "BGR10p" },
    { "BGR12", "video/x-raw", "BGR12" },
    { "BGR12p", "video/x-raw", "BGR12p" },
    { "BGR14", "video/x-raw", "BGR14" },
    { "BGR16", "video/x-raw", "BGR16" },
    { "BGR565p", "video/x-raw", "BGR565p" },

    // {"R8", "video/x-raw", "R8"},
    // {"R10", "video/x-raw", "R10"},
    // {"R12", "video/x-raw", "R12"},
    // {"R16", "video/x-raw", "R16"},
    // {"G8", "video/x-raw", "G8"},
    // {"G10", "video/x-raw", "G10"},
    // {"G12", "", "G12"},
    // {"G16", "", "G16"},
    // {"B8", "", "B8"},
    // {"B10", "", "B10"},
    // {"B12", "", "B12"},
    // {"B16", "", "B16"},
    // {"Coord3D_ABC8", "", "Coord3D_ABC8"},
    // {"Coord3D_ABC8_Planar", "", "Coord3D_ABC8_Planar"},
    // {"Coord3D_ABC10p", "",

    //  "Coord3D_ABC10p"},
    // {"Coord3D_ABC10p_Planar", "", "Coord3D_ABC10p_Planar"},
    // {"Coord3D_ABC12p", "", "Coord3D_ABC12p"},
    // {"Coord3D_ABC12p_Planar", "", "Coord3D_ABC12p_Planar"},
    // {"Coord3D_ABC16", "",  "Coord3D_ABC16"},
    // {"Coord3D_ABC16_Planar", "",  "Coord3D_ABC16_Planar"},
    // {"Coord3D_ABC32f", "",  "Coord3D_ABC32f"},
    // {"Coord3D_ABC32f_Planar", "", "Coord3D_ABC32f_Planar"},
    // {"Coord3D_AC8", "", "Coord3D_AC8"},
    // {"Coord3D_AC8_Planar", "", "Coord3D_AC8_Planar"},
    // {"Coord3D_AC10p", "", "Coord3D_AC10p"},
    // {"Coord3D_AC10p_Planar", "", "Coord3D_AC10p_Planar"},
    // {"Coord3D_AC12p", "", "Coord3D_AC12p"},
    // {"Coord3D_AC12p_Planar", "", "Coord3D_AC12p_Planar"},
    // {"Coord3D_AC16", "", "Coord3D_AC16"},
    // {"Coord3D_AC16_Planar", "", "Coord3D_AC16_Planar"},
    // {"Coord3D_AC32f", "", "Coord3D_AC32f"},
    // {"Coord3D_AC32f_Planar", "", "Coord3D_AC32f_Planar"},
    // {"Coord3D_A8", "", "Coord3D_A8"},
    // {"Coord3D_A10p", "", "Coord3D_A10p"},
    // {"Coord3D_A12p", "", "Coord3D_A12p"},
    // {"Coord3D_A16", "", "Coord3D_A16"},
    // {"Coord3D_A32f", "", "Coord3D_A32f"},
    // {"Coord3D_B8", "", "Coord3D_B8"},
    // {"Coord3D_B10p", "", "Coord3D_B10p"},
    // {"Coord3D_B12p", "", "Coord3D_B12p"},
    // {"Coord3D_B16", "", "Coord3D_B16"},
    // {"Coord3D_B32f", "", "Coord3D_B32f"},
    // {"Coord3D_C8", "", "Coord3D_C8"},
    // {"Coord3D_C10p", "", "Coord3D_C10p"},
    // {"Coord3D_C12p", "", "Coord3D_C12p"},
    // {"Coord3D_C16", "", "Coord3D_C16"},
    // {"Coord3D_C32f", "",

    //  "Coord3D_C32f"},
    // {"Confidence1", "",

    //  "Confidence1"},
    // {"Confidence1p", "", "Confidence1p"},
    // {"Confidence8", "", "Confidence8"},
    // {"Confidence16", "", "Confidence16"},
    // {"Confidence32f", "", "Confidence32f"},
    // {"BiColorBGRG8", "", "BiColorBGRG8"},
    // {"BiColorBGRG10", "", "BiColorBGRG10"},
    // {"BiColorBGRG10p", "",

    //  "BiColorBGRG10p"},
    // {"BiColorBGRG12", "", "BiColorBGRG12"},
    // {"BiColorBGRG12p", "", "BiColorBGRG12p"},
    // {"BiColorRGBG8", "", "BiColorRGBG8"},
    // {"BiColorRGBG10", "", "BiColorRGBG10"},
    // {"BiColorRGBG10p", "", "BiColorRGBG10p"},
    // {"BiColorRGBG12", "", "BiColorRGBG12"},
    // {"BiColorRGBG12p", "", "BiColorRGBG12p"},
    // {"SCF1WBWG8", "", "SCF1WBWG8"},
    // {"SCF1WBWG10", "", "SCF1WBWG10"},
    // {"SCF1WBWG10p", "", "SCF1WBWG10p"},
    // {"SCF1WBWG12", "", "SCF1WBWG12"},
    // {"SCF1WBWG12p", "", "SCF1WBWG12p"},
    // {"SCF1WBWG14", "", "SCF1WBWG14"},
    // {"SCF1WBWG16", "", "SCF1WBWG16"},
    // {"SCF1WGWB8", "", "SCF1WGWB8"},
    // {"SCF1WGWB10", "", "SCF1WGWB10"},
    // {"SCF1WGWB10p", "", "SCF1WGWB10p"},
    // {"SCF1WGWB12", "", "SCF1WGWB12"},
    // {"SCF1WGWB12p", "", "SCF1WGWB12p"},
    // {"SCF1WGWB14", "", "SCF1WGWB14"},
    // {"SCF1WGWB16", "", "SCF1WGWB16"},
    // {"SCF1WGWR8", "", "SCF1WGWR8"},
    // {"SCF1WGWR10", "", "SCF1WGWR10"},
    // {"SCF1WGWR10p", "", "SCF1WGWR10p"},
    // {"SCF1WGWR12", "", "SCF1WGWR12"},
    // {"SCF1WGWR12p", "", "SCF1WGWR12p"},
    // {"SCF1WGWR14", "", "SCF1WGWR14"},
    // {"SCF1WGWR16", "", "SCF1WGWR16"},
    // {"SCF1WRWG8", "", "SCF1WRWG8"},
    // {"SCF1WRWG10", "", "SCF1WRWG10"},
    // {"SCF1WRWG10p", "", "SCF1WRWG10p"},
    // {"SCF1WRWG12", "", "SCF1WRWG12"},
    // {"SCF1WRWG12p", "", "SCF1WRWG12p"},
    // {"SCF1WRWG14", "", "SCF1WRWG14"},
    // {"SCF1WRWG16", "", "SCF1WRWG16"},
    // {"YCbCr8", "", "YCbCr8"},
    // {"YCbCr8_CbYCr", "", "YCbCr8_CbYCr"},
    // {"YCbCr10_CbYCr", "", "YCbCr10_CbYCr"},
    // {"YCbCr10p_CbYCr", "", "YCbCr10p_CbYCr"},
    // {"YCbCr12_CbYCr", "", "YCbCr12_CbYCr"},
    // {"YCbCr12p_CbYCr", "", "YCbCr12p_CbYCr"},
    // {"YCbCr411_8", "", "YCbCr411_8"},
    // {"YCbCr411_8_CbYYCrYY", "", "YCbCr411_8_CbYYCrYY"},
    // {"YCbCr422_8", "", "YCbCr422_8"},
    // {"YCbCr422_8_CbYCrY", "", "YCbCr422_8_CbYCrY"},
    // {"YCbCr422_10", "", "YCbCr422_10"},
    // {"YCbCr422_10_CbYCrY", "", "YCbCr422_10_CbYCrY"},
    // {"YCbCr422_10p", "", "YCbCr422_10p"},
    // {"YCbCr422_10p_CbYCrY", "", "YCbCr422_10p_CbYCrY"},
    // {"YCbCr422_12", "", "YCbCr422_12"},
    // {"YCbCr422_12_CbYCrY", "", "YCbCr422_12_CbYCrY"},
    // {"YCbCr422_12p", "", "YCbCr422_12p"},
    // {"YCbCr422_12p_CbYCrY", "", "YCbCr422_12p_CbYCrY"},
    // {"YCbCr601_8_CbYCr", "", "YCbCr601_8_CbYCr"},
    // {"YCbCr601_10_CbYCr", "", "YCbCr601_10_CbYCr"},
    // {"YCbCr601_10p_CbYCr", "", "YCbCr601_10p_CbYCr"},
    // {"YCbCr601_12_CbYCr", "", "YCbCr601_12_CbYCr"},
    // {"YCbCr601_12p_CbYCr", "", "YCbCr601_12p_CbYCr"},
    // {"YCbCr601_411_8_CbYYCrYY", "", "YCbCr601_411_8_CbYYCrYY"},
    // {"YCbCr601_422_8", "", "YCbCr601_422_8"},
    // {"YCbCr601_422_8_CbYCrY", "", "YCbCr601_422_8_CbYCrY"},
    // {"YCbCr601_422_10", "", "YCbCr601_422_10"},
    // {"YCbCr601_422_10_CbYCrY", "", "YCbCr601_422_10_CbYCrY"},
    // {"YCbCr601_422_10p", "", "YCbCr601_422_10p"},
    // {"YCbCr601_422_10p_CbYCrY", "", "YCbCr601_422_10p_CbYCrY"},
    // {"YCbCr601_422_12", "", "YCbCr601_422_12"},
    // {"YCbCr601_422_12_CbYCrY", "", "YCbCr601_422_12_CbYCrY"},
    // {"YCbCr601_422_12p", "", "YCbCr601_422_12p"},
    // {"YCbCr601_422_12p_CbYCrY", "", "YCbCr601_422_12p_CbYCrY"},
    // {"YCbCr709_8_CbYCr", "", "YCbCr709_8_CbYCr"},
    // {"YCbCr709_10_CbYCr", "", "YCbCr709_10_CbYCr"},
    // {"YCbCr709_10p_CbYCr", "", "YCbCr709_10p_CbYCr"},
    // {"YCbCr709_12_CbYCr", "", "YCbCr709_12_CbYCr"},
    // {"YCbCr709_12p_CbYCr", "", "YCbCr709_12p_CbYCr"},
    // {"YCbCr709_411_8_CbYYCrYY", "", "YCbCr709_411_8_CbYYCrYY"},
    // {"YCbCr709_422_8", "", "YCbCr709_422_8"},
    // {"YCbCr709_422_8_CbYCrY", "", "YCbCr709_422_8_CbYCrY"},
    // {"YCbCr709_422_10", "", "YCbCr709_422_10"},
    // {"YCbCr709_422_10_CbYCrY", "", "YCbCr709_422_10_CbYCrY"},
    // {"YCbCr709_422_10p", "", "YCbCr709_422_10p"},
    // {"YCbCr709_422_10p_CbYCrY", "", "YCbCr709_422_10p_CbYCrY"},
    // {"YCbCr709_422_12", "", "YCbCr709_422_12"},
    // {"YCbCr709_422_12_CbYCrY", "", "YCbCr709_422_12_CbYCrY"},
    // {"YCbCr709_422_12p", "", "YCbCr709_422_12p"},
    // {"YCbCr709_422_12p_CbYCrY", "", "YCbCr709_422_12p_CbYCrY"},
    // {"YCbCr2020_8_CbYCr", "", "YCbCr2020_8_CbYCr"},
    // {"YCbCr2020_10_CbYCr", "", "YCbCr2020_10_CbYCr"},
    // {"YCbCr2020_10p_CbYCr", "", "YCbCr2020_10p_CbYCr"},
    // {"YCbCr2020_12_CbYCr", "", "YCbCr2020_12_CbYCr"},
    // {"YCbCr2020_12p_CbYCr", "", "YCbCr2020_12p_CbYCr"},
    // {"YCbCr2020_411_8_CbYYCrYY", "", "YCbCr2020_411_8_CbYYCrYY"},
    // {"YCbCr2020_422_8", "", "YCbCr2020_422_8"},
    // {"YCbCr2020_422_8_CbYCrY", "", "YCbCr2020_422_8_CbYCrY"},
    // {"YCbCr2020_422_10", "", "YCbCr2020_422_10"},
    // {"YCbCr2020_422_10_CbYCrY", "", "YCbCr2020_422_10_CbYCrY"},
    // {"YCbCr2020_422_10p", "", "YCbCr2020_422_10p"},
    // {"YCbCr2020_422_10p_CbYCrY", "", "YCbCr2020_422_10p_CbYCrY"},
    // {"YCbCr2020_422_12", "", "YCbCr2020_422_12"},
    // {"YCbCr2020_422_12_CbYCrY", "", "YCbCr2020_422_12_CbYCrY"},
    // {"YCbCr2020_422_12p", "", "YCbCr2020_422_12p"},
    // {"YCbCr2020_422_12p_CbYCrY", "", "YCbCr2020_422_12p_CbYCrY"},
    // {"YUV8_UYV", "", "YUV8_UYV"},
    // {"YUV411_8_UYYVYY", "", "YUV411_8_UYYVYY"},
    // {"YUV422_8", "", "YUV422_8"},
    // {"YUV422_8_UYVY", "", "YUV422_8_UYVY"},
    // {"Mono10Packed", "video/x-raw", "Mono10Packed"},
    // {"Mono12Packed", "video/x-raw", "Mono12Packed"},
    // {"BayerBG10Packed", "video/x-raw", "BayerBG10Packed"},
    // {"BayerBG12Packed", "video/x-raw", "BayerBG12Packed"},
    // {"BayerGB10Packed", "Video/x-bayer", "BayerGB10Packed"},
    // {"BayerGB12Packed", "Video/x-bayer", "BayerGB12Packed"},
    // {"BayerGR10Packed", "Video/x-bayer", "BayerGR10Packed"},
    // {"BayerGR12Packed", "Video/x-bayer", "BayerGR12Packed"},
    // {"BayerRG10Packed", "Video/x-bayer", "BayerRG10Packed"},
    // {"BayerRG12Packed", "Video/x-bayer", "BayerRG12Packed"},
    // {"RGB10V1Packed", "video/x-raw", "RGB10V1Packed"},
    // {"RGB12V1Packed", "video/x-raw", "RGB12V1Packed"},
};


const gst_pfnc* get_entry(const char* pfnc_str)
{

    for (const auto& e : format_list)
    {
        if (strcmp(pfnc_str, e.pfnc_str) == 0)
        {
            return &e;
        }
    }
    return nullptr;
}

} // namespace


const char* ic4::gst::caps_to_PixelFormat(const GstCaps& caps)
{

    if (!gst_caps_is_fixed(&caps))
    {
        return nullptr;
    }

    GstStructure* struc = gst_caps_get_structure(&caps, 0);

    const char* name = gst_structure_get_name(struc);
    const char* format = gst_structure_get_string(struc, "format");

    for (const auto& e : format_list)
    {
        if (strcmp(name, e.gst_name) == 0
            && strcmp(format, e.gst_format) == 0)
        {
            // gst_structure_free(struc);
            return e.pfnc_str;
        }
    }
    // gst_structure_free(struc);

    return nullptr;
}

GstCaps *ic4::gst::PixelFormat_to_GstCaps(const char* fmt) {

    for (const auto& e : format_list)
    {
        if (strcmp(fmt, e.pfnc_str) == 0)
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

struct image_size
{
    int width;
    int height;

    bool operator<(const struct image_size& other) const
    {
        if (height <= other.height && width <= other.width)
        {
            return true;
        }
        return false;
    }

    bool operator==(const image_size& other) const
    {
        return (height == other.height && width == other.width);
    }
};

std::vector<image_size> get_standard_resolutions(const image_size& min,
                                                 const image_size& max,
                                                 const image_size& step)
{
    static const image_size resolutions[] = {
        { 128, 96 },    { 320, 240 },   { 360, 280 },   { 544, 480 },   { 640, 480 },
        { 352, 288 },   { 576, 480 },   { 720, 480 },   { 960, 720 },   { 1280, 720 },
        { 1440, 1080 }, { 1920, 1080 }, { 1920, 1200 }, { 2048, 1152 }, { 2048, 1536 },
        { 2560, 1440 }, { 3840, 2160 }, { 4096, 3072 }, { 7680, 4320 }, { 7680, 4800 },
    };

    std::vector<struct image_size> ret;
    ret.reserve(std::size(resolutions));
    for (const auto& r : resolutions)
    {
        if ((min < r) && (r < max) && (r.width % step.width == 0) && (r.height % step.height == 0))
        {
            ret.push_back(r);
        }
    }

    return ret;
}


GstCaps *ic4::gst::create_caps(ic4::PropertyMap & props)
{
    auto p_fmt = props.findEnumeration("PixelFormat");

    auto p_width = props.findInteger("Width");
    int64_t width_min;
    int64_t width_max;
    int64_t width_step;
    std::vector<int64_t> width_values;

    if (p_width.incrementMode() == ic4::PropIncrementMode::Increment)
    {
        width_min = p_width.minimum();
        width_max = p_width.maximum();
        width_step = p_width.increment();
    }
    else
    {
        width_values = p_width.validValueSet();
    }

    auto p_height = props.findInteger("Height");
    int64_t height_min;
    int64_t height_max;
    int64_t height_step;
    std::vector<int64_t> height_values;

    if (p_height.incrementMode() == ic4::PropIncrementMode::Increment)
    {
        height_min = p_height.minimum();
        height_max = p_height.maximum();
        height_step = p_height.increment();
    }
    else
    {
        height_values = p_height.validValueSet();
    }

    auto p_fps = props.findFloat("AcquisitionFrameRate");

    int fps_min_num;
    int fps_min_den;
    int fps_max_num;
    int fps_max_den;

    std::vector<double> fps_values;
    GValue fps = G_VALUE_INIT;

    // all fpd cameras have ranges
    // v4l2 usb2 cameras all have value sets
    if (p_fps.incrementMode() != ic4::PropIncrementMode::ValueSet)
    {

        double fps_min;
        fps_min = p_fps.minimum();

        double fps_max;
        fps_max = p_fps.maximum();
        gst_util_double_to_fraction(fps_min, &fps_min_num, &fps_min_den);
        gst_util_double_to_fraction(fps_max, &fps_max_num, &fps_max_den);
    }
    else
    {
        fps_values = p_fps.validValueSet();

        g_value_init(&fps, GST_TYPE_LIST);
        for (const auto& val : fps_values)
        {
            int fps_num;
            int fps_den;
            gst_util_double_to_fraction(val, &fps_num, &fps_den);

            GValue v = G_VALUE_INIT;
            g_value_init(&v, GST_TYPE_FRACTION);

            gst_value_set_fraction(&v, fps_num, fps_den);

            gst_value_list_append_value(&fps, &v);

        }
    }

    GstCaps* caps = gst_caps_new_empty();

    // we either have both or none
    // values sets only exist for v4l2/fpd cameras
    assert(height_values.empty() == width_values.empty());

    bool do_ranges = true;
    if (!width_values.empty())
    {
        do_ranges = false;
    }

    for (const auto& f : p_fmt.entries())
    {
        auto fmt = get_entry(f.name().c_str());

        if (!fmt)
        {
            GST_ERROR("Unable to process pfnc format %s. Skipping.", f.name().c_str());
            continue;
        }

        GstStructure* s = gst_structure_new(fmt->gst_name,
                                            "format", G_TYPE_STRING, fmt->gst_format,
                                            nullptr);

        GValue val_fps = G_VALUE_INIT;
        g_value_init(&val_fps, GST_TYPE_FRACTION_RANGE);
        gst_value_set_fraction_range_full(
            &val_fps, fps_min_num, fps_min_den, fps_max_num, fps_max_den);

        gst_structure_take_value(s, "framerate", &val_fps);

        GValue val_width = G_VALUE_INIT;
        GValue val_height = G_VALUE_INIT;

        if (do_ranges)
        {
            g_value_init(&val_width, GST_TYPE_INT_RANGE);
            gst_value_set_int_range_step(&val_width, width_min, width_max, width_step);

            g_value_init(&val_height, GST_TYPE_INT_RANGE);
            ///g_value_init(&val_height, GST_TYPE_L);
            gst_value_set_int_range_step(&val_height, height_min, height_max, height_step);


            gst_structure_take_value(s, "width", &val_width);
            gst_structure_take_value(s, "height", &val_height);

            gst_caps_append_structure(caps, s);
        }
        else
        {
            auto min_w = std::min_element(width_values.begin(), width_values.end());
            auto max_w = std::max_element(width_values.begin(), width_values.end());
            auto min_h = std::min_element(height_values.begin(), height_values.end());
            auto max_h = std::max_element(height_values.begin(), height_values.end());

            image_size min_size = { (int)*min_w, (int)*min_h };
            image_size max_size = { (int)*max_w, (int)*max_h };
            image_size step_size = { 1, 1 };
            auto res = get_standard_resolutions(min_size, max_size, step_size);

            g_value_init(&val_width, GST_TYPE_LIST);
            g_value_init(&val_height, GST_TYPE_LIST);

            for (const auto& r : res)
            {
                GstStructure* s2 = gst_structure_copy(s);

                GValue w = G_VALUE_INIT;
                g_value_init(&w, G_TYPE_INT);
                g_value_set_int(&w, r.width);

                // gst_value_list_append_value(&val_width, &w);
                // g_value_unset(&w);

                GValue h = G_VALUE_INIT;
                g_value_init(&h, G_TYPE_INT);
                g_value_set_int(&h, r.height);

                // gst_value_list_append_value(&val_height, &h);
                // g_value_unset(&h);


                gst_structure_take_value(s, "width", &w);
                gst_structure_take_value(s, "height", &h);

                gst_caps_append_structure(caps, s2);
            }
            // since not appended, must be freed
            gst_structure_free(s);

            // for (const auto& val : width_values)
            // {
            //     GValue v = G_VALUE_INIT;
            //     g_value_init(&v, G_TYPE_INT);
            //     g_value_set_int(&v, val);

            //     gst_value_list_append_value(&val_width, &v);
            //     g_value_unset(&v);
            // }


            // for (const auto& val : height_values)
            // {
            //     GValue v = G_VALUE_INIT;
            //     g_value_init(&v, G_TYPE_INT);
            //     g_value_set_int(&v, val);

            //     gst_value_list_append_value(&val_height, &v);
            //     g_value_unset(&v);
            // }
        }


    }

    return caps;
}
