/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2025 Inviwo Foundation
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
#include <inviwo/core/common/moduleaction.h>
#include <inviwo/core/common/modulemanager.h>
#include <inviwo/core/datastructures/camera/camerafactory.h>
#include <inviwo/core/io/datareader.h>
#include <inviwo/core/io/datareaderfactory.h>
#include <inviwo/core/io/datawriter.h>
#include <inviwo/core/io/datawriterfactory.h>
#include <inviwo/core/metadata/metadata.h>
#include <inviwo/core/metadata/metadatafactory.h>
#include <inviwo/core/ports/portfactory.h>
#include <inviwo/core/ports/portinspectorfactory.h>
#include <inviwo/core/ports/portinspectorfactoryobject.h>
#include <inviwo/core/processors/processorfactory.h>
#include <inviwo/core/processors/processorwidgetfactory.h>
#include <inviwo/core/processors/compositeprocessorfactoryobject.h>
#include <inviwo/core/properties/propertyfactory.h>
#include <inviwo/core/properties/propertyconverter.h>
#include <inviwo/core/properties/propertyconvertermanager.h>
#include <inviwo/core/rendering/meshdrawer.h>
#include <inviwo/core/rendering/meshdrawerfactory.h>
#include <inviwo/core/rendering/datavisualizer.h>
#include <inviwo/core/rendering/datavisualizermanager.h>
#include <inviwo/core/util/dialogfactory.h>
#include <inviwo/core/util/capabilities.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/settings/settings.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/io/serialization/versionconverter.h>

#include <algorithm>

