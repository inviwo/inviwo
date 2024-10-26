/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2024 Inviwo Foundation
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
#ifdef _MSC_VER
#pragma comment(linker, "/SUBSYSTEM:CONSOLE")
#endif

#include <inviwo/core/common/defaulttohighperformancegpu.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/network/workspacemanager.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/util/consolelogger.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/commandlineparser.h>
#include <inviwo/core/util/filesystem.h>

#include <inviwo/sys/moduleregistration.h>
#include <inviwo/sys/moduleloading.h>

#include <fmt/format.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <memory>

#include <benchmark/benchmark.h>

// #include <os/signpost.h>

using namespace inviwo;

std::thread::id main_id = std::this_thread::get_id();
int64_t allocCount{0};
int64_t deallocCount{0};
int64_t allocSize{0};

#if 1
void* operator new(size_t count) {
    if (main_id == std::this_thread::get_id()) {
        ++allocCount;
        allocSize += count;
    }

    return malloc(count);
}
void* operator new[](std::size_t count) {
    if (main_id == std::this_thread::get_id()) {
        ++allocCount;
        allocSize += count;
    }

    return malloc(count);
}
void operator delete(void* ptr) noexcept {
    if (main_id == std::this_thread::get_id()) {
        ++deallocCount;
    }
    free(ptr);
}
void operator delete[](void* ptr) noexcept {
    if (main_id == std::this_thread::get_id()) {
        ++deallocCount;
    }
    free(ptr);
}
#endif

struct Log {
    Log() : logCentral{}, log{std::make_shared<ConsoleLogger>()} {
        LogCentral::init(&logCentral);
        LogCentral::getPtr()->registerLogger(log);
        logCentral.setVerbosity(LogVerbosity::Error);
    }

    LogCentral logCentral{};
    std::shared_ptr<ConsoleLogger> log;
};

struct App {
    App() : log{}, inviwo{} {
        inviwo.printApplicationInfo();
        inviwo.setPostEnqueueFront([]() { glfwPostEmptyEvent(); });

        // Initialize all modules
        inviwo::util::registerModulesFiltered(
            inviwo.getModuleManager(), [](ModuleContainer& container) {
                return container.identifier().contains("qt") ||
                       container.identifier() == "basecl" || container.identifier() == "opencl";
            });
    }

    virtual ~App() {}

    Log log;
    InviwoApplication inviwo;
};

static std::unique_ptr<App> app{};

struct Fixture : ::benchmark::Fixture {
    void SetUp(::benchmark::State& state) {
        testAllocCount = allocCount;
        testDeallocCount = deallocCount;
        testAllocSize = allocSize;

        if (!app) {
            // log = os_log_create("org.inviwo", OS_LOG_CATEGORY_POINTS_OF_INTEREST);

            app = std::make_unique<App>();
            // app->inviwo.getProcessorNetwork()->lock();
        }
    }
    void TearDown(::benchmark::State& state) {
        const auto loc = std::locale("en_US.UTF-8");
        const auto str =
            fmt::format(loc,
                        "Test: alloc {:14L}, dealloc {:14L}, diff {:14L}, Size {:14L} "
                        "Total: alloc {:14L}, dealloc {:14L}, diff {:14L}, Size {:14L}",
                        allocCount - testAllocCount, deallocCount - testDeallocCount,
                        (allocCount - testAllocCount) - (deallocCount - testDeallocCount),
                        (allocSize - testAllocSize), allocCount, deallocCount,
                        allocCount - deallocCount, allocSize);
        fmt::println("{}", str);
    }

    int64_t testAllocCount{0};
    int64_t testDeallocCount{0};
    int64_t testAllocSize{0};

    // os_log_t log;
};

BENCHMARK_DEFINE_F(Fixture, Loading)(benchmark::State& st) {
    auto workspace = filesystem::getPath(PathType::Workspaces) / "boron.inv";
    // os_signpost_id_t signpost_id = os_signpost_id_generate(log);
    // assert(signpost_id != OS_SIGNPOST_ID_INVALID);
    for (auto _ : st) {
        // os_signpost_event_emit(log, signpost_id, "start");
        app->inviwo.getWorkspaceManager()->load(workspace);
    }
}

BENCHMARK_DEFINE_F(Fixture, Saving)(benchmark::State& st) {
    auto workspace = filesystem::getPath(PathType::Workspaces) / "boron.inv";
    app->inviwo.getWorkspaceManager()->load(workspace);
    for (auto _ : st) {
        std::pmr::string xml;
        xml.reserve(1024 * 32);
        app->inviwo.getWorkspaceManager()->save(xml, workspace);
        benchmark::DoNotOptimize(xml);
    }
}

// BENCHMARK_REGISTER_F(Fixture, Saving)->Unit(benchmark::kMicrosecond)->MinTime(10.0);
BENCHMARK_REGISTER_F(Fixture, Loading)->Unit(benchmark::kMicrosecond)->MinTime(10.0);

BENCHMARK_MAIN();
