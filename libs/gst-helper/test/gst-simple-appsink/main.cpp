

#include <fmt/format.h>

#include <cctype>
#include <cstdio>
#include <cstring>

#include <gst/gst.h>

#include <unistd.h>
#include <cstdlib>

#include <filesystem>

#include "../lib/gst_pipeline_helper/pipeline.h"
#include "../lib/gst_pipeline_helper/appsink.h"

using namespace gst_pipeline_helper;

/*
  This function will be called in a separate thread when our appsink
  says there is data for us. user_data has to be defined
  when calling g_signal_connect. It can be used to pass objects etc.
  from your other function to the callback.
*/
static GstFlowReturn callback( GstAppSink* sink, GstSample* sample )
{
    if( !sample ) {
        return GST_FLOW_OK;
    }

    GstBuffer* buffer = gst_sample_get_buffer( sample );
    GstMapInfo info; // contains the actual image
    if( gst_buffer_map( buffer, &info, GST_MAP_READ ) )
    {
        auto caps = gst_sample_get_caps( sample );
        if( caps == nullptr ) {
            fmt::print( "Sample has no caps\n" );
        } else {
            gchar* tmp = gst_caps_to_string( caps );
            fmt::print( "img-caps: '{}'\n", tmp );
            g_free( tmp );
        }

        auto seg = gst_sample_get_segment( sample );

        //printf( "." );
        //fflush( stdout );

        gst_buffer_unmap( buffer, &info );
    }

    // delete our reference
    gst_sample_unref( sample );

    return GST_FLOW_OK;
}

void    register_callback( gst_pipeline& pipeline )
{
    /* retrieve the appsink from the pipeline */
    auto sink = pipeline.get_named_element( "sink" );

    gst_pipeline_helper::set_appsink_newsample_cb( *GST_APP_SINK( sink.get() ), &callback );
}

int gstreamer_main()
{
    //auto pipeline_str = append_gst_pipe_element( build_pipeline_string( params ), "appsink name=sink" );
    //auto pipeline_str = append_gst_pipe_element( "tcamsrc name=src", { "tcamdutils name=dutils", "capsfilter name=caps", "appsink name=sink" } );
    auto pipeline_str = append_gst_pipe_element( "tcambin name=src", { /*"tcamdutils name=dutils",*/ "capsfilter name=caps", "appsink name=sink" } );

    fmt::print( "Pipeline string used: '{}'\n", pipeline_str );

    auto pipeline = gst_pipeline::create( pipeline_str );

    register_callback( pipeline );

    auto src_ptr = pipeline.get_named_element( "src" );

    auto caps_filter = pipeline.get_named_element( "caps" );
    g_object_set( caps_filter.get(), "caps", make_caps( "video/x-raw", "BGRx", 640, 480, 30.f ).get(), nullptr );

    //for( int i = 0; i < 10; ++i )
    {
        pipeline.set_state( GST_STATE_READY );

        pipeline.wait_state();


        pipeline.set_state( GST_STATE_PLAYING );

        fmt::print( "Playing\n" );
        pipeline.wait_state();

        usleep( 500'000 );  // 100ms

        pipeline.set_state( GST_STATE_PAUSED );
    }

    printf( "Press Any key to exit\n" );
    fflush( stdout );
    getchar();

    return 0;
}

int main( int argc, char** argv )
{
    auto path = std::filesystem::path( argv[0] ).remove_filename().string();

    if( const char* test = getenv( "GST_PLUGIN_PATH" ); test != nullptr )
    {
        fmt::print( "GST_PLUGIN_PATH={}", test );
        path += ":" + std::string( test );
    }

    setenv( "GST_PLUGIN_PATH", path.c_str(), 1 );
    setenv( "GST_DEBUG", "tcampimipisrc:4,tcamdutils:4,3", 0 );
    setenv( "GST_DEBUG_NO_COLOR", "1", 1 ); // disabled for Visual Studio
    setenv( "DISPLAY", ":0.0", 1 );
    //setenv( "TCAM_MAX_CONCURRENCY", "1", 1 );
    setenv( "TCAM_SHOW_OVERLAY", "1", 1 );

    try
    {
        gst_init( &argc, &argv );

        gstreamer_main();
    }
    catch( const std::exception& ex )
    {
        fmt::print( "Failed to setup due to:\n\t{}\n", ex.what() );
        return -1;
    }

    fmt::print( "Ended successfully\n" );
    return 0;
}