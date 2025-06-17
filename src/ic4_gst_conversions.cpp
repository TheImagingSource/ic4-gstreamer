
#include "ic4_gst_conversions.h"
#include "ic4/Properties.h"
#include "gst/gst.h"
#include <algorithm>
#include <cstdint>
#include <ic4/ImageType.h>
#include <string>
#include <cstring>
#include <vector>
#include <fmt/format.h>

#include "gst_tcam_ic4_src.h"

// make gstreamer logging work
#define GST_CAT_DEFAULT ic4_src_debug

namespace {

struct gst_pfnc {
    ic4::PixelFormat ic4_fmt;
    const char* pfnc_str;
    const char* gst_name;
    const char* gst_format;
};

static const gst_pfnc format_list[] = {
    { ic4::PixelFormat::Mono8, "Mono8",   "video/x-raw", "GRAY8" },
    { ic4::PixelFormat::Mono10p, "Mono10p", "video/x-raw", "Mono10p" },
    { ic4::PixelFormat::Mono12p, "Mono12p", "video/x-raw", "Mono12p" },
    { ic4::PixelFormat::Mono16, "Mono16",  "video/x-raw", "GRAY16_LE" },
    { ic4::PixelFormat::BayerBG8, "BayerBG8",   "video/x-bayer", "bggr" },
    { ic4::PixelFormat::BayerBG10p, "BayerBG10p", "video/x-bayer", "bggr10p" },
    { ic4::PixelFormat::BayerBG12p, "BayerBG12p", "video/x-bayer", "bggr12p" },
    { ic4::PixelFormat::BayerBG16, "BayerBG16",  "video/x-bayer", "bggr16" },
    { ic4::PixelFormat::BayerGB8, "BayerGB8",   "video/x-bayer", "gbrg" },
    { ic4::PixelFormat::BayerGB10p, "BayerGB10p", "video/x-bayer", "gbrg10p" },
    { ic4::PixelFormat::BayerGB12p, "BayerGB12p", "video/x-bayer", "gbrg12p" },
    { ic4::PixelFormat::BayerGB16, "BayerGB16",  "video/x-bayer", "gbrg16" },
    { ic4::PixelFormat::BayerGR8, "BayerGR8",   "video/x-bayer", "grbg" },
    { ic4::PixelFormat::BayerGR10p, "BayerGR10p", "video/x-bayer", "grbg10p" },
    { ic4::PixelFormat::BayerGR12p, "BayerGR12p", "video/x-bayer", "grbg12p" },
    { ic4::PixelFormat::BayerGR16, "BayerGR16",  "video/x-bayer", "grbg16" },
    { ic4::PixelFormat::BayerRG8, "BayerRG8",   "video/x-bayer", "rggb" },
    { ic4::PixelFormat::BayerRG10p, "BayerRG10p", "video/x-bayer", "rggb10p" },
    { ic4::PixelFormat::BayerRG12p, "BayerRG12p", "video/x-bayer", "rggb12p" },
    { ic4::PixelFormat::BayerRG16, "BayerRG16",  "video/x-bayer", "rggb16" },

    { ic4::PixelFormat::BGR8, "BGR8",   "video/x-raw", "BGR"},
    { ic4::PixelFormat::BGRa8, "BGRa8",  "video/x-raw", "BGRx" },
    { ic4::PixelFormat::BGRa16, "BGRa16", "video/x-raw", "BGRA16_LE" },
// YUV422_8 	YUV 4:2:2 8-bit.
// YCbCr422_8 	YCbCr 4:2:2 8-bit.
// {"YCbCr411_8_CbYYCrYY", "video/x-raw", "IYU1"} //	YCbCr 4:1:1 8-bit (CbYYCrYY)
// YCbCr411_8 	YCbCr 4:1:1 8-bit (YYCbYYCr)
//     { "", "video/x-raw", "I420"},
//     { "", "video/x-raw", "YV12"},

    ////// dutils

    //{ img::fourcc::YUY2,                    "video/x-raw", "YUY2", },
    { ic4::PixelFormat::YCbCr422_8, "YCbCr422_8_CbYCrY",                    "video/x-raw",  "UYVY", },
        //{ img::fourcc::YCbCr411_8_CbYYCrYY,     "video/x-raw", "IYU1", },

    //{ ic4::PixelFormat::YCbCr420_, "YCbCr420_8_YY_CrCb_Semiplanar", "video/x-raw", "NV12", },
    { ic4::PixelFormat::YCbCr411_8_CbYYCrYY, "YCbCr420_8_YY_CrCb_Semiplanar", "video/x-raw", "NV12", },
        // { img::fourcc::YV12,                    "video/x-raw", "YV12", },

        // { PfncFormat::YCbCr422_8_CbYCrY,                    fourcc::UYVY },
        // { PfncFormat::YCbCr8_CbYCr,                         fourcc::IYU2 },
        // { PfncFormat::YCbCr420_8_YY_CrCb_Semiplanar,        fourcc::NV12 },


    //////

    // the following are either not supported
    // or not tested

    // { "RGBa10", "video/x-raw", "RGBa10" },
    // { "RGBa10p", "video/x-raw", "RGBa10p" },
    // { "RGBa12", "video/x-raw", "RGBa12" },
    // { "RGBa12p", "video/x-raw", "RGBa12p" },
    // { "RGBa14", "video/x-raw", "RGBa14" },
    // { "RGBa16", "video/x-raw", "RGBa16" },
    // { "RGB8", "video/x-raw", "RGB8" },
    // { "RGB8_Planar", "video/x-raw", "RGB8_Planar" },
    // { "RGB10", "video/x-raw", "RGB10" },
    // { "RGB10_Planar", "video/x-raw", "RGB10_Planar" },
    // { "RGB10p", "video/x-raw", "RGB10p" },
    // { "RGB10p32", "video/x-raw", "RGB10p32" },
    // { "RGB12", "video/x-raw", "RGB12" },
    // { "RGB12_Planar", "video/x-raw", "RGB12_Planar" },
    // { "RGB12p", "video/x-raw", "RGB12p" },
    // { "RGB14", "video/x-raw", "RGB14" },
    // { "RGB16", "video/x-raw", "RGB16" },
    // { "RGB16_Planar", "video/x-raw", "RGB16_Planar" },
    // { "RGB565p", "video/x-raw", "RGB565p" },
    // { "BGRa8", "video/x-raw", "BGRa8" },
    // { "BGRa10", "video/x-raw", "BGRa10" },
    // { "BGRa10p", "video/x-raw", "BGRa10p" },
    // { "BGRa12", "video/x-raw", "BGRa12" },
    // { "BGRa12p", "video/x-raw", "BGRa12p" },
    // { "BGRa14", "video/x-raw", "BGRa14" },
    // { "BGRa16", "video/x-raw", "BGRa16" },
    // { "BGR8", "video/x-raw", "BGR8" },
    // { "BGR10", "video/x-raw", "BGR10" },
    // { "BGR10p", "video/x-raw", "BGR10p" },
    // { "BGR12", "video/x-raw", "BGR12" },
    // { "BGR12p", "video/x-raw", "BGR12p" },
    // { "BGR14", "video/x-raw", "BGR14" },
    // { "BGR16", "video/x-raw", "BGR16" },
    // { "BGR565p", "video/x-raw", "BGR565p" },

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


const char* ic4::gst::PixelFormat_to_gst_format_string(const char* pixel_format)
{

    for (const auto& e : format_list)
    {
        if (strcmp(pixel_format, e.pfnc_str) == 0)
        {
            return e.gst_format;
        }
    }

    return nullptr;
}


const char* ic4::gst::format_string_to_PixelFormat(const char* format_str)
{

    for (const auto& e : format_list)
    {
        if (strcmp(format_str, e.gst_format) == 0)
        {
            return e.pfnc_str;
        }
    }

    return nullptr;
}


ic4::PixelFormat ic4::gst::caps_to_PixelFormat(const GstCaps& caps)
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
            return e.ic4_fmt;
        }
    }

    return ic4::PixelFormat::Invalid;
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


