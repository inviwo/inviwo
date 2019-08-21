/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2019 Inviwo Foundation
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

#include <warn/push>
#include <warn/ignore/unused-function>
#include <inviwo/core/properties/optionproperty.h>
#include <warn/pop>

// Cameras
#include <inviwo/core/datastructures/camera.h>

// Meta Data
#include <inviwo/core/metadata/metadata.h>
#include <inviwo/core/metadata/containermetadata.h>
#include <inviwo/core/metadata/processormetadata.h>
#include <inviwo/core/metadata/processorwidgetmetadata.h>

// Utilizes
#include <inviwo/core/util/settings/linksettings.h>

// Io
#include <inviwo/core/io/rawvolumereader.h>

// Others
#include <inviwo/core/processors/canvasprocessor.h>

// Ports
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/ports/bufferport.h>
#include <inviwo/core/datastructures/light/baselightsource.h>

// PortInspectors
#include <inviwo/core/ports/portinspector.h>

// Properties
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/buttongroupproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/directoryproperty.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/filepatternproperty.h>
#include <inviwo/core/properties/imageeditorproperty.h>
#include <inviwo/core/properties/isovalueproperty.h>
#include <inviwo/core/properties/isotfproperty.h>
#include <inviwo/core/properties/listproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/multifileproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/planeproperty.h>
#include <inviwo/core/properties/positionproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/properties/advancedmaterialproperty.h>
#include <inviwo/core/properties/raycastingproperty.h>
#include <inviwo/core/properties/simplelightingproperty.h>
#include <inviwo/core/properties/simpleraycastingproperty.h>
#include <inviwo/core/properties/stipplingproperty.h>
#include <inviwo/core/properties/volumeindicatorproperty.h>

#include <inviwo/core/properties/propertyconvertermanager.h>
#include <inviwo/core/properties/propertyconverter.h>

// Processors
#include <inviwo/core/processors/compositeprocessor.h>
#include <inviwo/core/processors/compositesink.h>
#include <inviwo/core/processors/compositesource.h>

#include <inviwo/core/util/stdextensions.h>

#include <inviwo/core/datastructures/representationutil.h>

