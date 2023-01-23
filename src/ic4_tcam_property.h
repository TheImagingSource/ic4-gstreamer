

#pragma once

#include "Tcam-1.0.h"
#include <ic4/ic4.h>
#include <tcam-property-1.0.h>

namespace ic4::gst
{

struct ic4_property {

    ic4::PropertyMap property_map;

    GSList *(*get_tcam_property_names)(TcamPropertyProvider *self,
                                       GError **err);
    TcamPropertyBase *(*get_tcam_property)(TcamPropertyProvider *self,
                                           const gchar *name, GError **err);

    void (*set_tcam_boolean)(TcamPropertyProvider *self, const gchar *name,
                             gboolean value, GError **err);
    void (*set_tcam_integer)(TcamPropertyProvider *self, const gchar *name,
                             gint64 value, GError **err);
    void (*set_tcam_float)(TcamPropertyProvider *self, const gchar *name,
                           gdouble value, GError **err);
    void (*set_tcam_enumeration)(TcamPropertyProvider *self, const gchar *name,
                                 const gchar *value, GError **err);
    void (*set_tcam_command)(TcamPropertyProvider *self, const gchar *name,
                             GError **err);

    gboolean (*get_tcam_boolean)(TcamPropertyProvider *self, const gchar *name,
                                 GError **err);
    gint64 (*get_tcam_integer)(TcamPropertyProvider *self, const gchar *name,
                               GError **err);
    gdouble (*get_tcam_float)(TcamPropertyProvider *self, const gchar *name,
                              GError **err);
    const gchar *(*get_tcam_enumeration)(TcamPropertyProvider *self,
                                         const gchar *name, GError **err);
};


void ic4_tcam_property_init(TcamPropertyProviderInterface *iface);

}
