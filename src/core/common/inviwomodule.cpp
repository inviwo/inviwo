/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
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

#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/ports/portfactory.h>
#include <inviwo/core/ports/portinspectorfactory.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/io/datawriterfactory.h>
#include <inviwo/core/metadata/metadatafactory.h>
#include <inviwo/core/processors/processorfactory.h>
#include <inviwo/core/processors/processorwidgetfactory.h>
#include <inviwo/core/properties/propertyfactory.h>
#include <inviwo/core/properties/propertywidgetfactory.h>
#include <inviwo/core/datastructures/representationconverterfactory.h>
#include <inviwo/core/rendering/geometryrendererfactory.h>
#include <inviwo/core/util/dialogfactory.h>

#include <pathsexternalmodules.h>

#include <algorithm>

namespace inviwo {

InviwoModule::InviwoModule() : identifier_("undefined"), initialized_(false) {}

InviwoModule::~InviwoModule() {
    if (isInitialized())
        LogWarn("Module '" + getIdentifier() +
                "' should have been deinitialized before destruction.");

    for (size_t i = 0; i < capabilities_.size(); i++) delete capabilities_[i];

    capabilities_.clear();

    for (size_t i = 0; i < data_.size(); i++) delete data_[i];

    data_.clear();

    for (size_t i = 0; i < dataReaders_.size(); i++) delete dataReaders_[i];

    dataReaders_.clear();

    for (size_t i = 0; i < dataRepresentations_.size(); i++) delete dataRepresentations_[i];

    dataRepresentations_.clear();

    for (size_t i = 0; i < dataWriters_.size(); i++) delete dataWriters_[i];

    dataWriters_.clear();

    for (size_t i = 0; i < dialogs_.size(); i++) delete dialogs_[i];

    dialogs_.clear();

    for (size_t i = 0; i < metadata_.size(); i++) delete metadata_[i];

    metadata_.clear();

    for (size_t i = 0; i < moduleSettings_.size(); i++) delete moduleSettings_[i];

    moduleSettings_.clear();

    for (size_t i = 0; i < ports_.size(); i++) delete ports_[i];

    ports_.clear();

    for (size_t i = 0; i < portInspectors_.size(); i++) delete portInspectors_[i];

    portInspectors_.clear();

    for (size_t i = 0; i < processors_.size(); i++) delete processors_[i];

    processors_.clear();

    for (size_t i = 0; i < processorWidgets_.size(); i++) delete processorWidgets_[i].second;

    processorWidgets_.clear();

    for (size_t i = 0; i < properties_.size(); i++) delete properties_[i];

    properties_.clear();

    for (size_t i = 0; i < propertyWidgets_.size(); i++) delete propertyWidgets_[i];

    propertyWidgets_.clear();

    for (size_t i = 0; i < representationConverters_.size(); i++)
        delete representationConverters_[i];

    representationConverters_.clear();

    for (size_t i = 0; i < resources_.size(); i++) delete resources_[i];

    resources_.clear();

    for (size_t i = 0; i < renderers_.size(); i++) delete renderers_[i];

    renderers_.clear();
}

std::string InviwoModule::getIdentifier() const { return identifier_; }

std::string InviwoModule::getPath() const {
    std::string moduleNameLowerCase = getIdentifier();
    std::transform(moduleNameLowerCase.begin(), moduleNameLowerCase.end(), moduleNameLowerCase.begin(), ::tolower);
    if (filesystem::directoryExists(InviwoApplication::getPtr()->getPath(InviwoApplication::PATH_MODULES) + moduleNameLowerCase)) {
        return InviwoApplication::getPtr()->getPath(InviwoApplication::PATH_MODULES) + moduleNameLowerCase;
    }
#ifdef IVW_EXTERNAL_MODULES_PATH_COUNT
    for(int i=0; i < IVW_EXTERNAL_MODULES_PATH_COUNT; ++i) {
        std::string directory = externalModulePaths_[i] + "/" + moduleNameLowerCase;
        if(filesystem::directoryExists(directory)) {
            return directory;
        }
    }
#endif
    LogWarn(moduleNameLowerCase << " directory was not found");
    return InviwoApplication::getPtr()->getPath(InviwoApplication::PATH_MODULES) + moduleNameLowerCase;
}

void InviwoModule::initialize() {
    for (size_t i = 0; i < capabilities_.size(); i++) {
        capabilities_[i]->initialize();
        capabilities_[i]->printInfo();
    }

    setupModuleSettings();
    initialized_ = true;
}

bool InviwoModule::isInitialized() const { return initialized_; }

void InviwoModule::deinitialize() { initialized_ = false; }

void InviwoModule::setIdentifier(const std::string& identifier) { identifier_ = identifier; }

void InviwoModule::setupModuleSettings() {
    for (size_t i = 0; i < moduleSettings_.size(); i++) moduleSettings_[i]->initialize();
}

const std::vector<Capabilities*>& InviwoModule::getCapabilities() const { return capabilities_; }
const std::vector<Data*>& InviwoModule::getData() const { return data_; }
const std::vector<DataReader*>& InviwoModule::getDataReaders() const { return dataReaders_; }
const std::vector<DataRepresentation*>& InviwoModule::getDataRepresentations() const {
    return dataRepresentations_;
}
const std::vector<DataWriter*>& InviwoModule::getDataWriters() const { return dataWriters_; }
const std::vector<DialogFactoryObject*>& InviwoModule::getDialogs() const { return dialogs_; }
const std::vector<MetaData*>& InviwoModule::getMetaData() const { return metadata_; }
const std::vector<PortFactoryObject*>& InviwoModule::getPorts() const { return ports_; }
const std::vector<PortInspectorFactoryObject*>& InviwoModule::getPortInspectors() const {
    return portInspectors_;
}
const std::vector<ProcessorFactoryObject*>& InviwoModule::getProcessors() const {
    return processors_;
}
const std::vector<std::pair<std::string, ProcessorWidget*> >& InviwoModule::getProcessorWidgets()
    const {
    return processorWidgets_;
}
const std::vector<PropertyFactoryObject*>& InviwoModule::getProperties() const {
    return properties_;
}
const std::vector<PropertyWidgetFactoryObject*>& InviwoModule::getPropertyWidgets() const {
    return propertyWidgets_;
}
const std::vector<RepresentationConverter*>& InviwoModule::getRepresentationConverters() const {
    return representationConverters_;
}
const std::vector<GeometryRenderer*>& InviwoModule::getRenderers() const { return renderers_; }
const std::vector<Resource*>& InviwoModule::getResources() const { return resources_; }
const std::vector<Settings*>& InviwoModule::getSettings() const { return moduleSettings_; }

void InviwoModule::registerCapabilities(Capabilities* info) { capabilities_.push_back(info); }
void InviwoModule::registerData(Data* data) { data_.push_back(data); }
void InviwoModule::registerDataRepresentation(DataRepresentation* dataRepresentation) {
    dataRepresentations_.push_back(dataRepresentation);
}
void InviwoModule::registerDataReader(DataReader* dataReader) {
    dataReaders_.push_back(dataReader);
    DataReaderFactory::getPtr()->registerObject(dataReader);
}
void InviwoModule::registerDataWriter(DataWriter* dataWriter) {
    dataWriters_.push_back(dataWriter);
    DataWriterFactory::getPtr()->registerObject(dataWriter);
}
void InviwoModule::registerDialogObject(DialogFactoryObject* dialog) {
    dialogs_.push_back(dialog);
    DialogFactory::getPtr()->registerObject(dialog);
}
void InviwoModule::registerMetaData(MetaData* meta) {
    metadata_.push_back(meta);
    MetaDataFactory::getPtr()->registerObject(meta);
}
void InviwoModule::registerPortObject(PortFactoryObject* port) {
    ports_.push_back(port);
    PortFactory::getPtr()->registeryObject(port);
}
void InviwoModule::registerPortInspectorObject(PortInspectorFactoryObject* portInspector) {
    portInspectors_.push_back(portInspector);
    PortInspectorFactory::getPtr()->registerObject(portInspector);
}
void InviwoModule::registerProcessorObject(ProcessorFactoryObject* processor) {
    processors_.push_back(processor);
    ProcessorFactory::getPtr()->registerObject(processor);
}
void InviwoModule::registerProcessorWidget(std::string processorClassName,
                                           ProcessorWidget* processorWidget) {
    processorWidgets_.push_back(std::make_pair(processorClassName, processorWidget));
    ProcessorWidgetFactory::getPtr()->registerObject(
        std::make_pair(processorClassName, processorWidget));
}
void InviwoModule::registerPropertyObject(PropertyFactoryObject* property) {
    properties_.push_back(property);
    PropertyFactory::getPtr()->registeryObject(property);
}
void InviwoModule::registerPropertyWidgetObject(PropertyWidgetFactoryObject* propertyWidget) {
    propertyWidgets_.push_back(propertyWidget);
    PropertyWidgetFactory::getPtr()->registerObject(propertyWidget);
}
void InviwoModule::registerRenderer(GeometryRenderer* renderer) {
    renderers_.push_back(renderer);
    GeometryRendererFactory::getPtr()->registerObject(renderer);
}
void InviwoModule::registerRepresentationConverter(
    RepresentationConverter* representationConverter) {
    representationConverters_.push_back(representationConverter);
    RepresentationConverterFactory::getPtr()->registerObject(representationConverter);
}
void InviwoModule::registerResource(Resource* resource) { resources_.push_back(resource); }
void InviwoModule::registerSettings(Settings* settings) { moduleSettings_.push_back(settings); }

}  // namespace