namespace inviwo {

namespace {

// Functors for registration of property converters
// Can't use a regular lambda since we need the explicit template arguments.
// We take a std::function to register the created converter since the registration function is
// protected in the inviwo module
struct ConverterRegFunctor {
    template <typename T, typename U>
    auto operator()(InviwoModule& m) {
        if (!std::is_same<T, U>::value) {
            m.registerPropertyConverter(
                std::make_unique<
                    OrdinalPropertyConverter<OrdinalProperty<T>, OrdinalProperty<U>>>());
        }
    }
};
struct ScalarStringConverterRegFunctor {
    template <typename T>
    auto operator()(InviwoModule& m) {
        m.registerPropertyConverter(
            std::make_unique<ScalarToStringConverter<OrdinalProperty<T>>>());
    }
};
struct VectorStringConverterRegFunctor {
    template <typename T>
    auto operator()(InviwoModule& m) {
        m.registerPropertyConverter(
            std::make_unique<VectorToStringConverter<OrdinalProperty<T>>>());
    }
};

enum class OptionRegEnumInt : int {};
enum class OptionRegEnumUInt : unsigned int {};

struct OptionStringConverterRegFunctor {
    template <typename T>
    auto operator()(InviwoModule& m) {
        m.registerPropertyConverter(
            std::make_unique<OptionToStringConverter<TemplateOptionProperty<T>>>());
    }
};

struct OptionIntConverterRegFunctor {
    template <typename T>
    auto operator()(InviwoModule& m) {
        m.registerPropertyConverter(
            std::make_unique<OptionToIntConverter<TemplateOptionProperty<T>>>());
    }
};

struct IntOptionConverterRegFunctor {
    template <typename T>
    auto operator()(InviwoModule& m) {
        m.registerPropertyConverter(
            std::make_unique<IntToOptionConverter<TemplateOptionProperty<T>>>());
    }
};

}  // namespace

template class TemplateOptionProperty<OptionRegEnumInt>;
template class TemplateOptionProperty<OptionRegEnumUInt>;

InviwoCore::Observer::Observer(InviwoCore& core, InviwoApplication* app)
    : FileObserver(app), core_(core) {}
void InviwoCore::Observer::fileChanged(const std::string& dir) { core_.scanDirForComposites(dir); }

InviwoCore::InviwoCore(InviwoApplication* app)
    : InviwoModule(app, "Core"), compositeDirObserver_{*this, app} {

    util::registerCoreRepresentations(*this);

    // Register MetaData
    registerMetaData(std::make_unique<BoolMetaData>());
    registerMetaData(std::make_unique<IntMetaData>());
    registerMetaData(std::make_unique<FloatMetaData>());
    registerMetaData(std::make_unique<DoubleMetaData>());
    registerMetaData(std::make_unique<StringMetaData>());
    registerMetaData(std::make_unique<SizeMetaData>());
    registerMetaData(std::make_unique<FloatVec2MetaData>());
    registerMetaData(std::make_unique<FloatVec3MetaData>());
    registerMetaData(std::make_unique<FloatVec4MetaData>());
    registerMetaData(std::make_unique<DoubleVec2MetaData>());
    registerMetaData(std::make_unique<DoubleVec3MetaData>());
    registerMetaData(std::make_unique<DoubleVec4MetaData>());
    registerMetaData(std::make_unique<IntVec2MetaData>());
    registerMetaData(std::make_unique<IntVec3MetaData>());
    registerMetaData(std::make_unique<IntVec4MetaData>());
    registerMetaData(std::make_unique<UIntVec2MetaData>());
    registerMetaData(std::make_unique<UIntVec3MetaData>());
    registerMetaData(std::make_unique<UIntVec4MetaData>());
    registerMetaData(std::make_unique<FloatMat2MetaData>());
    registerMetaData(std::make_unique<FloatMat3MetaData>());
    registerMetaData(std::make_unique<FloatMat4MetaData>());
    registerMetaData(std::make_unique<DoubleMat2MetaData>());
    registerMetaData(std::make_unique<DoubleMat4MetaData>());
    registerMetaData(std::make_unique<DoubleMat3MetaData>());
    registerMetaData(std::make_unique<Size2MetaData>());
    registerMetaData(std::make_unique<Size3MetaData>());
    registerMetaData(std::make_unique<Size4MetaData>());
    registerMetaData(std::make_unique<VectorMetaData<2, float>>());
    registerMetaData(std::make_unique<VectorMetaData<3, float>>());
    registerMetaData(std::make_unique<VectorMetaData<4, float>>());
    registerMetaData(std::make_unique<VectorMetaData<2, double>>());
    registerMetaData(std::make_unique<VectorMetaData<3, double>>());
    registerMetaData(std::make_unique<VectorMetaData<4, double>>());
    registerMetaData(std::make_unique<VectorMetaData<2, int>>());
    registerMetaData(std::make_unique<VectorMetaData<3, int>>());
    registerMetaData(std::make_unique<VectorMetaData<4, int>>());
    registerMetaData(std::make_unique<VectorMetaData<2, unsigned int>>());
    registerMetaData(std::make_unique<VectorMetaData<3, unsigned int>>());
    registerMetaData(std::make_unique<VectorMetaData<4, unsigned int>>());
    registerMetaData(std::make_unique<MatrixMetaData<2, float>>());
    registerMetaData(std::make_unique<MatrixMetaData<3, float>>());
    registerMetaData(std::make_unique<MatrixMetaData<4, float>>());
    registerMetaData(std::make_unique<MatrixMetaData<2, double>>());
    registerMetaData(std::make_unique<MatrixMetaData<3, double>>());
    registerMetaData(std::make_unique<MatrixMetaData<4, double>>());
    registerMetaData(std::make_unique<PositionMetaData>());
    registerMetaData(std::make_unique<ProcessorMetaData>());
    registerMetaData(std::make_unique<ProcessorWidgetMetaData>());
    registerMetaData(std::make_unique<StdUnorderedMapMetaData<std::string, std::string>>());
    registerMetaData(std::make_unique<StdVectorMetaData<std::string>>());

    // Register Cameras
    registerCamera<PerspectiveCamera>("PerspectiveCamera");
    registerCamera<OrthographicCamera>("OrthographicCamera");
    registerCamera<SkewedPerspectiveCamera>("SkewedPerspectiveCamera");

    // Register Data readers
    registerDataReader(std::make_unique<RawVolumeReader>());
    // Register Data writers

    // Register Ports
    registerPort<ImageInport>();
    registerPort<ImageMultiInport>();
    registerPort<ImageOutport>();
    registerProcessor<CompositeSource<ImageInport, ImageOutport>>();
    registerProcessor<CompositeSink<ImageInport, ImageOutport>>();
    registerProcessor<CompositeProcessor>();

    registerDefaultsForDataType<Mesh>();
    registerDefaultsForDataType<Volume>();
    registerDefaultsForDataType<VolumeSequence>();
    registerDefaultsForDataType<BufferBase>();
    registerDefaultsForDataType<LightSource>();

    registerDefaultsForDataType<vec2>();
    registerDefaultsForDataType<dvec2>();
    registerDefaultsForDataType<ivec2>();
    registerDefaultsForDataType<vec3>();
    registerDefaultsForDataType<dvec3>();
    registerDefaultsForDataType<ivec3>();
    registerDefaultsForDataType<vec4>();
    registerDefaultsForDataType<dvec4>();
    registerDefaultsForDataType<ivec4>();

    registerDefaultsForDataType<std::vector<vec2>>();
    registerDefaultsForDataType<std::vector<dvec2>>();
    registerDefaultsForDataType<std::vector<ivec2>>();
    registerDefaultsForDataType<std::vector<vec3>>();
    registerDefaultsForDataType<std::vector<dvec3>>();
    registerDefaultsForDataType<std::vector<ivec3>>();
    registerDefaultsForDataType<std::vector<vec4>>();
    registerDefaultsForDataType<std::vector<dvec4>>();
    registerDefaultsForDataType<std::vector<ivec4>>();

    // Register PortInspectors
    registerPortInspector(PortTraits<ImageOutport>::classIdentifier(),
                          app->getPath(PathType::PortInspectors, "/imageportinspector.inv"));
    registerPortInspector(PortTraits<VolumeOutport>::classIdentifier(),
                          app->getPath(PathType::PortInspectors, "/volumeportinspector.inv"));
    registerPortInspector(PortTraits<MeshOutport>::classIdentifier(),
                          app->getPath(PathType::PortInspectors, "/geometryportinspector.inv"));

    registerProperty<CompositeProperty>();
    registerProperty<AdvancedMaterialProperty>();
    registerProperty<BoolProperty>();
    registerProperty<ButtonGroupProperty>();
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
    registerProperty<FloatQuaternionProperty>();
    registerProperty<DoubleQuaternionProperty>();
    registerProperty<IsoValueProperty>();
    registerProperty<IsoTFProperty>();
    registerProperty<ListProperty>();
    registerProperty<OptionPropertyDouble>();
    registerProperty<OptionPropertyFloat>();
    registerProperty<OptionPropertyInt>();
    registerProperty<OptionPropertyString>();
    registerProperty<PlaneProperty>();
    registerProperty<PositionProperty>();
    registerProperty<RaycastingProperty>();
    registerProperty<SimpleLightingProperty>();
    registerProperty<SimpleRaycastingProperty>();
    registerProperty<StipplingProperty>();
    registerProperty<StringProperty>();
    registerProperty<TransferFunctionProperty>();
    registerProperty<VolumeIndicatorProperty>();
    registerProperty<BoolCompositeProperty>();

    using Scalars = std::tuple<float, double, int, glm::i64, size_t>;
    using Vec2s = std::tuple<vec2, dvec2, ivec2, size2_t>;
    using Vec3s = std::tuple<vec3, dvec3, ivec3, size3_t>;
    using Vec4s = std::tuple<vec4, dvec4, ivec4, size4_t>;

    registerPropertyConverter(std::make_unique<FileToStringConverter>());
    registerPropertyConverter(std::make_unique<StringToFileConverter>());
    registerPropertyConverter(std::make_unique<DirectoryToStringConverter>());
    registerPropertyConverter(std::make_unique<StringToDirectoryConverter>());

    registerPropertyConverter(std::make_unique<TransferfunctionToIsoTFConverter>());
    registerPropertyConverter(std::make_unique<IsoTFToTransferfunctionConverter>());
    registerPropertyConverter(std::make_unique<IsovalueToIsoTFConverter>());
    registerPropertyConverter(std::make_unique<IsoTFToIsovalueConverter>());

    // for_each_type_pair will call the functor with all permutation of types, and supplied
    // arguments like: ConverterRegFunctor<float, float>(*this), ConverterRegFunctor<float,
    // double>(*this), ...
    util::for_each_type_pair<Scalars, Scalars>{}(ConverterRegFunctor{}, *this);
    util::for_each_type_pair<Vec2s, Vec2s>{}(ConverterRegFunctor{}, *this);
    util::for_each_type_pair<Vec3s, Vec3s>{}(ConverterRegFunctor{}, *this);
    util::for_each_type_pair<Vec4s, Vec4s>{}(ConverterRegFunctor{}, *this);

    util::for_each_type<Scalars>{}(ScalarStringConverterRegFunctor{}, *this);
    util::for_each_type<Vec2s>{}(VectorStringConverterRegFunctor{}, *this);
    util::for_each_type<Vec3s>{}(VectorStringConverterRegFunctor{}, *this);
    util::for_each_type<Vec4s>{}(VectorStringConverterRegFunctor{}, *this);

    using OptionTypes = std::tuple<unsigned int, int, size_t, float, double, std::string>;
    util::for_each_type<OptionTypes>{}(OptionStringConverterRegFunctor{}, *this);
    util::for_each_type<OptionTypes>{}(OptionIntConverterRegFunctor{}, *this);
    util::for_each_type<OptionTypes>{}(IntOptionConverterRegFunctor{}, *this);

    using OptionEnumTypes = std::tuple<OptionRegEnumInt, OptionRegEnumUInt>;
    util::for_each_type<OptionEnumTypes>{}(OptionStringConverterRegFunctor{}, *this);
    util::for_each_type<OptionEnumTypes>{}(OptionIntConverterRegFunctor{}, *this);
    util::for_each_type<OptionEnumTypes>{}(IntOptionConverterRegFunctor{}, *this);

    // Observe composite processors
    auto userCompositeDir = app_->getPath(PathType::Settings, "/composites");
    scanDirForComposites(userCompositeDir);
    compositeDirObserver_.startFileObservation(userCompositeDir);

    auto coreCompositeDir = app_->getPath(PathType::Workspaces, "/composites");
    scanDirForComposites(coreCompositeDir);
    compositeDirObserver_.startFileObservation(coreCompositeDir);

    // Register Settings
    // Do this after the property registration since the settings use properties.
    registerSettings(std::make_unique<LinkSettings>("Link Settings", app_->getPropertyFactory()));
}

std::string InviwoCore::getPath() const { return filesystem::findBasePath(); }

void InviwoCore::scanDirForComposites(const std::string& dir) {
    for (auto&& file :
         filesystem::getDirectoryContentsRecursively(dir, filesystem::ListMode::Files)) {
        if (filesystem::getFileExtension(file) == "inv") {
            if (addedCompositeFiles_.count(file) == 0) {
                registerCompositeProcessor(file);
                addedCompositeFiles_.insert(file);
            }
        }
    }
}

}  // namespace inviwo
