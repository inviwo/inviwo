/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022-2025 Inviwo Foundation
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

#include <inviwo/core/common/factoryutil.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/property.h>
#include <inviwo/core/properties/propertyowner.h>

namespace inviwo::util {

CameraFactory* getCameraFactory() { return getCameraFactory(InviwoApplication::getPtr()); }
CameraFactory* getCameraFactory(InviwoApplication* app) {
    if (!app) return nullptr;
    return app->getCameraFactory();
}
CameraFactory* getCameraFactory(Processor* processor) {
    if (!processor) return nullptr;
    return getCameraFactory(processor->getInviwoApplication());
}
CameraFactory* getCameraFactory(Property* property) {
    if (!property) return nullptr;
    return getCameraFactory(property->getOwner()->getProcessor());
}

DataReaderFactory* getDataReaderFactory() {
    return getDataReaderFactory(InviwoApplication::getPtr());
}
DataReaderFactory* getDataReaderFactory(InviwoApplication* app) {
    if (!app) return nullptr;
    return app->getDataReaderFactory();
}
DataReaderFactory* getDataReaderFactory(Processor* processor) {
    if (!processor) return nullptr;
    return getDataReaderFactory(processor->getInviwoApplication());
}
DataReaderFactory* getDataReaderFactory(Property* property) {
    if (!property) return nullptr;
    return getDataReaderFactory(property->getOwner()->getProcessor());
}

DataWriterFactory* getDataWriterFactory() {
    return getDataWriterFactory(InviwoApplication::getPtr());
}
DataWriterFactory* getDataWriterFactory(InviwoApplication* app) {
    if (!app) return nullptr;
    return app->getDataWriterFactory();
}
DataWriterFactory* getDataWriterFactory(Processor* processor) {
    if (!processor) return nullptr;
    return getDataWriterFactory(processor->getInviwoApplication());
}
DataWriterFactory* getDataWriterFactory(Property* property) {
    if (!property) return nullptr;
    return getDataWriterFactory(property->getOwner()->getProcessor());
}

DialogFactory* getDialogFactory() { return getDialogFactory(InviwoApplication::getPtr()); }
DialogFactory* getDialogFactory(InviwoApplication* app) {
    if (!app) return nullptr;
    return app->getDialogFactory();
}
DialogFactory* getDialogFactory(Processor* processor) {
    if (!processor) return nullptr;
    return getDialogFactory(processor->getInviwoApplication());
}
DialogFactory* getDialogFactory(Property* property) {
    if (!property) return nullptr;
    return getDialogFactory(property->getOwner()->getProcessor());
}

MeshDrawerFactory* getMeshDrawerFactory() {
    return getMeshDrawerFactory(InviwoApplication::getPtr());
}
MeshDrawerFactory* getMeshDrawerFactory(InviwoApplication* app) {
    if (!app) return nullptr;
    return app->getMeshDrawerFactory();
}
MeshDrawerFactory* getMeshDrawerFactory(Processor* processor) {
    if (!processor) return nullptr;
    return getMeshDrawerFactory(processor->getInviwoApplication());
}
MeshDrawerFactory* getMeshDrawerFactory(Property* property) {
    if (!property) return nullptr;
    return getMeshDrawerFactory(property->getOwner()->getProcessor());
}

MetaDataFactory* getMetaDataFactory() { return getMetaDataFactory(InviwoApplication::getPtr()); }
MetaDataFactory* getMetaDataFactory(InviwoApplication* app) {
    if (!app) return nullptr;
    return app->getMetaDataFactory();
}
MetaDataFactory* getMetaDataFactory(Processor* processor) {
    if (!processor) return nullptr;
    return getMetaDataFactory(processor->getInviwoApplication());
}
MetaDataFactory* getMetaDataFactory(Property* property) {
    if (!property) return nullptr;
    return getMetaDataFactory(property->getOwner()->getProcessor());
}

InportFactory* getInportFactory() { return getInportFactory(InviwoApplication::getPtr()); }
InportFactory* getInportFactory(InviwoApplication* app) {
    if (!app) return nullptr;
    return app->getInportFactory();
}
InportFactory* getInportFactory(Processor* processor) {
    if (!processor) return nullptr;
    return getInportFactory(processor->getInviwoApplication());
}
InportFactory* getInportFactory(Property* property) {
    if (!property) return nullptr;
    return getInportFactory(property->getOwner()->getProcessor());
}

OutportFactory* getOutportFactory() { return getOutportFactory(InviwoApplication::getPtr()); }
OutportFactory* getOutportFactory(InviwoApplication* app) {
    if (!app) return nullptr;
    return app->getOutportFactory();
}
OutportFactory* getOutportFactory(Processor* processor) {
    if (!processor) return nullptr;
    return getOutportFactory(processor->getInviwoApplication());
}
OutportFactory* getOutportFactory(Property* property) {
    if (!property) return nullptr;
    return getOutportFactory(property->getOwner()->getProcessor());
}

PortInspectorFactory* getPortInspectorFactory() {
    return getPortInspectorFactory(InviwoApplication::getPtr());
}
PortInspectorFactory* getPortInspectorFactory(InviwoApplication* app) {
    if (!app) return nullptr;
    return app->getPortInspectorFactory();
}
PortInspectorFactory* getPortInspectorFactory(Processor* processor) {
    if (!processor) return nullptr;
    return getPortInspectorFactory(processor->getInviwoApplication());
}
PortInspectorFactory* getPortInspectorFactory(Property* property) {
    if (!property) return nullptr;
    return getPortInspectorFactory(property->getOwner()->getProcessor());
}

ProcessorFactory* getProcessorFactory() { return getProcessorFactory(InviwoApplication::getPtr()); }
ProcessorFactory* getProcessorFactory(InviwoApplication* app) {
    if (!app) return nullptr;
    return app->getProcessorFactory();
}
ProcessorFactory* getProcessorFactory(Processor* processor) {
    if (!processor) return nullptr;
    return getProcessorFactory(processor->getInviwoApplication());
}
ProcessorFactory* getProcessorFactory(Property* property) {
    if (!property) return nullptr;
    return getProcessorFactory(property->getOwner()->getProcessor());
}

ProcessorWidgetFactory* getProcessorWidgetFactory() {
    return getProcessorWidgetFactory(InviwoApplication::getPtr());
}
ProcessorWidgetFactory* getProcessorWidgetFactory(InviwoApplication* app) {
    if (!app) return nullptr;
    return app->getProcessorWidgetFactory();
}
ProcessorWidgetFactory* getProcessorWidgetFactory(Processor* processor) {
    if (!processor) return nullptr;
    return getProcessorWidgetFactory(processor->getInviwoApplication());
}
ProcessorWidgetFactory* getProcessorWidgetFactory(Property* property) {
    if (!property) return nullptr;
    return getProcessorWidgetFactory(property->getOwner()->getProcessor());
}

PropertyConverterManager* getPropertyConverterManager() {
    return getPropertyConverterManager(InviwoApplication::getPtr());
}
PropertyConverterManager* getPropertyConverterManager(InviwoApplication* app) {
    if (!app) return nullptr;
    return app->getPropertyConverterManager();
}
PropertyConverterManager* getPropertyConverterManager(Processor* processor) {
    if (!processor) return nullptr;
    return getPropertyConverterManager(processor->getInviwoApplication());
}
PropertyConverterManager* getPropertyConverterManager(Property* property) {
    if (!property) return nullptr;
    return getPropertyConverterManager(property->getOwner()->getProcessor());
}

PropertyFactory* getPropertyFactory() { return getPropertyFactory(InviwoApplication::getPtr()); }
PropertyFactory* getPropertyFactory(InviwoApplication* app) {
    if (!app) return nullptr;
    return app->getPropertyFactory();
}
PropertyFactory* getPropertyFactory(Processor* processor) {
    if (!processor) return nullptr;
    return getPropertyFactory(processor->getInviwoApplication());
}
PropertyFactory* getPropertyFactory(Property* property) {
    if (!property) return nullptr;
    return getPropertyFactory(property->getOwner()->getProcessor());
}

PropertyWidgetFactory* getPropertyWidgetFactory() {
    return getPropertyWidgetFactory(InviwoApplication::getPtr());
}
PropertyWidgetFactory* getPropertyWidgetFactory(InviwoApplication* app) {
    if (!app) return nullptr;
    return app->getPropertyWidgetFactory();
}
PropertyWidgetFactory* getPropertyWidgetFactory(Processor* processor) {
    if (!processor) return nullptr;
    return getPropertyWidgetFactory(processor->getInviwoApplication());
}
PropertyWidgetFactory* getPropertyWidgetFactory(Property* property) {
    if (!property) return nullptr;
    return getPropertyWidgetFactory(property->getOwner()->getProcessor());
}

RepresentationMetaFactory* getRepresentationMetaFactory() {
    return getRepresentationMetaFactory(InviwoApplication::getPtr());
}
RepresentationMetaFactory* getRepresentationMetaFactory(InviwoApplication* app) {
    if (!app) return nullptr;
    return app->getRepresentationMetaFactory();
}
RepresentationMetaFactory* getRepresentationMetaFactory(Processor* processor) {
    if (!processor) return nullptr;
    return getRepresentationMetaFactory(processor->getInviwoApplication());
}
RepresentationMetaFactory* getRepresentationMetaFactory(Property* property) {
    if (!property) return nullptr;
    return getRepresentationMetaFactory(property->getOwner()->getProcessor());
}

RepresentationConverterMetaFactory* getRepresentationConverterMetaFactory() {
    return getRepresentationConverterMetaFactory(InviwoApplication::getPtr());
}
RepresentationConverterMetaFactory* getRepresentationConverterMetaFactory(InviwoApplication* app) {
    if (!app) return nullptr;
    return app->getRepresentationConverterMetaFactory();
}
RepresentationConverterMetaFactory* getRepresentationConverterMetaFactory(Processor* processor) {
    if (!processor) return nullptr;
    return getRepresentationConverterMetaFactory(processor->getInviwoApplication());
}
RepresentationConverterMetaFactory* getRepresentationConverterMetaFactory(Property* property) {
    if (!property) return nullptr;
    return getRepresentationConverterMetaFactory(property->getOwner()->getProcessor());
}

ResourceManager* getResourceManager() {
    if (!InviwoApplication::isInitialized()) return nullptr;
    return getResourceManager(InviwoApplication::getPtr());
}
ResourceManager* getResourceManager(InviwoApplication* app) {
    if (!app) return nullptr;
    return app->getResourceManager();
}
ResourceManager* getResourceManager(Processor* processor) {
    if (!processor) return nullptr;
    return getResourceManager(processor->getInviwoApplication());
}
ResourceManager* getResourceManager(Property* property) {
    if (!property) return nullptr;
    return getResourceManager(property->getOwner()->getProcessor());
}

}  // namespace inviwo::util
