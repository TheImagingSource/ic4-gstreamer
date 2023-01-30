
#include "ic4_tcam_property.h"
#include "Tcam-1.0.h"
#include "gst_tcam_ic4_src.h"
#include "ic4/Error.h"
#include "ic4_device_state.h"
#include "ic4/Properties.h"
#include <cassert>

#include <cstdint>
#include <memory>
#include <outcome/result.hpp>
#include <system_error>
#include <vector>

#include "../libs/gst-helper/include/tcamprop1.0_base/tcamprop_property_interface.h"
#include "tcamprop1.0_base/tcamprop_base.h"

namespace
{

tcamprop1::Visibility_t vis_to_prop(ic4::PropVisibility vis)
{
    switch (vis)
    {
        case ic4::PropVisibility::IC4_PROPVIS_BEGINNER: ///< Beginner visibility
        {
            return tcamprop1::Visibility_t::Beginner;
        }
        case ic4::PropVisibility::IC4_PROPVIS_EXPERT: ///< Expert visibility
        {
            return tcamprop1::Visibility_t::Expert;
        }
        case ic4::PropVisibility::IC4_PROPVIS_GURU: ///< Guru visibility
        {
            return tcamprop1::Visibility_t::Guru;
        }
        case ic4::PropVisibility::IC4_PROPVIS_INVISIBLE:
        {
            return tcamprop1::Visibility_t::Invisible;
        } ///< Invisible
    }
}

TcamPropertyVisibility visibility_to_tcamprop(ic4::PropVisibility vis)
{
    switch (vis)
    {
        case ic4::PropVisibility::IC4_PROPVIS_BEGINNER: ///< Beginner visibility
        {
            return TcamPropertyVisibility::TCAM_PROPERTY_VISIBILITY_BEGINNER;
        }
        case ic4::PropVisibility::IC4_PROPVIS_EXPERT: ///< Expert visibility
        {
            return TcamPropertyVisibility::TCAM_PROPERTY_VISIBILITY_EXPERT;
        }
        case ic4::PropVisibility::IC4_PROPVIS_GURU: ///< Guru visibility
        {
            return TcamPropertyVisibility::TCAM_PROPERTY_VISIBILITY_GURU;
        }
        case ic4::PropVisibility::IC4_PROPVIS_INVISIBLE:
        {
            return TcamPropertyVisibility::TCAM_PROPERTY_VISIBILITY_INVISIBLE;
        } ///< Invisible
    }
}

} //namespace

namespace ic4::gst
{

template <class TBase> struct TcamPropertyBase : TBase
{
    TcamPropertyBase(ic4::Property& prop)
        : m_prop(prop)
    {
        m_name = m_prop.getName();
        m_display_name = m_prop.getDisplayName();
        m_description = m_prop.getDescription();
    }

    ic4::Property m_prop;
    // caching to prevent string_view issues
    std::string m_name;
    std::string m_display_name;
    std::string m_description;

    auto get_property_name() const noexcept -> std::string_view final
    {
        return m_name.c_str();
    }

    auto get_property_info() const noexcept -> tcamprop1::prop_static_info final
    {
        //return m_prop.get_static_info();

        tcamprop1::prop_static_info info = {};

        info.visibility = vis_to_prop(m_prop.getVisibility());
        info.name = m_name;
        info.display_name = m_display_name;
        info.description = m_description;
        //info.access = m_prop.

        return info;
    }

    auto get_property_state(uint32_t /*flags = 0*/)
        -> outcome::result<tcamprop1::prop_state> final
    {
        //auto flags = m_prop.get_flags();
        tcamprop1::prop_state ret = {};
        ret.is_implemented = true;
        ret.is_locked = m_prop.isLocked();
        ret.is_available = m_prop.isAvailable();
        ret.is_name_hidden = false;
        //flags & tcam::property::PropertyFlags::Hidden;
        return ret;
    }
};


struct TcamPropertyInteger : TcamPropertyBase<tcamprop1::property_interface_integer>
{
    TcamPropertyInteger(ic4::Property& prop)
        : TcamPropertyBase { prop }
    {
        auto tmp = m_prop.asInteger();

        m_unit = tmp.getUnit();
    }

