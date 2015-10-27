/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#ifndef IVW_VOLUMEBASISTRANSFORMER_H
#define IVW_VOLUMEBASISTRANSFORMER_H

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <modules/base/basemoduledefine.h>
#include <modules/base/properties/basisproperty.h>

namespace inviwo {

/** \docpage{org.inviwo.BasisTransformVolume, Basis Transform Volume}
 * ![](org.inviwo.BasisTransformVolume.png?classIdentifier=org.inviwo.BasisTransformVolume)
 *
 * Transforms the world of a volume.
 *
 * ### Inports
 *   * __VolumeInport__ Input volume.
 *
 * ### Outports
 *   * __VolumeOutport__ Transformed output volume.
 *
 * ### Properties
 *   * __Lengths__ Length of each basis vector
 *   * __Angles__ Angles between vectors
 *   * __Offset__ Offset of basis
 */

/** \docpage{org.inviwo.BasisTransformGeometry, Basis Transform Mesh}
 * ![](org.inviwo.BasisTransformGeometry.png?classIdentifier=org.inviwo.BasisTransformGeometry)
 *
 * Transforms the basis of a mesh.
 *
 * ### Inports
 *   * __MeshInport__ Input mesh.
 *
 * ### Outports
 *   * __MeshOutport__ Transformed output mesh.
 *
 * ### Properties
 *   * __Lengths__ Length of each basis vector
 *   * __Angles__ Angles between vectors
 *   * __Offset__ Offset of basis
 */

template <typename T>
class IVW_MODULE_BASE_API BasisTransform : public Processor {
public:
    BasisTransform();
    ~BasisTransform();

    virtual const ProcessorInfo getProcessorInfo() const override;

protected:
    virtual void process() override;

private:
    std::shared_ptr<T> data_;
    DataInport<T> inport_;
    DataOutport<T> outport_;

    BasisProperty basis_;
};

template <typename T>
const ProcessorInfo inviwo::BasisTransform<T>::getProcessorInfo() const {
    return ProcessorTraits<BasisTransform<T>>::getProcessorInfo();
}

class Mesh;
template <>
struct ProcessorTraits<BasisTransform<Mesh>> {
    static ProcessorInfo getProcessorInfo() {
        return {
            "org.inviwo.BasisTransformGeometry",  // Class identifier
            "Basis Transform Mesh",               // Display name
            "Coordinate Transforms",              // Category
            CodeState::Experimental,              // Code state
            Tags::CPU                             // Tags
        };
    }
};

class Volume;
template <>
struct ProcessorTraits<BasisTransform<Volume>> {
    static ProcessorInfo getProcessorInfo() {
        return {
            "org.inviwo.BasisTransformVolume",  // Class identifier
            "Basis Transform Volume",           // Display name
            "Coordinate Transforms",            // Category
            CodeState::Experimental,            // Code state
            Tags::CPU                           // Tags
        };
    }
};

template <typename T>
inviwo::BasisTransform<T>::BasisTransform()
    : Processor(), inport_("inport"), outport_("outport"), basis_("basis", "Basis") {
    addPort(inport_);
    addPort(outport_);
    addProperty(basis_);
}

template <typename T>
inviwo::BasisTransform<T>::~BasisTransform() {}

template <typename T>
void inviwo::BasisTransform<T>::process() {
    if (inport_.hasData()) {
        if (inport_.isChanged()) {
            data_.reset(inport_.getData()->clone());
            basis_.updateForNewEntity(*data_);
            outport_.setData(data_);
        }
        basis_.updateEntity(*data_);
    }
}

}  // namespace

#endif