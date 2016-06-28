/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#ifndef IVW_BRUSHINGANDLINKINGOUTPORT_H
#define IVW_BRUSHINGANDLINKINGOUTPORT_H

#include <modules/brushingandlinking/brushingandlinkingmoduledefine.h>
#include <modules/brushingandlinking/brushingandlinkingmanager.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>
#include <inviwo/core/ports/port.h>

namespace inviwo {


using BrushingAndLinkingInport = DataInport<BrushingAndLinkingManager>;

class IVW_MODULE_BRUSHINGANDLINKING_API BrushingAndLinkingOutport : public DataOutport<BrushingAndLinkingManager> {
public:
    BrushingAndLinkingOutport(std::string identifier) : DataOutport<BrushingAndLinkingManager>(identifier){}
    virtual ~BrushingAndLinkingOutport() = default;
};

template <>
struct port_traits<BrushingAndLinkingManager> {
    static std::string class_identifier() { return "BrushingAndLinkingManager"; }
    static uvec3 color_code() { return uvec3(160, 182, 240); }
    static std::string data_info(const BrushingAndLinkingManager* data) {
        std::ostringstream oss;
        oss << "Number of selected indices: " << data->getNumberOfSelected() << std::endl;
        oss << "Number of filtered indices: " << data->getNumberOfFiltered();
        return oss.str();
    }
};

} // namespace

#endif // IVW_BRUSHINGANDLINKINGOUTPORT_H

