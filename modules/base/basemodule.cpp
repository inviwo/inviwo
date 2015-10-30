/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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
#include <modules/base/processors/cubeproxygeometry.h>
#include <modules/base/processors/diffuselightsourceprocessor.h>
#include <modules/base/processors/directionallightsourceprocessor.h>
#include <modules/base/processors/distancetransformram.h>
#include <modules/base/processors/meshsource.h>
#include <modules/base/processors/heightfieldmapper.h>
#include <modules/base/processors/imageexport.h>
#include <modules/base/processors/imagesource.h>
#include <modules/base/processors/imagesourceseries.h>
#include <modules/base/processors/meshclipping.h>
#include <modules/base/processors/meshcreator.h>
#include <modules/base/processors/noiseprocessor.h>
#include <modules/base/processors/pixeltobufferprocessor.h>
#include <modules/base/processors/pointlightsourceprocessor.h>
#include <modules/base/processors/ordinalpropertyanimator.h>
#include <modules/base/processors/spotlightsourceprocessor.h>
#include <modules/base/processors/surfaceextraction.h>
#include <modules/base/processors/timestepselector.h>
#include <modules/base/processors/volumesource.h>
#include <modules/base/processors/volumeexport.h>
#include <modules/base/processors/volumebasistransformer.h>
#include <modules/base/processors/volumeslice.h>
#include <modules/base/processors/volumesubsample.h>
#include <modules/base/processors/volumesubset.h>
#include <modules/base/processors/volumevectorsource.h>

#include <modules/base/properties/basisproperty.h>
#include <modules/base/properties/gaussianproperty.h>
#include <modules/base/properties/volumeinformationproperty.h>
#include <modules/base/properties/sequencetimerproperty.h>

#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/ports/volumeport.h>

namespace inviwo {

using BasisTransformMesh = BasisTransform<Mesh>;
using BasisTransformVolume = BasisTransform<Volume>;

BaseModule::BaseModule(InviwoApplication* app) : InviwoModule(app, "Base") {
    registerProcessor<CubeProxyGeometry>();
    registerProcessor<DiffuseLightSourceProcessor>();
    registerProcessor<DirectionalLightSourceProcessor>();
    registerProcessor<DistanceTransformRAM>();
    registerProcessor<MeshSource>();
    registerProcessor<HeightFieldMapper>();
    registerProcessor<ImageExport>();
    registerProcessor<ImageSource>();
    registerProcessor<ImageSourceSeries>();
    registerProcessor<MeshClipping>();
    registerProcessor<MeshCreator>();
    registerProcessor<NoiseProcessor>();
    registerProcessor<PixelToBufferProcessor>();
    registerProcessor<PointLightSourceProcessor>();
    registerProcessor<OrdinalPropertyAnimator>();
    registerProcessor<SpotLightSourceProcessor>();
    registerProcessor<SurfaceExtraction>();
    registerProcessor<VolumeSource>();
    registerProcessor<VolumeExport>();
    registerProcessor<BasisTransformMesh>();
    registerProcessor<BasisTransformVolume>();
    registerProcessor<VolumeSlice>();
    registerProcessor<VolumeSubsample>();
    registerProcessor<VolumeSubset>();
    registerProcessor<VolumeVectorSource>();
    registerProcessor<TimeStepSelector>();

    registerProperty<SequenceTimerProperty>();
    registerProperty<BasisProperty>();
    registerProperty<VolumeInformationProperty>();

    registerProperty<Gaussian1DProperty>();
    registerProperty<Gaussian2DProperty>();

    registerPort<DataInport<LightSource>>("LightSourceInport");
    registerPort<DataOutport<LightSource>>("LightSourceOutport");
    registerPort<BufferInport>("BufferInport");
    registerPort<BufferOutport>("BufferOutport");
}

} // namespace
