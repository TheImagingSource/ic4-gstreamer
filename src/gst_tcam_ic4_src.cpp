
#include "ic4/Error.h"
#include "ic4/ImageBuffer.h"
#include "ic4/InitLibrary.h"
#include "ic4_gst_conversions.h"
#include "ic4src_gst_device_provider.h"
#include "gst_tcam_ic4_src.h"
#include "ic4/DeviceEnum.h"
#include "ic4/ImageType.h"
#include "ic4/Grabber.h"
#include "ic4/QueueSink.h"
#include <cstdint>
#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>

#include "ic4_device_state.h"

#include "format.h"

#ifdef ENABLE_TCAM_PROP
#include "ic4_tcam_property.h"
#endif


#ifdef ENABLE_TCAM_STATS

#include <gstmetatcamstatistics.h>

#endif

using namespace ic4;

GST_DEBUG_CATEGORY(ic4_src_debug);
#define GST_CAT_DEFAULT ic4_src_debug



#ifdef ENABLE_TCAM_PROP
G_DEFINE_TYPE_WITH_CODE(
    GstIC4Src, gst_ic4_src, GST_TYPE_PUSH_SRC,
     G_IMPLEMENT_INTERFACE(TCAM_TYPE_PROPERTY_PROVIDER,
                           ic4::gst::ic4_tcam_property_init))

#else
G_DEFINE_TYPE_WITH_CODE(
                        GstIC4Src, gst_ic4_src, GST_TYPE_PUSH_SRC, nullptr)

#endif

enum {
    SIGNAL_DEVICE_OPEN,
    SIGNAL_DEVICE_CLOSE,
    SIGNAL_LAST,
};

enum {
    PROP_0,
    PROP_IDENT,
    PROP_DEVICE_TYPE,
    PROP_DEVICE_PROP,
};

static guint gst_ic4src_signals[SIGNAL_LAST] = {
    0,
};

static GstStaticPadTemplate ic4_src_template = GST_STATIC_PAD_TEMPLATE(
                                                                       "src", GST_PAD_SRC, GST_PAD_ALWAYS, GST_STATIC_CAPS("ANY"));


static bool ic4_src_open_camera(GstIC4Src *self)
{
    if (!self->device->open_device())
    {
        return false;
    }

    auto lost_cb = [self] (Grabber& )
    {
        if (!self->device->is_streaming())
        {
            return;
        }

        auto serial = self->device->get_ident();

        // set serial as args entry and in actual message
        // that way users have multiple ways of accessing the serial
        GST_ELEMENT_ERROR_WITH_DETAILS(
            GST_ELEMENT(self), RESOURCE, NOT_FOUND,
            ("Device lost (%s)", serial.c_str()), ((nullptr)),
            ("serial", G_TYPE_STRING, serial.c_str(), nullptr));

        self->device->streaming_ = false;

        // the device is considered lost.
        // might as well inform via all possible channels to keep
        // property queries, etc from appearing while everything is shutting
        // down
        g_signal_emit(G_OBJECT(self),
                      gst_ic4src_signals[SIGNAL_DEVICE_CLOSE], 0);

        // do not send EOS here
        // this can cause a deadlock in the tcambin state handling
        // EOS will be triggered in mainsrc_create and transmitted through the
        // capture thread this has been triggered by setting
        // self->device->is_running to false

        // do not call stop
        // some users experience segfaults
        // let EOS handle this. gstreamer will call stop for us
    };

    self->device->dev_lost_token_ = self->device->grabber->eventAddDeviceLost(lost_cb);

    g_signal_emit(G_OBJECT(self), gst_ic4src_signals[SIGNAL_DEVICE_OPEN], 0);

    return true;
}


static void ic4_src_close_camera(GstIC4Src *self) {

    if (!self->device)
    {
        return;
    }

    if (self->device->grabber->isStreaming())
    {
        self->device->grabber->streamStop();
    }

    self->device->streaming_ = false;

    g_signal_emit(G_OBJECT(self),
                  gst_ic4src_signals[SIGNAL_DEVICE_CLOSE],
                  0);

    self->device->grabber->eventRemoveDeviceLost(self->device->dev_lost_token_);

    self->device->grabber = nullptr;
}


