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

#include <inviwo/core/datastructures/spatialdata.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/processors/processortraits.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/eventproperty.h>

#include <modules/basegl/interaction/gizmo.h>

namespace inviwo {

class Event;

template <typename T>
class TransformGizmo : public Processor {
public:
    TransformGizmo();
    TransformGizmo(const TransformGizmo&) = delete;
    TransformGizmo(TransformGizmo&&) = delete;
    TransformGizmo& operator=(const TransformGizmo&) = delete;
    TransformGizmo& operator=(TransformGizmo&&) = delete;
    virtual ~TransformGizmo() = default;

    virtual void process() override;
    virtual void invokeEvent(Event* e) override;

    virtual const ProcessorInfo& getProcessorInfo() const override;

private:
    DataInport<T> inport_;
    DataOutport<T> outport_;
    DataOutport<GizmoData> gizmoOutport_;

    BoolProperty enable_;
    OptionProperty<GizmoType> gizmoType_;
    OptionProperty<CoordinateSpace> space_;
    DoubleMat4Property gizmoBasis_;
    CompositeProperty outputProps_;
    DoubleMat4Property worldBasis_;
    CameraProperty camera_;
    CompositeProperty interaction_;
    EventProperty cancelInteractionEvent_;
    EventProperty hideGizmoShortcut_;
    EventProperty moveShortcut_;
    EventProperty rotateShortcut_;
    EventProperty moveRotateShortcut_;

