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
#include <inviwo/core/common/modulemanager.h>
#include <inviwo/core/common/runtimemoduleregistration.h>
#include <inviwo/core/util/singleton.h>
#include <inviwo/core/util/threadpool.h>
#include <inviwo/core/util/vectoroperations.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/pathtype.h>
#include <inviwo/core/util/dispatcher.h>
#include <inviwo/core/datastructures/representationfactory.h>
#include <inviwo/core/datastructures/representationmetafactory.h>
#include <inviwo/core/datastructures/representationconverterfactory.h>
#include <inviwo/core/datastructures/representationconvertermetafactory.h>
#include <inviwo/core/network/workspacemanager.h>
#include <inviwo/core/common/inviwoapplicationutil.h>

#include <warn/push>
#include <warn/ignore/all>
#include <queue>
#include <memory>
#include <mutex>
#include <future>
#include <locale>
#include <set>
#include <warn/pop>

namespace inviwo {

class ProcessorNetwork;
class ProcessorNetworkEvaluator;
class CommandLineParser;
struct AppResourceManagerObserver;

class ResourceManager;
class CameraFactory;
class DataReaderFactory;
class DataWriterFactory;
class MeshDrawerFactory;
class MetaDataFactory;
class ProcessorFactory;
class PropertyConverterManager;
class ProcessorWidgetFactory;
class DialogFactory;
class PropertyFactory;
class PropertyWidgetFactory;
class OutportFactory;
class InportFactory;
class PortInspectorFactory;
class PortInspectorManager;

class DataVisualizerManager;

class Settings;
class SystemSettings;
class Capabilities;
class InviwoModule;
class ModuleCallbackAction;
class FileObserver;

class Property;
class PropertyOwner;
class PropertyPresetManager;

class Processor;

class FileLogger;
class ConsoleLogger;

class TimerThread;
class FileSystemObserver;

class LayerRamResizer;

/**
 * \class InviwoApplication
 *
 * \brief The main application which holds the instances of all modules.
 *
 * All modules should be owned and accessed trough this singleton, as well as the processor network
 *and the evaluator.
 */
class IVW_CORE_API InviwoApplication : public Singleton<InviwoApplication> {
public:
    InviwoApplication();
    InviwoApplication(std::string_view displayName);
    InviwoApplication(int argc, char** argv, std::string_view displayName);
    InviwoApplication(const InviwoApplication& rhs) = delete;
    InviwoApplication& operator=(const InviwoApplication& that) = delete;

    virtual ~InviwoApplication();

    /**
     * \brief Registers modules from factories and takes ownership of input module factories.
     * Module is registered if dependencies exist and they have correct version.
     */
    void registerModules(std::vector<std::unique_ptr<InviwoModuleFactoryObject>> modules);

    /**
     * \brief Load modules from dynamic library files in the regular search paths.
     *
     * Will recursively search for all dll/so/dylib/bundle files in the regular search paths.
     * The library filename must contain "inviwo-module" to be loaded.
     *
     * @note Which modules to load can be specified by creating a file
     * (application_name-enabled-modules.txt) containing the names of the modules to load.
     * Forwards to ModuleManager.
     */
    void registerModules(RuntimeModuleLoading);

    void registerModules(RuntimeModuleLoading, std::function<bool(std::string_view)> isEnabled);

    /**
     * Get the base path of the application.
     * i.e. where the core data and modules folder and etc are.
     */
    std::filesystem::path getBasePath() const;

    /**
     * Get basePath + pathType + suffix.
     * @see PathType
     * @param pathType Enum for type of path
     * @param suffix Path extension
     * @param createFolder whether to create the folder if it does not exist.
     * @return basePath + pathType + suffix
     */
    std::filesystem::path getPath(PathType pathType, const std::string& suffix = "",
                                  const bool& createFolder = false);

    ModuleManager& getModuleManager();
    const ModuleManager& getModuleManager() const;
    
    template <class T>
    T* getModuleByType() const;
    InviwoModule* getModuleByIdentifier(const std::string& identifier) const;

