/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2017 Inviwo Foundation
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

#include <inviwo/core/common/inviwocore.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/util/filesystem.h>

//Cameras
#include <inviwo/core/datastructures/camera.h>

//Data Structures
#include <inviwo/core/datastructures/volume/volumeramconverter.h>
#include <inviwo/core/datastructures/image/layerramconverter.h>
#include <inviwo/core/datastructures/representationconverterfactory.h>

//Meta Data
#include <inviwo/core/metadata/metadata.h>
#include <inviwo/core/metadata/containermetadata.h>
#include <inviwo/core/metadata/processormetadata.h>
#include <inviwo/core/metadata/processorwidgetmetadata.h>
#include <inviwo/core/metadata/propertyeditorwidgetmetadata.h>

//Utilizes
#include <inviwo/core/util/systemcapabilities.h>
#include <inviwo/core/util/vectoroperations.h>
#include <inviwo/core/util/settings/systemsettings.h>
#include <inviwo/core/util/settings/linksettings.h>

//Io
#include <inviwo/core/io/rawvolumereader.h>

//Others
#include <inviwo/core/processors/canvasprocessor.h>

//Ports
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/volumeport.h>

//PortInspectors
#include <inviwo/core/ports/portinspector.h>

//Properties
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/directoryproperty.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/filepatternproperty.h>
#include <inviwo/core/properties/imageeditorproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/multifileproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/planeproperty.h>
#include <inviwo/core/properties/positionproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/properties/advancedmaterialproperty.h>
#include <inviwo/core/properties/simplelightingproperty.h>
#include <inviwo/core/properties/simpleraycastingproperty.h>
#include <inviwo/core/properties/stipplingproperty.h>
#include <inviwo/core/properties/volumeindicatorproperty.h>

#include <inviwo/core/properties/propertyconvertermanager.h>
#include <inviwo/core/properties/propertyconverter.h>

#include <inviwo/core/util/stdextensions.h>

