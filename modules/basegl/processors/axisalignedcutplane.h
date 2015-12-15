/*********************************************************************************
*
* Inviwo - Interactive Visualization Workshop
*
* Copyright (c) 2015 Inviwo Foundation
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

#ifndef IVW_VOLUMECUTPLANE_H
#define IVW_VOLUMECUTPLANE_H

#include <modules/basegl/baseglmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/ports/dataoutport.h>

#include <memory>
#include <modules/opengl/shader/shader.h>
#include <modules/opengl/shader/shaderutils.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/rendering/meshdrawerfactory.h>
#include <inviwo/core/rendering/meshdrawer.h> 
#include <inviwo/core/interaction/cameratrackball.h>
#include <inviwo/core/datastructures/geometry/simplemesh.h>

namespace inviwo {
    /** \docpage{<classIdentifier>, AxisAlignedCutPlane}
    * Explanation of how to use the processor.
    *
    * ### Inports
    *   * __<Inport1>__ <description>.
    *
    * ### Outports
    *   * __<Outport1>__ <description>.
    *
    * ### Properties
    *   * __<Prop1>__ <description>.
    *   * __<Prop2>__ <description>
    */

    /**
    * \class AxisAlignedCutPlane
    *
    * \brief <brief description>
    *
    * <Detailed description from a developer prespective>
    */
    class IVW_MODULE_BASEGL_API AxisAlignedCutPlane : public Processor {
        enum class Axis { X = 0, Y = 1, Z = 2 };
        template <Axis axis>
        class SliceProperty : public BoolCompositeProperty {
        public:
            SliceProperty(const std::string &identifier, const std::string &displayName);

            IntProperty slice_;

            std::unique_ptr<SimpleMesh> mesh_;
            std::unique_ptr<MeshDrawer> drawer_;

            void onVolumeChange(std::shared_ptr<const Volume> vol);

            void createDrawer(std::shared_ptr<const Volume> vol);

            void draw(Shader &shader);

        private:
            static vec3 forSlice(int axes, double a, double b, double t);
        };

    public:
        virtual const ProcessorInfo getProcessorInfo() const override;
        static const ProcessorInfo processorInfo_;
        AxisAlignedCutPlane();
        virtual ~AxisAlignedCutPlane() {}

    protected:
        virtual void process() override;
        virtual void initializeResources() override {
            if (disableTF_.get()) {
                sliceShader_.getFragmentShaderObject()->removeShaderDefine("USE_TF");
            }
            else {
                sliceShader_.getFragmentShaderObject()->addShaderDefine("USE_TF");
            }
            sliceShader_.build();
        }

        VolumeInport volume_;
        ImageInport imageInport_;
        ImageOutport outport_;

        SliceProperty<Axis::X> xSlide_;
        SliceProperty<Axis::Y> ySlide_;
        SliceProperty<Axis::Z> zSlide_;

        BoolProperty disableTF_;
        TransferFunctionProperty tf_;

        BoolProperty showBoundingBox_;
        FloatVec4Property boundingBoxColor_;
        FloatProperty renderPointSize_;
        FloatProperty renderLineWidth_;

        BoolProperty  nearestInterpolation_;

        CameraProperty camera_;
        CameraTrackball trackball_;


        Shader sliceShader_;

        Shader boundingBoxShader_;
        std::unique_ptr<SimpleMesh> boundingBoxMesh_;
        std::unique_ptr<MeshDrawer> boundingBoxDrawer_;

        void createBoundingBox();
        void drawBoundingBox();

    private:
    };

    template <AxisAlignedCutPlane::Axis axis>
    AxisAlignedCutPlane::SliceProperty<axis>::SliceProperty(const std::string &identifier,
        const std::string &displayName)
        : BoolCompositeProperty(identifier, displayName, true)
        , slice_("slice", "Slice", 50, 1, 100)
        , mesh_(nullptr)
        , drawer_(nullptr) {
        addProperty(slice_);
    }

    template <AxisAlignedCutPlane::Axis axis>
    void AxisAlignedCutPlane::SliceProperty<axis>::onVolumeChange(std::shared_ptr<const Volume> vol) {
        double t = static_cast<double>(slice_.get()) / static_cast<double>(slice_.getMaxValue());
        auto max = static_cast<int>(vol->getDimensions()[static_cast<int>(axis)]);
        slice_.setMaxValue(max);
        slice_.set(static_cast<int>(t * max));
        createDrawer(vol);
    }

    template <AxisAlignedCutPlane::Axis axis>
    void AxisAlignedCutPlane::SliceProperty<axis>::createDrawer(std::shared_ptr<const Volume> vol) {
        mesh_ = util::make_unique<SimpleMesh>(DrawType::Triangles, ConnectivityType::Strip);

        double z = (static_cast<double>(slice_.get())-0.5) / static_cast<double>(slice_.getMaxValue());

        auto v0 = forSlice(static_cast<int>(axis), 0, 0, z);
        auto v1 = forSlice(static_cast<int>(axis), 0, 1, z);
        auto v2 = forSlice(static_cast<int>(axis), 1, 0, z);
        auto v3 = forSlice(static_cast<int>(axis), 1, 1, z);

        mesh_->addVertex(v0, v0, vec4(v0, 1.0));
        mesh_->addVertex(v1, v1, vec4(v1, 1.0));
        mesh_->addVertex(v2, v2, vec4(v2, 1.0));
        mesh_->addVertex(v3, v3, vec4(v3, 1.0));

        mesh_->addIndices(0, 1, 2, 3);
        mesh_->setModelMatrix(vol->getModelMatrix());
        mesh_->setWorldMatrix(vol->getWorldMatrix());

        drawer_ = InviwoApplication::getPtr()->getMeshDrawerFactory()->create(mesh_.get());
    }

    template <AxisAlignedCutPlane::Axis axis>
    void AxisAlignedCutPlane::SliceProperty<axis>::draw(Shader &shader) {
        if (!isChecked()) return;
        LGL_ERROR;
        utilgl::setShaderUniforms(shader, *mesh_, "geometry_");
        LGL_ERROR;
        drawer_->draw();
        LGL_ERROR;
    }

    template <AxisAlignedCutPlane::Axis axis>
    vec3 inviwo::AxisAlignedCutPlane::SliceProperty<axis>::forSlice(int axes, double a, double b,
        double t) {
        switch (axes) {
        case 0:
            return vec3(t, a, b);
        case 1:
            return vec3(a, t, b);
        case 2:
            return vec3(a, b, t);
        }
        return vec3(0);
    }

}  // namespace

#endif  // IVW_VOLUMECUTPLANE_H
