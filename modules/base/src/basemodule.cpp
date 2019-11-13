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

#include <modules/base/basemodule.h>
#include <inviwo/core/io/serialization/versionconverter.h>
// Processors
#include <modules/base/processors/buffertomeshprocessor.h>
#include <modules/base/processors/convexhull2dprocessor.h>
#include <modules/base/processors/cubeproxygeometryprocessor.h>
#include <modules/base/processors/diffuselightsourceprocessor.h>
#include <modules/base/processors/directionallightsourceprocessor.h>
#include <modules/base/processors/distancetransformram.h>
#include <modules/base/processors/heightfieldmapper.h>
#include <modules/base/processors/imageinformation.h>
#include <modules/base/processors/inputselector.h>
#include <modules/base/processors/layerdistancetransformram.h>
#include <modules/base/processors/imageexport.h>
#include <modules/base/processors/imagesequenceelementselectorprocessor.h>
#include <modules/base/processors/imagesnapshot.h>
#include <modules/base/processors/imagesource.h>
#include <modules/base/processors/imagesourceseries.h>
#include <modules/base/processors/imagecontourprocessor.h>
#include <modules/base/processors/imagestackvolumesource.h>
#include <modules/base/processors/meshclipping.h>
#include <modules/base/processors/meshcreator.h>
#include <modules/base/processors/meshexport.h>
#include <modules/base/processors/meshinformation.h>
#include <modules/base/processors/meshmapping.h>
#include <modules/base/processors/meshplaneclipping.h>
#include <modules/base/processors/meshsequenceelementselectorprocessor.h>
#include <modules/base/processors/meshsource.h>
#include <modules/base/processors/noiseprocessor.h>
#include <modules/base/processors/noisevolumeprocessor.h>
#include <modules/base/processors/ordinalpropertyanimator.h>
#include <modules/base/processors/orientationindicator.h>
#include <modules/base/processors/pixeltobufferprocessor.h>
#include <modules/base/processors/pixelvalue.h>
#include <modules/base/processors/pointlightsourceprocessor.h>
#include <modules/base/processors/randommeshgenerator.h>
#include <modules/base/processors/randomspheregenerator.h>
#include <modules/base/processors/singlevoxel.h>
#include <modules/base/processors/spotlightsourceprocessor.h>
#include <modules/base/processors/stereocamerasyncer.h>
#include <modules/base/processors/surfaceextractionprocessor.h>
#include <modules/base/processors/transform.h>
#include <modules/base/processors/trianglestowireframe.h>
#include <modules/base/processors/volumeboundaryplanes.h>
#include <modules/base/processors/volumecreator.h>
#include <modules/base/processors/volumesequenceelementselectorprocessor.h>
#include <modules/base/processors/volumesource.h>
#include <modules/base/processors/volumeexport.h>
#include <modules/base/processors/volumebasistransformer.h>
#include <modules/base/processors/volumeslice.h>
#include <modules/base/processors/volumesubsample.h>
#include <modules/base/processors/volumesubset.h>
#include <modules/base/processors/volumesequencesource.h>
#include <modules/base/processors/volumetospatialsampler.h>
#include <modules/base/processors/volumeboundingbox.h>
#include <modules/base/processors/volumecurlcpuprocessor.h>
#include <modules/base/processors/volumedivergencecpuprocessor.h>
#include <modules/base/processors/volumegradientcpuprocessor.h>
#include <modules/base/processors/volumelaplacianprocessor.h>
#include <modules/base/processors/volumesequencetospatial4dsampler.h>
#include <modules/base/processors/worldtransformdeprecated.h>
#include <modules/base/processors/camerafrustum.h>
#include <modules/base/processors/imagetospatialsampler.h>
#include <modules/base/processors/volumesequencesingletimestepsampler.h>

// Properties
#include <modules/base/properties/basisproperty.h>
#include <modules/base/properties/gaussianproperty.h>
#include <modules/base/properties/imageinformationproperty.h>
#include <modules/base/properties/layerinformationproperty.h>
#include <modules/base/properties/meshinformationproperty.h>
#include <modules/base/properties/bufferinformationproperty.h>
#include <modules/base/properties/volumeinformationproperty.h>
#include <modules/base/properties/sequencetimerproperty.h>

// Io
#include <modules/base/io/binarystlwriter.h>
#include <modules/base/io/datvolumesequencereader.h>
#include <modules/base/io/datvolumewriter.h>
#include <modules/base/io/ivfvolumereader.h>
#include <modules/base/io/ivfvolumewriter.h>
#include <modules/base/io/ivfsequencevolumereader.h>
#include <modules/base/io/stlwriter.h>
#include <modules/base/io/wavefrontwriter.h>
#include <modules/base/processors/meshconverterprocessor.h>
#include <modules/base/processors/volumeinformation.h>
#include <modules/base/processors/tfselector.h>