namespace inviwo {

namespace {

struct ConverterRegFunctor {
    template <typename T, typename U>
    auto operator()(std::function<void(std::unique_ptr<PropertyConverter>)> reg) {
        if (!std::is_same<T, U>::value) {
            reg(util::make_unique<
                OrdinalPropertyConverter<OrdinalProperty<T>, OrdinalProperty<U>>>());
        }
    }
};
struct ScalarStringConverterRegFunctor {
    template <typename T>
    auto operator()(std::function<void(std::unique_ptr<PropertyConverter>)> reg) {
            reg(util::make_unique<ScalarToStringConverter<OrdinalProperty<T>>>());
    }
};
struct VectorStringConverterRegFunctor {
    template <typename T>
    auto operator()(std::function<void(std::unique_ptr<PropertyConverter>)> reg) {
        reg(util::make_unique<VectorToStringConverter<OrdinalProperty<T>>>());
    }
};

}  // namespace

InviwoCore::InviwoCore(InviwoApplication* app) : InviwoModule(app, "Core") {
    // Register Converter Factories
    registerRepresentationConverterFactory(
        util::make_unique<RepresentationConverterFactory<VolumeRepresentation>>());
    registerRepresentationConverterFactory(
        util::make_unique<RepresentationConverterFactory<LayerRepresentation>>());
    registerRepresentationConverterFactory(
        util::make_unique<RepresentationConverterFactory<BufferRepresentation>>());

    // Register Converters
    registerRepresentationConverter<VolumeRepresentation>(
        util::make_unique<VolumeDisk2RAMConverter>());
    registerRepresentationConverter<LayerRepresentation>(
        util::make_unique<LayerDisk2RAMConverter>());

    // Register MetaData
    registerMetaData(util::make_unique<BoolMetaData>());
    registerMetaData(util::make_unique<IntMetaData>());
    registerMetaData(util::make_unique<FloatMetaData>());
    registerMetaData(util::make_unique<DoubleMetaData>());
    registerMetaData(util::make_unique<StringMetaData>());
    registerMetaData(util::make_unique<FloatVec2MetaData>());
    registerMetaData(util::make_unique<FloatVec3MetaData>());
    registerMetaData(util::make_unique<FloatVec4MetaData>());
    registerMetaData(util::make_unique<DoubleVec2MetaData>());
    registerMetaData(util::make_unique<DoubleVec3MetaData>());
    registerMetaData(util::make_unique<DoubleVec4MetaData>());
    registerMetaData(util::make_unique<IntVec2MetaData>());
    registerMetaData(util::make_unique<IntVec3MetaData>());
    registerMetaData(util::make_unique<IntVec4MetaData>());
    registerMetaData(util::make_unique<UIntVec2MetaData>());
    registerMetaData(util::make_unique<UIntVec3MetaData>());
    registerMetaData(util::make_unique<UIntVec4MetaData>());
    registerMetaData(util::make_unique<FloatMat2MetaData>());
    registerMetaData(util::make_unique<FloatMat3MetaData>());
    registerMetaData(util::make_unique<FloatMat4MetaData>());
    registerMetaData(util::make_unique<DoubleMat2MetaData>());
    registerMetaData(util::make_unique<DoubleMat4MetaData>());
    registerMetaData(util::make_unique<DoubleMat3MetaData>());
    registerMetaData(util::make_unique<VectorMetaData<2,float>>());
    registerMetaData(util::make_unique<VectorMetaData<3,float>>());
    registerMetaData(util::make_unique<VectorMetaData<4,float>>());
    registerMetaData(util::make_unique<VectorMetaData<2,double>>());
    registerMetaData(util::make_unique<VectorMetaData<3,double>>());
    registerMetaData(util::make_unique<VectorMetaData<4,double>>());
    registerMetaData(util::make_unique<VectorMetaData<2,int>>());
    registerMetaData(util::make_unique<VectorMetaData<3,int>>());
    registerMetaData(util::make_unique<VectorMetaData<4,int>>());
    registerMetaData(util::make_unique<VectorMetaData<2,unsigned int>>());
    registerMetaData(util::make_unique<VectorMetaData<3,unsigned int>>());
    registerMetaData(util::make_unique<VectorMetaData<4,unsigned int>>());
    registerMetaData(util::make_unique<MatrixMetaData<2,float>>());
    registerMetaData(util::make_unique<MatrixMetaData<3,float>>());
    registerMetaData(util::make_unique<MatrixMetaData<4,float>>());
    registerMetaData(util::make_unique<MatrixMetaData<2,double>>());
    registerMetaData(util::make_unique<MatrixMetaData<3,double>>());
    registerMetaData(util::make_unique<MatrixMetaData<4,double>>());
    registerMetaData(util::make_unique<PositionMetaData>());
    registerMetaData(util::make_unique<ProcessorMetaData>());
    registerMetaData(util::make_unique<ProcessorWidgetMetaData>());
    registerMetaData(util::make_unique<PropertyEditorWidgetMetaData>());
    registerMetaData(util::make_unique<StdUnorderedMapMetaData<std::string, std::string>>());
    registerMetaData(util::make_unique<StdVectorMetaData<std::string>>());

    // Register Cameras
    registerCamera<PerspectiveCamera>("PerspectiveCamera");
    registerCamera<OrthographicCamera>("OrthographicCamera");
    registerCamera<SkewedPerspectiveCamera>("SkewedPerspectiveCamera");
    
    // Register Capabilities
    auto syscap = util::make_unique<SystemCapabilities>();
    registerCapabilities(std::move(syscap));
    
    // Register Data readers
    registerDataReader(util::make_unique<RawVolumeReader>());
    // Register Data writers

    // Register Ports
    registerPort<MeshInport>("org.inviwo.MeshInport");
    registerPort<MeshMultiInport>("org.inviwo.MeshMultiInport");
    registerPort<MeshOutport>("org.inviwo.MeshOutport");
    registerPort<ImageInport>("org.inviwo.ImageInport");
    registerPort<ImageMultiInport>("org.inviwo.MultiImageInport");
    registerPort<ImageOutport>("org.inviwo.ImageOutport");
    registerPort<VolumeInport>("org.inviwo.VolumeInport");
    registerPort<VolumeOutport>("org.inviwo.VolumeOutport");

    registerPort<DataInport<vec2>>("vec2Inport");
    registerPort<DataInport<vec2, 0>>("vec2MutliInport");
    registerPort<DataInport<vec2, 0, true>>("vec2FlatMultiInport");
    registerPort<DataOutport<std::vector<vec2>>>("vec2VectorOutport");
    registerPort<DataOutport<vec2>>("vec2Outport");
    registerPort<DataInport<dvec2>>("dvec2Inport");
    registerPort<DataInport<dvec2, 0>>("dvec2MutliInport");
    registerPort<DataInport<dvec2, 0, true>>("dvec2FlatMultiInport");
    registerPort<DataOutport<std::vector<dvec2>>>("dvec2VectorOutport");
    registerPort<DataOutport<dvec2>>("dvec2Outport");
    registerPort<DataInport<ivec2>>("ivec2Inport");
    registerPort<DataInport<ivec2, 0>>("ivec2MutliInport");
    registerPort<DataInport<ivec2, 0, true>>("ivec2FlatMultiInport");
    registerPort<DataOutport<std::vector<ivec2>>>("ivec2VectorOutport");
    registerPort<DataOutport<ivec2>>("ivec2Outport");
    registerPort<DataInport<vec3>>("vec3Inport");
    registerPort<DataInport<vec3, 0>>("vec3MutliInport");
    registerPort<DataInport<vec3, 0, true>>("vec3FlatMultiInport");
    registerPort<DataOutport<std::vector<vec3>>>("vec3VectorOutport");
    registerPort<DataOutport<vec3>>("vec3Outport");
    registerPort<DataInport<dvec3>>("dvec3Inport");
    registerPort<DataInport<dvec3, 0>>("dvec3MutliInport");
    registerPort<DataInport<dvec3, 0, true>>("dvec3FlatMultiInport");
    registerPort<DataOutport<std::vector<dvec3>>>("dvec3VectorOutport");
    registerPort<DataOutport<dvec3>>("dvec3Outport");
    registerPort<DataInport<ivec3>>("ivec3Inport");
    registerPort<DataInport<ivec3, 0>>("ivec3MutliInport");
    registerPort<DataInport<ivec3, 0, true>>("ivec3FlatMultiInport");
    registerPort<DataOutport<std::vector<ivec3>>>("ivec3VectorOutport");
    registerPort<DataOutport<ivec3>>("ivec3Outport");
    registerPort<DataInport<vec4>>("vec4Inport");
    registerPort<DataInport<vec4, 0>>("vec4MutliInport");
    registerPort<DataInport<vec4, 0, true>>("vec4FlatMultiInport");
    registerPort<DataOutport<std::vector<vec4>>>("vec4VectorOutport");
    registerPort<DataOutport<vec4>>("vec4Outport");
    registerPort<DataInport<dvec4>>("dvec4Inport");
    registerPort<DataInport<dvec4, 0>>("dvec4MutliInport");
    registerPort<DataInport<dvec4, 0, true>>("dvec4FlatMultiInport");
    registerPort<DataOutport<std::vector<dvec4>>>("dvec4VectorOutport");
    registerPort<DataOutport<dvec4>>("dvec4Outport");
    registerPort<DataInport<ivec4>>("ivec4Inport");
    registerPort<DataInport<ivec4, 0>>("ivec4MutliInport");
    registerPort<DataInport<ivec4, 0, true>>("ivec4FlatMultiInport");
    registerPort<DataOutport<std::vector<ivec4>>>("ivec4VectorOutport");
    registerPort<DataOutport<ivec4>>("ivec4Outport");

    // Register PortInspectors
    registerPortInspector("org.inviwo.ImageOutport", app->getPath(PathType::PortInspectors, "/imageportinspector.inv"));
    registerPortInspector("org.inviwo.VolumeOutport", app->getPath(PathType::PortInspectors, "/volumeportinspector.inv"));
    registerPortInspector("org.inviwo.MeshOutport", app->getPath(PathType::PortInspectors, "/geometryportinspector.inv"));
    
    //registerProperty<EventProperty>(); TODO fix "default" contructor with 2 args...
    registerProperty<CompositeProperty>();
    registerProperty<AdvancedMaterialProperty>();
    registerProperty<BoolProperty>();
    registerProperty<ButtonProperty>();
    registerProperty<CameraProperty>();
    registerProperty<DirectoryProperty>();
    registerProperty<DoubleMat2Property>();
    registerProperty<DoubleMat3Property>();
    registerProperty<DoubleMat4Property>();
    registerProperty<DoubleProperty>();
    registerProperty<DoubleVec2Property>();
    registerProperty<DoubleVec3Property>();
    registerProperty<DoubleVec4Property>();
    registerProperty<DoubleMinMaxProperty>();
    registerProperty<FileProperty>();
    registerProperty<FilePatternProperty>();
    registerProperty<MultiFileProperty>();
    registerProperty<FloatMat2Property>();
    registerProperty<FloatMat3Property>();
    registerProperty<FloatMat4Property>();
    registerProperty<FloatMinMaxProperty>();
    registerProperty<FloatProperty>();
    registerProperty<FloatVec2Property>();
    registerProperty<FloatVec3Property>();
    registerProperty<FloatVec4Property>();
    registerProperty<ImageEditorProperty>();
    registerProperty<IntSizeTMinMaxProperty>();
    registerProperty<Int64MinMaxProperty>();
    registerProperty<IntMinMaxProperty>();
    registerProperty<IntProperty>();
    registerProperty<IntVec2Property>();
    registerProperty<IntVec3Property>();
    registerProperty<IntVec4Property>();
    registerProperty<IntSizeTProperty>();
    registerProperty<IntSize2Property>();
    registerProperty<IntSize3Property>();
    registerProperty<IntSize4Property>();
    registerProperty<OptionPropertyDouble>();
    registerProperty<OptionPropertyFloat>();
    registerProperty<OptionPropertyInt>();
    registerProperty<OptionPropertyString>();
    registerProperty<PlaneProperty>();
    registerProperty<PositionProperty>();
    registerProperty<SimpleLightingProperty>();
    registerProperty<SimpleRaycastingProperty>();
    registerProperty<StipplingProperty>();
    registerProperty<StringProperty>();
    registerProperty<TransferFunctionProperty>();
    registerProperty<VolumeIndicatorProperty>();
    registerProperty<BoolCompositeProperty>();


    std::function<void(std::unique_ptr<PropertyConverter>)> registerPC =
        [this](std::unique_ptr<PropertyConverter> propertyConverter) {
            registerPropertyConverter(std::move(propertyConverter));
        };
    using Scalars = std::tuple<float, double, int, glm::i64, size_t>;
    using Vec2s = std::tuple<vec2, dvec2, ivec2, size2_t>;
    using Vec3s = std::tuple<vec3, dvec3, ivec3, size3_t>;
    using Vec4s = std::tuple<vec4, dvec4, ivec4, size4_t>;

    util::for_each_type_pair<Scalars, Scalars>{}(ConverterRegFunctor{}, registerPC);
    util::for_each_type_pair<Vec2s, Vec2s>{}(ConverterRegFunctor{}, registerPC);
    util::for_each_type_pair<Vec3s, Vec3s>{}(ConverterRegFunctor{}, registerPC);
    util::for_each_type_pair<Vec4s, Vec4s>{}(ConverterRegFunctor{}, registerPC);

    util::for_each_type<Scalars>{}(ScalarStringConverterRegFunctor{}, registerPC);
    util::for_each_type<Vec2s>{}(VectorStringConverterRegFunctor{}, registerPC);
    util::for_each_type<Vec3s>{}(VectorStringConverterRegFunctor{}, registerPC);
    util::for_each_type<Vec4s>{}(VectorStringConverterRegFunctor{}, registerPC);

    // Register Settings
    // Do this after the property registration since the settings use properties.
    registerSettings(util::make_unique<SystemSettings>());
    registerSettings(util::make_unique<LinkSettings>("Link Settings", app_->getPropertyFactory()));
}

std::string InviwoCore::getPath() const { return filesystem::findBasePath(); }

}  // namespace inviwo
