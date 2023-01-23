#pragma once

#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <functional>

namespace gst_pipeline_helper
{
    using appsink_new_sample_cb = std::function<GstFlowReturn( GstAppSink* sink, GstSample* sample )>;

    /**
     * Registers a function of type appsink_new_sample_cb as the callback
     * Note:
     *      The passed in function is a bit different to the actual GstAppSinkCallback. You don't need to pull the sample from the app sink
     *      and don't have to unref the sample when you are done.
     * Example:
        GstFlowReturn   callback( GstAppSink* sink, GstSample* sample )
        {
            if( sample == nullptr ) {
                return GST_FLOW_ERROR;
            }
            // do something with sample
            return GST_FLOW_OK;
        }
     * 
     */

    inline void    set_appsink_newsample_cb( GstAppSink& appsink_element, appsink_new_sample_cb func )
    {
        auto call_func = []( GstAppSink* sink, void* user_data ) -> GstFlowReturn
        {
            GstSample* sample = gst_app_sink_pull_sample( sink );

            auto res = static_cast<appsink_new_sample_cb*>(user_data)->operator()( sink, sample );

            if( sample != nullptr ) {
                gst_sample_unref( sample );
            }
            return res;
        };
        auto destroy_notify = []( void* ptr ) { delete static_cast<appsink_new_sample_cb*>(ptr); };

        GstAppSinkCallbacks callbacks = {
            nullptr,
            nullptr,
            call_func,
            {}
        };

        void* ptr = new appsink_new_sample_cb{ std::move( func ) };

        gst_app_sink_set_callbacks( &appsink_element, &callbacks, ptr, destroy_notify );
    }

}

