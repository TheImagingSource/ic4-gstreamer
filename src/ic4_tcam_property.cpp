
#include "ic4_tcam_property.h"


namespace {
template <class TBase> struct TcamPropertyBase : TBase {
  TcamPropertyBase(std::shared_ptr<tcam::property::IPropertyBase> prop)
      : m_prop(prop) {}

  std::shared_ptr<tcam::property::IPropertyBase> m_prop;

  auto get_property_name() const noexcept -> std::string_view final {
    return m_prop->get_name();
  }
  auto get_property_info() const noexcept -> tcamprop1::prop_static_info final {
    return m_prop->get_static_info();
  }

  auto get_property_state(uint32_t /*flags = 0*/)
      -> outcome::result<tcamprop1::prop_state> final {
    auto flags = m_prop->get_flags();

    tcamprop1::prop_state ret = {};
    ret.is_implemented = flags & tcam::property::PropertyFlags::Implemented;
    ret.is_locked = tcam::property::is_locked(flags);
    ret.is_available = flags & tcam::property::PropertyFlags::Available;
    ret.is_name_hidden = flags & tcam::property::PropertyFlags::Hidden;
    return ret;
  }
};
}
void ic4::gst::ic4_tcam_property_init(TcamPropertyProviderInterface *iface)
{}
