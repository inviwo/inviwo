/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2026 Inviwo Foundation
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

#include <modules/base/basemodule.h>

#include <inviwo/core/common/inviwomodule.h>              // for InviwoModule
#include <inviwo/core/datastructures/buffer/buffer.h>     // for Buffer
#include <inviwo/core/datastructures/buffer/bufferram.h>  // for BufferRAMPre...
#include <inviwo/core/datastructures/geometry/mesh.h>     // for Mesh
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/representationconverter.h>         // for Representati...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for Representati...
#include <inviwo/core/datastructures/volume/volume.h>                   // for Volume, Data...
#include <inviwo/core/io/datareader.h>                                  // for DataReader
#include <inviwo/core/io/datawriter.h>                                  // for DataWriter
#include <inviwo/core/io/serialization/ticpp.h>                         // for TxElement, Node
#include <inviwo/core/io/serialization/versionconverter.h>              // for Kind, Identi...
#include <inviwo/core/ports/datainport.h>                               // for MultiDataInport
#include <inviwo/core/ports/imageport.h>                                // for ImageMultiIn...
#include <inviwo/core/ports/meshport.h>                                 // for MeshOutport
#include <inviwo/core/ports/outportiterable.h>                          // for OutportIterable
#include <inviwo/core/ports/volumeport.h>                               // for VolumeOutport
#include <inviwo/core/properties/optionproperty.h>                      // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>                     // for OrdinalProperty
#include <inviwo/core/rendering/datavisualizer.h>                       // for DataVisualizer
#include <inviwo/core/util/charconv.h>
#include <inviwo/core/util/foreacharg.h>        // for for_each_type
#include <inviwo/core/util/glmvec.h>            // for vec3, vec4
#include <inviwo/core/util/staticstring.h>      // for operator+
#include <inviwo/core/util/stringconversion.h>  // for dotSeperated...

