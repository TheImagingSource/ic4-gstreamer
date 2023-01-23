
#pragma once

#include <string>
#include <tcamprop1.0_base/tcamprop_property_interface.h>
#include <memory>

namespace validator
{
    void    validate( const std::vector<std::unique_ptr<tcamprop1::property_interface>>& lst, const std::vector<std::string>& validation_filenames);
}


