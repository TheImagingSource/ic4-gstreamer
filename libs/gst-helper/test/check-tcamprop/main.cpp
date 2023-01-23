

#include <fmt/format.h>
#include <fmt/color.h>

#include <gst/gst.h>

#include <unistd.h> // isatty

#include "../lib/gst_pipeline_helper/pipeline.h"
#include "../lib/gst_pipeline_helper/appsink.h"

#include <map>

#include "validate_prop_names.h"

#include "cli-parser.h"
#include <tcamprop1.0_consumer/tcamprop1_consumer.h>

#include "stuff.h"

using namespace gst_pipeline_helper;

static bool use_color = true;

auto to_text_style( fmt::text_style style ) {
    return use_color ? style : fmt::text_style{};
}

auto get_color_error() { return to_text_style( fg( fmt::color::red ) ); }
auto get_color_warn() { return to_text_style( fg( fmt::color::yellow ) ); }
auto get_color_success() { return to_text_style( fg( fmt::color::green ) ); }


void    print_error( std::string_view txt, std::error_code errc )
{
    if( txt.empty() ) {
        txt = "Generic failure";
    }
    fmt::print( get_color_error(), "{}. Err={}\n", txt, errc.message() );
}

static void        print_single_property( tcamprop1::property_interface& entry, std::string_view prefix = {} )
{
    auto prop_name = entry.get_property_name();

    auto info = entry.get_property_info();

    fmt::print( "{}{:32} {:9} display-name='{}' vis={}\n",
        prefix,
        prop_name,
        to_string( entry.get_property_type() ),
        info.display_name,
        //info.iccategory,
        //info.description,
        to_string( info.visibility )
    );

    auto print_line_start = [prefix]( fmt::text_style style, std::string_view txt )
    {
        if( use_color ) {
            fmt::print( style, "{}{:43}{}", prefix, "", txt );
        } else {
            fmt::print( "{}{:43}{}", prefix, "", txt );
        }
    };


    tcamprop1::prop_state state;
    if( auto state_res = entry.get_property_state(); state_res.has_error() )
    {
        print_error( "Failed to fetch state", state_res.error() );
        return;
    } else {
        state = state_res.value();
    }

    if( !state.is_available ) {
        print_line_start( fg( fmt::color::red ), "Not Available\n" );
        return;
    }
    else if( state.is_locked ) {
        print_line_start( fg( fmt::color::yellow ), "Locked " );
    }
    else
    {
        print_line_start( fg( fmt::color::green ), "" );
    }

    auto print_local_err = [&]( std::string_view func, std::error_code errc ) {
        fmt::print( "\n" );
        print_error( fmt::format( "Property: {}, func={}", prop_name, func ), errc );
    };

    switch( entry.get_property_type() )
    {
    case tcamprop1::prop_type::Command:
    {
        fmt::print( "\n" );
        break;
    }
    case tcamprop1::prop_type::Boolean:
    {
        auto& prop_entry = static_cast<tcamprop1::property_interface_boolean&>( entry );
        if( auto val_res = prop_entry.get_property_value(); val_res.has_error() ) {
            print_local_err( "get_property_value", val_res.error() );
        } else {
            fmt::print( "val={}, ", val_res.value() );
        }
        if( auto val_res = prop_entry.get_property_default(); val_res.has_error() ) {
            print_local_err( "get_property_default", val_res.error() );
        }
        else {
            fmt::print( "def={}", val_res.value() );
        }
        fmt::print( "\n" );
        break;
    }
    case tcamprop1::prop_type::Integer:
    {
        auto& prop_entry = static_cast<tcamprop1::property_interface_integer&>(entry);
        if( auto val_res = prop_entry.get_property_value(); val_res.has_error() ) {
            print_local_err( "get_property_value", val_res.error() );
        }
        else {
            fmt::print( "val={}", val_res.value() );
        }
        if( auto val_res = prop_entry.get_property_range(); val_res.has_error() ) {
            print_local_err( "get_property_range", val_res.error() );
        } else {
            auto range = val_res.value();
            fmt::print( ", min={}, max={}, stp={}", range.min, range.max, range.stp );
        }
        if( auto def_res = prop_entry.get_property_default(); def_res.has_error() ) {
            print_local_err( "get_property_default", def_res.error() );
        } else {
            fmt::print( ", def={}", def_res.value() );
        }
        fmt::print( "\n" );
        break;
    }
    case tcamprop1::prop_type::Float:
    {
        auto& prop_entry = static_cast<tcamprop1::property_interface_float&>(entry);
        if( auto val_res = prop_entry.get_property_value(); val_res.has_error() ) {
            print_local_err( "get_property_value", val_res.error() );
        } else {
            fmt::print( "val={}", val_res.value() );
        }
        if( auto val_res = prop_entry.get_property_range(); val_res.has_error() ) {
            print_local_err( "get_property_range", val_res.error() );
        } else {
            auto range = val_res.value();
            fmt::print( ", min={}, max={}, stp={}", range.min, range.max, range.stp );
        }
        if( auto def_res = prop_entry.get_property_default(); def_res.has_error() ) {
            print_local_err( "get_property_default", def_res.error() );
        } else {
            fmt::print( ", def={}", def_res.value() );
        }
        fmt::print( "\n" );
        break;
    }
    case tcamprop1::prop_type::Enumeration:
    {
        auto& prop_entry = static_cast<tcamprop1::property_interface_enumeration&>(entry);
        if( auto val_res = prop_entry.get_property_value(); val_res.has_error() ) {
            print_local_err( "get_property_value", val_res.error() );
        }
        else {
            fmt::print( "val={}, ", val_res.value() );
        }
        if( auto val_res = prop_entry.get_property_range(); val_res.has_error() ) {
            print_local_err( "get_property_range", val_res.error() );
        }
        else {
            auto range = val_res.value();

            std::string tmp_str;
            for( auto&& e : range.enum_entries ) {
                if( !tmp_str.empty() )
                    tmp_str += ", ";
                tmp_str += "'" + e + "'";
            }
            fmt::print( "entries=[{}]", tmp_str );

            if( auto def_res = prop_entry.get_property_default(); def_res.has_error() ) {
                print_local_err( "get_property_default", def_res.error() );
            } else {
                fmt::print( ", def={}", def_res.value() );
            }
        }
        fmt::print( "\n" );
        break;
    }
    }
    fmt::print( "\n" );
}