#include <modules/base/datavisualizer/imageinformationvisualizer.h>
#include <modules/base/datavisualizer/meshinformationvisualizer.h>
#include <modules/base/datavisualizer/volumeinformationvisualizer.h>
#include <modules/base/datavisualizer/layerinformationvisualizer.h>
#include <modules/base/datavisualizer/layertoimagevisualizer.h>
#include <modules/base/datavisualizer/imagetolayervisualizer.h>
// Io
#include <modules/base/io/amirameshreader.h>
#include <modules/base/io/amiravolumereader.h>
#include <modules/base/io/binarystlwriter.h>
#include <modules/base/io/datvolumesequencereader.h>
#include <modules/base/io/datvolumewriter.h>
#include <modules/base/io/ivfvolumereader.h>
#include <modules/base/io/ivfvolumewriter.h>
#include <modules/base/io/stlwriter.h>
#include <modules/base/io/wavefrontwriter.h>
// Processors
#include <modules/base/processors/buffertomeshprocessor.h>
#include <modules/base/processors/camerafrustum.h>
#include <modules/base/processors/convexhull2dprocessor.h>
#include <modules/base/processors/cubeproxygeometryprocessor.h>
#include <modules/base/processors/diffuselightsourceprocessor.h>
#include <modules/base/processors/directionallightsourceprocessor.h>
#include <modules/base/processors/distancetransformram.h>
#include <modules/base/processors/filecache.h>
#include <modules/base/processors/gridplanes.h>
#include <modules/base/processors/heightfieldmapper.h>
#include <modules/base/processors/imagecontourprocessor.h>
#include <modules/base/processors/imageexport.h>
#include <modules/base/processors/imageinformation.h>
#include <modules/base/processors/imagesnapshot.h>
#include <modules/base/processors/imagesource.h>
#include <modules/base/processors/imagesourceseries.h>
#include <modules/base/processors/imagestackvolumesource.h>
#include <modules/base/processors/imagetolayer.h>
#include <modules/base/processors/imagetospatialsampler.h>
#include <modules/base/processors/inputselector.h>
#include <modules/base/processors/layerboundingbox.h>
#include <modules/base/processors/layercombiner.h>
#include <modules/base/processors/layercontour.h>
#include <modules/base/processors/layercreator.h>
#include <modules/base/processors/layerdistancetransform.h>
#include <modules/base/processors/imagedistancetransform.h>
#include <modules/base/processors/layerexport.h>
#include <modules/base/processors/layerinformation.h>
#include <modules/base/processors/layersequencesource.h>
#include <modules/base/processors/layerseriessource.h>
#include <modules/base/processors/layersource.h>
#include <modules/base/processors/layertoimage.h>
#include <modules/base/processors/layertospatialsampler.h>
#include <modules/base/processors/meshclipping.h>
#include <modules/base/processors/meshcolorfromnormals.h>
#include <modules/base/processors/meshconverterprocessor.h>
#include <modules/base/processors/meshcreator.h>
#include <modules/base/processors/meshexport.h>
#include <modules/base/processors/meshinformation.h>
#include <modules/base/processors/meshmapping.h>
#include <modules/base/processors/meshplaneclipping.h>
#include <modules/base/processors/meshsequencesource.h>
#include <modules/base/processors/meshsource.h>
#include <modules/base/processors/noisegenerator2d.h>
#include <modules/base/processors/noisegenerator3d.h>
#include <modules/base/processors/ordinalpropertyanimator.h>
#include <modules/base/processors/orientationindicator.h>
#include <modules/base/processors/pixeltobufferprocessor.h>
#include <modules/base/processors/pixelvalue.h>
#include <modules/base/processors/pointgenerationprocessor.h>
#include <modules/base/processors/pointlightsourceprocessor.h>
#include <modules/base/processors/randommeshgenerator.h>
#include <modules/base/processors/randomspheregenerator.h>
#include <modules/base/processors/singlevoxel.h>
#include <modules/base/processors/spotlightsourceprocessor.h>
#include <modules/base/processors/stereocamerasyncer.h>
#include <modules/base/processors/surfaceextractionprocessor.h>
#include <modules/base/processors/testvolumecreator.h>
#include <modules/base/processors/tfselector.h>
#include <modules/base/processors/transform.h>
#include <modules/base/processors/trianglestowireframe.h>
#include <modules/base/processors/vectortobuffer.h>
#include <modules/base/processors/volumebasistransformer.h>
#include <modules/base/processors/volumeboundaryplanes.h>
#include <modules/base/processors/volumeboundingbox.h>
#include <modules/base/processors/volumechannelcombiner.h>
#include <modules/base/processors/volumeconverter.h>
#include <modules/base/processors/volumecreator.h>
#include <modules/base/processors/volumecurlcpuprocessor.h>
#include <modules/base/processors/volumedivergencecpuprocessor.h>
#include <modules/base/processors/volumeexport.h>
#include <modules/base/processors/volumegradientcpuprocessor.h>
#include <modules/base/processors/volumehistogram2d.h>
#include <modules/base/processors/volumeinformation.h>
#include <modules/base/processors/volumelaplacianprocessor.h>
#include <modules/base/processors/volumesequenceexport.h>
#include <modules/base/processors/volumesequencesingletimestepsampler.h>
#include <modules/base/processors/volumesequencesource.h>
#include <modules/base/processors/volumesequencetospatial4dsampler.h>
#include <modules/base/processors/volumeshifter.h>
#include <modules/base/processors/volumesliceextractor.h>
#include <modules/base/processors/volumeslicetolayer.h>
#include <modules/base/processors/volumesource.h>
#include <modules/base/processors/volumedownsample.h>
#include <modules/base/processors/volumesubset.h>
#include <modules/base/processors/volumetospatialsampler.h>
#include <modules/base/processors/worldtransformdeprecated.h>
// Properties
#include <modules/base/properties/basisproperty.h>
#include <modules/base/properties/bufferinformationproperty.h>
#include <modules/base/properties/datarangeproperty.h>
#include <modules/base/properties/gaussianproperty.h>
#include <modules/base/properties/imageinformationproperty.h>
#include <modules/base/properties/layerinformationproperty.h>
#include <modules/base/properties/meshinformationproperty.h>
#include <modules/base/properties/sequencetimerproperty.h>
#include <modules/base/properties/transformlistproperty.h>
#include <modules/base/properties/valueaxisproperty.h>
#include <modules/base/properties/volumeinformationproperty.h>

#include <functional>
#include <initializer_list>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <fmt/core.h>
#include <fmt/format.h>
#include <glm/common.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/ext/vector_uint3.hpp>
#include <glm/gtx/io.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/vector_relational.hpp>