static GstCaps* gst_ic4_src_fixate_caps(GstBaseSrc* bsrc, GstCaps* caps)
{
    GstIC4Src* self = GST_IC4_SRC(bsrc);

    GstStructure* structure = nullptr;
    gint width = 0;
    gint height = 0;
    double frame_rate = 0.0;

    structure = gst_caps_get_structure(
        caps,
        0); // #TODO this seems to be at best curious, at another place we fixate to highest, and here fixate goes to lowest

    if (gst_structure_has_field(structure, "width"))
    {
        gst_structure_fixate_field_nearest_int(structure, "width", width);
    }
    if (gst_structure_has_field(structure, "height"))
    {
        gst_structure_fixate_field_nearest_int(structure, "height", height);
    }
    if (gst_structure_has_field(structure, "framerate"))
    {
        gst_structure_fixate_field_nearest_fraction(
            structure, "framerate", (double)(0.5 + frame_rate), 1);
    }

    GST_DEBUG_OBJECT(self, "Fixated caps to %s", gst_helper::to_string(*caps).c_str());

    return GST_BASE_SRC_CLASS(gst_ic4_src_parent_class)->fixate(bsrc, caps);
}


static gboolean gst_ic4_src_negotiate(GstBaseSrc* basesrc)
{
    /* first see what is possible on our source pad */
    GstCaps* src_caps = gst_pad_query_caps(GST_BASE_SRC_PAD(basesrc), NULL);

    // nothing or anything is allowed, we're done
    if (gst_caps_is_empty(src_caps) || gst_caps_is_any(src_caps))
    {
        GST_DEBUG_OBJECT(basesrc, "no negotiation needed");
        if (gst_caps_is_empty(src_caps))
        {
            GST_DEBUG_OBJECT(basesrc, "Our caps are EMPTY. This should not happen");
        }
        if (src_caps)
        {
            gst_caps_unref(src_caps);
        }
        return TRUE;
    }

    GstCaps* caps = nullptr;
    gboolean result = FALSE;

    GstCaps* peercaps = gst_pad_peer_query_caps(GST_BASE_SRC_PAD(basesrc), nullptr);

    GST_DEBUG_OBJECT(basesrc, "caps of peer: %s", gst_caps_to_string(peercaps));
    GST_DEBUG_OBJECT(basesrc, "caps of src: %s", gst_caps_to_string(src_caps));

    if (!gst_caps_is_empty(peercaps) && !gst_caps_is_any(peercaps))
    {
        GstCaps* tmp = gst_caps_intersect_full(src_caps, peercaps, GST_CAPS_INTERSECT_FIRST);
        //GST_DEBUG("tmp intersect: %" GST_PTR_FORMAT, static_cast<void*>(tmp));
        GstCaps* icaps = nullptr;

        int caps_count = static_cast<int>(gst_caps_get_size(tmp));

        /* Prefer the first caps we are compatible with that the peer proposed */
        for (int i = caps_count - 1; i >= 0; i--)
        {
            /* get intersection */
            GstCaps* ipcaps = gst_caps_copy_nth(tmp, i);

            /* Sometimes gst_caps_is_any returns FALSE even for ANY caps?!?! */
            bool is_any_caps = gst_helper::to_string(*ipcaps) == "ANY";

            if (gst_caps_is_any(ipcaps) || is_any_caps || gst_caps_is_empty(ipcaps))
            {
                continue;
            }

            GST_DEBUG("peer: %" GST_PTR_FORMAT, static_cast<void*>(ipcaps));

            //icaps = gst_caps_intersect(src_caps, ipcaps);
            icaps = gst_caps_intersect_full(src_caps, ipcaps, GST_CAPS_INTERSECT_FIRST);
            gst_caps_unref(ipcaps);

            if (icaps && !gst_caps_is_empty(icaps))
            {
                break;
            }
            gst_caps_unref(icaps);
            icaps = nullptr;
        }

        GST_DEBUG("intersect: %" GST_PTR_FORMAT, static_cast<void*>(icaps));

        if (icaps)
        {
            /* If there are multiple intersections pick the one with the smallest
             * resolution strictly bigger then the first peer caps */
            if (gst_caps_get_size(icaps) > 1)
            {
                int best = 0;
                int width = 0, height = 0;

                /* Walk the structure backwards to get the first entry of the
                 * smallest resolution bigger (or equal to) the preferred resolution)
                 */
                for (gint i = (gint)gst_caps_get_size(icaps) - 1; i >= 0; i--)
                {
                    GstStructure* is = gst_caps_get_structure(icaps, i);
                    int w, h;

                    if (gst_structure_get_int(is, "width", &w)
                        && gst_structure_get_int(is, "height", &h))
                    {
                        if (w >= width && h >= height)
                        {
                            width = w;
                            height = h;
                            best = i;
                        }
                    }
                }

                caps = gst_caps_copy_nth(icaps, best);
                gst_caps_unref(icaps);
            }
            else
            {
                // ensure that there is no range but a high resolution with adequate framerate

                int best = 0;
                int twidth = 0, theight = 0;
                int width = G_MAXINT, height = G_MAXINT;

                /* Walk the structure backwards to get the first entry of the
                 * smallest resolution bigger (or equal to) the preferred resolution)
                 */
                for (guint i = 0; i >= gst_caps_get_size(icaps); i++)
                {
                    GstStructure* is = gst_caps_get_structure(icaps, i);
                    int w, h;

                    if (gst_structure_get_int(is, "width", &w)
                        && gst_structure_get_int(is, "height", &h))
                    {
                        if (w >= twidth && w <= width && h >= theight && h <= height)
                        {
                            width = w;
                            height = h;
                            best = i;
                        }
                    }
                }

                /* caps = icaps; */
                caps = gst_caps_copy_nth(icaps, best);

                GstStructure* structure;
                double frame_rate = G_MAXINT;

                structure = gst_caps_get_structure(caps, 0);

                if (gst_structure_has_field(structure, "width"))
                {
                    gst_structure_fixate_field_nearest_int(structure, "width", G_MAXUINT);
                }
                if (gst_structure_has_field(structure, "height"))
                {
                    gst_structure_fixate_field_nearest_int(structure, "height", G_MAXUINT);
                }
                if (gst_structure_has_field(structure, "framerate"))
                {
                    gst_structure_fixate_field_nearest_fraction(
                        structure, "framerate", frame_rate, 1);
                }
                gst_caps_unref(icaps);
            }
        }
        gst_caps_unref(tmp);

        gst_caps_unref(src_caps);
    }
    else
    {
        /* no peer or peer have ANY caps, work with our own caps then */
        caps = src_caps;
    }

    if (peercaps)
    {
        gst_caps_unref(peercaps);
    }

    if (caps)
    {
        caps = gst_caps_truncate(caps);

        /* now fixate */
        if (!gst_caps_is_empty(caps))
        {
            caps = gst_ic4_src_fixate_caps(basesrc, caps);
            //GST_DEBUG_OBJECT(self, "fixated to: %" GST_PTR_FORMAT, static_cast<void*>(caps));

            if (gst_caps_is_any(caps))
            {
                /* hmm, still anything, so element can do anything and
                 * nego is not needed */
                result = TRUE;
            }
            else if (gst_caps_is_fixed(caps))
            {
                /* yay, fixed caps, use those then */
                result = gst_base_src_set_caps(basesrc, caps);
            }
        }
        gst_caps_unref(caps);
    }
    return result;
}


