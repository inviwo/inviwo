/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <modules/basegl/baseglmoduledefine.h>

#include <modules/basegl/shadercomponents/shadercomponent.h>
#include <modules/opengl/volume/volumeutils.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/selectioncolorproperty.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <string>
#include <vector>

namespace inviwo {

/**
 * Adds an option to render segments in range [0, 3] of type UINT, and render these
 * with or without lighting. See the Atlas Boundary processor for such data.
 */
class IVW_MODULE_BASEGL_API AtlasIsosurfaceComponent : public ShaderComponent {
public:
    AtlasIsosurfaceComponent(std::string_view volume);
    virtual std::string_view getName() const override;
    virtual void process(Shader& shader, TextureUnitContainer&) override;
    virtual std::vector<Property*> getProperties() override;
    virtual std::vector<Segment> getSegments() override;

private:
    VolumeInport volume_;
    std::string name_;
    std::shared_ptr<const Volume> smoothVolume_;

    BoolCompositeProperty useAtlasBoundary_;
    BoolProperty applyBoundaryLight_;
    SelectionColorProperty showHighlighted_;
    SelectionColorProperty showSelected_;
    SelectionColorProperty showFiltered_;
};

}  // namespace inviwo
