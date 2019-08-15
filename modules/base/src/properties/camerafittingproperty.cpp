/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <modules/base/properties/camerafittingproperty.h>
#include <modules/base/events/viewevent.h>

namespace inviwo {

const std::string CameraFittingProperty::classIdentifier = "org.inviwo.CameraFittingProperty";

std::string CameraFittingProperty::getClassIdentifier() const { return classIdentifier; }

CameraFittingProperty::CameraFittingProperty(std::string identifier, std::string displayName,
                                             CameraProperty* camera,
                                             std::function<std::optional<mat4>()> getBoundingBox)
    : CompositeProperty(identifier, displayName, InvalidationLevel::Valid)
    , camera_{camera}
    , getBoundingBox_{std::move(getBoundingBox)}
    , setView_{[this](auto side) {
        if (camera_ && getBoundingBox_) {
            if (auto bb = getBoundingBox_(); bb) {
                camerautil::setCameraView(*camera_, *bb, side, fittingRatio_.get(),
                                          updateNearFar_.get(), updateLookRanges_.get());
            }
        }
    }}
    , settings_("settings", "Settings")
    , flipUp_("flipUp", "Flip Up vector",
              [this]() {
                  if (camera_) {
                      camera_->setLookUp(-camera_->getLookUp());
                  }
              })
    , updateNearFar_("updateNearFar", "Update Near/Far Distances", true)
    , updateLookRanges_("updateLookRanges", "Update Look-to/-from Ranges", true)
    , fittingRatio_("fittingRatio", "Fitting Ratio", 1.05f, 0, 2, 0.01f)
    , lookAt_("lookAt", "Look At",
              {{std::nullopt, ":svgicons/view-x-m.svg",
                [this] { setView_(camerautil::Side::XNegative); }},
               {std::nullopt, ":svgicons/view-x-p.svg",
                [this] { setView_(camerautil::Side::XPositive); }},
               {std::nullopt, ":svgicons/view-y-m.svg",
                [this] { setView_(camerautil::Side::YNegative); }},
               {std::nullopt, ":svgicons/view-y-p.svg",
                [this] { setView_(camerautil::Side::YPositive); }},
               {std::nullopt, ":svgicons/view-z-m.svg",
                [this] { setView_(camerautil::Side::ZNegative); }},
               {std::nullopt, ":svgicons/view-z-p.svg",
                [this] { setView_(camerautil::Side::ZPositive); }}})

    , setNearFarButton_("setNearFarButton", "Set Near/Far Distances",
                        [this] {
                            if (camera_ && getBoundingBox_) {
                                if (auto bb = getBoundingBox_(); bb) {
                                    camerautil::setCameraNearFar(*camera_, *bb);
                                }
                            }
                        })
    , setLookRangesButton_("setLookRangesButton", "Set Look-to/-from Ranges", [this] {
        if (camera_ && getBoundingBox_) {
            if (auto bb = getBoundingBox_(); bb) {
                camerautil::setCameraLookRanges(*camera_, *bb);
            }
        }
    }) {

    addProperties(lookAt_, flipUp_, setNearFarButton_, setLookRangesButton_, settings_);
    settings_.addProperties(updateNearFar_, updateLookRanges_, fittingRatio_);
    settings_.setCollapsed(true);
}

CameraFittingProperty* CameraFittingProperty::clone() const {
    return new CameraFittingProperty(*this);
}

void CameraFittingProperty::invokeEvent(Event* event) {
    if (auto ve = event->getAs<ViewEvent>()) {
        setView_(ve->getSide());
        ve->markAsUsed();
    } else {
        CompositeProperty::invokeEvent(event);
    }
}

CameraFittingProperty::CameraFittingProperty(const CameraFittingProperty& rhs)
    : CompositeProperty(rhs)
    , camera_{rhs.camera_}
    , getBoundingBox_{rhs.getBoundingBox_}
    , setView_{[this](auto side) {
        if (camera_ && getBoundingBox_) {
            if (auto bb = getBoundingBox_(); bb) {
                camerautil::setCameraView(*camera_, *bb, side, fittingRatio_.get(),
                                          updateNearFar_.get(), updateLookRanges_.get());
            }
        }
    }}
    , settings_{rhs.settings_}
    , flipUp_{rhs.flipUp_,
              [this]() {
                  if (camera_) {
                      camera_->setLookUp(-camera_->getLookUp());
                  }
              }}
    , updateNearFar_{rhs.updateNearFar_}
    , updateLookRanges_{rhs.updateLookRanges_}
    , fittingRatio_{rhs.fittingRatio_}
    , lookAt_{rhs.lookAt_,
              {{std::nullopt, ":svgicons/view-x-m.svg",
                [this] { setView_(camerautil::Side::XNegative); }},
               {std::nullopt, ":svgicons/view-x-p.svg",
                [this] { setView_(camerautil::Side::XPositive); }},
               {std::nullopt, ":svgicons/view-y-m.svg",
                [this] { setView_(camerautil::Side::YNegative); }},
               {std::nullopt, ":svgicons/view-y-p.svg",
                [this] { setView_(camerautil::Side::YPositive); }},
               {std::nullopt, ":svgicons/view-z-m.svg",
                [this] { setView_(camerautil::Side::ZNegative); }},
               {std::nullopt, ":svgicons/view-z-p.svg",
                [this] { setView_(camerautil::Side::ZPositive); }}}}
    , setNearFarButton_{rhs.setNearFarButton_,
                        [this] {
                            if (camera_ && getBoundingBox_) {
                                if (auto bb = getBoundingBox_(); bb) {
                                    camerautil::setCameraNearFar(*camera_, *bb);
                                }
                            }
                        }}
    , setLookRangesButton_{rhs.setLookRangesButton_, [this] {
                               if (camera_ && getBoundingBox_) {
                                   if (auto bb = getBoundingBox_(); bb) {
                                       camerautil::setCameraLookRanges(*camera_, *bb);
                                   }
                               }
                           }} {

    addProperties(lookAt_, flipUp_, setNearFarButton_, setLookRangesButton_, settings_);
    settings_.addProperties(updateNearFar_, updateLookRanges_, fittingRatio_);
}

CameraFittingProperty::CameraFittingProperty(std::string identifier, std::string displayName,
                                             CameraProperty& camera, VolumeInport& volumePort)
    : CameraFittingProperty(
          identifier, displayName, &camera, [vp = &volumePort]() -> std::optional<mat4> {
              if (vp->hasData()) {
                  return vp->getData()->getCoordinateTransformer().getDataToWorldMatrix();
              } else {
                  return std::nullopt;
              }
          }) {}

}  // namespace inviwo
