/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2026 Inviwo Foundation
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

#include <inviwo/core/util/threadutil.h>
#include <inviwo/core/util/stringconversion.h>
#include <inviwo/core/common/inviwoapplication.h>

#ifdef WIN32
#include <windows.h>
#include <process.h>
#else
#include <pthread.h>
#include <unistd.h>
#endif

namespace inviwo {

void util::setThreadDescription(const std::string_view desc) {
#ifdef WIN32
    typedef HRESULT(WINAPI * SetThreadDescriptionFunc)(HANDLE hThread, PCWSTR threadDescription);
    // SetThreadDescription was introduced with Windows 10, version 1607
    // For that version and later Windows 10 version, the function should be in kernel32.dll
    static SetThreadDescriptionFunc setThreadDescription =
        reinterpret_cast<SetThreadDescriptionFunc>(
            GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "SetThreadDescription"));
    // some Windows versions (e.g. 2016 Server) have the function in KernelBase.dll
    if (!setThreadDescription) {
        setThreadDescription = reinterpret_cast<SetThreadDescriptionFunc>(
            GetProcAddress(GetModuleHandle(TEXT("KernelBase.dll")), "SetThreadDescription"));
    }
    if (setThreadDescription) {
        setThreadDescription(::GetCurrentThread(), util::toWstring(desc).c_str());
    }
#elif defined(__APPLE__)
    // Apple only supports setting the name on the same thread
    const std::string name(desc.substr(0, 15));  // limit to 15 + null characters
    pthread_setname_np(name.c_str());
#else
    const std::string name(desc.substr(0, 15));  // limit to 15 + null characters
    pthread_setname_np(pthread_self(), name.c_str());
#endif
}

std::string util::getThreadDescription() {
#ifdef WIN32
    typedef HRESULT(WINAPI * GetThreadDescriptionFunc)(HANDLE hThread, PWSTR * threadDescription);
    // GetThreadDescription was introduced with Windows 10, version 1607
    // For that version and later Windows 10 version, the function should be in kernel32.dll
    static GetThreadDescriptionFunc getThreadDescription =
        reinterpret_cast<GetThreadDescriptionFunc>(
            GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetThreadDescription"));
    // some Windows versions (e.g. 2016 Server) have the function in KernelBase.dll
    if (!getThreadDescription) {
        getThreadDescription = reinterpret_cast<GetThreadDescriptionFunc>(
            GetProcAddress(GetModuleHandle(TEXT("KernelBase.dll")), "GetThreadDescription"));
    }
    if (getThreadDescription) {
        PWSTR wstr = nullptr;
        if (SUCCEEDED(getThreadDescription(::GetCurrentThread(), &wstr))) {
            std::string desc = util::fromWstring(wstr);
            LocalFree(wstr);
            return desc;
        }
    }
    return "";
#else
    std::array<char, 16> name = {0};
    pthread_getname_np(pthread_self(), name.data(), name.size());
    return std::string{name.data()};
#endif
}

int util::getPid() {
#ifdef WIN32
    return _getpid();
#else
    return getpid();
#endif
}

ThreadPool& util::getThreadPool() { return getThreadPool(InviwoApplication::getPtr()); }

ThreadPool& util::getThreadPool(InviwoApplication* app) { return app->getThreadPool(); }

void util::waitForPool(InviwoApplication* app) { app->waitForPool(); }

void util::waitForPool() { waitForPool(InviwoApplication::getPtr()); }

size_t util::getPoolSize() {
    if (InviwoApplication::isInitialized()) {
        return InviwoApplication::getPtr()->getPoolSize();
    }
    return 0u;
}

size_t util::processFront() { return processFront(InviwoApplication::getPtr()); }
size_t util::processFront(InviwoApplication* app) { return app->processFront(); }

void util::dispatchFrontAndForget(std::function<void()> fun) {
    dispatchFrontAndForget(InviwoApplication::getPtr(), std::move(fun));
}
void util::dispatchFrontAndForget(InviwoApplication* app, std::function<void()> fun) {
    app->dispatchFrontAndForget(std::move(fun));
}

}  // namespace inviwo