namespace inviwo {
class InviwoApplication;

using BasisTransformMesh = BasisTransform<Mesh>;
using BasisTransformVolume = BasisTransform<Volume>;

using TransformLayer = Transform<Layer>;
using TransformMesh = Transform<Mesh>;
using TransformVolume = Transform<Volume>;

using WorldTransformMeshDeprecated = WorldTransformDeprecated<Mesh>;
using WorldTransformVolumeDeprecated = WorldTransformDeprecated<Volume>;

BaseModule::BaseModule(InviwoApplication* app) : InviwoModule(app, "Base") {
    registerProcessor<BasisTransformMesh>();
    registerProcessor<BasisTransformVolume>();
    registerProcessor<BufferToMeshProcessor>();
    registerProcessor<CameraFrustum>();
    registerProcessor<ConvexHull2DProcessor>();
    registerProcessor<CubeProxyGeometry>();
    registerProcessor<DiffuseLightSourceProcessor>();
    registerProcessor<DirectionalLightSourceProcessor>();
    registerProcessor<DistanceTransformRAM>();
    registerProcessor<GridPlanes>();
    registerProcessor<HeightFieldMapper>();
    registerProcessor<VolumeHistogram2D>();
    registerProcessor<ImageContourProcessor>();
    registerProcessor<ImageDistanceTransform>();
    registerProcessor<ImageExport>();
    registerProcessor<ImageInformation>();
    registerProcessor<ImageSnapshot>();
    registerProcessor<ImageSource>();
    registerProcessor<ImageSourceSeries>();
    registerProcessor<ImageStackVolumeSource>();
    registerProcessor<ImageToLayer>();
    registerProcessor<ImageToSpatialSampler>();
    registerProcessor<LayerBoundingBox>();
    registerProcessor<LayerCombiner>();
    registerProcessor<LayerContour>();
    registerProcessor<LayerCreator>();
    registerProcessor<LayerDistanceTransform>();
    registerProcessor<LayerExport>();
    registerProcessor<LayerInformation>();
    registerProcessor<LayerSequenceSource>();
    registerProcessor<LayerSeriesSource>();
    registerProcessor<LayerSource>();
    registerProcessor<LayerToImage>();
    registerProcessor<LayerToSpatialSampler>();
    registerProcessor<MeshClipping>();
    registerProcessor<MeshColorFromNormals>();
    registerProcessor<MeshConverterProcessor>();
    registerProcessor<MeshCreator>();
    registerProcessor<MeshExport>();
    registerProcessor<MeshInformation>();
    registerProcessor<MeshMapping>();
    registerProcessor<MeshPlaneClipping>();
    registerProcessor<MeshSequenceSource>();
    registerProcessor<MeshSource>();
    registerProcessor<NoiseGenerator2D>();
    registerProcessor<NoiseGenerator3D>();
    registerProcessor<OrdinalPropertyAnimator>();
    registerProcessor<OrientationIndicator>();
    registerProcessor<PixelToBufferProcessor>();
    registerProcessor<PixelValue>();
    registerProcessor<Point3DGenerationProcessor>();
    registerProcessor<PointLightSourceProcessor>();
    registerProcessor<RandomMeshGenerator>();
    registerProcessor<RandomSphereGenerator>();
    registerProcessor<SingleVoxel>();
    registerProcessor<SpotLightSourceProcessor>();
    registerProcessor<StereoCameraSyncer>();
    registerProcessor<SurfaceExtraction>();
    registerProcessor<TFSelector>();
    registerProcessor<TestVolumeCreator>();
    registerProcessor<TransformLayer>();
    registerProcessor<TransformMesh>();
    registerProcessor<TransformVolume>();
    registerProcessor<TrianglesToWireframe>();
    registerProcessor<VectorToBuffer<float>>();
    registerProcessor<VectorToBuffer<unsigned int>>();
    registerProcessor<VectorToBuffer<vec2>>();
    registerProcessor<VectorToBuffer<vec3>>();
    registerProcessor<VectorToBuffer<vec4>>();
    registerProcessor<VolumeBoundaryPlanes>();
    registerProcessor<VolumeBoundingBox>();
    registerProcessor<VolumeChannelCombiner>();
    registerProcessor<VolumeConverter>();
    registerProcessor<VolumeCreator>();
    registerProcessor<VolumeCurlCPUProcessor>();
    registerProcessor<VolumeDivergenceCPUProcessor>();
    registerProcessor<VolumeDownsample>();
    registerProcessor<VolumeExport>();
    registerProcessor<VolumeGradientCPUProcessor>();
    registerProcessor<VolumeInformation>();
    registerProcessor<VolumeLaplacianProcessor>();
    registerProcessor<VolumeSequenceExport>();
    registerProcessor<VolumeSequenceSingleTimestepSamplerProcessor>();
    registerProcessor<VolumeSequenceSource>();
    registerProcessor<VolumeSequenceToSpatial4DSampler>();
    registerProcessor<VolumeShifter>();
    registerProcessor<VolumeSliceExtractor>();
    registerProcessor<VolumeSliceToLayer>();
    registerProcessor<VolumeSource>();
    registerProcessor<VolumeSubset>();
    registerProcessor<VolumeToSpatialSampler>();
    registerProcessor<WorldTransformMeshDeprecated>();
    registerProcessor<WorldTransformVolumeDeprecated>();

    // input selectors
    registerProcessor<InputSelector<MultiDataInport<Volume>, VolumeOutport>>();
    registerProcessor<InputSelector<MultiDataInport<Mesh>, MeshOutport>>();
    registerProcessor<InputSelector<ImageMultiInport, ImageOutport>>();
    registerProcessor<InputSelector<LayerMultiInport, LayerOutport>>();

    // FileCache
    registerProcessor<FileCache<Volume>>();
    registerProcessor<FileCache<Mesh>>();
    registerProcessor<FileCache<Layer>>();
    registerProcessor<FileCache<Image, ImageInport, ImageOutport>>();

    registerProperty<BasisProperty>();
    registerProperty<BufferInformationProperty>();
    registerProperty<DataRangeProperty>();
    registerProperty<ImageInformationProperty>();
    registerProperty<IndexBufferInformationProperty>();
    registerProperty<LayerInformationProperty>();
    registerProperty<MeshBufferInformationProperty>();
    registerProperty<MeshInformationProperty>();
    registerProperty<SequenceTimerProperty>();
    registerProperty<ValueAxisProperty>();
    registerProperty<VolumeInformationProperty>();

    registerProperty<Gaussian1DProperty>();
    registerProperty<Gaussian2DProperty>();

    registerProperty<transform::TranslateProperty>();
    registerProperty<transform::RotateProperty>();
    registerProperty<transform::ScaleProperty>();
    registerProperty<transform::CustomTransformProperty>();
    registerProperty<TransformationList>();
    registerProperty<TransformListProperty>();

    // Register Data readers
    registerDataReader(std::make_unique<DatVolumeReader>());
    registerDataReader(std::make_unique<DatVolumeSequenceReader>());
    registerDataReader(std::make_unique<IvfVolumeReader>());
    registerDataReader(std::make_unique<IvfVolumeSequenceReader>());
    // Register Data writers
    registerDataWriter(std::make_unique<DatVolumeWriter>());
    registerDataWriter(std::make_unique<IvfVolumeWriter>());
    registerDataWriter(std::make_unique<IvfVolumeSequenceWriter>());
    registerDataWriter(std::make_unique<StlWriter>());
    registerDataWriter(std::make_unique<BinarySTLWriter>());
    registerDataWriter(std::make_unique<WaveFrontWriter>());
    registerDataReader(std::make_unique<AmiraMeshReader>());
    registerDataReader(std::make_unique<AmiraVolumeReader>());

    registerDataVisualizer(std::make_unique<ImageInformationVisualizer>(app));
    registerDataVisualizer(std::make_unique<MeshInformationVisualizer>(app));
    registerDataVisualizer(std::make_unique<VolumeInformationVisualizer>(app));
    registerDataVisualizer(std::make_unique<LayerInformationVisualizer>(app));
    registerDataVisualizer(std::make_unique<LayerToImageVisualizer>(app));
    registerDataVisualizer(std::make_unique<ImageToLayerVisualizer>(app));

    util::for_each_type<OrdinalPropertyAnimator::Types>{}(RegHelper{}, *this);
}

int BaseModule::getVersion() const { return 12; }

std::unique_ptr<VersionConverter> BaseModule::getConverter(int version) const {
    return std::make_unique<Converter>(version);
}

BaseModule::Converter::Converter(int version) : version_(version) {}

namespace {

bool updateV0(TxElement* root) {
    std::vector<xml::IdentifierReplacement> repl = {
        // CubeProxyGeometry
        {{xml::Kind::processor("org.inviwo.CubeProxyGeometry"),
          xml::Kind::inport("org.inviwo.VolumeInport")},
         "volume.inport",
         "volume"},
        {{xml::Kind::processor("org.inviwo.CubeProxyGeometry"),
          xml::Kind::outport("org.inviwo.MeshOutport")},
         "geometry.outport",
         "proxyGeometry"},
        // ImageSource
        {{xml::Kind::processor("org.inviwo.ImageSource"),
          xml::Kind::outport("org.inviwo.ImageOutport")},
         "image.outport",
         "image"},
        // MeshClipping
        {{xml::Kind::processor("org.inviwo.MeshClipping"),
          xml::Kind::inport("org.inviwo.MeshInport")},
         "geometry.input",
         "inputMesh"},
        {{xml::Kind::processor("org.inviwo.MeshClipping"),
          xml::Kind::outport("org.inviwo.MeshOutport")},
         "geometry.output",
         "clippedMesh"},

        // VolumeSlice
        {{xml::Kind::processor("org.inviwo.VolumeSlice"),
          xml::Kind::inport("org.inviwo.VolumeInport")},
         "volume.inport",
         "inputVolume"},
        {{xml::Kind::processor("org.inviwo.VolumeSlice"),
          xml::Kind::outport("org.inviwo.ImageOutport")},
         "image.outport",
         "outputImage"},

        // VolumeSubsample
        {{xml::Kind::processor("org.inviwo.VolumeSubsample"),
          xml::Kind::inport("org.inviwo.VolumeInport")},
         "volume.inport",
         "inputVolume"},
        {{xml::Kind::processor("org.inviwo.VolumeSubsample"),
          xml::Kind::outport("org.inviwo.VolumeOutport")},
         "volume.outport",
         "outputVolume"},

        // DistanceTransformRAM
        {{xml::Kind::processor("org.inviwo.DistanceTransformRAM"),
          xml::Kind::inport("org.inviwo.VolumeInport")},
         "volume.inport",
         "inputVolume"},
        {{xml::Kind::processor("org.inviwo.DistanceTransformRAM"),
          xml::Kind::outport("org.inviwo.VolumeOutport")},
         "volume.outport",
         "outputVolume"},

        // VolumeSubset
        {{xml::Kind::processor("org.inviwo.VolumeSubset"),
          xml::Kind::inport("org.inviwo.VolumeInport")},
         "volume.inport",
         "inputVolume"},
        {{xml::Kind::processor("org.inviwo.VolumeSubset"),
          xml::Kind::outport("org.inviwo.VolumeOutport")},
         "volume.outport",
         "outputVolume"}};

    for (const auto& id : {"org.inviwo.FloatProperty", "org.inviwo.FloatVec2Property",
                           "org.inviwo.FloatVec3Property", "org.inviwo.FloatVec4Property",
                           "org.inviwo.DoubleProperty", "org.inviwo.DoubleVec2Property",
                           "org.inviwo.DoubleVec3Property", "org.inviwo.DoubleVec4Property",
                           "org.inviwo.IntProperty", "org.inviwo.IntVec2Property",
                           "org.inviwo.IntVec3Property", "org.inviwo.IntVec4Property"}) {
        xml::IdentifierReplacement ir1 = {
            {xml::Kind::processor("org.inviwo.OrdinalPropertyAnimator"), xml::Kind::property(id)},
            id,
            dotSeperatedToPascalCase(id)};
        repl.push_back(ir1);
        xml::IdentifierReplacement ir2 = {
            {xml::Kind::processor("org.inviwo.OrdinalPropertyAnimator"), xml::Kind::property(id)},
            std::string(id) + "-Delta",
            dotSeperatedToPascalCase(id) + "-Delta"};
        repl.push_back(ir2);
    }

    return xml::changeIdentifiers(root, repl);
}

bool updateV2(TxElement* root) {
    bool res = false;
    TraversingVersionConverter conv{[&](TxElement* node) -> bool {
        const auto& key = node->Value();
        if (key != "Property") return true;
        const auto& type = node->GetAttribute("type");
        if (type != "org.inviwo.VolumeBasisProperty") return true;

        TxElement newNode{"overrideModel"};
        for (const auto& item : {std::make_tuple(0, "A", 0.0), std::make_tuple(1, "B", 0.0),
                                 std::make_tuple(2, "C", 0.0), std::make_tuple(3, "Offset", 1.0)}) {
            const auto path =
                fmt::format("Properties/Property&identifier=override{}/value", std::get<1>(item));
            if (auto* elem = xml::getElement(node, path)) {
                auto* data = elem->Clone();
                data->SetValue(fmt::format("col{}", std::get<0>(item)));
                data->ToElement()->SetAttribute("w", fmt::format("{:f}", std::get<2>(item)));
                newNode.InsertEndChild(*data);
            }
        }

        res = true;
        node->InsertEndChild(newNode);

        return true;
    }};

    conv.convert(root);
    return res;
}

bool updateV3(TxElement* root) {

    std::vector<xml::IdentifierReplacement> replV3 = {
        // WorldTransform (Deprecated)
        {{xml::Kind::processor("org.inviwo.WorldTransformMeshDeprecated")},
         "World Transform Mesh",
         "World Transform Mesh (Deprecated)"},
        {{xml::Kind::processor("org.inviwo.WorldTransformVolumeDeprecated")},
         "World Transform Volume",
         "World Transform Volume (Deprecated)"}};

    bool res = false;
    res |= xml::changeAttributeRecursive(
        root, {{xml::Kind::processor("org.inviwo.WorldTransformGeometry")}}, "type",
        "org.inviwo.WorldTransformGeometry", "org.inviwo.WorldTransformMeshDeprecated");
    res |= xml::changeAttributeRecursive(
        root, {{xml::Kind::processor("org.inviwo.WorldTransformVolume")}}, "type",
        "org.inviwo.WorldTransformVolume", "org.inviwo.WorldTransformVolumeDeprecated");
    res |= xml::changeIdentifiers(root, replV3);
    return res;
}

bool updateV7(TxElement* root) {
    bool res = false;

    TraversingVersionConverter conv{[&](TxElement* node) -> bool {
        const auto& key = node->Value();
        if (key != "Processor") return true;
        if (node->GetAttribute("type") != "org.inviwo.VolumeCreator") {
            return true;
        }
        if (auto* elem = xml::getElement(node, "Properties/Property&identifier=dimensions")) {

            elem->SetAttribute("identifier", "dims");
            res = true;
        }
        return true;
    }};
    conv.convert(root);

    return res;
}

bool updateV8(TxElement* root) {
    bool res = false;
    TraversingVersionConverter conv{[&](TxElement* node) -> bool {
        const auto& key = node->Value();
        if (key != "Processor") return true;
        const auto type = node->GetAttribute("type");
        if (type != "org.inviwo.TransformLayer" && type != "org.inviwo.TransformMesh" &&
            type != "org.inviwo.TransformVolume") {
            return true;
        }

        bool replaceTrafo = false;
        if (auto* value = xml::getElement(node, "Properties/Property&identifier=replace/value")) {
            replaceTrafo = (value->GetAttribute("content") == "1");
        }

        if (auto* elem =
                xml::getElement(node, "Properties/Property&identifier=space/selectedIdentifier")) {

            if (elem->GetAttribute("content") == "model") {
                if (replaceTrafo) {
                    elem->SetAttribute("content", "worldTransform");
                } else {
                    elem->SetAttribute("content", "world_TransformModel");
                }
            } else {
                if (replaceTrafo) {
                    elem->SetAttribute("content", "TransformModel");
                } else {
                    elem->SetAttribute("content", "worldTransform_Model");
                }
            }
            res = true;
        }
        return true;
    }};
    conv.convert(root);
    return res;
}

bool updateV11(TxElement* root) {
    bool res = false;
    TraversingVersionConverter conv{[&](TxElement* node) -> bool {
        const auto& key = node->Value();
        if (key != "Processor") return true;
        const auto& type = node->GetAttribute("type");
        if (type != "org.inviwo.ImageTimeStepSelector" &&
            type != "org.inviwo.LayerSequenceElementSelector" &&
            type != "org.inviwo.MeshTimeStepSelector" && type != "org.inviwo.TimeStepSelector") {
            return true;
        }

        if (type == "org.inviwo.ImageTimeStepSelector") {
            node->SetAttribute("type", "org.inviwo.Image.sequence.select");
        } else if (type == "org.inviwo.LayerSequenceElementSelector") {
            node->SetAttribute("type", "org.inviwo.Layer.sequence.select");
        } else if (type == "org.inviwo.MeshTimeStepSelector") {
            node->SetAttribute("type", "org.inviwo.Mesh.sequence.select");
        } else if (type == "org.inviwo.TimeStepSelector") {
            node->SetAttribute("type", "org.inviwo.Volume.sequence.select");
        }

        // replace the timeStep.selectedSequenceIndex property with a
        // index property and decrease the index by 1
        if (auto* props = xml::getElement(node, "Properties")) {
            if (auto* elem = xml::getElement(props,
                                             "Property&identifier=timeStep/Properties/"
                                             "Property&identifier=selectedSequenceIndex/value")) {
                if (auto maybeIndex = elem->Attribute("content")) {
                    if (auto index = util::fromStr<int>(*maybeIndex)) {
                        TxElement indexNode{"Property"};
                        indexNode.SetAttribute("type", "org.inviwo.Size_tProperty");
                        indexNode.SetAttribute("identifier", "index");
                        TxElement value{"value"};
                        value.SetAttribute("content", fmt::to_string(std::max(0, *index - 1)));
                        indexNode.InsertEndChild(value);
                        props->InsertEndChild(indexNode);
                    }
                }
            }
        }
        res = true;
        return true;
    }};

    conv.convert(root);

    return res;
}

}  // namespace

bool BaseModule::Converter::convert(TxElement* root) {
    bool res = false;
    switch (version_) {
        case 0: {
            res |= updateV0(root);
            [[fallthrough]];
        }
        case 1: {
            res |= xml::changeAttributeRecursive(
                root, {{xml::Kind::processor("org.inviwo.GeometeryGenerator")}}, "type",
                "org.inviwo.GeometeryGenerator", "org.inviwo.RandomMeshGenerator");
            [[fallthrough]];
        }
        case 2: {
            res |= updateV2(root);
            [[fallthrough]];
        }
        case 3: {
            res |= updateV3(root);
            [[fallthrough]];
        }
        case 4: {
            res |= xml::changeAttributeRecursive(
                root, {xml::Kind::processor("org.inviwo.VolumeSlice")}, "type",
                "org.inviwo.VolumeSlice", "org.inviwo.VolumeSliceExtractor");

            [[fallthrough]];
        }
        case 5: {
            res |= xml::changeAttributeRecursive(
                root,
                {{xml::Kind::processor("org.inviwo.VolumeSubset"),
                  xml::Kind::property("org.inviwo.Size_tMinMaxProperty")}},
                "type", "org.inviwo.Size_tMinMaxProperty", "org.inviwo.IntMinMaxProperty");
            [[fallthrough]];
        }
        case 6: {
            res |= xml::changeAttributeRecursive(
                root, {{xml::Kind::processor("org.inviwo.LayerDistanceTransformRAM")}}, "type",
                "org.inviwo.LayerDistanceTransformRAM", "org.inviwo.ImageDistanceTransform");
            res |= xml::changeAttributeRecursive(
                root, {{xml::Kind::processor("org.inviwo.NoiseProcessor")}}, "type",
                "org.inviwo.NoiseProcessor", "org.inviwo.NoiseGenerator2D");
            res |= xml::changeAttributeRecursive(
                root, {{xml::Kind::processor("org.inviwo.NoiseVolumeProcessor")}}, "type",
                "org.inviwo.NoiseVolumeProcessor", "org.inviwo.NoiseGenerator3D");
            [[fallthrough]];
        }
        case 7: {
            res |= updateV7(root);
            [[fallthrough]];
        }
        case 8: {
            res |= updateV8(root);
            [[fallthrough]];
        }
        case 9: {
            res |= xml::changeAttributeRecursive(
                root,
                {{xml::Kind::processor("org.inviwo.VolumeSubsample"),
                  xml::Kind::property("org.inviwo.IntMinMaxProperty")}},
                "identifier", "subSampleFactors", "org.inviwo.IntMinMaxProperty");

            res |= xml::changeAttributeRecursive(
                root, {{xml::Kind::processor("org.inviwo.VolumeSubsample")}}, "type",
                "org.inviwo.VolumeSubsample", "org.inviwo.VolumeDownsample");
            [[fallthrough]];
        }
        case 10: {
            res |= xml::changeAttributeRecursive(
                root, {{xml::Kind::processor("org.inviwo.Histogram2D")}}, "type",
                "org.inviwo.Histogram2D", "org.inviwo.VolumeHistogram2D");
            [[fallthrough]];
        }
        case 11: {
            res |= updateV11(root);
            return res;
        }

        default:
            return false;  // No changes
    }
}

}  // namespace inviwo
