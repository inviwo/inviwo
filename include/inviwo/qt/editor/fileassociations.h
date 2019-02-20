/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

// Windows implementation based on
// https://wiki.qt.io/Assigning_a_file_type_to_an_Application_on_Windows

// ————————————————————————————————-
/**
 * @file
 * @brief
 * @author Gerolf Reinwardt
 * @date 30. march 2011
 *
 * Copyright © 2011, Gerolf Reinwardt. All rights reserved.
 *
 * Simplified BSD License
 *
 * Redistribution and use in source and binary forms, with or without modification, are
 * permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY <COPYRIGHT HOLDER> ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are those of the
 * authors and should not be interpreted as representing official policies, either expressed
 * or implied, of Gerolf Reinwardt.
 */
// ————————————————————————————————-

#ifndef IVW_FILEASSOCIATIONS_H
#define IVW_FILEASSOCIATIONS_H

#include <inviwo/qt/editor/inviwoqteditordefine.h>
#include <inviwo/core/common/inviwo.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QAbstractNativeEventFilter>
#include <string>
#include <functional>
#include <warn/pop>

class QMainWindow;

namespace inviwo {

/*
 * Tools for creating file associations for inviwo.
 * At this point this is only supported under windows.
 */

struct IVW_QTEDITOR_API FileAssociationCommand {
    /**
     * Construct a FileAssociationCommand
     * @param command       Name of the command, this will be shown in the context menu in the
     *                      explorer for example.
     * @param cmdLineArg    The command line argument to pass to inviwo to invoke the command, the
     *                      passed file is available as '%1'. E.g. for 'open' we would pass '-w %1'
     * @param ddeCommand    The runtime command use to intercept the command when Inviwo is running
     * @param callback      A callback that will be called whenever the command is executed and
                            Inviwo is running
     */
    FileAssociationCommand(const std::string& command, const std::string& cmdLineArg,
                           const std::string& ddeCommand,
                           std::function<void(const std::string&)> callback)
        : command_{command}
        , cmdLineArg_{cmdLineArg}
        , ddeCommand_{ddeCommand}
        , callback_{callback} {}
    std::string command_;
    std::string cmdLineArg_;
    std::string ddeCommand_;
    std::function<void(const std::string&)> callback_;
};

class FileAssociationData;

class IVW_QTEDITOR_API FileAssociations : public QAbstractNativeEventFilter {
    friend FileAssociationData;

public:
    FileAssociations(QMainWindow* win);
    virtual ~FileAssociations();

    /**
     * Register a file type for opening with Inviwo
     *
     * @param documentId     Id of the document, e.g. "Inviwo.workspace"
     * @param fileTypeName   Name of the file type, e.g. "Inviwo Workspace"
     * @param fileExtension  File extension, including the dot (e.g. ".inv")
     * @param appIconIndex   Index of the app icon to use for the file in the windows explorer,
     *                       typically the application icon
     * @param commands       Vector FileAssociationCommands see @FileAssociationCommand
     */
    void registerFileType(const std::string& documentId, const std::string& fileTypeName,
                          const std::string& fileExtension, int appIconIndex = 0,
                          std::vector<FileAssociationCommand> commands = {});

    // QAbstractNativeEventFilter overrides
    bool nativeEventFilter(const QByteArray& eventType, void* message, long* result) override;

private:
    /**
     * This method is called to by the FileAssociationData on the execution of a ddeCommand
     */
    void executeCommand(const std::string& command, const std::string& param);

    std::unique_ptr<FileAssociationData> data_;
    std::unordered_map<std::string, std::function<void(const std::string&)>> commands_;
};

}  // namespace inviwo

#endif  // IVW_FILEASSOCIATIONS_H
