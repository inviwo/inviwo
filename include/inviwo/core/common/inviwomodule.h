/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2023 Inviwo Foundation
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

#pragma once

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/factoryutil.h>
#include <inviwo/core/common/modulepath.h>
#include <inviwo/core/datastructures/camera/camerafactoryobject.h>
#include <inviwo/core/datastructures/representationfactory.h>
#include <inviwo/core/datastructures/representationfactoryobject.h>
#include <inviwo/core/datastructures/representationmetafactory.h>
#include <inviwo/core/datastructures/representationconverter.h>
#include <inviwo/core/datastructures/representationconverterfactory.h>
#include <inviwo/core/datastructures/representationconvertermetafactory.h>
#include <inviwo/core/ports/portfactoryobject.h>
#include <inviwo/core/ports/datainport.h>
#include <inviwo/core/ports/dataoutport.h>
#include <inviwo/core/processors/processorfactoryobject.h>
#include <inviwo/core/processors/processorwidgetfactory.h>
#include <inviwo/core/processors/processorwidgetfactoryobject.h>
#include <inviwo/core/processors/compositesink.h>
#include <inviwo/core/processors/compositesource.h>
#include <inviwo/core/properties/propertyfactoryobject.h>
#include <inviwo/core/properties/propertywidgetfactory.h>
#include <inviwo/core/properties/propertywidgetfactoryobject.h>
#include <inviwo/core/util/dialogfactoryobject.h>
#include <inviwo/core/util/demangle.h>

#include <type_traits>
#include <fmt/core.h>

#include <vector>
#include <memory>

namespace inviwo {

class InviwoApplication;
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
    InviwoModule(InviwoModule&&) = delete;
    InviwoModule& operator=(InviwoModule&&) = delete;
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
    virtual std::filesystem::path getPath() const;
    std::filesystem::path getPath(ModulePath type) const;

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

    const std::vector<BaseRepresentationFactoryObject*> getRepresentationFactoryObjects() const;
    const std::vector<BaseRepresentationFactory*> getRepresentationFactories() const;

    const std::vector<BaseRepresentationConverter*> getRepresentationConverters() const;
    const std::vector<BaseRepresentationConverterFactory*> getRepresentationConverterFactories()
        const;
    const std::vector<Settings*>& getSettings() const;

    void registerCapabilities(std::unique_ptr<Capabilities> info);

    void registerCamera(std::unique_ptr<CameraFactoryObject> camera);
    template <typename T>
    void registerCamera(const std::string& classIdentifier);
    void registerDataReader(std::unique_ptr<DataReader> reader);
    void registerDataWriter(std::unique_ptr<DataWriter> writer);

    void registerDialog(std::unique_ptr<DialogFactoryObject> dialog);
    template <typename T>
    void registerDialog(std::string classIdentifier);
    void registerDrawer(std::unique_ptr<MeshDrawer> drawer);
    void registerMetaData(std::unique_ptr<MetaData> meta);

    void registerProcessor(std::unique_ptr<ProcessorFactoryObject> pfo);
    template <typename T>
    void registerProcessor();

    /**
     * Register a workspace file as a CompositeProcessor.
     * The CompositeProcessor will load the file as its sub network on construction.
     */
    void registerCompositeProcessor(const std::filesystem::path& file);

    void registerProcessorWidget(std::unique_ptr<ProcessorWidgetFactoryObject> widget);
    template <typename T, typename P>
    void registerProcessorWidget();

    void registerPortInspector(std::string portClassIdentifier,
                               const std::filesystem::path& inspectorPath);

    void registerDataVisualizer(std::unique_ptr<DataVisualizer> visualizer);

    void registerInport(std::unique_ptr<InportFactoryObject> inport);
    void registerOutport(std::unique_ptr<OutportFactoryObject> outport);

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

    void registerProperty(std::unique_ptr<PropertyFactoryObject> property);
    template <typename T>
    void registerProperty();

    void registerPropertyWidget(std::unique_ptr<PropertyWidgetFactoryObject> propertyWidget);

    template <typename T, typename P>
    void registerPropertyWidget(PropertySemantics semantics);
    template <typename T, typename P>
    void registerPropertyWidget(std::string semantics);

    void registerPropertyConverter(std::unique_ptr<PropertyConverter> propertyConverter);

    /**
     * Register a representation factory object for creating representations with the respective
     * representation factory. The template type BaseRepr is used to select representation
     * factory. A representation factory object should implement RepresentationFactoryObject
     * @see RepresentationFactory
     * @see RepresentationFactoryObject
     * @see DataRepresentation
     * @see InviwoApplication::getRepresentationFactory()
     */
    template <typename BaseRepr>
    void registerRepresentationFactoryObject(
        std::unique_ptr<RepresentationFactoryObject<BaseRepr>> representation);

    /**
     * Register a factory for representations. Each base representation (Volume
     * Representation, Layer Representation, Buffer Representation, etc) has its own representation
     * factory. A representation factory should implement RepresentationFactory
     * @see RepresentationFactory
     * @see DataRepresentation
     * @see InviwoApplication::getRepresentationMetaFactory()
     */
    void registerRepresentationFactory(
        std::unique_ptr<BaseRepresentationFactory> representationFactory);

