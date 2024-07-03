
#include "ic4_tcam_property.h"
#include "Tcam-1.0.h"
#include "gst/gstinfo.h"
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
#include <string>
#include <string_view>

#include "../libs/gst-helper/include/tcamprop1.0_base/tcamprop_property_interface.h"
#include "tcamprop1.0_base/tcamprop_base.h"
#include "tcamprop1.0_base/tcamprop_errors.h"
#include "tcamprop1.0_base/tcamprop_property_info.h"

namespace
{

tcamprop1::status ic4_error_to_std(const ic4::Error& err)
{
    switch (err.code())
    {
        case ic4::ErrorCode::NoError:
        {
            return tcamprop1::status::success;
        }
        case ic4::ErrorCode::OutOfMemory:
        case ic4::ErrorCode::InvalidOperation:
        case ic4::ErrorCode::Internal:
        {}
        case ic4::ErrorCode::LibraryNotInitialized:
        {}
        case ic4::ErrorCode::DriverError:
        {}
        case ic4::ErrorCode::InvalidParameter:
        {}
        case ic4::ErrorCode::ConversionNotSupported:
        {}
        case ic4::ErrorCode::NoData:
        {}
        case ic4::ErrorCode::GenICamFeatureNotFound:
        {}
        case ic4::ErrorCode::GenICamDeviceError:
        {}
        case ic4::ErrorCode::GenICamTypeMismatch:
        {}
        case ic4::ErrorCode::GenICamAccessDenied:
        {}
        case ic4::ErrorCode::GenICamNotImplemented:
        {}
        case ic4::ErrorCode::GenICamValueError:
        {}
        case ic4::ErrorCode::GenICamChunkdataNotConnected:
        {}
        case ic4::ErrorCode::BufferTooSmall:
        {}
        case ic4::ErrorCode::SinkTypeMismatch:
        {}
        case ic4::ErrorCode::SnapAborted:
        {}
        case ic4::ErrorCode::FileWriteError:
        {}
        case ic4::ErrorCode::FileAccessDenied:
        {}
        case ic4::ErrorCode::FilePathNotFound:
        {}
        case ic4::ErrorCode::FileReadError:
        {}
        case ic4::ErrorCode::DeviceInvalid:
        {}
        case ic4::ErrorCode::DeviceNotFound:
        {}
        case ic4::ErrorCode::DeviceError:
        {}
        case ic4::ErrorCode::Ambiguous:
        {}
        case ic4::ErrorCode::ParseError:
        {}
        case ic4::ErrorCode::Timeout:
        {
            //return tcamprop1::status::
        }
        case ic4::ErrorCode::Incomplete:
        {}
        case ic4::ErrorCode::SinkNotConnected:
        {}
        case ic4::ErrorCode::ImageTypeMismatch:
        {}
        case ic4::ErrorCode::SinkAlreadyAttached:
        {}
        case ic4::ErrorCode::SinkConnectAborted:
        {}
        case ic4::ErrorCode::HandlerAlreadyRegistered:
        {}
        case ic4::ErrorCode::HandlerNotFound:
        {}
        case ic4::ErrorCode::Unknown:
        default:
        {
            return tcamprop1::status::unknown;
        }
    }
}

tcamprop1::Visibility_t vis_to_prop(ic4::PropVisibility vis)
{
    switch (vis)
    {
        case ic4::PropVisibility::Beginner: ///< Beginner visibility
        {
            return tcamprop1::Visibility_t::Beginner;
        }
        case ic4::PropVisibility::Expert: ///< Expert visibility
        {
            return tcamprop1::Visibility_t::Expert;
        }
        case ic4::PropVisibility::Guru: ///< Guru visibility
        {
            return tcamprop1::Visibility_t::Guru;
        }
        case ic4::PropVisibility::Invisible:
        {
            return tcamprop1::Visibility_t::Invisible;
        } ///< Invisible
    }
    return tcamprop1::Visibility_t::Beginner;
}

TcamPropertyVisibility visibility_to_tcamprop(ic4::PropVisibility vis)
{
    switch (vis)
    {
        case ic4::PropVisibility::Beginner:
        {
            return TcamPropertyVisibility::TCAM_PROPERTY_VISIBILITY_BEGINNER;
        }
        case ic4::PropVisibility::Expert:
        {
            return TcamPropertyVisibility::TCAM_PROPERTY_VISIBILITY_EXPERT;
        }
        case ic4::PropVisibility::Guru:
        {
            return TcamPropertyVisibility::TCAM_PROPERTY_VISIBILITY_GURU;
        }
        case ic4::PropVisibility::Invisible:
        {
            return TcamPropertyVisibility::TCAM_PROPERTY_VISIBILITY_INVISIBLE;
        }
    }
    return TcamPropertyVisibility::TCAM_PROPERTY_VISIBILITY_BEGINNER;

}

} //namespace