    ProcessorNetwork* getProcessorNetwork();
    ProcessorNetworkEvaluator* getProcessorNetworkEvaluator();
    WorkspaceManager* getWorkspaceManager();
    PropertyPresetManager* getPropertyPresetManager();
    PortInspectorManager* getPortInspectorManager();
    DataVisualizerManager* getDataVisualizerManager();

    CommandLineParser& getCommandLineParser();
    const CommandLineParser& getCommandLineParser() const;
    /**
     * \brief Add an action that can be shown in for example property widget context menu.
     * Will be shown when right clicking on a property in the NetworkEditor.
     * Added callbacks will be removed in ~InviwoModule when your module is destroyed.
     * Example if you want to do it earlier:
     * @code
     * auto& callbackActions = app_->getCallbackActions();
     * std::erase_if(callbackActions, [&](auto& a) { return a->getModule() == this; });
     * @endcode
     * @see getCallbackActions
     */
    virtual void addCallbackAction(ModuleCallbackAction* callbackAction);

    /**
     * \brief Get list of ModuleCallbackAction shown in for example property widget context menu.
     * Do not keep references to elements in the list around since you cannot be notified when
     * they will be removed.
     * @see addCallbackAction
     */
    virtual std::vector<std::unique_ptr<ModuleCallbackAction>>& getCallbackActions();

    /**
     * Retrieve all Settings from all modules including the SystemSettings
     * @see Settings
     * @see InviwoModule
     */
    std::vector<Settings*> getModuleSettings();
    SystemSettings& getSystemSettings();
    template <class T>
    T* getSettingsByType();

    /**
     * Retrieve all Capabilities from all modules, and the system capabilities
     * @see Capabilities
     * @see InviwoModule
     */
    std::vector<Capabilities*> getModuleCapabilities();

    template <class T>
    T* getCapabilitiesByType();

    std::locale getUILocale() const;
    void setUILocale(const std::locale& locale);

