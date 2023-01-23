
#include "gst/gstmemory.h"
#include "gst/gststructure.h"
#include "ic4/Error.h"
#include "ic4/Frame.h"
#include "ic4/InitLibrary.h"
#include "ic4_gst_conversions.h"
#include "ic4src_gst_device_provider.h"
#include "gst_tcam_ic4_src.h"
#include "gst/gstcaps.h"
#include "gst/gstelement.h"
#include "gst/gstinfo.h"
#include "gst/gstpad.h"
#include "ic4/DeviceEnum.h"
#include "ic4/FrameType.h"
#include "ic4/Grabber.h"
#include "ic4/QueueSink.h"
#include <memory>
#include <atomic>
#include <mutex>
#include <condition_variable>

#include "../libs/gst-helper/include/gst-helper/gstelement_helper.h"
#include "../libs/gst-helper/include/gst-helper/helper_functions.h"

using namespace ic4;

GST_DEBUG_CATEGORY(tcam_ic4_src_debug);
#define GST_CAT_DEFAULT tcam_ic4_src_debug

struct sink_listener;

std::atomic<bool> _queued = false;

struct ic4_device_state {

    ///ic4::VideoCaptureDeviceItem

    std::shared_ptr<ic4::Grabber> grabber;

    std::shared_ptr<ic4::QueueSink> sink;

    std::shared_ptr<sink_listener> listener;
    void* dev_lost_token_ = nullptr;

    std::atomic<bool> streaming_ = false;
    std::mutex stream_mtx_;
    std::condition_variable stream_cv_;

    std::string serial_;

    std::string get_serial ()
    {
        return serial_;
    };

    bool set_serial (const std::string s)
    {
        if (is_open())
        {
            return false;
        }
        GST_DEBUG("Device serial is now: %s", s.c_str());
        serial_ = s;
        return true;
    };


    bool is_open ()
    {
        if (grabber)
        {
            return grabber->isDevOpen();
        }
        return false;
    };

    bool is_streaming ()
    {
        return streaming_;
    }

    GstCaps* get_caps ()
    {
        if (!is_open())
        {
            GST_WARNING("==== Device not open");
            return nullptr;
        }
        auto props = grabber->devPropertyMap();
        return ic4::gst::create_caps(props);
    }
};

struct sink_listener : public ic4::QueueSinkListener {

  bool sinkConnected(ic4::QueueSink &sink,
                     const ic4::FrameTypeInfo &frameType) {
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
        _queued = true;
        state->streaming_=true;
        state->stream_cv_.notify_all();
    };

    ic4_device_state* state;
};


G_DEFINE_TYPE_WITH_CODE(
    GstTcamIC4Src, gst_tcam_ic4_src, GST_TYPE_PUSH_SRC, NULL)
    // G_IMPLEMENT_INTERFACE(TCAM_TYPE_PROPERTY_PROVIDER,
    //                       tcam::mainsrc::gst_tcam_mainsrc_tcamprop_init))

enum {
    SIGNAL_DEVICE_OPEN,
    SIGNAL_DEVICE_CLOSE,
    SIGNAL_LAST,
};

enum {
    PROP_0,
    PROP_SERIAL,
};

static guint gst_tcamic4src_signals[SIGNAL_LAST] = {
    0,
};

static GstStaticPadTemplate tcam_ic4_src_template = GST_STATIC_PAD_TEMPLATE(
    "src", GST_PAD_SRC, GST_PAD_ALWAYS, GST_STATIC_CAPS("ANY"));


