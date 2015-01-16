/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2014-2015 Inviwo Foundation
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

#ifndef IVW_FILE_SYSTEM_H
#define IVW_FILE_SYSTEM_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

namespace inviwo {

namespace filesystem {

/**
 * Get the working directory of the application.
 *
 * @note getBasePath should be used in the framework in general.
 * @see getBasePath
 * @return Full path to working directory.
 */
IVW_CORE_API std::string getWorkingDirectory();

/**
 * \brief Check if a file exists.
 * @see directoryExists for directories
 * @param std::string fileName The path to the file
 * @return bool True if file exists, false otherwise
 */
IVW_CORE_API bool fileExists(const std::string& filePath);

/**
 * Check if the directory exists
 *
 * @see fileExists for files
 * @param path Directory path
 * @return bool True if directory exists, false otherwise
 */
IVW_CORE_API bool directoryExists(const std::string& path);

/**
 * Searches all parent folders of path and looks for parentFolder.
 *
 * @param path Folders to search, for example C:/a/b/c
 * @param parentFolder Folder to find.
 * @return The directory where the folder was found if found, otherwise path.
 */
IVW_CORE_API std::string getParentFolderPath(const std::string& path,
                                             const std::string& parentFolder);

/**
 * Finds base path which contains subfolders such as "data" and "modules" where external files are
 *stored
 *
 * @return The directory considered to be the basePath.
 */
IVW_CORE_API std::string findBasePath();

IVW_CORE_API void createDirectoryRecursivly(std::string path);

/**
 * Get inviwo settings folder for current user
 * Will for instance be AppData/Inviwo/ on windows.
 *
 * @return std::string Inviwo user settings folder
 */
IVW_CORE_API std::string getInviwoUserSettingsPath();

/**
 * \brief Adds the InviwoApplication base path before the url
 *
 * @see InviwoApplication::getBasePath
 * @param const std::string url Relative path
 * @return std::string InviwoApplication base path + url
 */
IVW_CORE_API std::string addBasePath(const std::string& url);

IVW_CORE_API std::string getFileDirectory(const std::string& url);
IVW_CORE_API std::string getFileNameWithExtension(const std::string& url);
IVW_CORE_API std::string getFileNameWithoutExtension(const std::string& url);
IVW_CORE_API std::string getFileExtension(const std::string& url);
IVW_CORE_API std::string replaceFileExtension(const std::string& url,
                                              const std::string& newFileExtension);
IVW_CORE_API std::string getRelativePath(const std::string& basePath,
                                         const std::string& absolutePath);
IVW_CORE_API bool isAbsolutePath(const std::string& path);
/**
 * \brief Checks whether the second path is on the same drive as the first path
 *
 * If both paths are relative, this function returns true. If only refPath is relative
 * InviwoApplication::getBasePath is used instead as reference.
 *
 * @param const std::string& refPath reference path, if relative then InviwoApplication::getBasePath
 * is used instead
 * @param const std::string& queryPath path to be checked
 * @return true if queryPath and refPath are located on the same drive (on Windows), always true on
 * all other systems
 */
IVW_CORE_API bool sameDrive(const std::string& refPath, const std::string& queryPath);

}  // namespace

}  // namespace

#endif  // IVW_FILE_SYSTEM_H