    template <class F, class... Args>
    auto dispatchPool(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>;

    /**
     * Enqueue a functor to be run in the GUI thread
     * @returns a future with the result of the functor.
     */
    template <class F, class... Args>
    auto dispatchFront(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>;

    /**
     * Enqueue a functor to be run in the GUI thread.
     */
    void dispatchFrontAndForget(std::function<void()> fun);

    size_t processFront();

    void processEvents();

    void setProcessEventsCallback(std::function<void()> callback);

    /**
     * Get the current number of worker threads in the thread pool
     */
    size_t getPoolSize() const;

    enum class LongWait { Start, Update, End };
    /**
     * Set the number of worker threads in the thread pool. This will block for working threads to
     * finish
     */
    void resizePool(size_t newSize);
    void setPoolResizeWaitCallback(std::function<void(LongWait)> callback);

    void waitForPool();
    void setPostEnqueueFront(std::function<void()> func);
    void setProgressCallback(std::function<void(std::string)> progressCallback);

    ThreadPool& getThreadPool();

    /**
     * Returns the ResourceManager owned the InviwoApplication
     *
     * @see ResourceManager
     */
    ResourceManager* getResourceManager();

    /** @name Factories */
    ///@{

    /**
     * Camera factory
     * @see Camera
     * @see CameraFactory
     */
    CameraFactory* getCameraFactory() const;

    /**
     * DataReader factory
     * @see DataReader
     * @see DataReaderFactory
     */
    DataReaderFactory* getDataReaderFactory() const;

    /**
     * DataWriter factory
     * @see DataWriter
     * @see DataWriterFactory
     */
    DataWriterFactory* getDataWriterFactory() const;

    /**
     * Dialog factory
     * @see Dialog
     * @see DialogFactory
     */
    DialogFactory* getDialogFactory() const;

    /**
     * MeshDrawer factory
     * @see MeshDrawer
     * @see MeshDrawerFactory
     */
    MeshDrawerFactory* getMeshDrawerFactory() const;
    /**
     * MetaData factory
     * @see MetaData
     * @see MetaDataFactory
     */
    MetaDataFactory* getMetaDataFactory() const;

    /**
     * Inport factory
     * @see Inport
     * @see InportFactory
     */
    InportFactory* getInportFactory() const;

    /**
     * Outport factory
     * @see Outport
     * @see OutportFactory
     */
    OutportFactory* getOutportFactory() const;

    /**
     * PortInspector factory
     * @see PortInspector
     * @see PortInspectorFactory
     */
    PortInspectorFactory* getPortInspectorFactory() const;

    /**
     * Processor factory
     * @see Processor
     * @see ProcessorFactory
     */
    ProcessorFactory* getProcessorFactory() const;

    /**
     * ProcessorWidget factory
     * @see ProcessorWidget
     * @see ProcessorWidgetFactory
     */
    ProcessorWidgetFactory* getProcessorWidgetFactory() const;

    /**
     * PropertyConverterManager
     * @see PropertyConverter
     * @see PropertyConverterManager
     */
    PropertyConverterManager* getPropertyConverterManager() const;

    /**
     * Property factory
     * @see Property
     * @see PropertyFactory
     */
    PropertyFactory* getPropertyFactory() const;

    /**
     * PropertyWidget factory
     * @see PropertyWidget
     * @see PropertyWidgetFactory
     */
    PropertyWidgetFactory* getPropertyWidgetFactory() const;

    /**
     * Get a Representation factory for a specific kind of representation (Volume Representation,
     * Layer Representation, Buffer Representation, etc)
     * @see Data
     * @see DataRepresentation
     * @see RepresentationFactory
     */
    template <typename BaseRepr>
    RepresentationFactory<BaseRepr>* getRepresentationFactory() const;

    /**
     * The Representation Meta Factory holds RepresentationFactories for various kinds of
     * representations (Volume Representation, Layer Representation, Buffer Representation, etc)
     * @see Data
     * @see DataRepresentation
     * @see RepresentationFactory
     * @see RepresentationMetaFactory
     */
    RepresentationMetaFactory* getRepresentationMetaFactory() const;

    /**
     * Get a Representation converter factory for a specific kind of representation (Volume
     * Representation, Layer Representation, Buffer Representation, etc)
     * @see Data
     * @see RepresentationConverter
     * @see RepresentationConverterFactory
     */
    template <typename BaseRepr>
    RepresentationConverterFactory<BaseRepr>* getRepresentationConverterFactory() const;

    /**
     * The Representation Converter Meta Factory holds RepresentationConverterFactories for
     * various kinds of representations (Volume Representation, Layer Representation, Buffer
     * Representation, etc)
     * @see Data
     * @see DataRepresentation
     * @see RepresentationConverter
     * @see RepresentationConverterFactory
     * @see RepresentationConverterMetaFactory
     */
    RepresentationConverterMetaFactory* getRepresentationConverterMetaFactory() const;

    ///@}

    /**
     * Sets the a FileSystemObserver for the application
     * The FileObservers will register themselves in the FileSystemObserver of the
     * InviwoApplication by default. Usually one would set the FileSystemObserver in the main
     * function just after creating the app.
     * By default the FileSystemObserver is nullptr, and no file observation is provided.
     * @see FileSystemObserver
     * @see FileObservers
     */
    void setFileSystemObserver(std::unique_ptr<FileSystemObserver> observer);
    FileSystemObserver* getFileSystemObserver() const;

    TimerThread& getTimerThread();
    const std::string& getDisplayName() const;
    virtual void printApplicationInfo();
    void postProgress(std::string progress);

    /**
     * Get the current LayerRamResizer
     * @see LayerRamResizer CIMGLayerRamResizer LayerRAM::copyRepresentationsTo()
     */
    LayerRamResizer* getLayerRamResizer() const;

    /**
     * Allow a module the register a LayerRamResizer with the inviwoapplication.
     * The module is responsible for unregistering the LayerRamResizer before it is removed, by
     * calling `setLayerRamResizer(nullptr)`, if the current LayerRamResizer was registered by that
     * module.
     * @see CIMGLayerRamResizer CImgModule
     */
    void setLayerRamResizer(LayerRamResizer* obj);

protected:
    struct Queue {
        // Task queue
        std::queue<std::function<void()>> tasks;
        // synchronization
        std::mutex mutex;

        // This is called after putting a task in the queue.
        std::function<void()> postEnqueue;
    };

    std::string displayName_;
    std::unique_ptr<CommandLineParser> commandLineParser_;
    std::shared_ptr<ConsoleLogger> consoleLogger_;
    std::shared_ptr<FileLogger> filelogger_;
    std::function<void(std::string)> progressCallback_;
    std::unique_ptr<FileSystemObserver> fileSystemObserver_;

    std::locale uiLocale_{};
    std::function<void(LongWait)> poolResizeCallback_;
    std::function<void()> processEventsCallback_;
    ThreadPool pool_;
    Queue queue_;  // "Interaction/GUI" queue

    util::OnScopeExit clearAllSingeltons_;

    std::unique_ptr<ResourceManager> resourceManager_;

    // Factories
    std::unique_ptr<CameraFactory> cameraFactory_;
    std::unique_ptr<DataReaderFactory> dataReaderFactory_;
    std::unique_ptr<DataWriterFactory> dataWriterFactory_;
    std::unique_ptr<DialogFactory> dialogFactory_;
    std::unique_ptr<MeshDrawerFactory> meshDrawerFactory_;
    std::unique_ptr<MetaDataFactory> metaDataFactory_;
    std::unique_ptr<OutportFactory> outportFactory_;
    std::unique_ptr<InportFactory> inportFactory_;
    std::unique_ptr<PortInspectorFactory> portInspectorFactory_;
    std::unique_ptr<DataVisualizerManager> dataVisualizerManager_;
    std::unique_ptr<ProcessorFactory> processorFactory_;
    std::unique_ptr<ProcessorWidgetFactory> processorWidgetFactory_;
    std::unique_ptr<PropertyConverterManager> propertyConverterManager_;
    std::unique_ptr<PropertyFactory> propertyFactory_;
    std::unique_ptr<PropertyWidgetFactory> propertyWidgetFactory_;
    std::unique_ptr<RepresentationMetaFactory> representationMetaFactory_;
    std::unique_ptr<RepresentationConverterMetaFactory> representationConverterMetaFactory_;
    std::unique_ptr<SystemSettings> systemSettings_;
    std::unique_ptr<AppResourceManagerObserver> resourcemanagerobserver_;
    std::vector<std::unique_ptr<ModuleCallbackAction>> moduleCallbackActions_;
    ModuleManager moduleManager_;
    std::unique_ptr<ProcessorNetwork> processorNetwork_;
    std::unique_ptr<ProcessorNetworkEvaluator> processorNetworkEvaluator_;
    std::unique_ptr<WorkspaceManager> workspaceManager_;
    std::unique_ptr<PropertyPresetManager> propertyPresetManager_;
    std::unique_ptr<PortInspectorManager> portInspectorManager_;
    WorkspaceManager::ClearHandle networkClearHandle_;
    WorkspaceManager::SerializationHandle networkSerializationHandle_;
    WorkspaceManager::DeserializationHandle networkDeserializationHandle_;

    WorkspaceManager::ClearHandle presetsClearHandle_;
    WorkspaceManager::SerializationHandle presetsSerializationHandle_;
    WorkspaceManager::DeserializationHandle presetsDeserializationHandle_;
    std::unique_ptr<TimerThread> timerThread_;
    LayerRamResizer* layerRamResizer_;

private:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
    friend Singleton<InviwoApplication>;
    static InviwoApplication* instance_;
#endif
};

/**
 * Enqueue a functor to be run in the GUI thread
 * @returns a future with the result of the functor.
 */
template <class F, class... Args>
auto dispatchFront(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
    return InviwoApplication::getPtr()->dispatchFront(std::forward<F>(f),
                                                      std::forward<Args>(args)...);
}

/**
 * Enqueue a functor to be run in the GUI thread.
 */
inline void dispatchFrontAndForget(std::function<void()> fun) {
    InviwoApplication::getPtr()->dispatchFrontAndForget(std::move(fun));
}

template <class F, class... Args>
auto dispatchPool(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
    return InviwoApplication::getPtr()->dispatchPool(std::forward<F>(f),
                                                     std::forward<Args>(args)...);
}

template <class T>
T* InviwoApplication::getSettingsByType() {
    return getTypeFromVector<T>(getModuleSettings());
}

template <class T>
T* InviwoApplication::getModuleByType() const {
    return moduleManager_.getModuleByType<T>();
}

template <class T>
T* InviwoApplication::getCapabilitiesByType() {
    return getTypeFromVector<T>(getModuleCapabilities());
}

template <class F, class... Args>
auto InviwoApplication::dispatchPool(F&& f, Args&&... args)
    -> std::future<std::invoke_result_t<F, Args...>> {
    return pool_.enqueue(std::forward<F>(f), std::forward<Args>(args)...);
}

template <class F, class... Args>
auto InviwoApplication::dispatchFront(F&& f, Args&&... args)
    -> std::future<std::invoke_result_t<F, Args...>> {
    using return_type = std::invoke_result_t<F, Args...>;

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_.mutex);
        queue_.tasks.emplace([task]() { (*task)(); });
    }