static void ic4_src_open_camera(GstTcamIC4Src *self)
{

    self->device->grabber = std::make_shared<ic4::Grabber>();

    auto dev_list = ic4::DeviceEnum::getAvailableVideoCaptureDevices();
    if (dev_list.empty())
    {
        GST_ERROR("No devices available");
        return;
    }
    // use first device
    if (self->device->serial_.empty())
    {

        if (!self->device->grabber->openDev(dev_list.at(0)))
        {
            GST_ERROR("Unable to open device");
            return;
        }
        self->device->serial_ = dev_list.at(0).getSerial();
    }
    else
    {
        for (auto& item : dev_list)
        {
            if (item.getSerial() == self->device->serial_)
            {
                if (!self->device->grabber->openDev(item))
                {
                    GST_ERROR("Unable to open device");
                    return;
                }
                break;
            }
        }
    }
    self->device->listener = std::make_shared<sink_listener>();

    self->device->listener->state = self->device;

    auto lost_cb = [self] (Grabber& g)
    {
        if (!self->device->is_streaming())
        {
            return;
        }

        auto serial = self->device->get_serial();

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
                      gst_tcamic4src_signals[SIGNAL_DEVICE_CLOSE], 0);

        // do not send EOS here
        // this can cause a deadlock in the tcambin state handling
        // EOS will be triggered in mainsrc_create and transmitted through the
        // capture thread this has been triggered by setting
        // self->device->is_running to false

        // do not call stop
        // some users experience segfaults
        // let EOS handle this. gstreamer will call stop for us
    };

    self->device->dev_lost_token_ = self->device->grabber->eventRegisterDeviceLost(lost_cb);

    g_signal_emit(G_OBJECT(self), gst_tcamic4src_signals[SIGNAL_DEVICE_OPEN], 0);

}


static void ic4_src_close_camera(GstTcamIC4Src *self) {

    if (!self->device)
    {
        return;
    }

    if (self->device->grabber->isStreaming())
    {
        self->device->grabber->streamStop();
    }

    self->device->streaming_ = false;

    g_signal_emit(G_OBJECT(self), gst_tcamic4src_signals[SIGNAL_DEVICE_CLOSE],
                  0);

    self->device->grabber->eventRemoveDeviceLost(self->device->dev_lost_token_);

    self->device->grabber->closeDev();

    self->device->grabber = nullptr;
}


static GstCaps* gst_tcam_ic4_src_fixate_caps(GstBaseSrc* bsrc, GstCaps* caps)
{
    GstTcamIC4Src* self = GST_TCAM_IC4_SRC(bsrc);

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

    return GST_BASE_SRC_CLASS(gst_tcam_ic4_src_parent_class)->fixate(bsrc, caps);
}