static GstCaps* gst_ic4_src_get_caps (GstBaseSrc* src,
                                      GstCaps* filter __attribute__((unused)))
{
    GstIC4Src* self = GST_IC4_SRC(src);

    auto caps = self->device->get_caps();

    // GST_DEBUG("Returning device caps: %s", gst_caps_to_string(caps));
  
    return caps;
}


static gboolean gst_ic4_src_set_caps(GstBaseSrc* src, GstCaps* caps)
{
    GstIC4Src *self = GST_IC4_SRC(src);

    GstStructure* struc = gst_caps_get_structure(caps, 0);

    GST_INFO("set_caps: %s", gst_structure_to_string(struc));

    int width;
    int height;

    gst_structure_get_int(struc, "width", &width);
    gst_structure_get_int(struc, "height", &height);

    int num;
    int denom;

    gst_structure_get_fraction(struc, "framerate", &num, &denom);

    double fps = (double)num/denom;

    auto p = self->device->grabber->devicePropertyMap();

    const ic4::PixelFormat fmt = ic4::gst::gst_caps_to_pixel_format(*caps);

    if (fmt == ic4::PixelFormat::Invalid)
    {
        GST_ERROR("Given gst caps can not be associated with ic4 format descriptions!");
        return FALSE;
    }
    
    p.setValue("Width", width);
    p.setValue("Height", height);
    p.setValue("AcquisitionFrameRate", fps);

    // if (gst_structure_has_field(struc, "binning"))
    // {
    //     std::string field_value = gst_structure_get_string(struc, "binning");

    //     uint32_t to_fill_horizontal;
    //     uint32_t to_fill_vertical;

    //     const std::string delimiter = "x";
    //     std::string token_horizontal = field_value.substr(0, field_value.find(delimiter));
    //     std::string token_vertical = field_value.substr(field_value.find(delimiter) + 1);

    //     try
    //     {
    //         to_fill_horizontal = std::atoi(token_horizontal.c_str());
    //         to_fill_vertical = std::atoi(token_vertical.c_str());
    //     }
    //     catch (const std::exception& e)
    //     {
    //         GST_ERROR("Error while handling binning string '%s'", field_value.c_str());
    //         to_fill_horizontal = 1;
    //         to_fill_vertical = 1;
    //     }
    //     p.setValue("BinningHorizontal", to_fill_horizontal);
    //     p.setValue("BinningVertical", to_fill_vertical);
    // }
    // else
    // {
    //     // ensure we are always in a defined state
    //     p.setValue("BinningHorizontal", 1);
    //     p.setValue("BinningVertical", 1);
    // }


    ic4::QueueSinkListener& listener = *self->device->listener.get();

    PixelFormat sink_format = fmt;

    const char* dev_fmt_str = gst_structure_get_string(struc, "device-format");

    if (dev_fmt_str)
    {
        auto dev_fmt = ic4::gst::gst_format_to_pixel_format(dev_fmt_str);

        if (dev_fmt == ic4::PixelFormat::Invalid)
        {
            GST_ERROR("Unable to convert \"%s\" to a valid ic4 PixelFormat. "
                      "Please sekect a different device-format.",
                      dev_fmt_str);
            return FALSE;
        }
        auto name = ic4::to_string(dev_fmt);

        GST_INFO("Setting device-caps to %s", name.c_str());

        //
        // iterate over device formats to ensure
        // the given caps::device-format is valid
        // 
        auto p_fmt = p.findEnumeration("PixelFormat");

        bool is_valid = false;

        for (const auto& f : p_fmt.entries())
        {
            if (f.name() == name)
            {
                GST_INFO("device format will be %s", f.name().c_str());
                p.setValue("PixelFormat", name);
                is_valid = true;

                break;
            }
        }
        if (!is_valid)
        {
            GST_ERROR("Given device-caps are not supported by the device! \"%s\"", dev_fmt_str);
            return false;
        }

    }
    else
    {
        //GST_INFO("No device-format given. Attempting caps \"%s\"as device PixelFormat.", ic4::to_string(fmt).c_str());

        //
        // iterate over device formats to ensure
        // the given caps::device-format is valid
        //
        auto p_fmt = p.findEnumeration("PixelFormat");

        bool is_valid = false;

        for (const auto& f : p_fmt.entries())
        {
            if (f.intValue() == (int64_t)fmt)
            {
                p.setValue("PixelFormat", f.name());
                is_valid = true;
                break;
            }

        }
        if (!is_valid)
        {
            GST_ERROR("Given caps are not in the device PixelFormat list. IC4 will attempt a conversion.");

            GST_INFO("IC4 conversion from %s to %s",
                     ic4::to_string(ic4::PixelFormat((int32_t)p.getValueInt64(ic4::PropId::PixelFormat))).c_str(),
                     ic4::to_string(sink_format).c_str());
        }

    }

    self->device->sink = ic4::QueueSink::create(listener, sink_format);

    self->device->grabber->streamSetup(self->device->sink);

    return TRUE;
}


