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

// ————————————————————————————————-
/**
 * @file
 * @brief
 * @author Gerolf Reinwardt
 * @date 30.01.2011
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

#include <inviwo/qt/editor/fileassociations.h>
#include <modules/qtwidgets/inviwoqtutils.h>

#include <inviwo/core/util/filesystem.h>

#include <warn/push>
#include <warn/ignore/all>

#ifdef WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
struct IUnknown;  // Workaround for "combaseapi.h(229): error C2187: syntax error: 'identifier' was
                  // unexpected here" when using /permissive-
#include <windows.h>
#include <dde.h>
#endif

#include <QMainWindow>
#include <QMessageBox>
#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QRegExp>
#include <warn/pop>

namespace inviwo {

#ifdef WIN32
class FileAssociationData {
public:
    FileAssociationData(FileAssociations& fa, QMainWindow* win) : fa_{fa}, win_{win} {
        QFileInfo fi(qApp->applicationFilePath());
        appAtomName_ = fi.baseName();

        appAtom_ = ::GlobalAddAtomW((const wchar_t*)appAtomName_.utf16());
        systemTopicAtom_ = ::GlobalAddAtomW((const wchar_t*)systemTopicAtomName_.utf16());
    }

    ~FileAssociationData() {
        if (0 != appAtom_) {
            ::GlobalDeleteAtom(appAtom_);
        }

        // remove dde commands when not running to avoid
        // "There was a problem sending the command to the program." errors
        for (const auto& ddeCommand : ddeCommandsToRemove_) {
            ::RegDeleteTree(HKEY_CURRENT_USER, (const wchar_t*)ddeCommand.utf16());
        }

        if (0 != systemTopicAtom_) {
            ::GlobalDeleteAtom(systemTopicAtom_);
        }
    }

    bool nativeEvent(void* message, long* result) {
        auto m = static_cast<MSG*>(message);
        switch (m->message) {
            case WM_DDE_INITIATE:
                return ddeInitiate(m, result);
                break;
            case WM_DDE_EXECUTE:
                return ddeExecute(m, result);
                break;
            case WM_DDE_TERMINATE:
                return ddeTerminate(m, result);
                break;
        }
        return false;
    };

    void registerFileType(const std::string& documentId, const std::string& fileTypeName,
                          const std::string& fileExtension, int appIconIndex,
                          const std::vector<FileAssociationCommand>& commands) {
        auto dId = utilqt::toQString(documentId);

        // first register the type ID of our server
        if (!SetHkcrUserRegKey(dId, utilqt::toQString(fileTypeName))) {
            LogError("Failed to register file type: " << fileTypeName << " for document type "
                                                      << documentId);
            return;
        }

        auto appPath = QDir::toNativeSeparators(qApp->applicationFilePath());
        appPath.prepend(QLatin1String("\""));
        appPath.append(QLatin1String("\""));
        if (!SetHkcrUserRegKey(QString("%1\\DefaultIcon").arg(dId),
                               QString("%1,%2").arg(appPath).arg(appIconIndex))) {
            return;
        }

        {
            // Remove any old shells we have set.
            auto shells = QString("Software\\Classes\\%1\\shell").arg(dId);
            ::RegDeleteTree(HKEY_CURRENT_USER, (const wchar_t*)shells.utf16());
        }
        for (const auto& command : commands) {
            auto cmd = utilqt::toQString(command.command_);
            if (!command.cmdLineArg_.empty()) {
                QString commandLine = appPath;
                commandLine.append(QChar(' '));
                commandLine.append(utilqt::toQString(command.cmdLineArg_));

                if (!SetHkcrUserRegKey(QString("%1\\shell\\%2\\command").arg(dId).arg(cmd),
                                       commandLine)) {
                    return;  // just skip it
                }
            }

            if (!command.ddeCommand_.empty()) {
                auto verb = QString("%1\\shell\\%2\\ddeexec").arg(dId).arg(cmd);
                if (!SetHkcrUserRegKey(verb, QString("[%1(\"%2\")]")
                                                 .arg(utilqt::toQString(command.ddeCommand_))
                                                 .arg("%1")))
                    return;

                ddeCommandsToRemove_.push_back("Software\\Classes\\" + verb);

                if (!SetHkcrUserRegKey(
                        QString("%1\\shell\\%2\\ddeexec\\application").arg(dId).arg(cmd),
                        appAtomName_))
                    return;

                if (!SetHkcrUserRegKey(QString("%1\\shell\\%2\\ddeexec\\topic").arg(dId).arg(cmd),
                                       systemTopicAtomName_))
                    return;
            }
        }

        auto ext = utilqt::toQString(fileExtension);

        LONG lSize = _MAX_PATH * 2;
        wchar_t szTempBuffer[_MAX_PATH * 2];
        LONG lResult =
            ::RegQueryValue(HKEY_CLASSES_ROOT, (const wchar_t*)ext.utf16(), szTempBuffer, &lSize);

        QString temp = QString::fromUtf16((unsigned short*)szTempBuffer);

        if (lResult != ERROR_SUCCESS || temp.isEmpty() || temp == dId) {
            // no association for that suffix
            SetHkcrUserRegKey(ext, dId);
        }
    }

private:
    // implementation of the WM_DDE_INITIATE windows message
    bool ddeInitiate(MSG* message, long* result) {
        if ((0 != LOWORD(message->lParam)) && (0 != HIWORD(message->lParam)) &&
            (LOWORD(message->lParam) == appAtom_) &&
            (HIWORD(message->lParam) == systemTopicAtom_)) {

            // make duplicates of the incoming atoms (really adding a reference)
            // wchar_t atomName[_MAX_PATH];
            // IVW_ASSERT(::GlobalGetAtomNameW(appAtom_, atomName, _MAX_PATH - 1) != 0, "");
            // IVW_ASSERT(::GlobalAddAtomW(atomName) == appAtom_, "");
            // IVW_ASSERT(::GlobalGetAtomNameW(systemTopicAtom_, atomName, _MAX_PATH - 1) != 0, "");
            // IVW_ASSERT(::GlobalAddAtomW(atomName) == systemTopicAtom_, "");

            // send the WM_DDE_ACK (caller will delete duplicate atoms)
            ::SendMessage((HWND)message->wParam, WM_DDE_ACK, (WPARAM)win_->winId(),
                          MAKELPARAM(appAtom_, systemTopicAtom_));
        }
        if (result) *result = 0;
        return true;
    }

    bool ddeExecute(MSG* message, long* result) {
        // unpack the DDE message
        UINT_PTR unused;
        HGLOBAL hData = nullptr;
        IVW_UNUSED_PARAM(unused);
        // IA64: Assume DDE LPARAMs are still 32-bit
        Q_ASSERT(::UnpackDDElParam(WM_DDE_EXECUTE, message->lParam, &unused, (UINT_PTR*)&hData));

        QString command = QString::fromWCharArray((LPCWSTR)::GlobalLock(hData));
        ::GlobalUnlock(hData);

        // acknowledge now - before attempting to execute
        ::PostMessage((HWND)message->wParam, WM_DDE_ACK, (WPARAM)win_->winId(),
                      // IA64: Assume DDE LPARAMs are still 32-bit
                      ReuseDDElParam(message->lParam, WM_DDE_EXECUTE, WM_DDE_ACK, (UINT)0x8000,
                                     (UINT_PTR)hData));

        // don't execute the command when the window is disabled
        if (!win_->isEnabled()) {
            if (result) *result = 0;
            return true;
        }

        QRegExp re("^\\[(\\w+)\\((.*)\\)\\]$");
        if (re.exactMatch(command)) {
            fa_.executeCommand(utilqt::fromQString(re.cap(1)), utilqt::fromQString(re.cap(2)));
        }

        if (result) *result = 0;
        return true;
    }

    bool ddeTerminate(MSG* message, long*) {
        // The client or server application should respond by posting a WM_DDE_TERMINATE message.
        ::PostMessageW((HWND)message->wParam, WM_DDE_TERMINATE, (WPARAM)win_->winId(),
                       message->lParam);
        return true;
    }

    /*
     * Sets specified value in the registry under HKCU\Software\Classes, which is mapped to HKCR
     * then.
     */
    bool SetHkcrUserRegKey(QString key, const QString& value,
                           const QString& valueName = QString()) {
        HKEY hKey;
        key.prepend("Software\\Classes\\");

        LONG lRetVal = RegCreateKey(HKEY_CURRENT_USER, (const wchar_t*)key.utf16(), &hKey);

        if (ERROR_SUCCESS == lRetVal) {
            LONG lResult = ::RegSetValueExW(
                hKey, valueName.isEmpty() ? 0 : (const wchar_t*)valueName.utf16(), 0, REG_SZ,
                (CONST BYTE*)value.utf16(), (value.length() + 1) * sizeof(wchar_t));

            if (::RegCloseKey(hKey) == ERROR_SUCCESS && lResult == ERROR_SUCCESS) return true;

            LogError("Error in setting Registry value: '"
                     << utilqt::fromQString(value) << "' for key '" << utilqt::fromQString(key)
                     << "'.");
        } else {
            wchar_t buffer[4096];
            ::FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, 0, lRetVal, 0, buffer, 4096, 0);
            QString szText = QString::fromUtf16((const ushort*)buffer);
            LogError("Error in setting Registry value: " << utilqt::fromQString(szText));
        }
        return false;
    }

    FileAssociations& fa_;
    QMainWindow* win_;
    std::vector<QString> ddeCommandsToRemove_;
    // the name of the application, without file extension
    QString appAtomName_ = "";
    // the name of the system topic atom, typically "System"
    QString systemTopicAtomName_ = "system";
    ATOM appAtom_ = 0;          // The windows atom needed for DDE communication
    ATOM systemTopicAtom_ = 0;  // The windows system topic atom needed for DDE communication
};

