
#include "ic4_gst_conversions.h"
#include "format.h"
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

bool format_is_bayer(const std::string &format)
{
    return format.find("Bayer") != std::string::npos;
}


bool format_is_mono(const std::string &format)
{
    return format.find("Mono") != std::string::npos;
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
    // auto p_bin_x = props.findInteger("BinningHorizontal");
    // auto p_bin_y = props.findInteger("BinningVertical");
    // bool has_binning = p_bin_x.is_valid();
    // std::vector<std::string> gst_binning_entries;

    // if (has_binning)
    // {
    //     // GST_INFO("HAS BINNING");

    //     // there are cameras with
    //     // BinningHorizontal == 1
    //     // BinningVertical == 1
    //     // for those we ignore binning
    //     if (p_bin_x.minimum() == p_bin_x.maximum())
    //     {
    //         gst_binning_entries.push_back("1x1");
    //         GST_DEBUG("only binning 1x1 exists");
    //         has_binning = false;
    //     }
    //     else
    //     {

    //     }
    // }
    // else
    // {
    //     gst_binning_entries.push_back("1x1");
    //     GST_INFO("No binning");
    // }


    // auto p_skip_x = props.findInteger("DecimationHorizontal");
    // auto p_skip_y = props.findInteger("DecimationVertical");
    // bool has_skipping = p_skip_x.is_valid();
    // std::vector<std::string> gst_skipping_entries;

    // if (has_skipping)
    // {
    //     GST_INFO("HAS SKIPPING");
    // }
    // else
    // {
    //     GST_INFO("NO SKIPPING");
    // }


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

    // if (has_binning)
    // {
    //     const auto& bin_x = p_bin_x.asInteger();
    //     auto entries = bin_x.validValueSet();

    //     // not a valueSet; determine via range
    //     if (entries.empty())
    //     {
    //         for (int i = bin_x.minimum(); i <= bin_x.maximum(); i++)
    //         {
    //             // TODO: does a camera exist with 8x binning
    //             if (i == 3)
    //             {
    //                 continue;
    //             }

    //             entries.push_back(i);
    //         }

    //     }

    //     for (const auto& e: entries)
    //     {
    //         gst_binning_entries.push_back(fmt::format("{}x{}", e, e));
    //     }

    //     // //gst_value_array_init(&binning, gst_binning_entries.size());
    //     // gst_value_list_init(&binning, gst_binning_entries.size());

    //     // // transform entries into something gstreamer can use
    //     // for (const auto& e: gst_binning_entries)
    //     // {
    //     //     GValue entry = G_VALUE_INIT;
    //     //     g_value_init(&entry, G_TYPE_STRING);
    //     //     g_value_set_string(&entry, e.c_str());

    //     //     gst_value_list_append_value(&binning, &entry);
    //     //     //gst_value_array_append_value(&binning, &entry);
    //     // }

    // }

    GstCaps* caps = gst_caps_new_empty();

    // we either have both or none
    // values sets only exist for v4l2/fpd cameras
    assert(height_values.empty() == width_values.empty());

    bool do_ranges = true;
    if (!width_values.empty())
    {
        do_ranges = false;
    }

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
        auto gst_f = pixel_format_name_to_gst_format(f);
        if (gst_f)
        {
            natural_formats.push_back(gst_f);
        }
    }


    auto add_to_caps = [&](GstCaps* caps, const std::vector<std::string>& fmt_names, bool add_device_format=false)
    {

    for (const auto& f : fmt_names)
    {
        auto fmt_ret = get_entry_by_pixel_format_name(f);

        if (!fmt_ret)
        {
            GST_ERROR("Unable to process pfnc format %s. Skipping.", f.c_str());
            continue;
        }

        auto fmt = fmt_ret.value();

        GstStructure* struc_base = gst_structure_new(fmt.gst_name,
                                            "format", G_TYPE_STRING, fmt.gst_format,
                                            nullptr);

        // 
        // device-format
        //

        if (add_device_format)
        {
            GValue format_list = G_VALUE_INIT;
            g_value_init(&format_list, GST_TYPE_LIST);

            for (const auto &dev_fmt : natural_formats)
            {
                auto in = gst_format_to_pixel_format(dev_fmt.c_str());

                if (!ic4::canTransform(in, fmt.ic4_format))
                {
                    continue;
                }
                GValue entry = G_VALUE_INIT;
                g_value_init(&entry, G_TYPE_STRING);

                g_value_set_string(&entry, dev_fmt.c_str());

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

        // helper function
        // adding resolutions needs to be done multiple times for binning
        auto add_fixed_res = [caps, struc_base] (int width,
                                                 int height,
                                                 int binning)
        {
            GstStructure* s = gst_structure_copy(struc_base);
            GValue val_width = G_VALUE_INIT;
            GValue val_height = G_VALUE_INIT;

            g_value_init(&val_width, G_TYPE_INT);
            g_value_set_int(&val_width, width);

            g_value_init(&val_height, G_TYPE_INT);
            g_value_set_int(&val_height, height);


            gst_structure_take_value(s, "width", &val_width);
            gst_structure_take_value(s, "height", &val_height);

            // caps now owns s
            gst_caps_append_structure(caps, s);

        };
        //
        // binning
        //

        // for (const auto& binning : gst_binning_entries)
        // {
        //     // no binning is equal to 1x1
        //     // in that case leave it at that
        //     if (binning != "1x1")
        //     {
        //         GValue b = G_VALUE_INIT;
        //         g_value_init(&b, G_TYPE_STRING);
        //         g_value_set_string(&b, binning.c_str());
        //         gst_structure_set_value(struc_base, "binning", &b);
        //     }

        //     if (do_ranges)
        //     {
        //         int b = 1;
        //         if (binning == "2x2")
        //         {
        //             b = 2;
        //         }
        //         else if (binning == "4x4")
        //         {
        //             b = 4;
        //         }

        if (width_min == width_max)
        {
            add_fixed_res(width_min, height_min, 1);
        }
        else
        {
            add_res_range(width_min, width_max, width_step,
                          height_min, height_max, height_step,
                          1);
        }
        //     }
        //     else
        //     {
        //         // TODO: implement; requires v4l2 provider
        //     }

        // }

        //
        // end binning
        //

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
            // GST_INFO("Binning not correctly implemented and thus missing");

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

                GValue h = G_VALUE_INIT;
                g_value_init(&h, G_TYPE_INT);
                g_value_set_int(&h, r.height);

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

    // add caps that we can transform into
    // we do this by asking ic4 for supported transform pixelformats

    std::vector<std::string> artificial_fmt;

    for (const auto &dev_fmt : p_fmt.entries())
    {

        auto dev_pix = ic4::PixelFormat(dev_fmt.intValue());
        auto transform_fmts = ic4::enumTransforms(dev_pix);

        // debug print input-> available conversion
        // {
        //     std::string s;
        //     for (const auto &t : transform_fmts)
        //     {

        //         s += ic4::to_string(t);
        //         s += " ";
        //     }

        //     GST_ERROR("%s ----> {%s}", ic4::to_string(dev_pix).c_str(), s.c_str());
        // }
        for (const auto &t : transform_fmts)
        {
            if (t == dev_pix)
            {
                continue;
            }
            if (std::find_if(artificial_fmt.begin(), artificial_fmt.end(),
                             [t](const auto &name) {
                                 return name == ic4::to_string(t);}) != artificial_fmt.end())
            {
                continue;
            }
            artificial_fmt.push_back(ic4::to_string(t));
        }
    }

    // make ordering predictable
    artificial_fmt = sort_fmt_names(artificial_fmt);

    add_to_caps(caps, artificial_fmt, true);

    return caps;
}