static GstStateChangeReturn
gst_ic4_src_change_state(GstElement* element, GstStateChange change)
{
    GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;
    GstIC4Src *self = GST_IC4_SRC(element);

    switch (change)
    {
        case GST_STATE_CHANGE_NULL_TO_READY:
        {
            if (!ic4_src_open_camera(self))
            {
                GST_ERROR("Unable to open requested device.");
                return GST_STATE_CHANGE_FAILURE;
            }
            break;
        }
        case GST_STATE_CHANGE_READY_TO_PAUSED:
        {
            ret = GST_STATE_CHANGE_NO_PREROLL;
            break;
        }
        case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
        {
            break;
        }
        default:
        {
            break;
        }
    }

    gst_element_set_locked_state(element, TRUE);
    ret = GST_ELEMENT_CLASS(gst_ic4_src_parent_class)->change_state(element, change);
    gst_element_set_locked_state(element, FALSE);

    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        return ret;
    }

    switch (change)
    {
        case GST_STATE_CHANGE_PAUSED_TO_READY:
        {
            //GST_INFO("paused->ready");

            self->device->streaming_ = false;
            if (self->device->grabber->isStreaming())
            {
                self->device->grabber->streamStop();
            }
            break;
        }
        case GST_STATE_CHANGE_READY_TO_NULL:
        {
            //GST_INFO("ready->null");

            if (self->device->is_open())
            {
                ic4_src_close_camera(self);
            }
            break;
        }
        default:
        {
            break;
        }
    }

    return ret;
}