#include <fmt/format.h>
#include <tuple>

namespace inviwo {

using BasisTransformMesh = BasisTransform<Mesh>;
using BasisTransformVolume = BasisTransform<Volume>;

using TransformMesh = Transform<Mesh>;
using TransformVolume = Transform<Volume>;

using WorldTransformMeshDeprecated = WorldTransformDeprecated<Mesh>;
using WorldTransformVolumeDeprecated = WorldTransformDeprecated<Volume>;

BaseModule::BaseModule(InviwoApplication* app) : InviwoModule(app, "Base") {
    registerProcessor<ConvexHull2DProcessor>();
    registerProcessor<CubeProxyGeometry>();
    registerProcessor<DiffuseLightSourceProcessor>();
    registerProcessor<DirectionalLightSourceProcessor>();
    registerProcessor<DistanceTransformRAM>();
    registerProcessor<MeshSource>();
    registerProcessor<HeightFieldMapper>();
    registerProcessor<ImageExport>();
    registerProcessor<ImageInformation>();
    registerProcessor<ImageSnapshot>();
    registerProcessor<ImageSource>();
    registerProcessor<ImageSourceSeries>();
    registerProcessor<ImageStackVolumeSource>();
    registerProcessor<LayerDistanceTransformRAM>();
    registerProcessor<MeshClipping>();
    registerProcessor<MeshCreator>();
    registerProcessor<MeshInformation>();
    registerProcessor<MeshMapping>();
    registerProcessor<MeshPlaneClipping>();
    registerProcessor<NoiseProcessor>();
    registerProcessor<PixelToBufferProcessor>();
    registerProcessor<PointLightSourceProcessor>();
    registerProcessor<OrdinalPropertyAnimator>();
    registerProcessor<SpotLightSourceProcessor>();
    registerProcessor<SurfaceExtraction>();
    registerProcessor<VolumeBoundaryPlanes>();
    registerProcessor<VolumeSource>();
    registerProcessor<VolumeExport>();
    registerProcessor<BasisTransformMesh>();
    registerProcessor<BasisTransformVolume>();
    registerProcessor<TrianglesToWireframe>();
    registerProcessor<TransformMesh>();
    registerProcessor<TransformVolume>();
    registerProcessor<WorldTransformMeshDeprecated>();
    registerProcessor<WorldTransformVolumeDeprecated>();
    registerProcessor<VolumeSlice>();
    registerProcessor<VolumeSubsample>();
    registerProcessor<VolumeSubset>();
    registerProcessor<ImageContourProcessor>();
    registerProcessor<VolumeSequenceSource>();
    registerProcessor<VolumeSequenceElementSelectorProcessor>();
    registerProcessor<ImageSequenceElementSelectorProcessor>();
    registerProcessor<MeshSequenceElementSelectorProcessor>();

    registerProcessor<VolumeBoundingBox>();
    registerProcessor<SingleVoxel>();
    registerProcessor<StereoCameraSyncer>();

    registerProcessor<VolumeToSpatialSampler>();
    registerProcessor<OrientationIndicator>();
    registerProcessor<PixelValue>();
    registerProcessor<VolumeSequenceToSpatial4DSampler>();
    registerProcessor<VolumeGradientCPUProcessor>();
    registerProcessor<VolumeCurlCPUProcessor>();
    registerProcessor<VolumeDivergenceCPUProcessor>();
    registerProcessor<VolumeLaplacianProcessor>();
    registerProcessor<MeshExport>();
    registerProcessor<RandomMeshGenerator>();
    registerProcessor<RandomSphereGenerator>();
    registerProcessor<NoiseVolumeProcessor>();
    registerProcessor<BufferToMeshProcessor>();
    registerProcessor<ImageToSpatialSampler>();
    registerProcessor<CameraFrustum>();
    registerProcessor<VolumeSequenceSingleTimestepSamplerProcessor>();
    registerProcessor<VolumeCreator>();
    registerProcessor<MeshConverterProcessor>();
    registerProcessor<VolumeInformation>();
    registerProcessor<TFSelector>();

    // input selectors
    registerProcessor<InputSelector<MultiDataInport<Volume>, VolumeOutport>>();
    registerProcessor<InputSelector<MultiDataInport<Mesh>, MeshOutport>>();
    registerProcessor<InputSelector<ImageMultiInport, ImageOutport>>();

    registerProperty<SequenceTimerProperty>();
    registerProperty<BasisProperty>();
    registerProperty<ImageInformationProperty>();
    registerProperty<LayerInformationProperty>();
    registerProperty<MeshInformationProperty>();
    registerProperty<BufferInformationProperty>();
    registerProperty<MeshBufferInformationProperty>();
    registerProperty<IndexBufferInformationProperty>();
    registerProperty<VolumeInformationProperty>();

    registerProperty<Gaussian1DProperty>();
    registerProperty<Gaussian2DProperty>();

    registerProperty<transform::TranslateProperty>();
    registerProperty<transform::RotateProperty>();
    registerProperty<transform::ScaleProperty>();
    registerProperty<transform::CustomTransformProperty>();

    // Register Data readers
    registerDataReader(std::make_unique<DatVolumeSequenceReader>());
    registerDataReader(std::make_unique<IvfVolumeReader>());
    registerDataReader(std::make_unique<IvfSequenceVolumeReader>());
    // Register Data writers
    registerDataWriter(std::make_unique<DatVolumeWriter>());
    registerDataWriter(std::make_unique<IvfVolumeWriter>());
    registerDataWriter(std::make_unique<StlWriter>());
    registerDataWriter(std::make_unique<BinarySTLWriter>());
    registerDataWriter(std::make_unique<WaveFrontWriter>());

    util::for_each_type<OrdinalPropertyAnimator::Types>{}(RegHelper{}, *this);
}

int BaseModule::getVersion() const { return 4; }

std::unique_ptr<VersionConverter> BaseModule::getConverter(int version) const {
    return std::make_unique<Converter>(version);
}

BaseModule::Converter::Converter(int version) : version_(version) {}

bool BaseModule::Converter::convert(TxElement* root) {
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

    std::vector<xml::IdentifierReplacement> replV3 = {
        // WorldTransform (Deprecated)
        {{xml::Kind::processor("org.inviwo.WorldTransformMeshDeprecated")},
         "World Transform Mesh",
         "World Transform Mesh (Deprecated)"},
        {{xml::Kind::processor("org.inviwo.WorldTransformVolumeDeprecated")},
         "World Transform Volume",
         "World Transform Volume (Deprecated)"}};

    bool res = false;
    switch (version_) {
        case 0: {
            res |= xml::changeIdentifiers(root, repl);
            [[fallthrough]];
        }
        case 1: {
            res |= xml::changeAttribute(
                root, {{xml::Kind::processor("org.inviwo.GeometeryGenerator")}}, "type",
                "org.inviwo.GeometeryGenerator", "org.inviwo.RandomMeshGenerator");
            [[fallthrough]];
        }
        case 2: {
            TraversingVersionConverter conv{[&](TxElement* node) -> bool {
                std::string key;
                node->GetValue(&key);
                if (key != "Property") return true;
                const auto type = node->GetAttributeOrDefault("type", "");
                if (type != "org.inviwo.VolumeBasisProperty") return true;

                TxElement newNode{"overrideModel"};
                for (const auto& item :
                     {std::make_tuple(0, "A", 0.0), std::make_tuple(1, "B", 0.0),
                      std::make_tuple(2, "C", 0.0), std::make_tuple(3, "Offset", 1.0)}) {
                    const auto path = fmt::format("Properties/Property&identifier=override{}/value",
                                                  std::get<1>(item));
                    if (auto elem = xml::getElement(node, path)) {
                        auto data = elem->Clone();
                        data->SetValue(fmt::format("col{}", std::get<0>(item)));
                        data->ToElement()->SetAttribute("w",
                                                        fmt::format("{:f}", std::get<2>(item)));
                        newNode.InsertEndChild(*data);
                    }
                }

                res = true;
                node->InsertEndChild(newNode);

                return true;
            }};

            conv.convert(root);
            [[fallthrough]];
        }
        case 3: {
            res |= xml::changeAttribute(
                root, {{xml::Kind::processor("org.inviwo.WorldTransformGeometry")}}, "type",
                "org.inviwo.WorldTransformGeometry", "org.inviwo.WorldTransformMeshDeprecated");
            res |= xml::changeAttribute(
                root, {{xml::Kind::processor("org.inviwo.WorldTransformVolume")}}, "type",
                "org.inviwo.WorldTransformVolume", "org.inviwo.WorldTransformVolumeDeprecated");
            res |= xml::changeIdentifiers(root, replV3);
            return res;
        }

        default:
            return false;  // No changes
    }
}

}  // namespace inviwo