    std::string m_unit;

    auto get_property_range(uint32_t /* flags = 0 */)
        -> outcome::result<tcamprop1::prop_range_integer> final
    {
        auto tmp = m_prop.asInteger();

        tcamprop1::prop_range_integer ret = {};

        tmp.getMinimum(ret.min);
        tmp.getMaximum(ret.max);
        tmp.getIncrement(ret.stp);

        return ret;
    }

    auto get_property_default(uint32_t /* flags = 0 */) -> outcome::result<int64_t> final
    {
        auto tmp = m_prop.asInteger();

        int64_t ret;

        tmp.getDefault(ret);
        return ret;
    }

    auto get_property_value(uint32_t /*flags*/) -> outcome::result<int64_t> final
    {
        auto tmp = m_prop.asInteger();

        int64_t ret;
        tmp.getValue(ret);

        return ret;
    }
    auto set_property_value(int64_t value, uint32_t /*flags*/) -> std::error_code final
    {
        // auto tmp = static_cast<tcam::property::IPropertyInteger*>(m_prop.get());

        // if (property::is_locked(m_prop.get_flags()))
        // {
        //     return tcam::status::PropertyNotWriteable;
        // }

        // auto ret = tmp->set_value(value);
        // if (ret)
        // {
        //     return tcam::status::Success;
        // }
        // return ret.error();


        auto tmp = m_prop.asInteger();
        ic4::Error err;
        auto ret = tmp.setValue(value, err);
        if (ret)
        {
//            return tcam::status::Success;
        }
        //return err.getVal().;
        // TODO: error handling
        //return ret.error();
    }

    auto get_representation() const noexcept -> tcamprop1::IntRepresentation_t final
    {
        auto tmp = m_prop.asInteger();

        auto ic4_rep = tmp.getRepresentation();

        auto int_rep_to_tcamprop = [] (ic4::PropIntRepresentation rep)
            -> tcamprop1::IntRepresentation_t
        {
            switch (rep)
            {
                case ic4::PropIntRepresentation::IC4_PROPINTREP_BOOLEAN:
                {
                    return tcamprop1::IntRepresentation_t::Boolean;
                }
                case ic4::PropIntRepresentation::IC4_PROPINTREP_HEXNUMBER:
                {
                    return tcamprop1::IntRepresentation_t::HexNumber;
                }
                case ic4::PropIntRepresentation::IC4_PROPINTREP_IPV4ADDRESS:
                {
                    return tcamprop1::IntRepresentation_t::IPV4Address;
                }
                case ic4::PropIntRepresentation::IC4_PROPINTREP_LOGARITHMIC:
                {
                    return tcamprop1::IntRepresentation_t::Logarithmic;
                }
                case ic4::PropIntRepresentation::IC4_PROPINTREP_LINEAR:
                {
                    return tcamprop1::IntRepresentation_t::Linear;
                }
                case ic4::PropIntRepresentation::IC4_PROPINTREP_MACADDRESS:
                {
                    return tcamprop1::IntRepresentation_t::MACAddress;
                }
                case ic4::PropIntRepresentation::IC4_PROPINTREP_PURENUMBER:
                {
                    return tcamprop1::IntRepresentation_t::PureNumber;
                }
            }
        };

        return int_rep_to_tcamprop(ic4_rep);
    }

    auto get_unit() const noexcept -> std::string_view final
    {
        return m_unit;
    }
};


struct TcamPropertyFloat : TcamPropertyBase<tcamprop1::property_interface_float>
{
    TcamPropertyFloat(ic4::Property& prop)
        : TcamPropertyBase { prop }
    {
        auto tmp = m_prop.asFloat();

        m_unit = tmp.getUnit();
    }

    std::string m_unit;

    auto get_property_range(uint32_t /* flags = 0 */)
        -> outcome::result<tcamprop1::prop_range_float> final
    {
        auto tmp = m_prop.asFloat();

        tcamprop1::prop_range_float ret = {};

        tmp.getMinimum(ret.min);
        tmp.getMaximum(ret.max);
        tmp.getIncrement(ret.stp);

        return ret;
    }

