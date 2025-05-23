/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2025 Inviwo Foundation
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
#include <inviwo/core/util/dispatcher.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/logcentral.h>
#include <inviwo/core/io/serialization/serializable.h>
#include <inviwo/core/util/inviwosetupinfo.h>

#include <iostream>
#include <string_view>
#include <vector>
#include <functional>
#include <filesystem>

namespace inviwo {

class FactoryBase;
class InviwoApplication;
class NetworkModified;

/**
 * The WorkspaceManager is responsible for clearing, loading, and saving a workspace. Different
 * items such as the processor network can register callbacks for clearing, loading, or saving a
 * workspace. Other object can also register callbacks. It also responsible for keeping a list of
 * factories to use while deserializing. Other module can register factories that should be used
 * while deserializing.
 * The user interface should use the Workspace Manager to clear, load, and save workspaces.
 * instead of calling the Processor Network directly.
 * The workspace manager is owned by the InviwoApplication.
 */
class IVW_CORE_API WorkspaceManager {
public:
    using ExceptionHandler = std::function<void(SourceContext)>;

private:
    using ClearDispatcher = Dispatcher<void()>;
    using SerializationDispatcher =
        Dispatcher<void(Serializer&, const ExceptionHandler&, WorkspaceSaveMode mode)>;
    using DeserializationDispatcher =
        Dispatcher<void(Deserializer&, const ExceptionHandler&, WorkspaceSaveMode mode)>;

    using ModifiedChangedDispatcher = Dispatcher<void(bool)>;
    using ModifiedDispatcher = Dispatcher<void(bool)>;

public:
    using ClearCallback = typename ClearDispatcher::Callback;
    using ClearHandle = typename ClearDispatcher::Handle;

    using SerializationCallback = std::function<void(Serializer&)>;
    using SerializationHandle = typename SerializationDispatcher::Handle;

    using DeserializationCallback = std::function<void(Deserializer&)>;
    using DeserializationHandle = typename DeserializationDispatcher::Handle;

    using ModifiedChangedCallback = typename ModifiedChangedDispatcher::Callback;
    using ModifiedChangedHandle = typename ModifiedChangedDispatcher::Handle;

    using ModifiedCallback = typename ModifiedDispatcher::Callback;
    using ModifiedHandle = typename ModifiedDispatcher::Handle;

    WorkspaceManager(InviwoApplication* app);
    ~WorkspaceManager();

    /**
     * Clear the current workspace. This will invoke all the clear callback that have been added.
     */
    void clear();

    /**
     * Save the current workspace to a stream
     * @param stream the stream to write to.
     * @param refPath a reference that can be use by the serializer to store relative paths.
     *      The same refPath should be given when loading. Most often this should be the path to the
     *      saved file.
     * @param exceptionHandler A callback for handling errors.
     * @param mode to indicate if we are saving to disk or undo-stack
     */
    void save(std::ostream& stream, const std::filesystem::path& refPath,
              const ExceptionHandler& exceptionHandler = StandardExceptionHandler(),
              WorkspaceSaveMode mode = WorkspaceSaveMode::Disk);

    /**
     * Save the current workspace to a stream
     * @param xml the string to write to.
     * @param refPath a reference that can be use by the serializer to store relative paths.
     *      The same refPath should be given when loading. Most often this should be the path to the
     *      saved file.
     * @param exceptionHandler A callback for handling errors.
     * @param mode to indicate if we are saving to disk or undo-stack
     */
    void save(std::pmr::string& xml, const std::filesystem::path& refPath,
              const ExceptionHandler& exceptionHandler = StandardExceptionHandler(),
              WorkspaceSaveMode mode = WorkspaceSaveMode::Disk);

    /**
     * Save the current workspace to a file
     * @param path the file to save into.
     * @param exceptionHandler A callback for handling errors.
     * @param mode to indicate if we are saving to disk or undo-stack
     */
    void save(const std::filesystem::path& path,
              const ExceptionHandler& exceptionHandler = StandardExceptionHandler(),
              WorkspaceSaveMode mode = WorkspaceSaveMode::Disk);

