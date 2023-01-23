#pragma once

#include <gst/gst.h>
#include <string>
#include <memory>
#include <fmt/format.h>

#include <gst-helper/helper_functions.h>

namespace gst_pipeline_helper
{
    std::string    append_with_seperator( const std::string& sep, const std::string& str, const std::string& append );
    std::string    append_gst_pipe_element( const std::string& str, const std::string& append );

    std::string    append_gst_pipe_element( const std::string& str, std::initializer_list<std::string> lst );

    inline gst_helper::gst_ptr<GstCaps> make_caps( const std::string& media_type, const std::string& format, int width, int height, float fps );

    using gst_helper::to_string;
    using gst_helper::gst_ptr;

    class gst_pipeline
    {
    public:
        gst_pipeline() = delete;

        gst_pipeline( const gst_pipeline& ) = delete;
        gst_pipeline& operator=( const gst_pipeline& ) = delete;

        gst_pipeline( gst_pipeline&& rhs ) noexcept = default;
        gst_pipeline& operator=( gst_pipeline&& rhs ) noexcept = default;

        ~gst_pipeline() noexcept
        {
            // this stops the pipeline and frees all resources
            gst_element_set_state( pipeline_.get(), GST_STATE_NULL );
        }

        static gst_pipeline    create( const std::string& pipeline_string ) noexcept( false )
        {
            GError* err = nullptr;
            auto pipeline = gst_helper::make_consume_ptr( gst_parse_launch( pipeline_string.c_str(), &err ) );
            if( pipeline == nullptr )
            {
                fmt::print( "Could not create pipeline. Cause: {}\n", err->message );

                if( err ) {
                    g_error_free( err );
                }
                throw std::runtime_error( "Failed to open pipeline" );
            }
            return gst_pipeline( std::move( pipeline ) );
        }

        bool    set_state( GstState state ) noexcept
        {
            GstStateChangeReturn ret = gst_element_set_state( pipeline_.get(), state );
            switch( ret )
            {
            case GST_STATE_CHANGE_FAILURE:
                return false;
            case  GST_STATE_CHANGE_SUCCESS:
                return true;
            case  GST_STATE_CHANGE_ASYNC:
                return true;
            case  GST_STATE_CHANGE_NO_PREROLL:
                return true;
            }
            return true;
        }

        GstState    get_state() const noexcept
        {
            GstState state = GST_STATE_NULL;
            GstState pending = GST_STATE_NULL;
            auto res = gst_element_get_state( pipeline_.get(), &state, &pending, 0 );
            if( res == GST_STATE_CHANGE_SUCCESS || res == GST_STATE_CHANGE_NO_PREROLL )
            {
                return state;
            }
            fmt::print( "Failed to fetch the current state due to GstStateChangeReturn={}.\n", (int)res );
            return state;
        }

        bool    wait_state() const noexcept
        {
            GstState state = GST_STATE_NULL;
            GstState pending = GST_STATE_NULL;
            auto res = gst_element_get_state( pipeline_.get(), &state, &pending, GST_CLOCK_TIME_NONE );
            if( res == GST_STATE_CHANGE_SUCCESS || res == GST_STATE_CHANGE_NO_PREROLL )
            {
                return true;
            }
            return false;
        }

        auto    get_state_full() const noexcept
        {
            struct result 
            {
                GstStateChangeReturn    rval;
                GstState current_state;
                GstState pending_state;
            };

            result res = {};

            res.rval = gst_element_get_state( pipeline_.get(), &res.current_state, &res.pending_state, 0 );
            return res;
        }


        GstElement* get_gst_pipeline() const noexcept { return pipeline_.get(); }

        gst_ptr<GstElement> get_named_element( const std::string& name ) const noexcept
        {
            return gst_helper::make_wrap_ptr( gst_bin_get_by_name( GST_BIN( pipeline_.get() ), name.c_str() ) );
        }

        std::vector<gst_ptr<GstElement>>    get_all_elements() const noexcept
        {
            auto iter = gst_bin_iterate_elements( GST_BIN( pipeline_.get() ) );
            if( iter == nullptr ) {
                return {};
            }

            std::vector<gst_ptr<GstElement>> rval;
            while( true )
            {
                GValue val = {};
                if( gst_iterator_next( iter, &val ) != GST_ITERATOR_OK ) {
                    break;
                }

                auto ptr = g_value_get_object( &val );

                if( ptr ) {
                    rval.push_back( gst_helper::make_addref_ptr( static_cast<GstElement*>(ptr) ) );
                }

                g_value_unset( &val );
            }
            gst_iterator_free( iter );

            return rval;
        }

    private:
        gst_pipeline( gst_ptr<GstElement>&& pipeline )
            : pipeline_( std::move( pipeline ) )
        {
        }

        gst_ptr<GstElement> pipeline_;
    };

    inline std::string    append_gst_pipe_element( const std::string& str, const std::string& append )
    {
        return append_with_seperator( " ! ", str, append );
    }

    inline std::string    append_gst_pipe_element( const std::string& start, std::initializer_list<std::string> str_list )
    {
        auto str = start;
        for( auto&& e : str_list ) {
            str += append_with_seperator( " ! ", str, e );
        }
        return str;
    }

    inline std::string    append_with_seperator( const std::string& sep, const std::string& str, const std::string& append )
    {
        if( str.empty() ) {
            return append;
        }
        if( append.empty() ) {
            return str;
        }
        return str + sep + append;
    }


    inline gst_helper::gst_ptr<GstCaps> make_caps( const std::string& media_type, const std::string& format, int width, int height, float fps )
    {
        return gst_helper::make_wrap_ptr( gst_caps_new_simple(
            media_type.c_str(),
            "format", G_TYPE_STRING, format.c_str(),
            "width", G_TYPE_INT, width,
            "height", G_TYPE_INT, height,
            "framerate", GST_TYPE_FRACTION, (int)fps, (int)1,
            nullptr ) );
    }

}

