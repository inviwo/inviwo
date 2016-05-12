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

#ifndef IVW_INVIWOMODULE_H
#define IVW_INVIWOMODULE_H

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/datastructures/camerafactory.h>
#include <inviwo/core/datastructures/camerafactoryobject.h>
#include <inviwo/core/ports/portfactory.h>
#include <inviwo/core/ports/portfactoryobject.h>
#include <inviwo/core/processors/processorfactory.h>
#include <inviwo/core/processors/processorfactoryobject.h>
#include <inviwo/core/processors/processorwidgetfactory.h>
#include <inviwo/core/processors/processorwidgetfactoryobject.h>
#include <inviwo/core/properties/propertyfactory.h>
#include <inviwo/core/properties/propertyfactoryobject.h>
#include <inviwo/core/properties/propertywidgetfactory.h>
#include <inviwo/core/properties/propertywidgetfactoryobject.h>
#include <inviwo/core/util/dialogfactory.h>
#include <inviwo/core/util/dialogfactoryobject.h>

#include <type_traits>

namespace inviwo {

class Settings;
class MetaData;
class Capabilities;
class Resource;
class RepresentationConverter;
class DataReader;
class DataWriter;
class PortInspectorFactoryObject;
class MeshDrawer;
class PropertyConverter;


enum class ModulePath {
    Data,               // /data
    Images,             // /data/images
    PortInspectors,     // /data/portinspectors
    Scripts,            // /data/scripts
    Volumes,            // /data/volumes
    Workspaces,         // /data/workspaces
    Docs,               // /docs
    Tests,              // /tests
    TestImages,         // /tests/images
    TestVolumes,        // /tests/volumes
    UnitTests,          // /tests/unittests
    RegressionTests,    // /tests/regression
    GLSL,               // /glsl
    CL                  // /cl
};

/**
 * \class InviwoModule
 * \brief A module class contains registrations of functionality, such as processors, ports,
 * properties etc.
 */
class IVW_CORE_API InviwoModule {
public:
    /**
     * @param identifier Name of module folder
     */
    InviwoModule(InviwoApplication* app, const std::string& identifier);
    InviwoModule(const InviwoModule&) = delete;
    InviwoModule& operator=(const InviwoModule&) = delete;
    virtual ~InviwoModule();

    /**
     * \brief Get module identifier, i.e the module folder name.
     */
    std::string getIdentifier() const;

    /**
     * Override to provide a description of the module
     */
    virtual std::string getDescription() const;

    /**
     * Get the path to this module directory.
     * For instance: C:/inviwo/modules/mymodule/
     * @note Assumes that getIdentifier() returns the module folder name.
     *       The folder name should always be lower case.
     * @note The returned directory might not exist in the case that the app is
     *       deployed and the module does not contain any resources.
     * @return std::string Path to module directory
     */
    virtual std::string getPath() const;
    std::string getPath(ModulePath type) const;

    const std::vector<CameraFactoryObject*> getCameras() const;
    const std::vector<Capabilities*> getCapabilities() const;
    const std::vector<DataReader*> getDataReaders() const;
    const std::vector<DataWriter*> getDataWriters() const;
    const std::vector<DialogFactoryObject*> getDialogs() const;
    const std::vector<MeshDrawer*> getDrawers() const;
    const std::vector<MetaData*> getMetaData() const;
    const std::vector<InportFactoryObject*> getInports() const;
    const std::vector<OutportFactoryObject*> getOutports() const;
    const std::vector<PortInspectorFactoryObject*> getPortInspectors() const;
    const std::vector<ProcessorFactoryObject*> getProcessors() const;
    const std::vector<ProcessorWidgetFactoryObject*> getProcessorWidgets() const;
    const std::vector<PropertyFactoryObject*> getProperties() const;
    const std::vector<PropertyWidgetFactoryObject*> getPropertyWidgets() const;
    const std::vector<RepresentationConverter*> getRepresentationConverters() const;
    const std::vector<Resource*> getResources() const;
    const std::vector<Settings*> getSettings() const;

protected:
    void registerCapabilities(std::unique_ptr<Capabilities> info);
    template <typename T>
    void registerCamera(std::string classIdentifier);
    void registerDataReader(std::unique_ptr<DataReader> reader);
    void registerDataWriter(std::unique_ptr<DataWriter> writer);

    template <typename T>
    void registerDialog(std::string classIdentifier);
    void registerDrawer(std::unique_ptr<MeshDrawer> drawer);
    void registerMetaData(std::unique_ptr<MetaData> meta);

    template <typename T, typename P>
    void registerPropertyWidget(PropertySemantics semantics);
    template <typename T, typename P>
    void registerPropertyWidget(std::string semantics);

    template <typename T>
    void registerProcessor();

template <typename T, typename P>
    void registerProcessorWidget();

    void registerPortInspector(std::string portClassIdentifier, std::string inspectorPath);

    template <typename T>
    void registerPort(std::string classIdentifier);

    template <typename T>
    void registerProperty();

    void registerPropertyConverter(std::unique_ptr<PropertyConverter> propertyConverter);
    void registerRepresentationConverter(std::unique_ptr<RepresentationConverter> converter);
    void registerResource(std::unique_ptr<Resource> resource);
    void registerSettings(std::unique_ptr<Settings> settings);

