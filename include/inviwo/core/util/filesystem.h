/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
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

#include <vector>

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
 * Returns the file listing of a directory
 *
 * @param path Files are listed for this directory
 * @return List of files residing in the given path
 */
IVW_CORE_API std::vector<std::string> getDirectoryContents(const std::string& path);


/** 
 * Checks whether a given string matches a pattern. The pattern
 * might contain '*' matching any string including the empty string
 * and '?' matching a single character.
 * 
 * @param std::string pattern  The pattern used for matching, might contain '*' and '?'
 * @param std::string str      String which needs to be checked
 * @return True if the given string matches the pattern, false otherwise.
 */
IVW_CORE_API bool wildcardStringMatch(const std::string &pattern, const std::string &str);

/**
* Checks whether a given string matches a pattern including digits. 
* The pattern might contain a single sequence of '#' for indicating a number
* besides '*' matching any string including the empty string and '?' matching a 
* single character.
*
* The digit sequence indicated by '#' is extracted and returned. Depending on the 
* flags, the number have to exactly match sequence or might be shorter (matchLess)
* or longer (matchMore). For example, the sequence '###' matches only a three-digit
* number. Enabling 'matchLess' also matches one-digit and two-digit numbers whereas
* 'matchMore' allows for numbers with more digits.
*
* Examples:
*  * '###*.jpg' will match all jpeg files starting with a 3-digit sequence. Setting 
*       'matchMore = true' matches the same files, but might extract longer numbers.
*  * 'myfile#.png' matches all files containing exactly one digit with 'matchMore = false'.
*
* @param std::string pattern  The pattern used for matching, might contain a single 
*             sequence of '#' besides '*', and '?'
* @param std::string str      String which needs to be checked
* @param int index  if the match is successful, this index contains the extracted 
*             digit sequence indicated by '#'
* @param bool matchLess   allows to match digit sequences shorter than defined by the number of '#' (default false)
* @param bool matchMore   allows to match longer digit sequences (default true)
* @return True if the given string matches the pattern, false otherwise.
*/
IVW_CORE_API bool wildcardStringMatchDigits(const std::string &pattern, const std::string &str,
                                            int &index, bool matchLess=false, bool matchMore=true);

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

IVW_CORE_API void createDirectoryRecursively(std::string path);

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