static gboolean gst_tcam_ic4_src_negotiate(GstBaseSrc* basesrc)
{
    GstTcamIC4Src* self = GST_TCAM_IC4_SRC(basesrc);

    /* first see what is possible on our source pad */
    //GstCaps *thiscaps = gst_pad_query_caps(GST_BASE_SRC_PAD(basesrc), NULL);
     gst_pad_query_caps(GST_BASE_SRC_PAD(basesrc), NULL);
    GstCaps* thiscaps = gst_caps_from_string(
"video/x-raw,format=GRAY8,width=320,height=240,framerate={15/1,25/1,30/1,60/1,80/1,120/1,5000/31};"
"video/x-raw,format=GRAY8,width=640,height=480,framerate={5/1,15/2,15/1,25/1,30/1,60/1,87/1};"
"video/x-raw,format=GRAY8,width=744,height=480,framerate={5/1,15/2,15/1,25/1,30/1,60/1,20000/263};"
"video/x-bayer,format=gbrg,width=320,height=240,framerate={15/1,25/1,30/1,60/1,80/1,120/1,5000/31};"
"video/x-bayer,format=gbrg,width=640,height=480,framerate={5/1,15/2,15/1,25/1,30/1,60/1,87/1};"
"video/x-bayer,format=gbrg,width=744,height=480,framerate={5/1,15/2,15/1,25/1,30/1,60/1,20000/263}");

    // nothing or anything is allowed, we're done
    if (gst_caps_is_empty(thiscaps) || gst_caps_is_any(thiscaps))
    {
        GST_DEBUG_OBJECT(basesrc, "no negotiation needed");
        if (thiscaps)
        {
            gst_caps_unref(thiscaps);
        }
        return TRUE;
    }

    GstCaps* caps = nullptr;
    gboolean result = FALSE;

    GstCaps* peercaps = gst_pad_peer_query_caps(GST_BASE_SRC_PAD(basesrc), nullptr);
    GST_DEBUG_OBJECT(basesrc, "caps of peer: %s", gst_caps_to_string(peercaps));

    if (!gst_caps_is_empty(peercaps) && !gst_caps_is_any(peercaps))
    {
        GstCaps* tmp = gst_caps_intersect_full(thiscaps, peercaps, GST_CAPS_INTERSECT_FIRST);

        GstCaps* icaps = NULL;

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

            //GST_DEBUG("peer: %" GST_PTR_FORMAT, static_cast<void*>(ipcaps));

            icaps = gst_caps_intersect_full(thiscaps, ipcaps, GST_CAPS_INTERSECT_FIRST);
            gst_caps_unref(ipcaps);

            if (icaps && !gst_caps_is_empty(icaps))
            {
                break;
            }
            gst_caps_unref(icaps);
            icaps = NULL;
        }

        //GST_DEBUG("intersect: %" GST_PTR_FORMAT, static_cast<void*>(icaps));

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

        gst_caps_unref(thiscaps);
    }
    else
    {
        /* no peer or peer have ANY caps, work with our own caps then */
        caps = thiscaps;
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
            caps = gst_tcam_ic4_src_fixate_caps(basesrc, caps);
            GST_DEBUG_OBJECT(self, "fixated to: %" GST_PTR_FORMAT, static_cast<void*>(caps));

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
    // //gst_tcam_i

    // caps = gst_caps_from_string("video/x-bayer,format=gbrg,width=744,height=480,framerate=30/1 ");

    // gboolean result = gst_base_src_set_caps(basesrc, caps);

    // gst_caps_unref(caps);
    // return TRUE;
}

static GstCaps *gst_tcam_ic4_src_get_caps(GstBaseSrc *src, GstCaps *filter
                                              __attribute__((unused))) {
  GstTcamIC4Src *self = GST_TCAM_IC4_SRC(src);

  auto caps = self->device->get_caps();

  GST_ERROR("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Generated caps: \n%s", gst_caps_to_string(caps));
  // auto caps = self->device->get_device_caps();
  // if (caps == nullptr) {
  //   GST_WARNING_OBJECT(
  //       self, "Device not initialized. Must be in state >= GST_STATE_READY.");
  //   return nullptr;
  // }
  return caps;
}

static gboolean gst_tcam_ic4_src_set_caps(GstBaseSrc *src, GstCaps *caps)
{
    GstTcamIC4Src *self = GST_TCAM_IC4_SRC(src);

    GstStructure* struc = gst_caps_get_structure(caps, 0);

    int width;
    int height;

    gst_structure_get_int(struc, "width", &width);
    gst_structure_get_int(struc, "height", &height);

    int num;
    int denom;

    gst_structure_get_fraction(struc, "framerate", &num, &denom);

    double fps = num/denom;
    //gst_structure_get_double(struc, "framerate", &fps);

    GST_ERROR("SET CAPS");

    auto p = self->device->grabber->devPropertyMap();

    const char* fmt = ic4::gst::caps_to_PixelFormat(*caps);

    p.setEnumerationEntry("PixelFormat", fmt);
    p.setIntegerValue("Width", width);
    p.setIntegerValue("Height", height);
    p.setFloatValue("AcquisitionFrameRate", fps);

    ic4::QueueSinkListener& listener = *self->device->listener.get();

    self->device->sink = ic4::QueueSink::create(listener);

    self->device->grabber->streamSetup(self->device->sink);

    return TRUE;
}


