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

#include <inviwo/core/ports/portfactory.h>
#include <inviwo/core/ports/portfactoryobject.h>
#include <inviwo/core/processors/processorfactory.h>
#include <inviwo/core/processors/processorfactoryobject.h>
#include <inviwo/core/properties/propertyfactory.h>
#include <inviwo/core/properties/propertyfactoryobject.h>
#include <inviwo/core/properties/propertywidgetfactory.h>
#include <inviwo/core/properties/propertywidgetfactoryobject.h>
#include <inviwo/core/util/dialogfactory.h>
#include <inviwo/core/util/dialogfactoryobject.h>

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
     * @return std::string Path to module directory
     */
    std::string getPath() const;

    const std::vector<Capabilities*> getCapabilities() const;
    const std::vector<DataReader*> getDataReaders() const;
    const std::vector<DataWriter*> getDataWriters() const;
    const std::vector<DialogFactoryObject*> getDialogs() const;
    const std::vector<MeshDrawer*> getDrawers() const;
    const std::vector<MetaData*> getMetaData() const;
    const std::vector<PortFactoryObject*> getPorts() const;
    const std::vector<PortInspectorFactoryObject*> getPortInspectors() const;
    const std::vector<ProcessorFactoryObject*> getProcessors() const;
    const std::vector<PropertyFactoryObject*> getProperties() const;
    const std::vector<PropertyWidgetFactoryObject*> getPropertyWidgets() const;
    const std::vector<RepresentationConverter*> getRepresentationConverters() const;
    const std::vector<Resource*> getResources() const;
    const std::vector<Settings*> getSettings() const;
    const std::vector<std::pair<std::string, ProcessorWidget*>> getProcessorWidgets() const;

protected:
    void registerCapabilities(std::unique_ptr<Capabilities> info);
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

    void registerProcessorWidget(std::string processorClassName,
                                 std::unique_ptr<ProcessorWidget> processorWidget);

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

    const std::string identifier_;  ///< Module folder name

    std::vector<std::unique_ptr<Capabilities>> capabilities_;
    std::vector<std::unique_ptr<DataReader>> dataReaders_;
    std::vector<std::unique_ptr<DataWriter>> dataWriters_;
    std::vector<std::unique_ptr<DialogFactoryObject>> dialogs_;
    std::vector<std::unique_ptr<MeshDrawer>> drawers_;
    std::vector<std::unique_ptr<MetaData>> metadata_;
    std::vector<std::unique_ptr<PortFactoryObject>> ports_;
    std::vector<std::unique_ptr<PortInspectorFactoryObject>> portInspectors_;
    std::vector<std::unique_ptr<ProcessorFactoryObject>> processors_;
    std::vector<std::unique_ptr<PropertyConverter>> propertyConverters_;
    std::vector<std::unique_ptr<PropertyFactoryObject>> properties_;
    std::vector<std::unique_ptr<PropertyWidgetFactoryObject>> propertyWidgets_;
    std::vector<std::unique_ptr<RepresentationConverter>> representationConverters_;
    std::vector<std::unique_ptr<Resource>> resources_;
    std::vector<std::unique_ptr<Settings>> settings_;

    std::vector<std::pair<std::string, std::unique_ptr<ProcessorWidget>>> processorWidgets_;
};

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

template <typename T>
void InviwoModule::registerPort(std::string classIdentifier) {
    auto port = util::make_unique<PortFactoryObjectTemplate<T>>(classIdentifier);
    if (app_->getPortFactory()->registerObject(port.get())) {
        ports_.push_back(std::move(port));
    }
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
