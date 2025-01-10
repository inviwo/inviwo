/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2023-2025 Inviwo Foundation
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
#pragma once

#include <inviwo/core/common/inviwocoredefine.h>

#include <inviwo/core/datastructures/datatraits.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>
#include <inviwo/core/util/document.h>

namespace inviwo {

using LayerOutport = DataOutport<Layer>;
using LayerInport = DataInport<Layer>;
using LayerMultiInport = DataInport<Layer, 0>;
using LayerFlatMultiInport = DataInport<Layer, 0, true>;

using LayerSequenceOutport = DataOutport<LayerSequence>;

template <>
struct DataTraits<Layer> {
    static constexpr std::string_view classIdentifier() { return "org.inviwo.Layer"; }
    static constexpr std::string_view dataName() { return "Layer"; }
    static constexpr uvec3 colorCode() { return {95, 204, 114}; }
    static Document info(const Layer& layer) {
        using H = utildoc::TableBuilder::Header;
        using P = Document::PathComponent;
        Document doc;
        doc.append("b", "Layer", {{"style", "color:white;"}});

        utildoc::TableBuilder tb(doc.handle(), P::end());

        tb(H("Format"), layer.getDataFormat()->getString());
        tb(H("Dimension"), layer.getDimensions());
        tb(H("SwizzleMask"), layer.getSwizzleMask());
        tb(H("Interpolation"), layer.getInterpolation());
        tb(H("Wrapping"), layer.getWrapping());
        tb(H("Data Range"), layer.dataMap.dataRange);
        tb(H("Value Range"), layer.dataMap.valueRange);
        tb(H("Value"),
           fmt::format("{}{: [}", layer.dataMap.valueAxis.name, layer.dataMap.valueAxis.unit));
        tb(H("Axis 1"), fmt::format("{}{: [}", layer.axes[0].name, layer.axes[0].unit));
        tb(H("Axis 2"), fmt::format("{}{: [}", layer.axes[1].name, layer.axes[1].unit));

        tb(H("Basis"), layer.getBasis());
        tb(H("Offset"), layer.getOffset());

        return doc;
    }
};

}  // namespace inviwo
