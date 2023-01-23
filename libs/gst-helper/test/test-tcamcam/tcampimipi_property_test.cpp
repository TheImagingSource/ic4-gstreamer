
#include "doctest.h"

#include "../lib/gst_pipeline_helper/pipeline.h"
#include "../lib/gst_pipeline_helper/gerror_wrapper.h"
#include "../../libs/tcamprop/src/tcam-property-1.0.h"

#include <gst-helper/gst_gvalue_helper.h>

using namespace gst_pipeline_helper;

TEST_CASE( "test_exposure_present" )
{
    auto pipeline_str = append_gst_pipe_element( "tcampimipisrc name=src", "appsink name=sink" );
    
    auto pipeline = gst_pipeline::create( pipeline_str );

    CHECK( pipeline.set_state( GST_STATE_READY ) );
    CHECK( pipeline.wait_state() );

    auto src_ptr = pipeline.get_named_element( "src" );
    CHECK( src_ptr != nullptr );

    TcamPropertyProvider* prop = TCAM_PROPERTY_PROVIDER( src_ptr.get() );
    CHECK( prop != nullptr );

    GError_wrapper err;
    auto prop_gs_list = tcam_property_provider_get_tcam_property_names( prop, err.reset_and_get() );
    CHECK( !err.is_error() );

    auto prop_list = gst_helper::convert_GSList_to_string_vector_consume( prop_gs_list );
    CHECK( !prop_list.empty() );

    for( auto&& prop_name : prop_list )
    {
        //CHECK
    }

    auto value = tcam_property_provider_get_tcam_float( prop, "ExposureTime", err.reset_and_get() );
    CHECK( !err.is_error() );
    CHECK( value > 0.0 );

}