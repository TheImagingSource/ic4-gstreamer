#include "validate_prop_names.h"

#include <pugixml.hpp>

#include <string_view>

#include <fmt/format.h>
#include <fmt/color.h>
#include <tcamprop1.0_base/tcamprop_base.h>

using namespace std::literals;


namespace
{
    auto get_node_type( const pugi::xml_node& node ) -> tcamprop1::prop_type
    {
        if( node.name() == "Integer"sv ) {
            return tcamprop1::prop_type::Integer;
        }
        if( node.name() == "Float"sv ) {
            return tcamprop1::prop_type::Float;
        }
        if( node.name() == "Enumeration"sv ) {
            return tcamprop1::prop_type::Enumeration;
        }
        if( node.name() == "Boolean"sv ) {
            return  tcamprop1::prop_type::Boolean;
        }
        if( node.name() == "Command"sv ) {
            return tcamprop1::prop_type::Command;
        }
        throw std::runtime_error( fmt::format( "Failed to recognize SFNC file XML node type of {}", node.name() ) );
    }

    auto get_node_extension( const pugi::xml_node& node, std::string extension_name )
    {
        for( auto&& extension_node : node.children( "Extension" ) ) {
            for( auto&& extension : extension_node.children() )
            {
                if( extension.name() == extension_name ) {
                    return extension;
                }
            }
        }
        return pugi::xml_node{};
    }

    auto get_node_extension_contents( const pugi::xml_node& node, std::string extension_name ) -> std::string
    {
        auto res = get_node_extension( node, extension_name );
        if( !res ) {
            return {};
        }
        return res.first_child().text().as_string();
    }

    constexpr auto color_err = fmt::fg( fmt::color::red );
    constexpr auto color_warn = fmt::fg( fmt::color::yellow );
    constexpr auto color_ok = fmt::fg( fmt::color::green );
}

static pugi::xml_node find_genicam_node( const pugi::xml_node& root, const std::string_view& name )
{
    for( auto&& e : root.children() )
    {
        if( e.name() == "Group"sv ) {
            return find_genicam_node( e, name );
        } else {
            auto attrib = e.attribute( "Name");
            if( attrib && attrib.value() == name ) {
                return e;
            }
        }
    }
    return pugi::xml_node{};
}

static auto open_file_( const std::string& filename ) -> pugi::xml_document
{
    pugi::xml_document doc;
    auto res = doc.load_file( filename.c_str() );
    if( res.status != pugi::status_ok ) {
        fmt::print( color_err, "Failed to open file '{}'  with parse_result={}\n", filename, res.description() );
        return {};
    }
    return doc;
}

static void     verfiy_prop_against_SFNC_entry( tcamprop1::property_interface& prop_itf, pugi::xml_node prop_xml_node )
{
    auto prop_name = prop_itf.get_property_name();

    auto type_of_genicam_node = get_node_type( prop_xml_node );
    if( prop_itf.get_property_type() != type_of_genicam_node ) {
        fmt::print( color_err, "    SFNC node has different type of '{}' to tcamprop '{}'\n", prop_name, prop_xml_node.name(), to_string( prop_itf.get_property_type() ) );
    }

    std::string category = get_node_extension_contents( prop_xml_node, "VCDCategoryName" );
    if( category != prop_itf.get_property_info().iccategory )
    {
        fmt::print( color_err, "    SFNC has different VCDCategoryName '{}' to tcamprop '{}'\n", category, prop_itf.get_property_info().iccategory );
    }
}

void    validator::validate( const std::vector<std::unique_ptr<tcamprop1::property_interface>>& lst, const std::vector<std::string>& validation_filenames )
{
    for( auto&& filename : validation_filenames )
    {
        pugi::xml_document doc = open_file_( filename );
        if( doc.empty() ) {
            continue;
        }

        fmt::print( "Document: {}\n", filename );
        auto root = doc.first_child();
        for( auto&& e : lst )
        {
            auto prop_name = e->get_property_name();

            auto res = find_genicam_node( root, prop_name );
            if( !res ) {
                fmt::print( color_warn, "  Failed to find property '{}'\n", prop_name );
                continue;
            } else {
                fmt::print( color_ok, "  Found property '{}'\n", prop_name );
            }
            try
            {
                verfiy_prop_against_SFNC_entry( *e, res );
            }
            catch( const std::exception& ex )
            {
            	fmt::print( "  Failed to validate property '{}' due to: {}\n", prop_name, ex.what() );
            }
        }
    }
}
