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

#ifndef IVW_INVIWOAPPLICATION_H
#define IVW_INVIWOAPPLICATION_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/common/inviwocore.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/common/moduleaction.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/util/commandlineparser.h>
#include <inviwo/core/util/fileobserver.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/vectoroperations.h>
#include <inviwo/core/util/settings/settings.h>
#include <inviwo/core/util/singleton.h>
#include <inviwo/core/util/timer.h>
#include <inviwo/core/util/threadpool.h>

#include <warn/push>
#include <warn/ignore/all>
#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <future>
#include <warn/pop>

namespace inviwo {

class ProcessorNetworkEvaluator;

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
    typedef void (*registerModuleFuncPtr)(InviwoApplication*);

    InviwoApplication();
    InviwoApplication(std::string displayName, std::string basePath);
    InviwoApplication(int argc, char** argv, std::string displayName, std::string basePath);
    virtual ~InviwoApplication();

    virtual void initialize(registerModuleFuncPtr);
    virtual void deinitialize();
    virtual bool isInitialized() { return initialized_; }

    enum PathType {
        PATH_DATA,               // data/
        PATH_VOLUMES,            // data/volumes/
        PATH_MODULES,            // modules/
        PATH_WORKSPACES,         // data/workspaces/
        PATH_SCRIPTS,            // data/workspaces/
        PATH_PORTINSPECTORS,     // data/workspaces/portinspectors/
        PATH_IMAGES,             // data/images/
        PATH_DATABASES,          // data/databases/
        PATH_RESOURCES,          // resources/
        PATH_TRANSFERFUNCTIONS,  // data/transferfunctions/
        PATH_SETTINGS,           //
        PATH_HELP                // data/help
    };

    virtual void closeInviwoApplication() {
        LogWarn("this application have not implemented close inviwo function");
    }

    /**
     * Get the base path of the application.
     *
     * @return
     */
    const std::string& getBasePath() const;

    std::string getDisplayName() const;

    /**
     * Get basePath +  pathType + suffix.
     * @see PathType
     * @param pathType Enum for type of path
     * @param suffix Path extension
     * @return basePath +  pathType + suffix
     */
    std::string getPath(PathType pathType, const std::string& suffix = "",
                        const bool& createFolder = false);

    void registerModule(InviwoModule* module);
    const std::vector<InviwoModule*>& getModules() const;

    ProcessorNetwork* getProcessorNetwork();
    ProcessorNetworkEvaluator* getProcessorNetworkEvaluator();

    template <class T>
    T* getSettingsByType();

    const CommandLineParser* getCommandLineParser() const;
    template <class T>
    T* getModuleByType();

    virtual void registerFileObserver(FileObserver* fileObserver) {}
    virtual void startFileObservation(std::string fileName) {}
    virtual void stopFileObservation(std::string fileName) {}

    enum class Message { Ok, Error };
    virtual void playSound(Message soundID) {}

    /**
     * Creates a timer. Caller is responsible for deleting returned object.
     * @see Timer
     * @return new Timer
     */
    virtual Timer* createTimer() const;

    virtual void addCallbackAction(ModuleCallbackAction* callbackAction);
    virtual std::vector<ModuleCallbackAction*> getCallbackActions();
    std::vector<Settings*> getModuleSettings(size_t startIdx = 0);

    void addNonSupportedTags(const Tags);
    bool checkIfAllTagsAreSupported(const Tags) const;

    template <class F, class... Args>
    auto dispatchPool(F&& f, Args&&... args)
        -> std::future<typename std::result_of<F(Args...)>::type>;

    template <class F, class... Args>
    auto dispatchFront(F&& f, Args&&... args)
        -> std::future<typename std::result_of<F(Args...)>::type>;

    virtual void processFront();


protected:
    void printApplicationInfo();
    void setPostEnqueueFront(std::function<void()> func);

private:
    struct Queue {
        // Task queue
        std::queue<std::function<void()> > tasks;
        // synchronization
        std::mutex mutex;

        // This is called after putting a task in the queue.
        std::function<void()> postEnqueue;
    };


    std::string displayName_;
    std::string basePath_;

    CommandLineParser commandLineParser_;
    bool initialized_;
    ThreadPool pool_;
    Queue queue_; // "Interaction/GUI" queue

    ProcessorNetwork* processorNetwork_;
    ProcessorNetworkEvaluator* processorNetworkEvaluator_;

    Tags nonSupportedTags_;
    std::vector<InviwoModule*> modules_;
    std::vector<ModuleCallbackAction*> moudleCallbackActions_;
};

template <class T>
T* InviwoApplication::getSettingsByType() {
    T* settings = getTypeFromVector<T>(getModuleSettings());
    return settings;
}

template <class T>
T* InviwoApplication::getModuleByType() {
    T* module = getTypeFromVector<T>(getModules());
    return module;
}

template <class F, class... Args>
auto InviwoApplication::dispatchPool(F&& f, Args&&... args)
    -> std::future<typename std::result_of<F(Args...)>::type> {
    return pool_.enqueue(std::forward<F>(f), std::forward<Args>(args)...);
}

template <class F, class... Args>
auto InviwoApplication::dispatchFront(F&& f, Args&&... args)
    -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;

    auto task = std::make_shared<std::packaged_task<return_type()> >(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_.mutex);
        queue_.tasks.emplace([task]() { (*task)(); });
    }

    if(queue_.postEnqueue) queue_.postEnqueue();
    return res;
}
template <class F, class... Args>
auto dispatchFront(F&& f, Args&&... args)
    -> std::future<typename std::result_of<F(Args...)>::type> {
    return InviwoApplication::getPtr()->dispatchFront(std::forward<F>(f),
                                                      std::forward<Args>(args)...);
}
template <class F, class... Args>
auto dispatchPool(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
    return InviwoApplication::getPtr()->dispatchPool(std::forward<F>(f),
                                                     std::forward<Args>(args)...);
}
template <class F, class... Args>
auto dispatchPoolAndInvalidate(Processor* p, F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
    using return_type = typename std::result_of<F(Args...)>::type;
    auto task = std::make_shared<std::packaged_task<return_type()> >(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));
    
    std::future<return_type> res = task->get_future();
    
    InviwoApplication::getPtr()->dispatchPool([task, p](){
        (*task)();
        dispatchFront([p](){if(p) p->invalidate(INVALID_OUTPUT); });
    });
    
    return res;
}



}  // namespace

#endif  // IVW_INVIWOAPPLICATION_H
