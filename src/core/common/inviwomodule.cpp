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

#include <inviwo/core/common/inviwomodule.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/datastructures/representationconverter.h>
#include <inviwo/core/datastructures/representationconverterfactory.h>
#include <inviwo/core/io/datareader.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/io/datawriter.h>
#include <inviwo/core/io/datawriterfactory.h>
#include <inviwo/core/metadata/metadata.h>
#include <inviwo/core/metadata/metadatafactory.h>
#include <inviwo/core/ports/portinspectorfactory.h>
#include <inviwo/core/ports/portinspectorfactoryobject.h>
#include <inviwo/core/processors/processorwidgetfactory.h>
#include <inviwo/core/properties/propertyconverter.h>
#include <inviwo/core/properties/propertyconvertermanager.h>
#include <inviwo/core/rendering/meshdrawer.h>
#include <inviwo/core/rendering/meshdrawerfactory.h>
#include <inviwo/core/resources/resource.h>
#include <inviwo/core/util/capabilities.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/settings/settings.h>

#include <pathsexternalmodules.h>

#include <algorithm>

namespace inviwo {

InviwoModule::InviwoModule(InviwoApplication* app, const std::string& identifier)
    : app_(app), identifier_(identifier) {}

InviwoModule::~InviwoModule() {
    // deregersiter everything...
    for (auto& elem : dataReaders_) {
        app_->getDataReaderFactory()->unRegisterObject(elem.get());
    }
    for (auto& elem : dataWriters_) {
        app_->getDataWriterFactory()->unRegisterObject(elem.get());
    }
    for (auto& elem : dialogs_) {
        app_->getDialogFactory()->unRegisterObject(elem.get());
    }
    for (auto& elem : drawers_) {
        app_->getMeshDrawerFactory()->unRegisterObject(elem.get());
    }
    for (auto& elem : metadata_) {
        app_->getMetaDataFactory()->unRegisterObject(elem.get());
    }
    for (auto& elem : ports_) {
        app_->getPortFactory()->unRegisterObject(elem.get());
    }
    for (auto& elem : portInspectors_) {
        app_->getPortInspectorFactory()->unRegisterObject(elem.get());
    }
    for (auto& elem : processors_) {
        app_->getProcessorFactory()->unRegisterObject(elem.get());
    }
    for (auto& elem : propertyConverters_) {
        app_->getPropertyConverterManager()->unRegisterObject(elem.get());
    }
    for (auto& elem : properties_) {
        app_->getPropertyFactory()->unRegisterObject(elem.get());
    }
    for (auto& elem : propertyWidgets_) {
        app_->getPropertyWidgetFactory()->unRegisterObject(elem.get());
    }
    for (auto& elem : representationConverters_) {
        app_->getRepresentationConverterFactory()->unRegisterObject(elem.get());
    }
}

std::string InviwoModule::getIdentifier() const { return identifier_; }

std::string InviwoModule::getPath() const {
    std::string moduleNameLowerCase = getIdentifier();
    std::transform(moduleNameLowerCase.begin(), moduleNameLowerCase.end(),
                   moduleNameLowerCase.begin(), ::tolower);
    if (filesystem::directoryExists(app_->getPath(InviwoApplication::PATH_MODULES) + "/" +
                                    moduleNameLowerCase)) {
        return app_->getPath(InviwoApplication::PATH_MODULES) + "/" + moduleNameLowerCase;
    }
#ifdef IVW_EXTERNAL_MODULES_PATH_COUNT
    for (auto& elem : externalModulePaths_) {
        std::string directory = elem + "/" + moduleNameLowerCase;
        if (filesystem::directoryExists(directory)) {
            return directory;
        }
    }
#endif
    LogWarn(moduleNameLowerCase << " directory was not found");
    return app_->getPath(InviwoApplication::PATH_MODULES) + "/" + moduleNameLowerCase;
}

const std::vector<Capabilities*> InviwoModule::getCapabilities() const {
    return uniqueToPtr(capabilities_);
}
const std::vector<DataReader*> InviwoModule::getDataReaders() const {
    return uniqueToPtr(dataReaders_);
}
const std::vector<DataWriter*> InviwoModule::getDataWriters() const {
    return uniqueToPtr(dataWriters_);
}
const std::vector<DialogFactoryObject*> InviwoModule::getDialogs() const {
    return uniqueToPtr(dialogs_);
}
const std::vector<MetaData*> InviwoModule::getMetaData() const { return uniqueToPtr(metadata_); }
const std::vector<PortFactoryObject*> InviwoModule::getPorts() const { return uniqueToPtr(ports_); }
const std::vector<PortInspectorFactoryObject*> InviwoModule::getPortInspectors() const {
    return uniqueToPtr(portInspectors_);
}
const std::vector<ProcessorFactoryObject*> InviwoModule::getProcessors() const {
    return uniqueToPtr(processors_);
}
const std::vector<std::pair<std::string, ProcessorWidget*> > InviwoModule::getProcessorWidgets()
    const {
    std::vector<std::pair<std::string, ProcessorWidget*> > res;
    for (auto& elem : processorWidgets_)
        res.push_back(std::make_pair(elem.first, elem.second.get()));
    return res;
}
const std::vector<PropertyFactoryObject*> InviwoModule::getProperties() const {
    return uniqueToPtr(properties_);
}
const std::vector<PropertyWidgetFactoryObject*> InviwoModule::getPropertyWidgets() const {
    return uniqueToPtr(propertyWidgets_);
}
const std::vector<RepresentationConverter*> InviwoModule::getRepresentationConverters() const {
    return uniqueToPtr(representationConverters_);
}
const std::vector<MeshDrawer*> InviwoModule::getDrawers() const { return uniqueToPtr(drawers_); }
const std::vector<Resource*> InviwoModule::getResources() const { return uniqueToPtr(resources_); }
const std::vector<Settings*> InviwoModule::getSettings() const { return uniqueToPtr(settings_); }

std::string InviwoModule::getDescription() const { return "No description available"; }

void InviwoModule::registerCapabilities(std::unique_ptr<Capabilities> info) {
    capabilities_.push_back(std::move(info));
}

void InviwoModule::registerDataReader(std::unique_ptr<DataReader> dataReader) {
    if (app_->getDataReaderFactory()->registerObject(dataReader.get())) {
        dataReaders_.push_back(std::move(dataReader));
    }
}
void InviwoModule::registerDataWriter(std::unique_ptr<DataWriter> dataWriter) {
    if (app_->getDataWriterFactory()->registerObject(dataWriter.get())) {
        dataWriters_.push_back(std::move(dataWriter));
    }
}
void InviwoModule::registerDrawer(std::unique_ptr<MeshDrawer> drawer) {
    if (app_->getMeshDrawerFactory()->registerObject(drawer.get())) {
        drawers_.push_back(std::move(drawer));
    }
}
void InviwoModule::registerMetaData(std::unique_ptr<MetaData> meta) {
    if (app_->getMetaDataFactory()->registerObject(meta.get())) {
        metadata_.push_back(std::move(meta));
    }
}
void InviwoModule::registerPropertyConverter(std::unique_ptr<PropertyConverter> propertyConverter) {
    if (app_->getPropertyConverterManager()->registerObject(propertyConverter.get())) {
        propertyConverters_.push_back(std::move(propertyConverter));
    }
}
void InviwoModule::registerRepresentationConverter(
    std::unique_ptr<RepresentationConverter> converter) {
    if (app_->getRepresentationConverterFactory()->registerObject(converter.get())) {
        representationConverters_.push_back(std::move(converter));
    }
}
void InviwoModule::registerResource(std::unique_ptr<Resource> resource) {
    resources_.push_back(std::move(resource));
}
void InviwoModule::registerSettings(std::unique_ptr<Settings> settings) {
    settings_.push_back(std::move(settings));
}

void InviwoModule::registerPortInspector(std::string portClassIdentifier,
                                         std::string inspectorPath) {
    auto portInspector =
        util::make_unique<PortInspectorFactoryObject>(portClassIdentifier, inspectorPath);

    if (app_->getPortInspectorFactory()->registerObject(portInspector.get())) {
        portInspectors_.push_back(std::move(portInspector));
    }
}

void InviwoModule::registerProcessorWidget(std::string processorClassName,
                                           std::unique_ptr<ProcessorWidget> processorWidget) {
    if (app_->getProcessorWidgetFactory()->registerObject(
            std::make_pair(processorClassName, processorWidget.get()))) {
        processorWidgets_.push_back(std::make_pair(processorClassName, std::move(processorWidget)));
    }
}

}  // namespace