    /**
     * Register a representation converter with the respective representation converter factory.
     * The template type BaseRepr is used to select representation converter factory.
     * A representation converter should implement RepresentationConverterType
     * @see RepresentationConverterFactory
     * @see RepresentationConverter
     * @see DataRepresentation
     * @see InviwoApplication::getRepresentationConverterFactory()
     */
    template <typename BaseRepr>
    void registerRepresentationConverter(
        std::unique_ptr<RepresentationConverter<BaseRepr>> converter);

    /**
     * Register a factory for representation converters. Each base representation (Volume
     * Representation, Layer Representation, Buffer Representation, etc) has its own representation
     * converter factory. A converter factory should implement RepresentationConverterFactory
     * @see RepresentationConverterFactory
     * @see RepresentationConverter
     * @see DataRepresentation
     * @see InviwoApplication::getRepresentationConverterMetaFactory()
     */
    void registerRepresentationConverterFactory(
        std::unique_ptr<BaseRepresentationConverterFactory> converterFactory);

    /**
     * Register a Settings class and hand over ownership.
     * @see Settings
     * @see InviwoApplication::getModuleSettings()
     */
    void registerSettings(std::unique_ptr<Settings> settings);

    /**
     * Register a Settings class.
     * @see Settings
     * @see InviwoApplication::getModuleSettings()
     */
    void registerSettings(Settings* settings);

    InviwoApplication* getInviwoApplication() const;

protected:
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
        registerInport(std::make_unique<InportFactoryObjectTemplate<T>>());
    }

    template <typename T,
              typename std::enable_if<std::is_base_of<Outport, T>::value, int>::type = 0>
    void registerPortInternal() {
        registerOutport(std::make_unique<OutportFactoryObjectTemplate<T>>());
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

    std::vector<std::unique_ptr<BaseRepresentationFactoryObject>> representationFactoryObjects_;
    std::vector<std::function<void()>> representationUnRegFunctors_;
    std::vector<std::unique_ptr<BaseRepresentationFactory>> representationFactories_;

    std::vector<std::unique_ptr<BaseRepresentationConverter>> representationConverters_;
    std::vector<std::function<void()>> representationConvertersUnRegFunctors_;

    std::vector<std::unique_ptr<BaseRepresentationConverterFactory>>
        representationConverterFactories_;

    std::vector<std::unique_ptr<Settings>> ownedSettings_;
    std::vector<Settings*> settings_;
    std::vector<std::unique_ptr<DataVisualizer>> dataVisualizers_;
};

template <typename T>
void InviwoModule::registerCamera(const std::string& classIdentifier) {
    registerCamera(std::make_unique<CameraFactoryObjectTemplate<T>>(classIdentifier));
}

template <typename T>
void InviwoModule::registerDialog(std::string classIdentifier) {
    registerDialog(std::make_unique<DialogFactoryObjectTemplate<T>>(classIdentifier));
}

template <typename T>
void InviwoModule::registerProcessor() {
    registerProcessor(std::make_unique<ProcessorFactoryObjectTemplate<T>>());
}

template <typename T, typename P>
void InviwoModule::registerProcessorWidget() {
    registerProcessorWidget(std::make_unique<ProcessorWidgetFactoryObjectTemplate<T, P>>());
}

template <typename T>
void InviwoModule::registerPort() {
    static_assert(std::is_base_of<Inport, T>::value || std::is_base_of<Outport, T>::value,
                  "A port has to derive from either Inport of Outport");
    try {
        registerPortInternal<T>();
    } catch (const Exception& e) {
        LogError(fmt::format(
            "Error registering port '{0}' in module {1}. Reason: {2}. Have you provided a "
            "DataTraits<{0}> specialization?",
            util::parseTypeIdName(typeid(typename T::type).name()), getIdentifier(),
            e.getMessage()));
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
    registerProperty(std::make_unique<PropertyFactoryObjectTemplate<T>>());
}
template <typename T, typename P>
void InviwoModule::registerPropertyWidget(PropertySemantics semantics) {
    registerPropertyWidget(std::make_unique<PropertyWidgetFactoryObjectTemplate<T, P>>(semantics));
}
template <typename T, typename P>
void InviwoModule::registerPropertyWidget(std::string semantics) {
    registerPropertyWidget<T, P>(PropertySemantics(semantics));
}

template <typename BaseRepr>
void InviwoModule::registerRepresentationConverter(
    std::unique_ptr<RepresentationConverter<BaseRepr>> converter) {

    if (auto metaFactory = util::getRepresentationConverterMetaFactory(app_)) {
        if (auto factory = metaFactory->getConverterFactory<BaseRepr>()) {
            if (factory->registerObject(converter.get())) {
                representationConvertersUnRegFunctors_.push_back(
                    [factory, conv = converter.get()]() { factory->unRegisterObject(conv); });
                representationConverters_.push_back(std::move(converter));
            }
        }
    }
}

template <typename BaseRepr>
void InviwoModule::registerRepresentationFactoryObject(
    std::unique_ptr<RepresentationFactoryObject<BaseRepr>> representation) {

    if (auto metaFactory = util::getRepresentationMetaFactory(app_)) {
        if (auto factory = metaFactory->getRepresentationFactory<BaseRepr>()) {
            if (factory->registerObject(representation.get())) {
                representationUnRegFunctors_.push_back(
                    [factory, repr = representation.get()]() { factory->unRegisterObject(repr); });
                representationFactoryObjects_.push_back(std::move(representation));
            }
        }
    }
}

}  // namespace inviwo