    auto get_property_default(uint32_t /* flags = 0 */) -> outcome::result<double> final
    {
        auto tmp = m_prop.asFloat();

        double ret;

        tmp.getDefault(ret);
        return ret;
    }

    auto get_property_value(uint32_t /*flags*/) -> outcome::result<double> final
    {
        auto tmp = m_prop.asFloat();

        double ret;
        tmp.getValue(ret);

        return ret;
    }

    auto set_property_value(double value, uint32_t /*flags*/) -> std::error_code final
    {
        auto tmp = m_prop.asFloat();
        ic4::Error err;
        auto ret = tmp.setValue(value, err);
        if (ret)
        {
            //            return tcam::status::Success;
        }
        //return err.getVal().;
        // TODO: error handling
    }

    auto get_representation() const noexcept -> tcamprop1::FloatRepresentation_t final
    {
        auto tmp = m_prop.asFloat();

        auto ic4_rep = tmp.getRepresentation();

        auto float_rep_to_tcamprop =
            [](ic4::PropFloatRepresentation rep) -> tcamprop1::FloatRepresentation_t
        {
            switch (rep)
            {
                case ic4::PropFloatRepresentation::IC4_PROPFLOATREP_LINEAR:
                {
                    return tcamprop1::FloatRepresentation_t::Linear;
                }
                case ic4::PropFloatRepresentation::IC4_PROPFLOATREP_LOGARITHMIC:
                {
                    return tcamprop1::FloatRepresentation_t::Logarithmic;
                }
                case ic4::PropFloatRepresentation::IC4_PROPFLOATREP_PURENUMBER:
                {
                    return tcamprop1::FloatRepresentation_t::PureNumber;
                }

            }
        };

        return float_rep_to_tcamprop(ic4_rep);

    }

    auto get_unit() const noexcept -> std::string_view final
    {
        return m_unit;
    }
};


struct TcamPropertyBoolean : TcamPropertyBase<tcamprop1::property_interface_boolean>
{
    TcamPropertyBoolean(ic4::Property& prop)
        : TcamPropertyBase { prop }
    {
    }

    auto get_property_default(uint32_t /* flags = 0 */) -> outcome::result<bool> final
    {
        auto tmp = m_prop.asBoolean();

        bool ret;
        tmp.getDefault(ret);
        return ret;
    }

    auto get_property_value(uint32_t /*flags*/) -> outcome::result<bool> final
    {
        auto tmp = m_prop.asBoolean();

        bool val;
        tmp.getValue(val);
        return val;
    }

    auto set_property_value(bool value, uint32_t /*flags*/) -> std::error_code final
    {
        auto tmp = m_prop.asBoolean();

        tmp.setValue(value);

        // if (property::is_locked(m_prop.get_flags()))
        // {
        //     return tcam::status::PropertyNotWriteable;
        // }

        // auto ret = tmp->set_value(value);

        // if (ret)
        // {
        //     return tcam::status::Success;
        // }
        // return ret.error();
    }
};


struct TcamPropertyEnumeration : TcamPropertyBase<tcamprop1::property_interface_enumeration>
{
    TcamPropertyEnumeration(ic4::Property& prop)
        : TcamPropertyBase { prop }
    {
        auto tmp = m_prop.asEnumeration();
        auto entries = tmp.getEntries();
        if (!entries.empty())
        {
            m_default = entries.at(0).getName();
        }
    }

    std::string m_default;

    auto get_property_range(uint32_t /* flags = 0 */)
        -> outcome::result<tcamprop1::prop_range_enumeration> final
    {
        auto tmp = m_prop.asEnumeration();
        auto entries = tmp.getEntries();

        std::vector<std::string> vals;
        for (const auto& e : entries)
        {
            vals.push_back(e.getName());
        }

        return tcamprop1::prop_range_enumeration { vals };
    }

