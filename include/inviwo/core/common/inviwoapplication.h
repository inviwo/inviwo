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


namespace inviwo {

class ProcessorNetworkEvaluator;

/**
 * \class InviwoApplication
 *
 * \brief The main application which holds the instances of all modules.
 *
 * All modules should be owned and accessed trough this singleton, as well as the processor network and the evaluator.
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

    virtual void closeInviwoApplication() {LogWarn("this application have not implemented close inviwo function");}

    /**
     * Get the base path of the application.
     *
     * @return
     */
    const std::string& getBasePath() const { return basePath_; }

    /**
     * Get basePath +  pathType + suffix.
     * @see PathType
     * @param pathType Enum for type of path
     * @param suffix Path extension
     * @return basePath +  pathType + suffix
     */
    std::string getPath(PathType pathType, const std::string& suffix = "", const bool &createFolder = false);

    void registerModule(InviwoModule* module) { modules_.push_back(module); }
    const std::vector<InviwoModule*> &getModules() const { return modules_; }

    ProcessorNetwork* getProcessorNetwork() { return processorNetwork_; }
    ProcessorNetworkEvaluator* getProcessorNetworkEvaluator() { return processorNetworkEvaluator_; }

    template<class T> T* getSettingsByType();

    const CommandLineParser* getCommandLineParser() const { return commandLineParser_; }
    template<class T> T* getModuleByType();

    virtual void registerFileObserver(FileObserver* fileObserver) { //LogWarn("This Inviwo application does not support FileObservers."); 
    }
    virtual void startFileObservation(std::string fileName) { //LogWarn("This Inviwo application does not support FileObservers."); 
    }
    virtual void stopFileObservation(std::string fileName) {// LogWarn("This Inviwo application does not support FileObservers."); 
    }

    std::string getDisplayName()const {return displayName_;}

    enum MessageType {
        IVW_OK,
        IVW_ERROR
    };
    virtual void playSound(unsigned int soundID) { /*LogWarn("This Inviwo application does not support sound feedback.");*/ }

    /**
     * Creates a timer. Caller is responsible for deleting returned object.
     * @see Timer
     * @return new Timer
     */
    virtual Timer* createTimer() const;

    virtual void addCallbackAction(ModuleCallbackAction* callbackAction);

    virtual std::vector<ModuleCallbackAction*> getCallbackActions();

    std::vector<Settings*> getModuleSettings(size_t startIdx = 0);

protected:
    void printApplicationInfo();

private:
    std::string displayName_;

    std::string basePath_;

    std::vector<InviwoModule*> modules_;

    ProcessorNetwork* processorNetwork_;

    ProcessorNetworkEvaluator* processorNetworkEvaluator_;

    CommandLineParser* commandLineParser_;

    bool initialized_;

    std::vector<ModuleCallbackAction*> moudleCallbackActions_;
};

template<class T>
T* InviwoApplication::getSettingsByType() {
    T* settings = getTypeFromVector<T>(getModuleSettings());
    return settings;
}

template<class T>
T* InviwoApplication::getModuleByType() {
    T* module = getTypeFromVector<T>(getModules());
    return module;
}

} // namespace

#endif // IVW_INVIWOAPPLICATION_H
