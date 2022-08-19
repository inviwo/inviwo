/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2021 Inviwo Foundation
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
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/interaction/events/mouseevent.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/interaction/interactionhandler.h>
#include <inviwo/core/ports/imageport.h>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/inviwoopengl.h>
#include <modules/basegl/viewmanager.h>

namespace inviwo {

class Shader;

/*! \class OverlayProperty
 *
 * \brief CompositeProperty for overlay images. An overlay is defined by
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

/** \docpage{org.inviwo.ImageOverlayGL, Image Overlay}
 * Places one or more input images on top of the source image.
 * ![](org.inviwo.ImageOverlayGL.png?classIdentifier=org.inviwo.ImageOverlayGL)
 *
 * ### Inports
 *   * __ImageInport__ Source image.
 *   * __ImageInport__ Overlay images (multi-port).
 *
 * ### Outports
 *   * __ImageOutport__ The output image.
 *
 * ### Properties
 *   * __Overlay Interaction__ Allow interactions on overlay images.
 *   * __Pass Events to Main View__  Events unhandled by the overlay will be passed
 *                           on to the main view
 *   * __Size__              Size of the overlay image.
 *   * __Position__          Position of the overlay image.
 *   * __Anchor Position__   Anchor of the overlay image for alignment.
 *   * __Blend Mode__        Blend mode used for mixing the overlay image.
 */

/*! \class ImageOverlayGL
 *
 * \brief Places one or more input images on top of the source image.
 */
class IVW_MODULE_BASEGL_API ImageOverlayGL : public Processor {
public:
    ImageOverlayGL();
    virtual ~ImageOverlayGL();

    virtual const ProcessorInfo getProcessorInfo() const override;
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
