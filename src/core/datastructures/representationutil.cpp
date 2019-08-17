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

#include <inviwo/core/datastructures/representationutil.h>

#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>
#include <inviwo/core/datastructures/volume/volumeramconverter.h>
#include <inviwo/core/datastructures/image/layerramprecision.h>
#include <inviwo/core/datastructures/image/layerramconverter.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>

#include <inviwo/core/datastructures/representationfactory.h>
#include <inviwo/core/datastructures/representationfactoryobject.h>
#include <inviwo/core/datastructures/representationconverterfactory.h>

#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/datastructures/representationfactorymanager.h>

namespace inviwo {

namespace util {

namespace {

class BufferRAMFactoryObject
    : public RepresentationFactoryObjectTemplate<BufferRepresentation, BufferRAM> {
public:
    virtual std::unique_ptr<BufferRepresentation> create(
        const typename BufferRepresentation::ReprOwner* buffer) {
        return dispatching::dispatch<std::unique_ptr<BufferRepresentation>,
                                     dispatching::filter::All>(buffer->getDataFormat()->getId(),
                                                               Dispatcher{}, buffer);
    }

private:
    struct Dispatcher {
        template <typename Result, typename Format>
        Result operator()(const BufferBase* buffer) {
            switch (buffer->getBufferTarget()) {
                case BufferTarget::Index:
                    return std::make_unique<
                        BufferRAMPrecision<typename Format::type, BufferTarget::Index>>(
                        buffer->getSize(), buffer->getBufferUsage());

                case BufferTarget::Data:
                default:
                    return std::make_unique<
                        BufferRAMPrecision<typename Format::type, BufferTarget::Data>>(
                        buffer->getSize(), buffer->getBufferUsage());
            }
        }
    };
};

class LayerRAMFactoryObject
    : public RepresentationFactoryObjectTemplate<LayerRepresentation, LayerRAM> {
public:
    virtual std::unique_ptr<LayerRepresentation> create(
        const typename LayerRepresentation::ReprOwner* layer) {
        return dispatching::dispatch<std::unique_ptr<LayerRepresentation>,
                                     dispatching::filter::All>(layer->getDataFormat()->getId(),
                                                               Dispatcher{}, layer);
    }

private:
    struct Dispatcher {
        template <typename Result, typename Format>
        Result operator()(const Layer* layer) {
            return std::make_unique<LayerRAMPrecision<typename Format::type>>(
                layer->getDimensions(), layer->getLayerType(), layer->getSwizzleMask());
        }
    };
};

class VolumeRAMFactoryObject
    : public RepresentationFactoryObjectTemplate<VolumeRepresentation, VolumeRAM> {
public:
    virtual std::unique_ptr<VolumeRepresentation> create(
        const typename VolumeRepresentation::ReprOwner* volume) {
        return dispatching::dispatch<std::unique_ptr<VolumeRepresentation>,
                                     dispatching::filter::All>(volume->getDataFormat()->getId(),
                                                               Dispatcher{}, volume);
    }

private:
    struct Dispatcher {
        template <typename Result, typename Format>
        Result operator()(const Volume* volume) {
            return std::make_unique<VolumeRAMPrecision<typename Format::type>>(
                volume->getDimensions(), volume->getSwizzleMask());
        }
    };
};

template <typename T>
void registerCoreRepresentationsHelper(T& obj) {

    // Register representation Factories
    obj.registerRepresentationFactory(
        std::make_unique<RepresentationFactory<BufferRepresentation>>(typeid(BufferRAM)));
    obj.registerRepresentationFactory(
        std::make_unique<RepresentationFactory<LayerRepresentation>>(typeid(LayerRAM)));
    obj.registerRepresentationFactory(
        std::make_unique<RepresentationFactory<VolumeRepresentation>>(typeid(VolumeRAM)));

    // Register RAM Representations
    obj.template registerRepresentationFactoryObject<BufferRepresentation>(
        std::make_unique<BufferRAMFactoryObject>());
    obj.template registerRepresentationFactoryObject<LayerRepresentation>(
        std::make_unique<LayerRAMFactoryObject>());
    obj.template registerRepresentationFactoryObject<VolumeRepresentation>(
        std::make_unique<VolumeRAMFactoryObject>());

    // Register Converter Factories
    obj.registerRepresentationConverterFactory(
        std::make_unique<RepresentationConverterFactory<VolumeRepresentation>>());
    obj.registerRepresentationConverterFactory(
        std::make_unique<RepresentationConverterFactory<LayerRepresentation>>());
    obj.registerRepresentationConverterFactory(
        std::make_unique<RepresentationConverterFactory<BufferRepresentation>>());

    // Register Converters
    obj.template registerRepresentationConverter<VolumeRepresentation>(
        std::make_unique<VolumeDisk2RAMConverter>());
    obj.template registerRepresentationConverter<LayerRepresentation>(
        std::make_unique<LayerDisk2RAMConverter>());
}

}  // namespace

void registerCoreRepresentations(InviwoModule& obj) { registerCoreRepresentationsHelper(obj); }

void registerCoreRepresentations(RepresentationFactoryManager& obj) {
    registerCoreRepresentationsHelper(obj);
}

}  // namespace util

}  // namespace inviwo