namespace ic4::gst
{

template <class TBase> struct TcamPropertyBase : TBase
{
    TcamPropertyBase(ic4::Property& prop, const std::string& category)
        : m_prop(prop)
    {
        m_name = m_prop.name();
        m_display_name = m_prop.displayName();
        m_description = m_prop.description();
        m_category = category;
    }

    ic4::Property m_prop;
    // caching to prevent string_view issues
    std::string m_name;
    std::string m_display_name;
    std::string m_description;
    std::string m_category;

    auto get_property_name() const noexcept -> std::string_view final
    {
        return m_name.c_str();
    }

    auto get_property_info() const noexcept -> tcamprop1::prop_static_info final
    {
        //return m_prop.get_static_info();

        tcamprop1::prop_static_info info = {};

        info.visibility = vis_to_prop(m_prop.visibility());
        info.name = m_name;
        info.display_name = m_display_name;
        info.description = m_description;

        if (!m_category.empty())
        {
            info.iccategory = m_category;
        }

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
    TcamPropertyInteger(ic4::Property& prop, const std::string& category)
        : TcamPropertyBase { prop, category }
    {
        auto tmp = m_prop.asInteger();

        m_unit = tmp.unit();
    }

    std::string m_unit;

    auto get_property_range(uint32_t /* flags = 0 */)
        -> outcome::result<tcamprop1::prop_range_integer> final
    {
        auto tmp = m_prop.asInteger();

        tcamprop1::prop_range_integer ret = {};

        ret.min = tmp.minimum();
        ret.max = tmp.maximum();
        ret.stp = tmp.increment();

        return ret;
    }

    auto get_property_default(uint32_t /* flags = 0 */) -> outcome::result<int64_t> final
    {
        auto tmp = m_prop.asInteger();

        //int64_t ret = tmp.getDefault();
        //return ret;
        return 0;
    }

    auto get_property_value(uint32_t /*flags*/) -> outcome::result<int64_t> final
    {
        auto tmp = m_prop.asInteger();

        ic4::Error err;
        int64_t ret = tmp.getValue(err);
        if (err.isError())
        {
            return ic4_error_to_std(err);
        }
        else
        {
            GST_INFO("%s is value %ld", m_name.c_str(), ret);
        }

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
            return tcamprop1::status::success;
            //tcam::status::Success;
        }
        //return err.getVal().;
        // TODO: error handling
        //return err;//. .error();
        return ic4_error_to_std(err);
    }

    auto get_representation() const noexcept -> tcamprop1::IntRepresentation_t final
    {
        auto tmp = m_prop.asInteger();

        auto ic4_rep = tmp.representation();

        auto int_rep_to_tcamprop = [] (ic4::PropIntRepresentation rep)
            -> tcamprop1::IntRepresentation_t
        {
            switch (rep)
            {
                case ic4::PropIntRepresentation::Boolean:
                {
                    return tcamprop1::IntRepresentation_t::Boolean;
                }
                case ic4::PropIntRepresentation::HexNumber:
                {
                    return tcamprop1::IntRepresentation_t::HexNumber;
                }
                case ic4::PropIntRepresentation::IPV4Address:
                {
                    return tcamprop1::IntRepresentation_t::IPV4Address;
                }
                case ic4::PropIntRepresentation::Logarithmic:
                {
                    return tcamprop1::IntRepresentation_t::Logarithmic;
                }
                case ic4::PropIntRepresentation::Linear:
                {
                    return tcamprop1::IntRepresentation_t::Linear;
                }
                case ic4::PropIntRepresentation::MACAddress:
                {
                    return tcamprop1::IntRepresentation_t::MACAddress;
                }
                case ic4::PropIntRepresentation::PureNumber:
                {
                    return tcamprop1::IntRepresentation_t::PureNumber;
                }
            }
            return tcamprop1::IntRepresentation_t::Linear;
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
    TcamPropertyFloat(ic4::Property& prop, const std::string& category)
        : TcamPropertyBase { prop, category }
    {
        auto tmp = m_prop.asFloat();

        m_unit = tmp.unit();
    }

    std::string m_unit;

    auto get_property_range(uint32_t /* flags = 0 */)
        -> outcome::result<tcamprop1::prop_range_float> final
    {
        auto tmp = m_prop.asFloat();

        tcamprop1::prop_range_float ret = {};

        ret.min = tmp.minimum();
        ret.max = tmp.maximum();
        ret.stp = tmp.increment();

        return ret;
    }

    auto get_property_default(uint32_t /* flags = 0 */) -> outcome::result<double> final
    {
        return 0.0;
        // auto tmp = m_prop.asFloat();

        // double ret;

        // tmp.getDefault(ret);
        // return ret;
    }

    auto get_property_value(uint32_t /*flags*/) -> outcome::result<double> final
    {
        auto tmp = m_prop.asFloat();

        ic4::Error err;
        double ret = tmp.getValue(err);
        if (err.isError())
        {
            GST_ERROR("%s - %f - %s", m_name.c_str(), ret, err.message().c_str());
        }
        else
        {

            GST_ERROR("got float value %s -> %f", m_name.c_str(), ret);
        }

        return ret;
    }

    auto set_property_value(double value, uint32_t /*flags*/) -> std::error_code final
    {
        auto tmp = m_prop.asFloat();
        ic4::Error err;
        auto ret = tmp.setValue(value, err);
        if (ret)
        {
            return tcamprop1::status::success;
            //            return tcam::status::Success;
        }
        else
        {
            GST_ERROR("%s", err.message().c_str());
        }
        //return err.getVal().;
        // TODO: error handling
        return ic4_error_to_std(err);
    }

    auto get_representation() const noexcept -> tcamprop1::FloatRepresentation_t final
    {
        auto tmp = m_prop.asFloat();

        auto ic4_rep = tmp.representation();

        auto float_rep_to_tcamprop =
            [](ic4::PropFloatRepresentation rep) -> tcamprop1::FloatRepresentation_t
        {
            switch (rep)
            {
                case ic4::PropFloatRepresentation::Linear:
                {
                    return tcamprop1::FloatRepresentation_t::Linear;
                }
                case ic4::PropFloatRepresentation::Logarithmic:
                {
                    return tcamprop1::FloatRepresentation_t::Logarithmic;
                }
                case ic4::PropFloatRepresentation::PureNumber:
                {
                    return tcamprop1::FloatRepresentation_t::PureNumber;
                }

            }
            return tcamprop1::FloatRepresentation_t::Linear;

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
    TcamPropertyBoolean(ic4::Property& prop, const std::string& category)
        : TcamPropertyBase { prop, category }
    {}

    auto get_property_default(uint32_t /* flags = 0 */) -> outcome::result<bool> final
    {
        return false;
        // auto tmp = m_prop.asBoolean();

        // bool ret;
        // tmp.getDefault(ret);
        // return ret;
    }

    auto get_property_value(uint32_t /*flags*/) -> outcome::result<bool> final
    {
        auto tmp = m_prop.asBoolean();

        bool val = tmp.getValue();
        return val;
    }

    auto set_property_value(bool value, uint32_t /*flags*/) -> std::error_code final
    {
        auto tmp = m_prop.asBoolean();

        ic4::Error err;
        auto ret = tmp.setValue(value, err);

        // if (property::is_locked(m_prop.get_flags()))
        // {
        //     return tcam::status::PropertyNotWriteable;
        // }

        // auto ret = tmp->set_value(value);

        if (ret)
        {
            return tcamprop1::status::success;
        }

        GST_ERROR("Error while setting Bool %s: %s", m_prop.name().c_str(), err.message().c_str());
        return ic4_error_to_std(err);
    }
};


struct TcamPropertyEnumeration : TcamPropertyBase<tcamprop1::property_interface_enumeration>
{

    TcamPropertyEnumeration(ic4::Property& prop, const std::string& category)
        : TcamPropertyBase { prop, category }
    {
        auto tmp = m_prop.asEnumeration();
        m_entries = tmp.entries();
        if (!m_entries.empty())
        {
            m_default = m_entries.at(0).name();
        }
    }

    std::string m_default;
    std::string m_value;
    std::vector<ic4::PropEnumEntry> m_entries;

    auto get_property_range(uint32_t /* flags = 0 */)
        -> outcome::result<tcamprop1::prop_range_enumeration> final
    {
        std::vector<std::string> vals;
        for (const auto& e : m_entries)
        {
            vals.push_back(e.name());
        }

        return tcamprop1::prop_range_enumeration { vals };
    }

    auto get_property_default(uint32_t /*flags = 0 */) -> outcome::result<std::string_view> final
    {
        return m_default;
    }

    auto get_property_value(uint32_t /*flags*/) -> outcome::result<std::string_view> final
    {
        auto tmp = m_prop.asEnumeration();

        auto entry = tmp.selectedEntry();

        m_value = entry.name();

        return m_value;
    }

    auto set_property_value(std::string_view value, uint32_t /*flags*/) -> std::error_code final
    {

        auto tmp = m_prop.asEnumeration();
        ic4::Error err;
        auto entries = tmp.entries();
        for (const auto& e : entries)
        {
            if (value ==  e.name())
            {
                tmp.selectEntry(e, err);
                break;
            }
        }

        return ic4_error_to_std(err);
    }
};


struct TcamPropertyCommand : TcamPropertyBase<tcamprop1::property_interface_command>
{
    TcamPropertyCommand(ic4::Property& prop, const std::string& category)
        : TcamPropertyBase { prop, category }
    {
    }

    auto execute_command(uint32_t /* flags */) -> std::error_code final
    {
        auto tmp = m_prop.asCommand();

        tmp.execute();

        return tcamprop1::status::success;

    }
};


struct TcamPropertyString : TcamPropertyBase<tcamprop1::property_interface_string>
{
    TcamPropertyString(ic4::Property& prop, const std::string& category)
        : TcamPropertyBase { prop, category }
    {}


    auto get_property_value(uint32_t /*flags*/) -> outcome::result<std::string> final
    {
        auto tmp = m_prop.asString();
        ic4::Error err;
        std::string ret = tmp.getValue(err);
        return ret;
    }

    auto set_property_value(std::string_view value, uint32_t /*flags*/) -> std::error_code final
    {
        auto tmp = m_prop.asString();

        ic4::Error err;

        auto ret = tmp.setValue(std::string(value), err);

        if (ret)
        {
            return tcamprop1::status::success;
        }

        return ic4_error_to_std(err);
        //return  err.code();
        //return ret.Code;
    }
};

} // namespace ic4::gst


auto ic4::gst::make_wrapper_instance(ic4::Property& prop, const std::string& category)
    -> std::unique_ptr<tcamprop1::property_interface>
{
    switch (prop.type())
    {
        case ic4::PropType::Integer:
        {
            return std::make_unique<ic4::gst::TcamPropertyInteger>(prop, category);
        }
        case ic4::PropType::Float:
        {
            return std::make_unique<ic4::gst::TcamPropertyFloat>(prop, category);
        }
        case ic4::PropType::Boolean:
        {
            return std::make_unique<ic4::gst::TcamPropertyBoolean>(prop, category);
        }
        case ic4::PropType::Enumeration:
        {
            return std::make_unique<ic4::gst::TcamPropertyEnumeration>(prop, category);
        }
        case ic4::PropType::Command:
        {
            return std::make_unique<ic4::gst::TcamPropertyCommand>(prop, category);
        }
        case ic4::PropType::String:
        {
            return std::make_unique<ic4::gst::TcamPropertyString>(prop, category);
        }
        case ic4::PropType::Category:
        case ic4::PropType::EnumEntry:
        case ic4::PropType::Register:
        case ic4::PropType::Port:
        case ic4::PropType::Invalid:
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
