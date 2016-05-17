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

#ifndef IVW_MESHCREATOR_H
#define IVW_MESHCREATOR_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/datastructures/geometry/simplemesh.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <modules/base/properties/basisproperty.h>
#include <inviwo/core/util/stdextensions.h>

namespace inviwo {

/** \docpage{org.inviwo.MeshCreator, Mesh Creator}
 * ![](org.inviwo.MeshCreator.png?classIdentifier=org.inviwo.MeshCreator)
 *
 * ...
 * 
 * 
 * ### Outports
 *   * __outport__ ...
 * 
 * ### Properties
 *   * __Color__ ...
 *   * __Stop Position__ ...
 *   * __Mesh Type__ ...
 *   * __Normal__ ...
 *   * __Size scaling__ ...
 *   * __Mesh resolution__ ...
 *   * __Start Position__ ...
 *
 */
class IVW_MODULE_BASE_API MeshCreator : public Processor {
public:
    MeshCreator();
    ~MeshCreator();

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

    std::shared_ptr<Mesh> createMesh();

protected:
    virtual void process() override;

private:
    template <typename... Args>
    void show(Args&&... args) {
        util::for_each_argument([](Property& p) { p.setVisible(true); }, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void hide(Args&&... args) {
        util::for_each_argument([](Property& p) { p.setVisible(false); }, std::forward<Args>(args)...);
    }

    enum class MeshType {
        Sphere,
        ColorSphere,
        CubeBasicMesh,
        CubeSimpleMesh,
        LineCube,
        LineCubeAdjacency,
        Plane,
        Disk,
        Cone,
        Cylinder,
        Arrow,
        CoordAxes, 
        Torus
    };

    MeshOutport outport_;
    
    FloatVec3Property position1_;
    FloatVec3Property position2_;
    FloatVec3Property normal_;
    FloatVec4Property color_;
    FloatProperty torusRadius1_;
    FloatProperty torusRadius2_;
    
    BasisProperty basis_;

    FloatProperty meshScale_; // Scale size of mesh
    IntVec2Property meshRes_; // mesh resolution
    TemplateOptionProperty<MeshType> meshType_;
};

} // namespace

#endif // IVW_MESHCREATOR_H