static GstStateChangeReturn
gst_tcam_ic4_src_change_state(GstElement *element, GstStateChange change)
{
    GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;
    GstTcamIC4Src *self = GST_TCAM_IC4_SRC(element);

    switch (change)
    {
        case GST_STATE_CHANGE_NULL_TO_READY:
        {
            GST_INFO("null->ready");
            ic4_src_open_camera(self);
            break;
        }
        case GST_STATE_CHANGE_READY_TO_PAUSED: {
          // self->device->n_buffers_delivered_ = 0; //
          GST_INFO("ready->paused");
          ret = GST_STATE_CHANGE_NO_PREROLL;
          break;
        }
        case GST_STATE_CHANGE_PAUSED_TO_PLAYING:
        {
            //self->device->grabber->s
            break;
        }
        default:
        {
            break;
        }
    }

    gst_element_set_locked_state(element, TRUE);
    ret = GST_ELEMENT_CLASS(gst_tcam_ic4_src_parent_class)->change_state(element, change);
    gst_element_set_locked_state(element, FALSE);

    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        return ret;
    }

    switch (change)
    {
        case GST_STATE_CHANGE_PAUSED_TO_READY:
        {
            GST_INFO("paused->ready");

            self->device->streaming_ = false;
            if (self->device->grabber->isStreaming()) {
                self->device->grabber->streamStop();
            }
            break;
        }
        case GST_STATE_CHANGE_READY_TO_NULL:
        {
            GST_INFO("ready->null");

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

struct destroy_transfer {
    GstTcamIC4Src* self;
    std::shared_ptr<ic4::Frame> frame;
};


static void buffer_destroy(gpointer data) {


    destroy_transfer* trans = static_cast<destroy_transfer*>(data);

    delete trans;

//    GST_INFO("Freed buffer");
}


static GstFlowReturn
gst_tcam_ic4_src_create(GstPushSrc *push_src, GstBuffer **buffer) {

    // TODO: add streamStats
    //GST_INFO("create func");

    GstTcamIC4Src *self = GST_TCAM_IC4_SRC(push_src);

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
    auto frame = self->device->sink->popOutputQueueBuffer(err);
    //self->device->sink->
    if (!frame)
    {
        GST_ERROR("Unable to retrieve buffer: %s", err.toString().c_str());

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

    //GST_ERROR("????????? %d", frame->getPitch());

    auto type = frame->getFrameType();

    //GST_ERROR("buffer size: %zu", type.buffer_size());
    GstBuffer *new_buf = gst_buffer_new_wrapped_full(
        static_cast<GstMemoryFlags>(GST_MEMORY_FLAG_READONLY), frame->getPtr(),
        type.buffer_size(),
        //type.height() * type.width(),
        0,
        //type.buffer_size(),
        type.height() * type.width(),
        trans, buffer_destroy);
    //wait_result.buf, buffer_destroy_callback);

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

static bool is_state_null(GstTcamIC4Src* self)
{
    return is_gst_state_equal_or_less(GST_ELEMENT(self), GST_STATE_NULL);
}

static bool is_state_ready_or_lower(GstTcamIC4Src* self)
{
    return is_gst_state_equal_or_less(GST_ELEMENT(self), GST_STATE_READY);
}


static void gst_tcam_ic4_src_set_property(GObject *object, guint prop_id,
                                          const GValue *value,
                                          GParamSpec *pspec)
{
    GstTcamIC4Src *self = GST_TCAM_IC4_SRC(object);

    switch (prop_id)
    {
        case PROP_SERIAL:
        {
            if (!is_state_null(self))
            {
                GST_ERROR_OBJECT(self, "GObject property 'serial' is not writable "
                                 "in state >= GST_STATE_READY.");
                return;
            }
            if (g_value_get_string(value) == nullptr)
            {
                self->device->set_serial(std::string{});
            }
            else
            {
                std::string string_value = g_value_get_string(value);

                // auto [s, t] = tcambind::separate_serial_and_type(string_value);
                // if (!t.empty())
                // {
                //     state.set_device_serial(s);
                //     //state.set_device_type(tcam::tcam_device_from_string(t));

                //     GST_INFO_OBJECT(
                //         self, "Set camera serial to '%s', Type to '%s'. (from %s).",
                //         self->device->get_serial().c_str(),
                //         tcam::tcam_device_type_to_string(state.get_device_type())
                //         .c_str(),
                //         string_value.c_str());
                // }
                // else
                // {
                if (!self->device->set_serial(string_value))
                {
                    GST_ERROR("Unable to open device");
                }
                              // }
            }
            break;
          }
        default: {
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
        }
    }
}

static void gst_tcam_ic4_src_get_property(
    GObject * object, guint prop_id, GValue * value, GParamSpec * pspec) {
    GstTcamIC4Src *self = GST_TCAM_IC4_SRC(object);
    // auto &state = *self->device;

    switch (prop_id) {
        case PROP_SERIAL:
        {
            g_value_set_string(value, self->device->get_serial().c_str());
            break;
        }

        default: {
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
        }
    }
}

static void gst_tcam_ic4_src_init(GstTcamIC4Src *self) {
    gst_base_src_set_live(GST_BASE_SRC(self), TRUE);
    gst_base_src_set_format(GST_BASE_SRC(self), GST_FORMAT_TIME);

    ic4::InitLibrary();

    self->device = new ic4_device_state();

    // this has to be defined in set_caps
    //self->fps = 0.0;
}

static void gst_tcam_ic4_src_finalize(GObject *object)
{

    GstTcamIC4Src *self = GST_TCAM_IC4_SRC(object);

    if (self->device)
    {
        delete self->device;
        self->device = nullptr;
    }

    ic4::ExitLibrary();
}

static void gst_tcam_ic4_src_class_init(GstTcamIC4SrcClass * klass) {


    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    GstElementClass *element_class = GST_ELEMENT_CLASS(klass);
    GstBaseSrcClass *gstbasesrc_class = GST_BASE_SRC_CLASS(klass);
    GstPushSrcClass *gstpushsrc_class = GST_PUSH_SRC_CLASS(klass);

    gobject_class->finalize = gst_tcam_ic4_src_finalize;
    gobject_class->set_property = gst_tcam_ic4_src_set_property;
    gobject_class->get_property = gst_tcam_ic4_src_get_property;

    g_object_class_install_property(
        gobject_class, PROP_SERIAL,
        g_param_spec_string("serial", "Camera serial", "Serial of the camera",
                            NULL,
                            static_cast<GParamFlags>(G_PARAM_READWRITE |
                                                     G_PARAM_STATIC_STRINGS)));

    // g_object_class_install_property(
    //     gobject_class, PROP_DEVICE_TYPE,
    //     g_param_spec_string("type", "Camera type", "type/backend of the camera",
    //                         "auto",
    //                         static_cast<GParamFlags>(G_PARAM_READWRITE |
    //                                                  G_PARAM_STATIC_STRINGS)));

    // g_object_class_install_property(
    //     gobject_class, PROP_CAMERA_BUFFERS,
    //     g_param_spec_int("camera-buffers", "Number of Buffers",
    //                      "Number of buffers to use for retrieving images", 1, 256,
    //                      GST_TCAM_MAINSRC_DEFAULT_N_BUFFERS,
    //                      static_cast<GParamFlags>(G_PARAM_READWRITE |
    //                                               G_PARAM_STATIC_STRINGS)));

    // g_object_class_install_property(
    //     gobject_class, PROP_IO_MODE,
    //     g_param_spec_enum("io-mode", "IO Mode", "", GST_TYPE_TCAM_IO_MODE, 0,
    //                       static_cast<GParamFlags>(G_PARAM_READWRITE |
    //                                                G_PARAM_STATIC_STRINGS)));

    // g_object_class_install_property(
    //     gobject_class, PROP_NUM_BUFFERS,
    //     g_param_spec_int(
    //         "num-buffers", "Number of Buffers",
    //         "Number of buffers to send before ending pipeline (-1 = unlimited)",
    //         -1, G_MAXINT, -1,
    //         static_cast<GParamFlags>(G_PARAM_READWRITE |
    //                                  G_PARAM_STATIC_STRINGS)));
    // g_object_class_install_property(
    //     gobject_class, PROP_DROP_INCOMPLETE_BUFFER,
    //     g_param_spec_boolean("drop-incomplete-buffer", "Drop incomplete buffers",
    //                          "Drop buffer that are incomplete.", true,
    //                          static_cast<GParamFlags>(G_PARAM_READWRITE |
    //                                                   G_PARAM_STATIC_STRINGS |
    //                                                   G_PARAM_CONSTRUCT)));

    // g_object_class_install_property(
    //     gobject_class, PROP_TCAM_PROPERTIES_GSTSTRUCT,
    //     g_param_spec_boxed(
    //         "tcam-properties", "Properties via GstStructure",
    //         "In GST_STATE_NULL, sets the initial values for tcam-property 1.0 "
    //         "properties."
    //         "In GST_STATE_READY, sets the current properties of the device, or "
    //         "reads the current "
    //         "state of all properties"
    //         "Names and types are the ones found in the tcam-property 1.0 "
    //         "interface."
    //         "(Usage e.g.: 'gst-launch-1.0 tcammainsrc "
    //         "tcam-properties=tcam,ExposureAuto=Off,ExposureTime=33333 ! ...')",
    //         GST_TYPE_STRUCTURE,
    //         static_cast<GParamFlags>(G_PARAM_READWRITE |
    //                                  G_PARAM_STATIC_STRINGS)));

    gst_tcamic4src_signals[SIGNAL_DEVICE_OPEN] =
        g_signal_new("device-open", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_LAST,
                     0, nullptr, nullptr, nullptr, G_TYPE_NONE, 0, G_TYPE_NONE);
    gst_tcamic4src_signals[SIGNAL_DEVICE_CLOSE] =
        g_signal_new("device-close", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_LAST,
                     0, nullptr, nullptr, nullptr, G_TYPE_NONE, 0, G_TYPE_NONE);

    GST_DEBUG_CATEGORY_INIT(tcam_ic4_src_debug, "tcamic4src", 0,
                            "tcam interface");

    gst_element_class_set_static_metadata(
        element_class, "Tcam Video Source", "Source/Video", "Tcam based source",
        "The Imaging Source <support@theimagingsource.com>");

    gst_element_class_add_pad_template(
        element_class, gst_static_pad_template_get(&tcam_ic4_src_template));

    element_class->change_state = gst_tcam_ic4_src_change_state;

    gstbasesrc_class->get_caps = gst_tcam_ic4_src_get_caps;
    gstbasesrc_class->set_caps = gst_tcam_ic4_src_set_caps;
    gstbasesrc_class->fixate = gst_tcam_ic4_src_fixate_caps;
    gstbasesrc_class->negotiate = gst_tcam_ic4_src_negotiate;
    //gstbasesrc_class->query = gst_tcam_mainsrc_query;
    //gstbasesrc_class->decide_allocation = tcam_mainsrc_decide_allocation;

    gstpushsrc_class->create = gst_tcam_ic4_src_create;
}

static gboolean plugin_init(GstPlugin *plugin) {
  gst_device_provider_register(plugin, "tcamic4srcdeviceprovider",
                               GST_RANK_PRIMARY,
                               TCAM_TYPE_IC4_SRC_DEVICE_PROVIDER);
  gst_element_register(plugin, "tcamic4src", GST_RANK_PRIMARY,
                       GST_TYPE_TCAM_IC4_SRC);

  GST_DEBUG_CATEGORY_INIT(tcam_ic4_src_debug, "tcamic4src", 0,
                          "tcam interface");

  return TRUE;
}

#ifndef PACKAGE
#define PACKAGE "tcam"
#endif

GST_PLUGIN_DEFINE(GST_VERSION_MAJOR, GST_VERSION_MINOR, tcamic4src,
                  "TCam Video Source", plugin_init, "1.0.0", "Proprietary",
                  "tcamic4src", "theimagingsource.com")
