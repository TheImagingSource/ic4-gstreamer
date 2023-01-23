
#include "cli-parser.h"

#include "../lib/CLI11.hpp"

auto check_tcamprop::exec_cli_parser( int argc, char** argv ) -> exec_cli_parser_result
{
    CLI::App program{ "check-tcamprop program to test tcamprop implementations", "1.0.0" };

    cli_params rval;

    program.add_option( "-s,--state", rval.state, "GstState to set [4 = 'GST_STATE_PLAYING, 3 ='GST_STATE_PAUSED' ..]" )
        ->check( CLI::Range( 0, 4 ) )
        ->default_val( rval.state )
        ;
    program.add_option( "-p,--pipeline", rval.pipeline_string, "A pipeline in the form of 'tcamsrc ! tcamdutils ! video/x-raw,format=BGRx'" )
        ->required()
        ->default_val( rval.pipeline_string )
        ;
    program.add_option( "-c,--caps", rval.caps, "A caps string for the output or a shortcut for this" )
        ->default_val( "video/x-raw,format=BGRx" )
        ;
    program.add_flag( "--validate", rval.validate, "Validate against validation files" );
    program.add_flag( "--wait-debug", rval.wait_for_debugger, "Wait for a debugger to attach" );

    program.add_option( "--print-prop-info", rval.print_prop_info, "Print property infos and ranges for all properties" )
        ->default_val( rval.print_prop_info );

    std::string footer =
        R"(
There are certain shortcuts for the '--pipeline' option:
    'tcamdutils' is the same as 'tcamsrc ! tcamdutils'

Shortcuts for the '--caps' option:
    'BGRx' => 'video/x-raw,format=BGRx'
    'BGRx-def' => 'video/x-raw,format=BGRx,width=640,height=480'
    'bayer' => 'video/x-bayer'

Note:
    An appsink is always appended and a callback installed

Examples:
    check-tcamprop -s 4 -p "tcambin"
    check-tcamprop -s 3 -p "tcamsrc ! tcamconvert ! video/x.raw,format=BGRx"
    check-tcamprop -s 4 -p "tcampimipisrc ! video/x-bayer,format=pwl-rggb12m ! tcamdutils ! video/x-raw,format=BGRf,width=640,height=480"

    check-tcamprop -s 4 -p "tcambin" -caps BGRx
    check-tcamprop -s 4 -p "tcamsrc" -caps bayer

)";


    program.footer( footer );
    try
    {
        program.parse( argc, argv );
    }
    catch( const CLI::ParseError& ex )
    {
        return exec_cli_parser_result{ std::nullopt, program.exit( ex ) };
    }

    return exec_cli_parser_result{ rval, 0 };
}
