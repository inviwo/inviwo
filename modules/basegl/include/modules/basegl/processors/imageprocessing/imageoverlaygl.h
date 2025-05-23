/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2025 Inviwo Foundation
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

#include <modules/basegl/baseglmoduledefine.h>  // for IVW_MODULE_BASEGL_API

#include <inviwo/core/ports/imageport.h>                   // for ImageInport, ImageOutport
#include <inviwo/core/processors/processor.h>              // for Processor
#include <inviwo/core/processors/processorinfo.h>          // for ProcessorInfo
#include <inviwo/core/properties/boolcompositeproperty.h>  // for BoolCompositeProperty
#include <inviwo/core/properties/boolproperty.h>           // for BoolProperty
#include <inviwo/core/properties/compositeproperty.h>      // for CompositeProperty
#include <inviwo/core/properties/invalidationlevel.h>      // for InvalidationLevel, Invalidatio...
#include <inviwo/core/properties/optionproperty.h>         // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>        // for FloatVec2Property, IntVec2Prop...
#include <inviwo/core/properties/propertysemantics.h>      // for PropertySemantics, PropertySem...
#include <inviwo/core/util/glmvec.h>                       // for ivec2, ivec4
#include <inviwo/core/util/staticstring.h>                 // for operator+
#include <modules/basegl/viewmanager.h>                    // for ViewManager
#include <modules/opengl/inviwoopengl.h>                   // for GLint, GL_NONE, GL_SRC_ALPHA
#include <modules/opengl/shader/shader.h>                  // for Shader

#include <functional>   // for __base
#include <string>       // for operator==, string
#include <string_view>  // for operator==
#include <vector>       // for operator!=, vector, operator==

namespace inviwo {
class Deserializer;
class Event;
class Outport;

/** @class OverlayProperty
 *
 * @brief CompositeProperty for overlay images. An overlay is defined by
 * a position, a anchor position, and the blend mode used for compositing.
 */
// TODO: implement absolute positioning.
//    this will require the image dimensions of the source and the overlay
class IVW_MODULE_BASEGL_API OverlayProperty : public CompositeProperty {
public:
    enum class Positioning { Relative, Absolute };
    enum class BlendMode : GLint { Replace = GL_NONE, Over = GL_SRC_ALPHA };

    OverlayProperty(std::string identifier, std::string displayName,
                    InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
                    PropertySemantics semantics = PropertySemantics::Default);
    virtual ~OverlayProperty() {}

    void setViewDimensions(ivec2 viewDim);

    const ivec4& getViewport() const;

    BlendMode getBlendMode() const;
    GLint getBlendModeGL() const;

private:
    virtual void deserialize(Deserializer& d) override;
    void updateViewport();
    void updateVisibilityState();

    FloatVec2Property pos_;   //<! relative position [0,1]
    FloatVec2Property size_;  //<! relative size[0,1]

    IntVec2Property absolutePos_;   //<! absolute position [pixel]
    IntVec2Property absoluteSize_;  //<! absolute size [pixel]

    FloatVec2Property anchorPos_;

    OptionProperty<BlendMode> blendMode_;

    // consider absolute positioning
    OptionProperty<Positioning> positioningMode_;
    OptionProperty<Positioning> sizeMode_;

    ivec2 viewDimensions_;
    ivec4 viewport_;

    bool isDeserializing_;
};

/*!
 * @brief Places one or more input images on top of the source image.
 */
class IVW_MODULE_BASEGL_API ImageOverlayGL : public Processor {
public:
    ImageOverlayGL();
    virtual ~ImageOverlayGL();

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    virtual void propagateEvent(Event*, Outport* source) override;

protected:
    virtual void process() override;
    virtual void initializeResources() override;

    void updateViewports(ivec2 size, bool force = false);
    void onStatusChange();

private:
    ImageInport inport_;
    ImageInport overlayPort_;
    ImageOutport outport_;

    BoolProperty enabled_;
    BoolProperty overlayInteraction_;  //<! allows to interact with overlay images, otherwise only
                                       // the source image will receive interaction events
    BoolProperty passThroughEvent_;

    OverlayProperty overlayProperty_;

    BoolCompositeProperty border_;
    FloatVec4Property borderColor_;
    IntProperty borderWidth_;

    Shader shader_;
    ViewManager viewManager_;
    ivec2 currentDim_;
};

}  // namespace inviwo