static void    print_tcamprop_interface( const std::vector<std::unique_ptr<tcamprop1::property_interface>>& prop_list, std::string_view prefix = "" )
{
    std::map<std::string, std::vector<tcamprop1::property_interface*>> cat_map;
    for( auto&& prop : prop_list )
    {
        cat_map[std::string{prop->get_property_info().iccategory}].push_back( prop.get() );
    }

    for( auto&& [cat, entries] : cat_map )
    {
        fmt::print( "{}Category: {}\n", prefix, cat );
        for( auto&& entry : entries ) {
            print_single_property( *entry, std::string( prefix ) + "  " );
        }
    }
}

static auto    fetch_property_entries( _TcamPropertyProvider* prop_provider ) -> std::vector<std::unique_ptr<tcamprop1::property_interface>>
{
    auto prop_list_opt = tcamprop1_consumer::get_property_names( prop_provider );
    if( prop_list_opt.has_error() ) {
        print_error( "get_property_names failed", prop_list_opt.error() );
        return {};
    }

    std::vector<std::unique_ptr<tcamprop1::property_interface>> rval_list;

    for( auto&& name : prop_list_opt.value() ) {
        auto ptr_res = tcamprop1_consumer::get_property_interface( prop_provider, name.c_str() );
        if( ptr_res.has_error() ) {
            print_error( fmt::format( "get_property_interface with name={} failed", name ), prop_list_opt.error() );
            continue;
        }

        rval_list.push_back( std::move( ptr_res.value() ) );
    }
    return rval_list;
}

static int     gstreamer_main2( std::string pipeline_str, int state, const check_tcamprop::cli_params& cli_params )
{
    pipeline_str = append_gst_pipe_element( pipeline_str, "appsink name=sink" );
    fmt::print( "Pipeline string used: '{}'\n", pipeline_str );

    auto pipeline = gst_pipeline::create( pipeline_str );

    if( !pipeline.set_state( (GstState)state ) ) {
        fmt::print( "Failed set pipeline state of {}", gst_pipeline_helper::to_string( pipeline.get_state() ) );
        return -1;
    }
    if( !pipeline.wait_state() ) {
        fmt::print( "Failed set pipeline state of {}", gst_pipeline_helper::to_string( pipeline.get_state() ) );
        return -1;
    }

    fmt::print( "Pipeline state: '{}'\n", gst_pipeline_helper::to_string( pipeline.get_state() ) );
    for( auto&& elem : pipeline.get_all_elements() ) {
        if( tcamprop1_consumer::has_TcamPropertyProvider( *elem ) ) {
            fmt::print( "\n---------------------------------------\n\n" );
            fmt::print( "GstElement <{}>\n", gst_helper::get_type_name( *elem ) );

            auto list = fetch_property_entries( tcamprop1_consumer::get_TcamPropertyProvider( *elem ) );

            if( cli_params.print_prop_info ) {
                print_tcamprop_interface( list, "    " );
            }
            if( cli_params.validate ) {
                validator::validate( list, cli_params.validation_file_names );
            }
            //for( auto& item : list ) {
            //    fmt::print( "  {} type={}\n", item->get_property_name(), to_string( item->get_property_type() ) );
            //}
        }
    }

    pipeline.set_state( GST_STATE_NULL );

    return 0;
}


int main( int argc, char** argv )
{
    auto res = check_tcamprop::exec_cli_parser( argc, argv );
    if( !res.params ) {
        return res.exit_code;
    }

    auto cli_params = res.params.value();

    auto pipeline_string = res.params->pipeline_string;
    auto caps = res.params->caps;

    use_color = isatty( fileno( stdout ) );

    bool wait_for_debugger = res.params->wait_for_debugger;
    while( wait_for_debugger && !stuff::debuggerIsAttached() ) {
        usleep( 10'000 );
        //fmt::print( "." );
        //puts( "." );
    }

    if( !caps.empty() ) {
        if( caps == "BGRx" ) {
            caps = "video/x-raw,format=BGRx";
        } else if( caps == "bayer" ) {
            caps = "video/x-bayer";
        } else if( caps == "BGRx-def" ) {
            caps = "video/x-raw,format=BGRx,width=640,height=480";
        }
    }

    if( pipeline_string == "tcamdutils" ) {
        pipeline_string = "tcamsrc ! tcamdutils";
    }

    if( !caps.empty() ) {
        pipeline_string = append_gst_pipe_element( pipeline_string, caps );
    }

    try
    {
        gst_init( &argc, &argv );

        gstreamer_main2( pipeline_string, cli_params.state, cli_params );
    }
    catch( const std::exception& ex )
    {
        fmt::print( "Failed to setup due to:\n\t{}\n", ex.what() );
        return -1;
    }

    fmt::print( "Ended successfully\n" );
    return 0;
}