    Gizmo gizmo_;
};

template <typename T>
const ProcessorInfo& TransformGizmo<T>::getProcessorInfo() const {
    static const ProcessorInfo info{ProcessorTraits<TransformGizmo<T>>::getProcessorInfo()};
    return info;
}

class Layer;

template <>
struct ProcessorTraits<TransformGizmo<Layer>> {
    static ProcessorInfo getProcessorInfo() {
        return {"org.inviwo.TransformGizmoLayer",  // Class identifier
                "Transform Layer Gizmo",           // Display name
                "Coordinate Transforms",           // Category
                CodeState::Stable,                 // Code state
                Tags::CPU | Tag{"Transform"} | Tag{"Gizmo"} | Tag{"Interaction"} |
                    Tag{"Layer"},  // Tags
                R"(Apply a transformation adjusted with a interactive gizmo to a Layer.
    The transform and Gizmo interactions take place in different coordinate spaces:
    * World     Apply the transform in world coordinates, this will update the World Matrix (Transform * World * Model)
    * Model     Apply the transform in model coordinates, that is after the model transform, this will update the Model Matrix (World * (Transform * Model))
    * Data      Apply the transform before the model transform in data coordinates, this will update the Model Matrix (World * Model * Transform)"_unindentHelp};
    }
};

class Mesh;

template <>
struct ProcessorTraits<TransformGizmo<Mesh>> {
    static ProcessorInfo getProcessorInfo() {
        return {
            "org.inviwo.TransformGizmoMesh",  // Class identifier
            "Transform Mesh Gizmo",           // Display name
            "Coordinate Transforms",          // Category
            CodeState::Stable,                // Code state
            Tags::CPU | Tag{"Transform"} | Tag{"Gizmo"} | Tag{"Interaction"} | Tag{"Mesh"},  // Tags
            R"(Apply a transformation adjusted with a interactive gizmo to a Mesh.
    The transform and Gizmo interactions take place in different coordinate spaces:
    * World     Apply the transform in world coordinates, this will update the World Matrix (Transform * World * Model)
    * Model     Apply the transform in model coordinates, that is after the model transform, this will update the Model Matrix (World * (Transform * Model))
    * Data      Apply the transform before the model transform in data coordinates, this will update the Model Matrix (World * Model * Transform)"_unindentHelp};
    }
};

class Volume;

template <>
struct ProcessorTraits<TransformGizmo<Volume>> {
    static ProcessorInfo getProcessorInfo() {
        return {"org.inviwo.TransformGizmoVolume",  // Class identifier
                "Transform Volume Gizmo",           // Display name
                "Coordinate Transforms",            // Category
                CodeState::Stable,                  // Code state
                Tags::CPU | Tag{"Transform"} | Tag{"Gizmo"} | Tag{"Interaction"} |
                    Tag{"Volume"},  // Tags
                R"(Apply a transformation adjusted with a interactive gizmo to a Volume.
    The transform and Gizmo interactions take place in different coordinate spaces:
    * World     Apply the transform in world coordinates, this will update the World Matrix (Transform * World * Model)
    * Model     Apply the transform in model coordinates, that is after the model transform, this will update the Model Matrix (World * (Transform * Model))
    * Data      Apply the transform before the model transform in data coordinates, this will update the Model Matrix (World * Model * Transform)"_unindentHelp};
    }
};

template <typename T>
TransformGizmo<T>::TransformGizmo()
    : Processor{}
    , inport_{"inport"}
    , outport_{"outport"}
    , gizmoOutport_{"gizmo"}
    , enable_{"enable", "Enabled", true}
    , gizmoType_{"gizmoType", "Gizmo Type",
                 OptionPropertyState<GizmoType>{
                     .options = {{"none", "None", GizmoType::None},
                                 {"move", "Move", GizmoType::Move},
                                 {"rotate", "Rotate", GizmoType::Rotate},
                                 {"moveRotate", "Move & Rotate", GizmoType::MoveRotate}},
                     .help = "Sets the type of interaction gizmo"_help}
                     .setSelectedValue(GizmoType::MoveRotate)}
    , space_{"space", "Space",
             OptionPropertyState<CoordinateSpace>{
                 .options = {{"world", "World", CoordinateSpace::World},
                             {"model", "Model", CoordinateSpace::Model},
                             {"data", "Data", CoordinateSpace::Data}},
                 .help = "Domain of sample position (model, world, or data)"_help}
                 .setSelectedValue(CoordinateSpace::World)}
    , gizmoBasis_("gizmoBasis", "Gizmo Basis", dmat4{1.0},
                  {util::filled<dmat4>(-10.0), ConstraintBehavior::Ignore},
                  {util::filled<dmat4>(10.0), ConstraintBehavior::Ignore},
                  util::filled<dmat4>(0.001), InvalidationLevel::InvalidOutput,
                  PropertySemantics::Text)
    , outputProps_{"outputProperties", "Output"}
    , worldBasis_("worldBasis", "World Basis", dmat4{1.0},
                  {util::filled<dmat4>(-10.0), ConstraintBehavior::Ignore},
                  {util::filled<dmat4>(10.0), ConstraintBehavior::Ignore},
                  util::filled<dmat4>(0.001), InvalidationLevel::Valid, PropertySemantics::Text)
    , camera_{"camera", "Camera"}
    , interaction_{"interaction", "Interaction"}
    , cancelInteractionEvent_{"cancel",
                              "Cancel Interaction",
                              "Shortcut to cancel an interaction"_help,
                              [](Event*) { /* dummy, handled in invokeEvent() */ },
                              IvwKey::Escape,
                              KeyState::Press,
                              KeyModifiers{flags::none}}
    , hideGizmoShortcut_{"hideGizmo",
                         "Hide Gizmo",
                         "Shortcut to hide the gizmo"_help,
                         [this](Event* e) {
                             gizmoType_.setSelectedValue(GizmoType::None);
                             e->markAsUsed();
                         },
                         IvwKey::Q,
                         KeyState::Press,
                         KeyModifiers{flags::none}}
    , moveShortcut_{"moveGizmo",
                    "Move Gizmo",
                    "Shortcut to show the move gizmo"_help,
                    [this](Event* e) {
                        if (gizmo_.getType() != GizmoType::Move) {
                            gizmoType_.setSelectedValue(GizmoType::Move);
                        } else {
                            gizmoType_.setSelectedValue(GizmoType::None);
                        }
                        e->markAsUsed();
                    },
                    IvwKey::M,
                    KeyState::Press,
                    KeyModifiers{flags::none}}
    , rotateShortcut_{"rotateGizmo",
                      "Rotation Gizmo",
                      "Shortcut to show the rotation gizmo"_help,
                      [this](Event* e) {
                          if (gizmo_.getType() != GizmoType::Rotate) {
                              gizmoType_.setSelectedValue(GizmoType::Rotate);
                          } else {
                              gizmoType_.setSelectedValue(GizmoType::None);
                          }
                          e->markAsUsed();
                      },
                      IvwKey::R,
                      KeyState::Press,
                      KeyModifiers{flags::none}}
    , moveRotateShortcut_{"moveRotateGizmo",
                          "Move-Rotation Gizmo",
                          "Shortcut to show the move-rotation gizmo"_help,
                          [this](Event* e) {
                              if (gizmo_.getType() != GizmoType::MoveRotate) {
                                  gizmoType_.setSelectedValue(GizmoType::MoveRotate);
                              } else {
                                  gizmoType_.setSelectedValue(GizmoType::None);
                              }
                              e->markAsUsed();
                          },
                          IvwKey::M,
                          KeyState::Press,
                          KeyModifier::Shift}
    , gizmo_{this, [this]() -> const Camera& { return camera_.get(); }} {

    addPorts(inport_, outport_, gizmoOutport_);

    outputProps_.addProperties(worldBasis_);
    outputProps_.setCollapsed(true);
    interaction_.addProperties(cancelInteractionEvent_, hideGizmoShortcut_, moveShortcut_,
                               rotateShortcut_, moveRotateShortcut_);
    interaction_.setCollapsed(true);
    addProperties(enable_, gizmoType_, space_, camera_, gizmoBasis_, outputProps_, interaction_);

    space_.onChange([&]() {
        // transform gizmo offset when space changes
        const auto prevSpace = gizmo_.getSpace();
        const auto newSpace = space_.get();
        if (prevSpace != newSpace && inport_.hasData()) {
            const auto& ct = inport_.getData()->getCoordinateTransformer();
            const dmat4 m = ct.getMatrix(prevSpace, newSpace);
            gizmoBasis_.set(m * gizmoBasis_.get() * glm::inverse(m));
        }
    });

    gizmo_.onInvalidate([this]() {
        if (inport_.hasData()) {
            gizmoBasis_.set(gizmo_.getBasis(space_, *inport_.getData()));
        } else {
            gizmoBasis_.set(gizmo_.getBasis());
        }
        worldBasis_.set(gizmo_.getBasis());
        invalidate(InvalidationLevel::InvalidOutput);
    });
}

template <typename T>
void TransformGizmo<T>::process() {
    if (gizmoBasis_.isModified() || space_.isModified() || inport_.isChanged()) {
        gizmo_.setBasis(gizmoBasis_.get(), *inport_.getData(), space_.get());
    }
    gizmo_.setType(gizmoType_);

    if (!enable_) {
        outport_.setData(inport_.getData());
        return;
    }

    auto entity = std::shared_ptr<T>(inport_.getData()->clone());
    const auto basis{gizmo_.getBasis(space_, *inport_.getData())};
    switch (space_.get()) {
        using enum CoordinateSpace;
        case Data:  entity->setModelMatrix(entity->getModelMatrix() * basis); break;
        case Model: entity->setModelMatrix(basis * entity->getModelMatrix()); break;
        case World: [[fallthrough]];
        default:    entity->setWorldMatrix(basis * entity->getWorldMatrix()); break;
    }

    outport_.setData(entity);
    gizmoOutport_.setData(std::make_shared<GizmoData>(gizmo_.getData()));
}

template <typename T>
void TransformGizmo<T>::invokeEvent(Event* e) {
    if ((*cancelInteractionEvent_.getEventMatcher())(e)) {
        gizmo_.cancelInteraction();
        e->markAsUsed();
    }

    if (!e->hasBeenUsed()) {
        Processor::invokeEvent(e);
    }
}

}  // namespace inviwo