#else

// Dummy implementation for mac / linux
class FileAssociationData {
public:
    FileAssociationData(FileAssociations& fa, QMainWindow* win) {}

    bool nativeEvent(void* message, long* result) { return false; }
    void registerFileType(const std::string& documentId, const std::string& fileTypeName,
                          const std::string& fileExtension, int appIconIndex,
                          const std::vector<FileAssociationCommand>& commands) {}
};

#endif

FileAssociations::FileAssociations(QMainWindow* win)
    : data_{std::make_unique<FileAssociationData>(*this, win)} {}

FileAssociations::~FileAssociations() = default;

bool FileAssociations::nativeEventFilter(const QByteArray& eventType, void* message, long* result) {
    /*
     * The type of event eventType is specific to the platform plugin chosen at run-time, and can
     * be used to cast message to the right type.
     *
     * On X11, eventType is set to "xcb_generic_event_t", and the message can be casted to a
     * xcb_generic_event_t pointer.
     *
     * On Windows, eventType is set to "windows_generic_MSG" for
     * messages sent to toplevel windows, and "windows_dispatcher_MSG" for system-wide messages
     * such as messages from a registered hot key. In both cases, the message can be casted to a
     * MSG pointer. The result pointer is only used on Windows, and corresponds to the LRESULT
     * pointer.
     *
     * On macOS, eventType is set to "mac_generic_NSEvent", and the message can be casted to an
     * NSEvent pointer.
     *
     * In your reimplementation of this function, if you want to filter the message
     * out, i.e. stop it being handled further, return true; otherwise return false.
     */
    if (eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG") {
        return data_->nativeEvent(message, result);
    }
    return false;
}

void FileAssociations::registerFileType(const std::string& documentId,
                                        const std::string& fileTypeName,
                                        const std::string& fileExtension, int appIconIndex,
                                        std::vector<FileAssociationCommand> commands) {
    data_->registerFileType(documentId, fileTypeName, fileExtension, appIconIndex, commands);

    for (auto&& command : commands) {
        commands_[command.ddeCommand_] = std::move(command.callback_);
    }
}

void FileAssociations::executeCommand(const std::string& command, const std::string& param) {
    auto it = commands_.find(command);
    if (it != commands_.end()) {
        it->second(filesystem::cleanupPath(param));
    }
}

}  // namespace inviwo
