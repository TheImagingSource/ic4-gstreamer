
#include <Tcam-1.0.h>
#include <cstddef>
#include <cstdint>
#include <doctest/doctest.h>
#include <fmt/format.h>

#include <iostream>

#include <tcam-property-1.0.h>

#include <gst/gst.h>
#include "glib-object.h"
#include "test_helper.h"


// TEST_CASE("read-tcam-property")
// {
//     implicitly tested via write-tcam-property
// }


TEST_CASE("write-tcam-property")
{
    std::string serial = test_helper::get_test_serial();

    std::string pipeline_desc = fmt::format("ic4src serial={} name=source ! appsink ", serial);
    
    GError* err = nullptr;
    GstElement* pipeline = gst_parse_launch(pipeline_desc.c_str(), &err);

    if (pipeline == nullptr)
    {
        CHECK(false);
    }


    gst_element_set_state(pipeline, GST_STATE_READY);
    GstElement* source = gst_bin_get_by_name(GST_BIN(pipeline), "source");

    CHECK(source);

    // setup end

    TcamPropertyBase* base_exposure_auto = tcam_property_provider_get_tcam_property(TCAM_PROPERTY_PROVIDER(source), "ExposureAuto", &err);
    TcamPropertyBase* base_contrast = tcam_property_provider_get_tcam_property(TCAM_PROPERTY_PROVIDER(source), "Contrast", &err);
    TcamPropertyBase* base_exposure = tcam_property_provider_get_tcam_property(TCAM_PROPERTY_PROVIDER(source), "ExposureTime", &err);

    CHECK(base_exposure_auto);
    CHECK(base_contrast);
    CHECK(base_exposure);

    TcamPropertyEnumeration* exposure_auto = TCAM_PROPERTY_ENUMERATION(base_exposure_auto);
    TcamPropertyInteger* contrast = TCAM_PROPERTY_INTEGER(base_contrast);
    TcamPropertyFloat* exposure = TCAM_PROPERTY_FLOAT(base_exposure);
    
    // set properties

    std::string trigger_value = "Off";
    int64_t int_value = 255;
    double float_value = 5.0;
    

    tcam_property_enumeration_set_value(exposure_auto, trigger_value.c_str(), &err);
    CHECK(!err);
    tcam_property_integer_set_value(contrast, int_value, &err);
    CHECK(!err);
    tcam_property_float_set_value(exposure, float_value, &err);
    CHECK(!err);

    
    // read back

    std::string trigger_value_read = tcam_property_enumeration_get_value(exposure_auto, &err);
    CHECK(trigger_value_read == trigger_value);
    CHECK(tcam_property_integer_get_value(contrast, &err) == int_value);
    CHECK(tcam_property_float_get_value(exposure, &err) == float_value);

    // cleanup

    gst_element_set_state(pipeline, GST_STATE_NULL);


    gst_object_unref(source);
    gst_object_unref(pipeline);

}


TEST_CASE("write-gst-properties")
{
    std::string serial = test_helper::get_test_serial();
    
    std::string pipeline_desc = fmt::format("ic4src serial={} name=source ! appsink ", serial);

    GError* err = nullptr;
    GstElement* pipeline = gst_parse_launch(pipeline_desc.c_str(), &err);

    if (pipeline == nullptr)
    {
        CHECK(false);
    }


    gst_element_set_state(pipeline, GST_STATE_READY);
    GstElement* source = gst_bin_get_by_name(GST_BIN(pipeline), "source");

    CHECK(source);
    
    GValue val = G_VALUE_INIT;
    g_value_init(&val, G_TYPE_STRING);

    std::string new_props = "ExposureAuto=Off Contrast=1 ExposureTime=1000.0";

    g_value_set_static_string(&val, new_props.c_str());
    g_object_set_property(G_OBJECT(source), "prop", &val);

    TcamPropertyBase* base_exposure_auto = tcam_property_provider_get_tcam_property(TCAM_PROPERTY_PROVIDER(source), "ExposureAuto", &err);
    TcamPropertyBase* base_contrast = tcam_property_provider_get_tcam_property(TCAM_PROPERTY_PROVIDER(source), "Contrast", &err);
    TcamPropertyBase* base_exposure = tcam_property_provider_get_tcam_property(TCAM_PROPERTY_PROVIDER(source), "ExposureTime", &err);

    CHECK(base_exposure_auto);
    CHECK(base_contrast);
    CHECK(base_exposure);

    TcamPropertyEnumeration* exposure_auto = TCAM_PROPERTY_ENUMERATION(base_exposure_auto);
    TcamPropertyInteger* contrast = TCAM_PROPERTY_INTEGER(base_contrast);
    TcamPropertyFloat* exposure = TCAM_PROPERTY_FLOAT(base_exposure);

    std::string exposure_auto_value = tcam_property_enumeration_get_value(exposure_auto, &err);
    CHECK(exposure_auto_value == "Off");
    CHECK(tcam_property_integer_get_value(contrast, &err) == 1);
    CHECK(tcam_property_float_get_value(exposure, &err) == 1000.0);

    gst_element_set_state(pipeline, GST_STATE_NULL);


    gst_object_unref(source);
    gst_object_unref(pipeline);

}