struct destroy_transfer
{
    GstIC4Src* self;
    std::shared_ptr<ic4::ImageBuffer> frame;
};


static void buffer_destroy(gpointer data)
{
    destroy_transfer* trans = static_cast<destroy_transfer*>(data);

    delete trans;
}


static GstFlowReturn gst_ic4_src_create(GstPushSrc* push_src, GstBuffer** buffer)
{

    GstIC4Src* self = GST_IC4_SRC(push_src);

    int cnt = 0;
    int cnt_max = 10;

get_buf:
    std::unique_lock<std::mutex> lck(self->device->stream_mtx_);

    self->device->stream_cv_.wait(lck);

    if (!self->device->is_streaming())
    {
        return GST_FLOW_EOS;
    }

    ic4::Error err;
    auto frame = self->device->sink->popOutputBuffer(err);

    if (!frame)
    {
        GST_ERROR("Unable to retrieve buffer: %s", err.message().c_str());

        if (cnt >= cnt_max)
        {

            //return GST_FLOW_ERROR;
        }
        cnt++;
        goto get_buf;
    }
    cnt = 0;

    destroy_transfer* trans = new destroy_transfer;
    trans->self = self;
    trans->frame = frame;

    GstBuffer* new_buf =
        gst_buffer_new_wrapped_full(static_cast<GstMemoryFlags>(GST_MEMORY_FLAG_READONLY),
                                    frame->ptr(),
                                    frame->bufferSize(),
                                    0,
                                    frame->bufferSize(),
                                    trans,
                                    buffer_destroy);

#ifdef ENABLE_TCAM_STATS

    ic4::ImageBuffer::MetaData meta_data = frame->metaData(err);
    if (err.isSuccess())
    {
        GstStructure* struc = gst_structure_new_empty("TcamStatistics");

        gst_structure_set(struc,
                          "frame_count",
                          G_TYPE_UINT64,
                          meta_data.device_frame_number,
                          "frames_dropped",
                          G_TYPE_UINT64,
                          0, // potentially implementable
                          "capture_time_ns",
                          G_TYPE_UINT64,
                          0, // driver time not supported by gentl
                          "camera_time_ns",
                          G_TYPE_UINT64,
                          meta_data.device_timestamp_ns,
                          "is_damaged",
                          G_TYPE_BOOLEAN,
                          FALSE, // seen as not supported, at this time
                          nullptr);

        gst_buffer_add_tcam_statistics_meta(new_buf, struc);
    }

#endif

    *buffer = new_buf;
    gst_buffer_set_flags(*buffer, GST_BUFFER_FLAG_LIVE);

    //GST_INFO("Create func end");

    return GST_FLOW_OK;
}