bool format_is_bayer(const std::string &format)
{
  static const std::vector<std::string> bayer_formats = {
    "BayerRG8",
    "BayerGR8",
    "BayerGB8",
    "BayerBG8",
    "BayerRG10",
    "BayerGR10",
    "BayerGB10",
    "BayerBG10",
    "BayerRG12",
    "BayerGR12",
    "BayerGB12",
    "BayerBG12",
    "BayerRG14",
    "BayerGR14",
    "BayerGB14",
    "BayerBG14",
    "BayerRG16",
    "BayerGR16",
    "BayerGB16",
    "BayerBG16",
  };

    auto res = std::find(bayer_formats.begin(),
                         bayer_formats.end(),
                         format);

    return res != bayer_formats.end();
}


bool format_is_mono(const std::string &format)
{
    static const std::vector<std::string> mono_formats = {
        "Mono8",
        "Mono16",
    };

    auto res = std::find(mono_formats.begin(), mono_formats.end(), format);

    return res != mono_formats.end();

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


GstCaps* ic4::gst::create_caps(ic4::PropertyMap& props)
{

    auto p_bin_x = props.findInteger("BinningHorizontal");
    auto p_bin_y = props.findInteger("BinningVertical");
    bool has_binning = p_bin_x.is_valid();
    std::vector<std::string> gst_binning_entries;
    
    if (p_bin_x.is_valid())
    {


    }
    else
    {
        gst_binning_entries.push_back("1x1");
        GST_INFO("No binning");
    }

    
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

    if (has_binning)
    {
        const auto& bin_x = p_bin_x.asInteger();
        auto entries = bin_x.validValueSet();

        // not a valueSet; determine via range
        if (entries.empty())
        {
            for (int i = bin_x.minimum(); i <= bin_x.maximum(); i++)
            {
                // TODO: does a camera exist with 8x binning
                if (i == 3)
                {
                    continue;
                }

                entries.push_back(i);
            }

        }

        for (const auto& e: entries)
        {
            gst_binning_entries.push_back(fmt::format("{}x{}", e, e));
        }

        // //gst_value_array_init(&binning, gst_binning_entries.size());
        // gst_value_list_init(&binning, gst_binning_entries.size());

        // // transform entries into something gstreamer can use
        // for (const auto& e: gst_binning_entries)
        // {
        //     GValue entry = G_VALUE_INIT;
        //     g_value_init(&entry, G_TYPE_STRING);
        //     g_value_set_string(&entry, e.c_str());

        //     gst_value_list_append_value(&binning, &entry);
        //     //gst_value_array_append_value(&binning, &entry);
        // }

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

    p_fmt.entries();
    std::vector<std::string> fmt_names;
    fmt_names.reserve(p_fmt.entries().size());
    for (const auto& f : p_fmt.entries())
    {
        fmt_names.push_back(f.name());
    }

    auto sort_fmt_names = [] (std::vector<std::string>& fmt_names) -> std::vector<std::string>
    {

        std::vector<std::string> bayer;
        std::vector<std::string> mono;
        std::vector<std::string> rest;

        for (const auto& fmt: fmt_names)
        {
            if (format_is_bayer(fmt))
            {
                bayer.push_back(fmt);
            }
            else if (format_is_mono(fmt))
            {
                mono.push_back(fmt);
            }
            else
            {
                rest.push_back(fmt);
            }
        }

        std::vector<std::string> res;
        res.reserve(fmt_names.size());
        res.insert(res.end(), bayer.begin(), bayer.end());
        res.insert(res.end(), mono.begin(), mono.end());
        res.insert(res.end(), rest.begin(), rest.end());
        return res;
    };

    fmt_names = sort_fmt_names(fmt_names);

    std::vector<std::string> natural_formats;

    for (const auto& f : fmt_names)
    {
        auto gst_f = PixelFormat_to_gst_format_string(f.c_str());
        if (gst_f)
        {
            natural_formats.push_back(gst_f);
        }
    }


    auto add_to_caps = [&](GstCaps* caps, const std::vector<std::string>& fmt_names, bool add_device_format=false)
    {

    for (const auto& f : fmt_names)
    {
        auto fmt = get_entry(f.c_str());

        if (!fmt)
        {
            GST_ERROR("Unable to process pfnc format %s. Skipping.", f.c_str());
            continue;
        }

        GstStructure* struc_base = gst_structure_new(fmt->gst_name,
                                            "format", G_TYPE_STRING, fmt->gst_format,
                                            nullptr);

        // 
        // device-format
        //

        if (add_device_format)
        {
            GValue format_list = G_VALUE_INIT;
            g_value_init(&format_list, GST_TYPE_LIST);

            for (const auto& e : natural_formats)
            {
                GValue entry = G_VALUE_INIT;
                g_value_init(&entry, G_TYPE_STRING);

                g_value_set_string(&entry, e.c_str());

                gst_value_list_append_value(&format_list, &entry);

            }

            gst_structure_set_value(struc_base, "device-format", &format_list);

        }

        // 
        // fps
        //

        GValue val_fps = G_VALUE_INIT;
        g_value_init(&val_fps, GST_TYPE_FRACTION_RANGE);
        gst_value_set_fraction_range_full(
            &val_fps, fps_min_num, fps_min_den, fps_max_num, fps_max_den);

        gst_structure_take_value(struc_base, "framerate", &val_fps);

        //
        // width / height
        //

        // helper function
        // adding resolutions needs to be done multiple times for binning
        auto add_res_range = [caps, struc_base] (int width_min, int width_max, int width_step,
                                                int height_min, int height_max, int height_step, 
                                                int binning)
        {
            GstStructure* s = gst_structure_copy(struc_base);
            GValue val_width = G_VALUE_INIT;
            GValue val_height = G_VALUE_INIT;

            int w_max = width_max / binning;
            int h_max = height_max / binning;

            g_value_init(&val_width, GST_TYPE_INT_RANGE);
            gst_value_set_int_range_step(&val_width, width_min, w_max, width_step);

            g_value_init(&val_height, GST_TYPE_INT_RANGE);
            gst_value_set_int_range_step(&val_height, height_min, h_max, height_step);


            gst_structure_take_value(s, "width", &val_width);
            gst_structure_take_value(s, "height", &val_height);

            // caps now owns s
            gst_caps_append_structure(caps, s);

        };

        //
        // binning
        //

        for (const auto& binning : gst_binning_entries)
        {
            // no binning is equal to 1x1
            // in that case leave it at that
            if (binning != "1x1")
            {
                GValue b = G_VALUE_INIT;
                g_value_init(&b, G_TYPE_STRING);
                g_value_set_string(&b, binning.c_str());
                gst_structure_set_value(struc_base, "binning", &b);
            }

            if (do_ranges)
            {
                int b = 1;
                if (binning == "2x2")
                {
                    b = 2;
                }
                else if (binning == "4x4")
                {
                    b = 4;
                }

                add_res_range(width_min, width_max, width_step,
                              height_min, height_max, height_step,
                              b);
            }
            else
            {
                // TODO: implement; requires v4l2 provider
            }

        }

        if (do_ranges)
        {

            // g_value_init(&val_width, GST_TYPE_INT_RANGE);
            // gst_value_set_int_range_step(&val_width, width_min, width_max, width_step);

            // g_value_init(&val_height, GST_TYPE_INT_RANGE);
            // ///g_value_init(&val_height, GST_TYPE_L);
            // gst_value_set_int_range_step(&val_height, height_min, height_max, height_step);


            // gst_structure_take_value(struc_base, "width", &val_width);
            // gst_structure_take_value(struc_base, "height", &val_height);

            // gst_caps_append_structure(caps, struc_base);
        }
        else
        {
            GST_INFO("Binning not correctly implemented and thus missing");

            auto min_w = std::min_element(width_values.begin(), width_values.end());
            auto max_w = std::max_element(width_values.begin(), width_values.end());
            auto min_h = std::min_element(height_values.begin(), height_values.end());
            auto max_h = std::max_element(height_values.begin(), height_values.end());

            image_size min_size = { (int)*min_w, (int)*min_h };
            image_size max_size = { (int)*max_w, (int)*max_h };
            image_size step_size = { 1, 1 };
            auto res = get_standard_resolutions(min_size, max_size, step_size);

            if (std::find(res.begin(), res.end(), min_size) == res.end())
            {
                res.insert(res.begin(), min_size);
            }

            if (std::find(res.begin(), res.end(), max_size) == res.end())
            {
                res.insert(res.begin(), max_size);
            }

            GValue val_width = G_VALUE_INIT;
            GValue val_height = G_VALUE_INIT;
            g_value_init(&val_width, GST_TYPE_LIST);
            g_value_init(&val_height, GST_TYPE_LIST);

            for (const auto& r : res)
            {
                GstStructure* s2 = gst_structure_copy(struc_base);

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


                gst_structure_take_value(struc_base, "width", &w);
                gst_structure_take_value(struc_base, "height", &h);

                gst_caps_append_structure(caps, s2);
            }
            // since not appended, must be freed
            gst_structure_free(struc_base);

        }


    }


    };

    add_to_caps(caps, fmt_names);

    std::vector<std::string> artificial_fmt;

    if (std::find(fmt_names.begin(), fmt_names.end(), "BGR8") == fmt_names.end())
    {
        artificial_fmt.push_back("BGR8");
    }

    if (std::find(fmt_names.begin(), fmt_names.end(), "BGRa8") == fmt_names.end())
    {
        artificial_fmt.push_back("BGRa8");
    }
    if (std::find(fmt_names.begin(), fmt_names.end(), "BGRa16") == fmt_names.end())
    {
        artificial_fmt.push_back("BGRa16");
    }

    if (std::find(fmt_names.begin(), fmt_names.end(), "Mono8") == fmt_names.end())
    {
        artificial_fmt.push_back("Mono8");
    }

    if (std::find(fmt_names.begin(), fmt_names.end(), "YCbCr422_8_CbYCrY") == fmt_names.end())
    {
        artificial_fmt.push_back("YCbCr422_8_CbYCrY");
    }
    if (std::find(fmt_names.begin(), fmt_names.end(), "YCbCr420_8_YY_CrCb_Semiplanar") == fmt_names.end())
    {
        artificial_fmt.push_back("YCbCr420_8_YY_CrCb_Semiplanar");
    }


    add_to_caps(caps, artificial_fmt, true);

    return caps;
}