namespace inviwo {

InviwoModule::InviwoModule(InviwoApplication* app, std::string_view identifier)
    : app_(app)
    , identifier_(identifier)
    , moduleRoot_{app->getModuleManager().locateModule(*this)} {}

InviwoModule::~InviwoModule() {
    // unregister everything...
    for (auto& elem : cameras_) {
        app_->getCameraFactory()->unRegisterObject(elem.get());
    }
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
    for (auto& elem : inports_) {
        app_->getInportFactory()->unRegisterObject(elem.get());
    }
    for (auto& elem : outports_) {
        app_->getOutportFactory()->unRegisterObject(elem.get());
    }
    for (auto& elem : portInspectors_) {
        app_->getPortInspectorFactory()->unRegisterObject(elem.get());
    }
    for (auto& elem : processors_) {
        app_->getProcessorFactory()->unRegisterObject(elem.get());
    }
    for (auto& elem : processorWidgets_) {
        app_->getProcessorWidgetFactory()->unRegisterObject(elem.get());
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
    for (auto& unRegFunctor : representationUnRegFunctors_) {
        unRegFunctor();
    }
    for (auto& elem : representationFactories_) {
        app_->getRepresentationMetaFactory()->unRegisterObject(elem.get());
    }
    for (auto& unRegFunctor : representationConvertersUnRegFunctors_) {
        unRegFunctor();
    }
    for (auto& elem : representationConverterFactories_) {
        app_->getRepresentationConverterMetaFactory()->unRegisterObject(elem.get());
    }
    for (auto& elem : dataVisualizers_) {
        app_->getDataVisualizerManager()->unRegisterObject(elem.get());
    }
    for (auto* elem : settings_) {
        app_->unregisterSettings(elem);
    }

    // Remove any potential ModuleCallbackAction associated with this module
    auto& callbackActions = app_->getCallbackActions();
    std::erase_if(callbackActions, [&](const auto& a) { return a->getModule() == this; });
}

const std::string& InviwoModule::getIdentifier() const { return identifier_; }

const std::filesystem::path& InviwoModule::getPath() const { return moduleRoot_; }

std::filesystem::path InviwoModule::getPath(ModulePath type) const {
    const auto& basePath = getPath();
    // clang-format off
    auto path = [&](){
        switch (type) {
            case ModulePath::Data:               return basePath / "data";
            case ModulePath::Images:             return basePath / "data/images";
            case ModulePath::PortInspectors:     return basePath / "data/portinspectors";
            case ModulePath::Scripts:            return basePath / "data/scripts";
            case ModulePath::TransferFunctions:  return basePath / "data/transferfunctions";
            case ModulePath::Volumes:            return basePath / "data/volumes";
            case ModulePath::Workspaces:         return basePath / "data/workspaces";
            case ModulePath::Docs:               return basePath / "docs";
            case ModulePath::Tests:              return basePath / "tests";
            case ModulePath::TestImages:         return basePath / "tests/images";
            case ModulePath::TestVolumes:        return basePath / "tests/volumes";
            case ModulePath::UnitTests:          return basePath / "tests/unittests";
            case ModulePath::RegressionTests:    return basePath / "tests/regression";
            case ModulePath::GLSL:               return basePath / "glsl";
            case ModulePath::CL:                 return basePath / "cl";
            default:                             return basePath;
        }
    }();
    // clang-format on
    return path.lexically_normal();
}

int InviwoModule::getVersion() const { return 0; }

std::unique_ptr<VersionConverter> InviwoModule::getConverter(int) const { return nullptr; }

const std::vector<Capabilities*> InviwoModule::getCapabilities() const {
    return uniqueToPtr(capabilities_);
}

const std::vector<CameraFactoryObject*> InviwoModule::getCameras() const {
    return uniqueToPtr(cameras_);
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
const std::vector<InportFactoryObject*> InviwoModule::getInports() const {
    return uniqueToPtr(inports_);
}
const std::vector<OutportFactoryObject*> InviwoModule::getOutports() const {
    return uniqueToPtr(outports_);
}

const std::vector<PortInspectorFactoryObject*> InviwoModule::getPortInspectors() const {
    return uniqueToPtr(portInspectors_);
}
const std::vector<ProcessorFactoryObject*> InviwoModule::getProcessors() const {
    return uniqueToPtr(processors_);
}
const std::vector<ProcessorWidgetFactoryObject*> InviwoModule::getProcessorWidgets() const {
    return uniqueToPtr(processorWidgets_);
}
const std::vector<PropertyFactoryObject*> InviwoModule::getProperties() const {
    return uniqueToPtr(properties_);
}
const std::vector<PropertyWidgetFactoryObject*> InviwoModule::getPropertyWidgets() const {
    return uniqueToPtr(propertyWidgets_);
}
const std::vector<BaseRepresentationFactoryObject*> InviwoModule::getRepresentationFactoryObjects()
    const {
    return uniqueToPtr(representationFactoryObjects_);
}
const std::vector<BaseRepresentationFactory*> InviwoModule::getRepresentationFactories() const {
    return uniqueToPtr(representationFactories_);
}
const std::vector<BaseRepresentationConverter*> InviwoModule::getRepresentationConverters() const {
    return uniqueToPtr(representationConverters_);
}

const std::vector<BaseRepresentationConverterFactory*>
InviwoModule::getRepresentationConverterFactories() const {
    return uniqueToPtr(representationConverterFactories_);
}

const std::vector<MeshDrawer*> InviwoModule::getDrawers() const { return uniqueToPtr(drawers_); }
const std::vector<Settings*>& InviwoModule::getSettings() const { return settings_; }

std::string InviwoModule::getDescription() const {
    if (auto* item = app_->getModuleManager().getFactoryObject(identifier_)) {
        return item->description;
    }
    return "No description available";
}

void InviwoModule::registerCapabilities(std::unique_ptr<Capabilities> info) {
    capabilities_.push_back(std::move(info));
}

void InviwoModule::registerCamera(std::unique_ptr<CameraFactoryObject> camera) {
    if (app_->getCameraFactory()->registerObject(camera.get())) {
        cameras_.push_back(std::move(camera));
    }
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
void InviwoModule::registerDialog(std::unique_ptr<DialogFactoryObject> dialog) {
    if (app_->getDialogFactory()->registerObject(dialog.get())) {
        dialogs_.push_back(std::move(dialog));
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
void InviwoModule::registerProperty(std::unique_ptr<PropertyFactoryObject> property) {
    if (app_->getPropertyFactory()->registerObject(property.get())) {
        properties_.push_back(std::move(property));
    }
}
void InviwoModule::registerPropertyWidget(
    std::unique_ptr<PropertyWidgetFactoryObject> propertyWidget) {
    if (app_->getPropertyWidgetFactory()->registerObject(propertyWidget.get())) {
        propertyWidgets_.push_back(std::move(propertyWidget));
    }
}
void InviwoModule::registerPropertyConverter(std::unique_ptr<PropertyConverter> propertyConverter) {
    if (app_->getPropertyConverterManager()->registerObject(propertyConverter.get())) {
        propertyConverters_.push_back(std::move(propertyConverter));
    }
}

void InviwoModule::registerRepresentationFactory(
    std::unique_ptr<BaseRepresentationFactory> representationFactory) {
    if (app_->getRepresentationMetaFactory()->registerObject(representationFactory.get())) {
        representationFactories_.push_back(std::move(representationFactory));
    }
}

void InviwoModule::registerRepresentationConverterFactory(
    std::unique_ptr<BaseRepresentationConverterFactory> converterFactory) {
    if (app_->getRepresentationConverterMetaFactory()->registerObject(converterFactory.get())) {
        representationConverterFactories_.push_back(std::move(converterFactory));
    }
}

void InviwoModule::registerSettings(std::unique_ptr<Settings> settings) {
    registerSettings(settings.get());
    ownedSettings_.push_back(std::move(settings));
}

void InviwoModule::registerSettings(Settings* settings) {
    app_->registerSettings(settings);
    settings_.push_back(settings);
}

InviwoApplication* InviwoModule::getInviwoApplication() const { return app_; }

void InviwoModule::registerProcessor(std::unique_ptr<ProcessorFactoryObject> pfo) {
    if (app_->getProcessorFactory()->registerObject(pfo.get())) {
        processors_.push_back(std::move(pfo));
    }
}

void InviwoModule::registerCompositeProcessor(const std::filesystem::path& file) {
    auto processor = std::make_unique<CompositeProcessorFactoryObject>(file);
    if (app_->getProcessorFactory()->registerObject(processor.get())) {
        processors_.push_back(std::move(processor));
    }
}

void InviwoModule::registerProcessorWidget(std::unique_ptr<ProcessorWidgetFactoryObject> widget) {
    if (app_->getProcessorWidgetFactory()->registerObject(widget.get())) {
        processorWidgets_.push_back(std::move(widget));
    }
}

void InviwoModule::registerPortInspector(std::string_view portClassIdentifier,
                                         const std::filesystem::path& inspectorPath) {
    auto portInspector =
        std::make_unique<PortInspectorFactoryObject>(portClassIdentifier, inspectorPath);

    if (app_->getPortInspectorFactory()->registerObject(portInspector.get())) {
        portInspectors_.push_back(std::move(portInspector));
    }
}

void InviwoModule::registerDataVisualizer(std::unique_ptr<DataVisualizer> visualizer) {
    app_->getDataVisualizerManager()->registerObject(visualizer.get());
    dataVisualizers_.push_back(std::move(visualizer));
}

void InviwoModule::registerInport(std::unique_ptr<InportFactoryObject> inport) {
    if (app_->getInportFactory()->registerObject(inport.get())) {
        inports_.push_back(std::move(inport));
    }
}

void InviwoModule::registerOutport(std::unique_ptr<OutportFactoryObject> outport) {
    if (app_->getOutportFactory()->registerObject(outport.get())) {
        outports_.push_back(std::move(outport));
    }
}

}  // namespace inviwo