bool is_gst_state_equal_or_greater(GstElement* self, GstState state) noexcept
{
    GstState cur_state = GST_STATE_NULL;
    auto res = gst_element_get_state(self, &cur_state, NULL, GST_CLOCK_TIME_NONE);
    if (res == GST_STATE_CHANGE_FAILURE)
    {
        return false;
    }
    return cur_state >= state;
}

bool is_gst_state_equal_or_less(GstElement* self, GstState state) noexcept
{
    GstState cur_state = GST_STATE_NULL;
    auto res = gst_element_get_state(self, &cur_state, NULL, GST_CLOCK_TIME_NONE);
    if (res == GST_STATE_CHANGE_FAILURE)
    {
        return false;
    }
    return cur_state <= state;
}

static bool is_state_null(GstIC4Src* self)
{
    return is_gst_state_equal_or_less(GST_ELEMENT(self), GST_STATE_NULL);
}


static void gst_ic4_src_set_property(GObject* object,
                                     guint prop_id,
                                     const GValue* value,
                                     GParamSpec* pspec)
{
    GstIC4Src* self = GST_IC4_SRC(object);

    switch (prop_id)
    {
        case PROP_IDENT:
        {
            if (!is_state_null(self))
            {
                GST_ERROR_OBJECT(self, "GObject property 'ident' is not writable "
                                 "in state >= GST_STATE_READY.");
                return;
            }
            if (g_value_get_string(value) == nullptr)
            {
                self->device->set_ident(std::string{});
            }
            else
            {
                std::string string_value = g_value_get_string(value);

                if (!self->device->set_ident(string_value))
                {
                    GST_ERROR("Unable to open device");
                }
            }
            break;
        }
        case PROP_DEVICE_PROP:
        {
            std::string string_value = g_value_get_string(value);
            self->device->set_properties_from_string(string_value);
            break;
        }
        default: {
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
        }
    }
}

static void gst_ic4_src_get_property(GObject* object,
                                     guint prop_id,
                                     GValue* value,
                                     GParamSpec* pspec)
{
    GstIC4Src* self = GST_IC4_SRC(object);

    switch (prop_id)
    {
        case PROP_IDENT:
        {
            g_value_set_string(value, self->device->get_ident().c_str());
            break;
        }
        case PROP_DEVICE_TYPE:
        {
            g_value_set_string(value, "ic4");
            break;
        }
        default:
        {
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
        }
    }
}

static void gst_ic4_src_init(GstIC4Src* self)
{
    gst_base_src_set_live(GST_BASE_SRC(self), TRUE);
    gst_base_src_set_format(GST_BASE_SRC(self), GST_FORMAT_TIME);

    ic4::initLibrary();

    self->device = new ic4_device_state();
}

