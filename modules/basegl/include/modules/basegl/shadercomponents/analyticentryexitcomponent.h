/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#include <inviwo/core/datastructures/coordinatetransformer.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <modules/basegl/shadercomponents/shadercomponent.h>

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace inviwo {
class Property;
class Shader;
class TextureUnitContainer;

/**
 * A replacement for the EntryExitComponent that computes the entry and exit points analytically
 * in the fragment shader, by intersecting the camera ray of each fragment with a parallelepiped or
 * a sphere. No entry and exit point images are needed.
 *
 * The geometry is defined in a selectable coordinate space of the volume (Data, Model, or World).
 * In Data space the volume occupies the unit cube, hence the default parallelepiped (origin 0 and
 * unit edges) reproduces the volume bounding box. Optionally, the geometry is also clipped
 * against the volume bounding box, and it is always clipped against the near and far planes of
 * the camera.
 *
 * Like the EntryExitComponent, the setup segment defines `entryPoint`, `exitPoint`,
 * `entryPointDepth`, `exitPointDepth`, `rayLength`, and `rayDirection`. The `surfaceNormal`
 * sampler and `useSurfaceNormals` are declared for compatibility with other components, but
 * `useSurfaceNormals` is always false. Requires a camera uniform named by `cameraName`.
 */
class IVW_MODULE_BASEGL_API AnalyticEntryExitComponent : public ShaderComponent {
public:
    enum class Shape : std::uint8_t { Parallelepiped, Sphere };

    explicit AnalyticEntryExitComponent(VolumeInport& volume,
                                        std::string_view cameraName = "camera");

    virtual std::string_view getName() const override;

    virtual void process(Shader& shader, TextureUnitContainer& cont) override;

    virtual std::vector<Property*> getProperties() override;

    virtual std::vector<Segment> getSegments() override;

    CompositeProperty geometry;
    OptionProperty<Shape> shape;
    OptionProperty<CoordinateSpace> space;
    BoolProperty clipToVolume;

    FloatVec3Property origin;
    FloatVec3Property edge1;
    FloatVec3Property edge2;
    FloatVec3Property edge3;

    FloatVec3Property center;
    FloatProperty radius;

private:
    VolumeInport* volume_;
    std::string cameraName_;
};

}  // namespace inviwo
