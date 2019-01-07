/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2018 Inviwo Foundation
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
#include <inviwo/core/datastructures/representationconverter.h>
#include <inviwo/core/datastructures/representationconverterfactory.h>

#include <inviwo/core/ports/portfactory.h>
#include <inviwo/core/ports/portfactoryobject.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>

#include <inviwo/core/processors/processorfactory.h>
#include <inviwo/core/processors/processorfactoryobject.h>
#include <inviwo/core/processors/processorwidgetfactory.h>
#include <inviwo/core/processors/processorwidgetfactoryobject.h>
#include <inviwo/core/processors/compositesink.h>
#include <inviwo/core/processors/compositesource.h>

#include <inviwo/core/properties/propertyfactory.h>
#include <inviwo/core/properties/propertyfactoryobject.h>
#include <inviwo/core/properties/propertywidgetfactory.h>
#include <inviwo/core/properties/propertywidgetfactoryobject.h>

#include <inviwo/core/util/dialogfactory.h>
#include <inviwo/core/util/dialogfactoryobject.h>
#include <inviwo/core/util/stringconversion.h>

#include <type_traits>

namespace inviwo {

class Settings;
class MetaData;
class Capabilities;
class DataReader;
class DataWriter;
class PortInspectorFactoryObject;
class MeshDrawer;
class PropertyConverter;
class VersionConverter;
class DataVisualizer;

enum class ModulePath {
    Data,               // /data
    Images,             // /data/images
    PortInspectors,     // /data/portinspectors
    Scripts,            // /data/scripts
    TransferFunctions,  // /data/transferfunctions
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
     * @param app the inviwo application.
     * @param identifier Name of module folder.
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

    /**
     * Returns the module version. This is used for converting old processor networks in connection
     * with the getConverter function. By default it will return 0. Overload this function to return
     * a larger value when you need to update the module version.
     */
    virtual int getVersion() const;

    /**
     * Should return a converter that updates a processor network from the oldModuleVersion to the
     * current module version returned by getVersion. You need to overload this together with
     * getVersion to implement conversioning for the module. This is needed whenever you modify a
     * processor in such a was as breaking the deserialization of a old network. For example by
     * changing the identifier of a property. By the default it will return a nullptr. Since there
     * is no need to convert to version 0. Look at BaseModule for an example of this in use.
     */
    virtual std::unique_ptr<VersionConverter> getConverter(int oldModuleVersion) const;

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
    const std::vector<BaseRepresentationConverter*> getRepresentationConverters() const;
    const std::vector<BaseRepresentationConverterFactory*> getRepresentationConverterFactories()
        const;
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

    /**
     * Register a workspace file as a CompositeProcessor.
     * The CompositeProcessor will load the file as its sub network on construction.
     */
    void registerCompositeProcessor(const std::string& file);

    template <typename T, typename P>
    void registerProcessorWidget();

    void registerPortInspector(std::string portClassIdentifier, std::string inspectorPath);

    void registerDataVisualizer(std::unique_ptr<DataVisualizer> visualizer);

    /**
     * Register port type T, PortTraits<T>::classIdentifier has to be defined and return a non
     * empty and unique string. We use reverse DNS for class identifiers, i.e. org.inviwo.classname
     * Prefer using registerDefaultsForDataType to registerPort since it adds support for
     * CompositeProcessor
     * @see PortTraits
     * @see registerDefaultsForDataType
     */
    template <typename T>
    void registerPort();

    /**
     * Utility for register a standard set of ports and processors for a data type T
     * Will register the following ports:
     *     DataInport<T>           Inport
     *     DataInport<T, 0>        Multi Inport (accepts multiple input connections)
     *     DataInport<T, 0, true>  Flat Multi Inport (accepts input connections with
     *                             vector<shared_ptr<T>>)
     *     DataOutport<T>          Outport
     * and Sink and Source Processors:
     *     CompositeSink<DataInport<T>, DataOutport<T>>
     *     CompositeSource<DataInport<T>, DataOutport<T>>
     *
     * @see DataInport
     * @see DataOutport
     * @see CompositeSource
     * @see CompositeSink
     */
    template <typename T>
    void registerDefaultsForDataType();

    template <typename T>
    void registerProperty();

    void registerPropertyConverter(std::unique_ptr<PropertyConverter> propertyConverter);

    template <typename BaseRepr>
    void registerRepresentationConverter(
        std::unique_ptr<RepresentationConverter<BaseRepr>> converter);

    void registerRepresentationConverterFactory(
        std::unique_ptr<BaseRepresentationConverterFactory> converterFactory);

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
    void registerPortInternal() {
        auto port = util::make_unique<InportFactoryObjectTemplate<T>>();
        if (app_->getInportFactory()->registerObject(port.get())) {
            inports_.push_back(std::move(port));
        }
    }

    template <typename T,
              typename std::enable_if<std::is_base_of<Outport, T>::value, int>::type = 0>
    void registerPortInternal() {
        auto port = util::make_unique<OutportFactoryObjectTemplate<T>>();
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
    std::vector<std::unique_ptr<BaseRepresentationConverter>> representationConverters_;
    std::vector<std::function<void()>> representationConvertersUnRegFunctors_;
    std::vector<std::unique_ptr<BaseRepresentationConverterFactory>>
        representationConverterFactories_;
    std::vector<std::unique_ptr<Settings>> settings_;
    std::vector<std::unique_ptr<DataVisualizer>> dataVisualizers_;
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
void InviwoModule::registerPort() {
    static_assert(std::is_base_of<Inport, T>::value || std::is_base_of<Outport, T>::value,
                  "A port has to derive from either Inport of Outport");
    try {
        registerPortInternal<T>();
    } catch (const Exception& e) {
        LogError("Error registering port \"" << parseTypeIdName(std::string(typeid(T).name()))
                                             << "\" in module " << getIdentifier()
                                             << ". Reason: " << e.getMessage());
    }
}

template <typename T>
void InviwoModule::registerDefaultsForDataType() {
    registerPort<DataInport<T>>();
    registerPort<DataInport<T, 0>>();
    registerPort<DataInport<T, 0, true>>();
    registerPort<DataOutport<T>>();

    registerProcessor<CompositeSink<DataInport<T>, DataOutport<T>>>();
    registerProcessor<CompositeSource<DataInport<T>, DataOutport<T>>>();
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
    registerPropertyWidget<T, P>(PropertySemantics(semantics));
}

template <typename BaseRepr>
void InviwoModule::registerRepresentationConverter(
    std::unique_ptr<RepresentationConverter<BaseRepr>> converter) {

    if (auto factory = app_->getRepresentationConverterFactory<BaseRepr>()) {
        if (factory->registerObject(converter.get())) {
            representationConvertersUnRegFunctors_.push_back(
                [factory, conv = converter.get()]() { factory->unRegisterObject(conv); });
            representationConverters_.push_back(std::move(converter));
        }
    }
}

}  // namespace inviwo

#endif  // IVW_INVIWOMODULE_H