    /**
     * Load a workspace from a stream
     * @param stream the stream to read from.
     * @param refPath a reference that can be use by the deserializer to calculate relative
     *      paths. The same refPath should be given when loading. Most often this should be the
     *      path to the saved file.
     * @param exceptionHandler A callback for handling errors.
     * @param mode to indicate if we are saving to disk or undo-stack
     */
    void load(std::istream& stream, const std::filesystem::path& refPath,
              const ExceptionHandler& exceptionHandler = StandardExceptionHandler(),
              WorkspaceSaveMode mode = WorkspaceSaveMode::Disk);

    /**
     * Load a workspace from a string
     * @param xml the string to read from.
     * @param refPath a reference that can be use by the deserializer to calculate relative
     *      paths. The same refPath should be given when loading. Most often this should be the
     *      path to the saved file.
     * @param exceptionHandler A callback for handling errors.
     * @param mode to indicate if we are saving to disk or undo-stack
     */
    void load(const std::pmr::string& xml, const std::filesystem::path& refPath,
              const ExceptionHandler& exceptionHandler = StandardExceptionHandler(),
              WorkspaceSaveMode mode = WorkspaceSaveMode::Disk);

    /**
     * Load a workspace from a file
     * @param path the file to read from.
     * @param exceptionHandler A callback for handling errors.
     * @param mode to indicate if we are saving to disk or undo-stack
     */
    void load(const std::filesystem::path& path,
              const ExceptionHandler& exceptionHandler = StandardExceptionHandler(),
              WorkspaceSaveMode mode = WorkspaceSaveMode::Disk);

    /**
     * Callback for clearing the workspace.
     */
    ClearHandle onClear(const ClearCallback& callback);

    /**
     * Callback for saving the workspace.
     */
    SerializationHandle onSave(const SerializationCallback& callback,
                               WorkspaceSaveModes modes = WorkspaceSaveModes{flags::any});

    /**
     * Callback for loading the workspace.
     */
    DeserializationHandle onLoad(const DeserializationCallback& callback,
                                 WorkspaceSaveModes modes = WorkspaceSaveModes{flags::any});

    void setModified();
    bool isModified() const;

    ModifiedChangedHandle onModifiedChanged(const ModifiedChangedCallback& callback);
    ModifiedHandle onModified(const ModifiedCallback& callback);

    /**
     *	Register a factory that should be used by the workspace loading to create items.
     */
    void registerFactory(FactoryBase* factory);

    /**
     *	Create a deserializer for a workspace stream, and apply all needed version updates.
     */
    Deserializer createWorkspaceDeserializer(
        std::istream& stream, const std::filesystem::path& refPath,
        Logger* logger = LogCentral::getPtr(),
        std::pmr::polymorphic_allocator<std::byte> alloc = {}) const;

    /**
     *	Create a deserializer for a workspace stream, and apply all needed version updates.
     */
    std::pair<Deserializer, InviwoSetupInfo> createWorkspaceDeserializerAndInfo(
        std::istream& stream, const std::filesystem::path& refPath,
        Logger* logger = LogCentral::getPtr(),
        std::pmr::polymorphic_allocator<std::byte> alloc = {}) const;

    /**
     *	Create a deserializer for a workspace string, and apply all needed version updates.
     */
    std::pair<Deserializer, InviwoSetupInfo> createWorkspaceDeserializerAndInfo(
        const std::pmr::string& xml, const std::filesystem::path& refPath,
        Logger* logger = LogCentral::getPtr(),
        std::pmr::polymorphic_allocator<std::byte> alloc = {}) const;

    using allocator_type = std::pmr::polymorphic_allocator<std::byte>;

private:
    void configureWorkspaceDeserializerAndInfo(Deserializer& deserializer, InviwoSetupInfo& info,
                                               Logger* logger) const;

    void setModified(bool modified);
    InviwoApplication* app_;
    std::vector<FactoryBase*> registeredFactories_;

    ClearDispatcher clears_;
    SerializationDispatcher serializers_;
    DeserializationDispatcher deserializers_;

    bool modified_;
    ModifiedChangedDispatcher modifiedChangedDispatcher_;
    ModifiedDispatcher modifiedDispatcher_;
    std::unique_ptr<NetworkModified> networkModified_;
};

}  // namespace inviwo