    InviwoApplication* app_;  // reference to the app that we belong to

private:
    template <typename T>
    std::vector<T*> uniqueToPtr(std::vector<std::unique_ptr<T>>& v) {
        std::vector<T*> res;
        for (auto& elem : v) res.push_back(elem.get());
        return res;
    }
    template <typename T>
    const std::vector<T*> uniqueToPtr(const std::vector<std::unique_ptr<T>>& v) const {
        std::vector<T*> res;
        for (auto& elem : v) res.push_back(elem.get());
        return res;
    }
    template <typename T, typename std::enable_if<std::is_base_of<Inport, T>::value, int>::type = 0>
    void registerPortInternal(std::string classIdentifier) {
        auto port = util::make_unique<InportFactoryObjectTemplate<T>>(classIdentifier);
        if (app_->getInportFactory()->registerObject(port.get())) {
            inports_.push_back(std::move(port));
        }
    }

    template <typename T,
        typename std::enable_if<std::is_base_of<Outport, T>::value, int>::type = 0>
        void registerPortInternal(std::string classIdentifier) {
        auto port = util::make_unique<OutportFactoryObjectTemplate<T>>(classIdentifier);
        if (app_->getOutportFactory()->registerObject(port.get())) {
            outports_.push_back(std::move(port));
        }
    }

    const std::string identifier_;  ///< Module folder name

    std::vector<std::unique_ptr<CameraFactoryObject>> cameras_;
    std::vector<std::unique_ptr<Capabilities>> capabilities_;
    std::vector<std::unique_ptr<DataReader>> dataReaders_;
    std::vector<std::unique_ptr<DataWriter>> dataWriters_;
    std::vector<std::unique_ptr<DialogFactoryObject>> dialogs_;
    std::vector<std::unique_ptr<MeshDrawer>> drawers_;
    std::vector<std::unique_ptr<MetaData>> metadata_;
    std::vector<std::unique_ptr<InportFactoryObject>> inports_;
    std::vector<std::unique_ptr<OutportFactoryObject>> outports_;
    std::vector<std::unique_ptr<PortInspectorFactoryObject>> portInspectors_;
    std::vector<std::unique_ptr<ProcessorFactoryObject>> processors_;
    std::vector<std::unique_ptr<ProcessorWidgetFactoryObject>> processorWidgets_;
    std::vector<std::unique_ptr<PropertyConverter>> propertyConverters_;
    std::vector<std::unique_ptr<PropertyFactoryObject>> properties_;
    std::vector<std::unique_ptr<PropertyWidgetFactoryObject>> propertyWidgets_;
    std::vector<std::unique_ptr<RepresentationConverter>> representationConverters_;
    std::vector<std::unique_ptr<Resource>> resources_;
    std::vector<std::unique_ptr<Settings>> settings_;


};



template <typename T>
void InviwoModule::registerCamera(std::string classIdentifier) {
    auto camera = util::make_unique<CameraFactoryObjectTemplate<T>>(classIdentifier);
    if (app_->getCameraFactory()->registerObject(camera.get())) {
        cameras_.push_back(std::move(camera));
    }
}

template <typename T>
void InviwoModule::registerDialog(std::string classIdentifier) {
    auto dialog = util::make_unique<DialogFactoryObjectTemplate<T>>(classIdentifier);
    if (app_->getDialogFactory()->registerObject(dialog.get())) {
        dialogs_.push_back(std::move(dialog));
    }
}

template <typename T>
void InviwoModule::registerProcessor() {
    auto processor = util::make_unique<ProcessorFactoryObjectTemplate<T>>();
    if (app_->getProcessorFactory()->registerObject(processor.get())) {
        processors_.push_back(std::move(processor));
    }
}

template <typename T, typename P>
void InviwoModule::registerProcessorWidget() {
    auto widget = util::make_unique<ProcessorWidgetFactoryObjectTemplate<T, P>>();
    if (app_->getProcessorWidgetFactory()->registerObject(widget.get())) {
        processorWidgets_.push_back(std::move(widget));
    }
}

template <typename T>
void InviwoModule::registerPort(std::string classIdentifier) {
    static_assert(std::is_base_of<Inport, T>::value || std::is_base_of<Outport, T>::value,
                  "A port has to derive from either Inport of Outport");

    registerPortInternal<T>(classIdentifier);
}

template <typename T>
void InviwoModule::registerProperty() {
    auto property = util::make_unique<PropertyFactoryObjectTemplate<T>>();
    if (app_->getPropertyFactory()->registerObject(property.get())) {
        properties_.push_back(std::move(property));
    }
}
template <typename T, typename P>
void InviwoModule::registerPropertyWidget(PropertySemantics semantics) {
    auto propertyWidget = util::make_unique<PropertyWidgetFactoryObjectTemplate<T, P>>(semantics);
    if (app_->getPropertyWidgetFactory()->registerObject(propertyWidget.get())) {
        propertyWidgets_.push_back(std::move(propertyWidget));
    }
}
template <typename T, typename P>
void InviwoModule::registerPropertyWidget(std::string semantics) {
    registerPropertyWidget<T,P>(PropertySemantics(semantics));
}

}  // namespace

#endif  // IVW_INVIWOMODULE_H
