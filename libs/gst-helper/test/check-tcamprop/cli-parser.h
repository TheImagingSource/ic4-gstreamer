#pragma once

#include <string>
#include <optional>

namespace check_tcamprop
{
struct cli_params 
{
    bool print_prop_info = true;
    bool validate = false;

    bool wait_for_debugger = false;

    int state = 4;
    std::string pipeline_string = "tcambin";
    std::string caps;

    std::vector<std::string>    validation_file_names = {
        "tiscamera-property-list.xml",
        //"SFNC_Device_Reference_Version_2_6_0_Schema_1_1_xml.xml"
    };
};


struct exec_cli_parser_result
{
    std::optional<cli_params> params;
    int exit_code = 0;
};

exec_cli_parser_result  exec_cli_parser( int argc, char** argv );

}