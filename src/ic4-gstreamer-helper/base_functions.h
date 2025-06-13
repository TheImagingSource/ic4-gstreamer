/*
 * Copyright 2020 The Imaging Source Europe GmbH
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

#include <string>
#include <gst/gst.h>
#include "../helper/gst_ptr.h"

namespace ic4::gst::helper
{


    bool is_valid_device_serial (const std::string& serial);


    gst_helper::gst_ptr<GstElement> open_element (const std::string& element_name);


    /*
     * Helper struct for state handling
     */
    struct ElementStateGuard
    {
        explicit ElementStateGuard(GstElement& element);
        
        ~ElementStateGuard();
        
        bool set_state(GstState state);
        
        GstElement& p_element;
    };

    bool block_until_state_change_done (GstElement* pipeline);


} // namespace ic4::gst::helper