static void gst_ic4_src_finalize(GObject *object)
{

    GstIC4Src* self = GST_IC4_SRC(object);

    if (self->device)
    {
        delete self->device;
        self->device = nullptr;
    }

    ic4::exitLibrary();
}

static void gst_ic4_src_class_init(GstIC4SrcClass *klass)
{
    GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
    GstElementClass* element_class = GST_ELEMENT_CLASS(klass);
    GstBaseSrcClass* gstbasesrc_class = GST_BASE_SRC_CLASS(klass);
    GstPushSrcClass* gstpushsrc_class = GST_PUSH_SRC_CLASS(klass);

    gobject_class->finalize = gst_ic4_src_finalize;
    gobject_class->set_property = gst_ic4_src_set_property;
    gobject_class->get_property = gst_ic4_src_get_property;

    g_object_class_install_property(
        gobject_class, PROP_IDENT,
        g_param_spec_string("ident", "Camera identifier", "Identifier of the camera",
                            NULL,
                            static_cast<GParamFlags>(G_PARAM_READWRITE |
                                                     G_PARAM_STATIC_STRINGS)));

    g_object_class_install_property(
        gobject_class, PROP_DEVICE_TYPE,
        g_param_spec_string("type", "Camera type", "type/backend of the camera",
                            "ic4",
                            static_cast<GParamFlags>(G_PARAM_READABLE |
                                                     G_PARAM_STATIC_STRINGS)));

    g_object_class_install_property(gobject_class,
                                    PROP_DEVICE_PROP,
                                    g_param_spec_string("prop", "Camera properties", "properties of the camera",
                                    NULL,
                                    static_cast<GParamFlags>(G_PARAM_WRITABLE |
                                                             G_PARAM_STATIC_STRINGS)));

    gst_ic4src_signals[SIGNAL_DEVICE_OPEN] =
        g_signal_new("device-open", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_LAST,
                     0, nullptr, nullptr, nullptr, G_TYPE_NONE, 0, G_TYPE_NONE);
    gst_ic4src_signals[SIGNAL_DEVICE_CLOSE] =
        g_signal_new("device-close", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_LAST,
                     0, nullptr, nullptr, nullptr, G_TYPE_NONE, 0, G_TYPE_NONE);

    GST_DEBUG_CATEGORY_INIT(ic4_src_debug, "ic4src", 0,
                            "tcam interface");

    gst_element_class_set_static_metadata(
        element_class, "IC4 Video Source", "Source/Video", "IC4 based source",
        "The Imaging Source <support@theimagingsource.com>");

    gst_element_class_add_pad_template(
        element_class, gst_static_pad_template_get(&ic4_src_template));

    element_class->change_state = gst_ic4_src_change_state;

    gstbasesrc_class->get_caps = gst_ic4_src_get_caps;
    gstbasesrc_class->set_caps = gst_ic4_src_set_caps;
    gstbasesrc_class->fixate = gst_ic4_src_fixate_caps;
    gstbasesrc_class->negotiate = gst_ic4_src_negotiate;

    gstpushsrc_class->create = gst_ic4_src_create;
}


static gboolean plugin_init(GstPlugin* plugin)
{
    gst_device_provider_register(plugin, "ic4srcdeviceprovider",
                                 GST_RANK_PRIMARY,
                                 TYPE_IC4_SRC_DEVICE_PROVIDER);
    gst_element_register(plugin, "ic4src", GST_RANK_PRIMARY,
                         GST_TYPE_IC4_SRC);

    GST_DEBUG_CATEGORY_INIT(ic4_src_debug, "ic4src", 0,
                            "tcam interface");

    return TRUE;
}

#ifndef PACKAGE
#define PACKAGE "ic4src"
#endif

GST_PLUGIN_DEFINE(GST_VERSION_MAJOR, GST_VERSION_MINOR, ic4src,
                  "IC4 Video Source", plugin_init, IC4SRC_VERSION, "Proprietary",
                  "ic4src", "theimagingsource.com")