    if (queue_.postEnqueue) queue_.postEnqueue();
    return res;
}

inline ResourceManager* InviwoApplication::getResourceManager() { return resourceManager_.get(); }

inline CameraFactory* InviwoApplication::getCameraFactory() const { return cameraFactory_.get(); }

inline DataReaderFactory* InviwoApplication::getDataReaderFactory() const {
    return dataReaderFactory_.get();
}

inline DataWriterFactory* InviwoApplication::getDataWriterFactory() const {
    return dataWriterFactory_.get();
}

inline DialogFactory* InviwoApplication::getDialogFactory() const { return dialogFactory_.get(); }

inline MeshDrawerFactory* InviwoApplication::getMeshDrawerFactory() const {
    return meshDrawerFactory_.get();
}

inline MetaDataFactory* InviwoApplication::getMetaDataFactory() const {
    return metaDataFactory_.get();
}

inline OutportFactory* InviwoApplication::getOutportFactory() const {
    return outportFactory_.get();
}

inline InportFactory* InviwoApplication::getInportFactory() const { return inportFactory_.get(); }

inline PortInspectorFactory* InviwoApplication::getPortInspectorFactory() const {
    return portInspectorFactory_.get();
}

inline ProcessorFactory* InviwoApplication::getProcessorFactory() const {
    return processorFactory_.get();
}

inline PropertyConverterManager* InviwoApplication::getPropertyConverterManager() const {
    return propertyConverterManager_.get();
}

inline PropertyFactory* InviwoApplication::getPropertyFactory() const {
    return propertyFactory_.get();
}

inline PropertyWidgetFactory* InviwoApplication::getPropertyWidgetFactory() const {
    return propertyWidgetFactory_.get();
}

inline RepresentationMetaFactory* InviwoApplication::getRepresentationMetaFactory() const {
    return representationMetaFactory_.get();
}

template <typename BaseRepr>
RepresentationFactory<BaseRepr>* InviwoApplication::getRepresentationFactory() const {
    return representationMetaFactory_->getRepresentationFactory<BaseRepr>();
}

template <typename BaseRepr>
RepresentationConverterFactory<BaseRepr>* InviwoApplication::getRepresentationConverterFactory()
    const {
    return representationConverterMetaFactory_->getConverterFactory<BaseRepr>();
}

inline RepresentationConverterMetaFactory*
InviwoApplication::getRepresentationConverterMetaFactory() const {
    return representationConverterMetaFactory_.get();
}

inline ProcessorWidgetFactory* InviwoApplication::getProcessorWidgetFactory() const {
    return processorWidgetFactory_.get();
}

}  // namespace inviwo
