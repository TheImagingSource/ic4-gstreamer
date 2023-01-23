/*
 * Copyright 2021 The Imaging Source Europe GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <gst/gst.h>

G_BEGIN_DECLS

#define TCAM_IC4_TYPE_DEVICE tcam_ic4_device_get_type()
#define TCAM_IC4_DEVICE(obj)                                                   \
  (G_TYPE_CHECK_INSTANCE_CAST((obj), TCAM_IC4_TYPE_DEVICE, TcamIC4Device))

typedef struct _TcamIC4Device TcamIC4Device;
typedef struct _TcamIC4DeviceClass TcamIC4DeviceClass;

struct _TcamIC4DeviceClass {
  GstDeviceClass parent_class;
};

struct _TcamIC4Device {
  GstDevice parent;
  GstElementFactory *factory;
};

GType tcam_ic4_device_get_type(void);

G_END_DECLS
