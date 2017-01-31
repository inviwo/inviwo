/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2017 Inviwo Foundation
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

#ifndef IVW_WORLDTRANSFORM_H
#define IVW_WORLDTRANSFORM_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>


namespace inviwo {

/** \docpage{org.inviwo.WorldTransformVolume, Basis Transform Mesh}
 * ![](org.inviwo.WorldTransformVolume.png?classIdentifier=org.inviwo.WorldTransformVolume)
 *
 * Sets the world transform of a volume.
 *
 * ### Inports
 *   * __VolumeInport__ Input volume.
 *
 * ### Outports
 *   * __VolumeOutport__ Transformed output volume.
 *
 * ### Properties
 *   * __Type__ There are 3 modes: Translate, Rotate, Scale, User Defined
 *   * __Translate__ A translation
 *   * __Axis__ Axis of rotation
 *   * __Angle__ Angle of rotation
 *   * __Scale__ Scaling for each axis
 *   * __Transformation__ Arbitrary transformation
 */

/** \docpage{org.inviwo.WorldTransformGeometry, Basis Transform Volume}
 * ![](org.inviwo.WorldTransformGeometry.png?classIdentifier=org.inviwo.WorldTransformGeometry)
 *
 * Sets the world transform of a mesh.
 *
 * ### Inports
 *   * __MeshInport__ Input mesh.
 *
 * ### Outports
 *   * __MeshOutport__ Transformed output mesh.
 *
 * ### Properties
 *   * __Type__ There are 3 modes: Translate, Rotate, Scale, User Defined
 *   * __Translate__ A translation
 *   * __Axis__ Axis of rotation
 *   * __Angle__ Angle of rotation
 *   * __Scale__ Scaling for each axis
 *   * __Transformation__ Arbitrary transformation
 */

template <typename T>
class IVW_MODULE_BASE_API WorldTransform : public Processor {
public:
    WorldTransform();

    virtual ~WorldTransform() = default;

    virtual const ProcessorInfo getProcessorInfo() const override;

protected:
    void onMatrixChange();
    void updateValues();
    void changeVisibility();

    virtual void process() override;

    DataInport<T> inport_;
    DataOutport<T> outport_;

    OptionPropertyInt type_;

    FloatVec3Property translate_;
    FloatVec3Property scale_;

    FloatVec3Property rotationAxis_;
    FloatProperty rotationAngle_;

    FloatMat4Property matrix_;

private:
    bool updatingValues_;
};

template <typename T>
const ProcessorInfo inviwo::WorldTransform<T>::getProcessorInfo() const {
    return ProcessorTraits<WorldTransform<T>>::getProcessorInfo();
}

class Mesh;
template <>
struct ProcessorTraits<WorldTransform<Mesh>> {
    static ProcessorInfo getProcessorInfo() {
        return {
            "org.inviwo.WorldTransformGeometry",  // Class identifier
            "World Transform Mesh",               // Display name
            "Coordinate Transforms",              // Category
            CodeState::Stable,                    // Code state
            Tags::None                            // Tags
        };
    }
};

class Volume;
template <>
struct ProcessorTraits<WorldTransform<Volume>> {
    static ProcessorInfo getProcessorInfo() {
        return {
            "org.inviwo.WorldTransformVolume",  // Class identifier
            "World Transform Volume",           // Display name
            "Coordinate Transforms",            // Category
            CodeState::Stable,                  // Code state
            Tags::None                          // Tags
        };
    }
};

template <typename T>
WorldTransform<T>::WorldTransform()
    : Processor()
    , inport_("inport_")
    , outport_("outport_")
    , type_("type_", "Type")
    , translate_("translate_", "Translate", vec3(0), vec3(-10), vec3(10))
    , scale_("scale", "Scale", vec3(1), vec3(0.001f), vec3(10))
    , rotationAxis_("rotationAxis", "Axis", vec3(1, 0, 0), vec3(-1), vec3(1))
    , rotationAngle_("rotationAngle", "Angle", 0, 0, 2.0f * static_cast<float>(M_PI))
    , matrix_("matrix_", "Transformation", mat4(1))
    , updatingValues_(false) {

    addPort(inport_);
    addPort(outport_);

    type_.addOption("translate", "Translate", 0);
    type_.addOption("rotate", "Rotate", 1);
    type_.addOption("scale", "Scale", 2);
    type_.addOption("user", "User Defined", 10);
    type_.setCurrentStateAsDefault();

    addProperty(type_);

    addProperty(translate_);
    addProperty(scale_);
    addProperty(rotationAxis_);
    addProperty(rotationAngle_);

    addProperty(matrix_);

    type_.onChange(this, &WorldTransform::changeVisibility);
    translate_.onChange(this, &WorldTransform::updateValues);
    scale_.onChange(this, &WorldTransform::updateValues);
    rotationAxis_.onChange(this, &WorldTransform::updateValues);
    rotationAngle_.onChange(this, &WorldTransform::updateValues);
    matrix_.onChange(this, &WorldTransform::onMatrixChange);
    changeVisibility();
}


template <typename T>
void WorldTransform<T>::process() {
    std::shared_ptr<T> data(inport_.getData()->clone());
    data->setWorldMatrix(matrix_.get() * data->getWorldMatrix());
    outport_.setData(data);
}

template <typename T>
void WorldTransform<T>::changeVisibility() {
    bool translate = type_.get() == 0;
    bool rotate = type_.get() == 1;
    bool scale = type_.get() == 2;
    // bool user = type_.get() == 10; //Not used

    translate_.setVisible(translate);
    scale_.setVisible(scale);
    rotationAxis_.setVisible(rotate);
    rotationAngle_.setVisible(rotate);
}

template <typename T>
void WorldTransform<T>::updateValues() {
    updatingValues_ = true;
    switch (type_.get()) {
        case 0:
            matrix_.set(glm::translate(translate_.get()));
            break;  // translate
        case 1:
            matrix_.set(glm::rotate(rotationAngle_.get(), rotationAxis_.get()));
            break;  // rotate
        case 2:
            matrix_.set(glm::scale(scale_.get()));
            break;  // scale
        default:
            break;
    }
    updatingValues_ = false;
}

template <typename T>
void WorldTransform<T>::onMatrixChange() {
    if (updatingValues_) return;
    type_.set(10);
}

}  // namespace

#endif  // IVW_WORLDTRANSFORM_H