    auto get_property_default(uint32_t /*flags = 0 */) -> outcome::result<std::string_view> final
    {
        auto tmp = m_prop.asEnumeration();

        auto entries = tmp.getEntries();

        if (entries.empty())
        {
            return nullptr;
        }

        return m_default;
        //tmp.
        //m_prop.get
        // auto tmp = static_cast<tcam::property::IPropertyEnum*>(m_prop.get());

        // return tmp->get_default();
    }

    auto get_property_value(uint32_t /*flags*/) -> outcome::result<std::string_view> final
    {
        auto tmp = m_prop.asEnumeration();

        auto entry = tmp.getSelectedEntry();
        return entry.getName();
    }

    auto set_property_value(std::string_view value, uint32_t /*flags*/) -> std::error_code final
    {

        auto tmp = m_prop.asEnumeration();

        auto entries = tmp.getEntries();
        for (const auto& e : entries)
        {
            if (value ==  e.getName())
            {
                // TODO: error handling
                tmp.selectEntry(e);
            }
        }
    }
};


struct TcamPropertyCommand : TcamPropertyBase<tcamprop1::property_interface_command>
{
    TcamPropertyCommand(ic4::Property& prop)
        : TcamPropertyBase { prop }
    {
    }

    auto execute_command(uint32_t /* flags */) -> std::error_code final
    {
        auto tmp = m_prop.asCommand();

        tmp.execute();
    }
};


struct TcamPropertyString : TcamPropertyBase<tcamprop1::property_interface_string>
{
    TcamPropertyString(ic4::Property& prop)
        : TcamPropertyBase { prop }
    {
    }


    auto get_property_value(uint32_t /*flags*/) -> outcome::result<std::string> final
    {
        auto tmp = m_prop.asString();
        std::string ret;
        tmp.getValue(ret);
        return ret;
    }

    auto set_property_value(std::string_view value, uint32_t /*flags*/) -> std::error_code final
    {
        auto tmp = m_prop.asString();

        tmp.setValue(std::string(value));
    }
};

} // namespace ic4::gst


auto ic4::gst::make_wrapper_instance(
    ic4::Property& prop)
    -> std::unique_ptr<tcamprop1::property_interface>
{
    switch (prop.getType())
    {
        case ic4::PropType::IC4_PROPTYPE_INTEGER:
        {
            return std::make_unique<ic4::gst::TcamPropertyInteger>(prop);
        }
        case ic4::PropType::IC4_PROPTYPE_FLOAT:
        {
            return std::make_unique<ic4::gst::TcamPropertyFloat>(prop);
        }
        case ic4::PropType::IC4_PROPTYPE_BOOLEAN:
        {
            return std::make_unique<ic4::gst::TcamPropertyBoolean>(prop);
        }
        case ic4::PropType::IC4_PROPTYPE_ENUMERATION:
        {
            return std::make_unique<ic4::gst::TcamPropertyEnumeration>(prop);
        }
        case ic4::PropType::IC4_PROPTYPE_COMMAND:
        {
            return std::make_unique<ic4::gst::TcamPropertyCommand>(prop);
        }
        case ic4::PropType::IC4_PROPTYPE_STRING:
        {
            return std::make_unique<ic4::gst::TcamPropertyString>(prop);
        }
        case ic4::PropType::IC4_PROPTYPE_CATEGORY:
        case ic4::PropType::IC4_PROPTYPE_ENUMENTRY:
        case ic4::PropType::IC4_PROPTYPE_REGISTER:
        case ic4::PropType::IC4_PROPTYPE_PORT:
        case ic4::PropType::IC4_PROPTYPE_INVALID:
        {
            return nullptr;
        }
    }
    return nullptr;
}

static auto tcamic4src_get_provider_impl_from_interface(TcamPropertyProvider* iface)
    -> tcamprop1_gobj::tcam_property_provider*
{
    assert(iface != nullptr);

    GstTcamIC4Src* self = GST_TCAM_IC4_SRC(iface);
    assert(self != nullptr);
    assert(self->device != nullptr);

    return &self->device->get_container();
}

void ic4::gst::ic4_tcam_property_init(TcamPropertyProviderInterface *iface)
{
    tcamprop1_gobj::init_provider_interface<tcamic4src_get_provider_impl_from_interface>(iface);
}
