/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#ifndef IVW_SINGLEFILEOBSERVER_H
#define IVW_SINGLEFILEOBSERVER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/util/fileobserver.h>
#include <inviwo/core/util/callback.h>

namespace inviwo {

/**
 * \class SingleFileObserver
 * \brief SingleFileObserver observes a single file for changes on disk.
 * SingleFileObserver observes a single file for changes on disk. When the application detects the
 * file has been modified it will invoke all lambda functions registered with onChange(...)
 */
class IVW_CORE_API SingleFileObserver : public FileObserver {
public:
    /**
     * Creates a file observer for filename and start observing that file
     */
    SingleFileObserver(std::string filename = "");
    virtual ~SingleFileObserver();

    void setFilename(const std::string& filename);
    const std::string& getFilename() const;

    /**
     * Start observing the file, not only needed to call if stop has been called in the first
     * place since the constructor calls start()
     */
    void start();
    /**
     * Stop observing the file. This is useful if you know the file will change and a callback is
     * not needed, for example if you write to it yourself or if you know it will be updated several
     * times
     */
    void stop();

    /**
     * Register a callback that will be called once the file has changed on disk.
     */
    const BaseCallBack* onChange(std::function<void()> callback);
    /**
     * Remove a callback from the list of callbacks
     */
    void removeOnChange(const BaseCallBack* callback);

private:
    virtual void fileChanged(const std::string& filename);

    std::string filename_;
    CallBackList onChangeCallbacks_;
};

}  // namespace inviwo

#endif  // IVW_SIMPLEFILEOBSERVER_H
