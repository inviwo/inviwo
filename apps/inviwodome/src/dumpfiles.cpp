/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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

#include <inviwo/dome/dumpfiles.h>

#ifdef WIN32
#include <Windows.h>
#include <dbghelp.h>
#endif

#include <fmt/format.h>

namespace inviwo {

#ifdef WIN32

LONG __stdcall generateMiniDump(EXCEPTION_POINTERS* exceptionPointers) {
    SYSTEMTIME stLocalTime;
    GetLocalTime(&stLocalTime);

    std::string dumpFile =
        fmt::format("InviwoDome-{}-{}-{}-{}-{}-{}--{}--{}.dmp", stLocalTime.wYear,
                    stLocalTime.wMonth, stLocalTime.wDay, stLocalTime.wHour, stLocalTime.wMinute,
                    stLocalTime.wSecond, GetCurrentProcessId(), GetCurrentThreadId());

    HANDLE hDumpFile =
        CreateFileA(dumpFile.c_str(), GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_WRITE | FILE_SHARE_READ, nullptr, CREATE_ALWAYS, 0, nullptr);

    MINIDUMP_EXCEPTION_INFORMATION exceptionParameter;
    exceptionParameter.ThreadId = GetCurrentThreadId();
    exceptionParameter.ExceptionPointers = exceptionPointers;
    exceptionParameter.ClientPointers = TRUE;

    BOOL success = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile,
                                     MiniDumpWithDataSegs, &exceptionParameter, nullptr, nullptr);

    CloseHandle(hDumpFile);

    return EXCEPTION_EXECUTE_HANDLER;
}

#endif

}  // namespace inviwo