/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2023 Inviwo Foundation
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
#include <inviwo/core/util/pathtype.h>

#include <vector>
#include <fstream>
#include <cstdio>
#include <ctime>
#include <optional>
#include <filesystem>
#include <span>

namespace inviwo {

namespace filesystem {

/**
 * Creates and returns a FILE pointer for the given file name (utf-8 encoded). The call
 * `auto f = filesystem::fopen(filename, mode);` is functionally equivalent to the
 * statement `fopen(filename, mode);` or `_wfopen();`, respectively.
 * No checks whether the file exists or was successfully opened are performed. That is the caller
 * has to check it. For more details check the documentation of fopen.
 *
 * Since all strings within Inviwo are utf-8 encoded, this function should be used to create a
 * file handle when reading from/writing to files.
 *
 * On Windows, the file name is first converted from a utf-8 string to std::wstring and then the
 * file handle is created using the std::wstring as fopen(const char*) does not support utf-8.
 *
 * @param filename  utf-8 encoded string
 * @param mode      mode to open the file (input or output)
 * @return file handle for the given file, i.e. `fopen(filename, mode);`
 *
 * \see fopen, _wfopen
 */
IVW_CORE_API FILE* fopen(const std::filesystem::path& filename, const char* mode);

/**
 * Creates and returns a std::fstream for the given file name (utf-8 encoded). The call
 * `auto f = filesystem::fstream(filename, mode);` is functionally equivalent to the
 * statement `std::fstream f(filename, mode);`.
 * No checks whether the file exists or was successfully opened are performed. That is the caller
 * has to check it. For more details check the documentation of std::fstream.
 *
 * Since all strings within Inviwo are utf-8 encoded, this function should be used to create a
 * stream when reading from/writing to files using streams.
 *
 * On Windows, the file name is first converted from a utf-8 string to std::wstring and then the
 * stream is created using the std::wstring as std::fstream(std::string) does not support utf-8.
 *
 * @param filename  utf-8 encoded string
 * @param mode      mode to open the file (input or output)
 * @return stream for the given file, i.e. `std::fstream(filename, mode);`
 *
 * \see std::fstream
 */
[[deprecated("use std::fstream")]] IVW_CORE_API std::fstream fstream(
    const std::filesystem::path& filename,
    std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out);

/**
 * Creates and returns a std::ifstream for the given file name (utf-8 encoded). The call
 * `auto in = filesystem::ifstream(filename, mode);` is functionally equivalent to the
 * statement `std::ifstream in(filename, mode);`.
 * No checks whether the file exists or was successfully opened are performed. That is the caller
 * has to check it. For more details check the documentation of std::ifstream.
 *
 * Since all strings within Inviwo are utf-8 encoded, this function should be used to create a
 * stream when reading from files using streams.
 *
 * On Windows, the file name is first converted from a utf-8 string to std::wstring and then the
 * stream is created using the std::wstring as std::ifstream(std::string) does not support utf-8.
 *
 * @param filename   utf-8 encoded string
 * @param mode       mode to open the file (input or output)
 * @return stream for the given file, i.e. `std::ifstream(filename, mode);`
 *
 * \see std::ifstream
 */
[[deprecated("use std::ifstream")]] IVW_CORE_API std::ifstream ifstream(
    const std::filesystem::path& filename, std::ios_base::openmode mode = std::ios_base::in);

/**
 * Creates and returns a std::ofstream for the given file name (utf-8 encoded). The call
 * `auto out = filesystem::ofstream(filename, mode);` is functionally equivalent to the
 * statement `std::ofstream out(filename, mode);`.
 * No checks whether the file exists or was successfully opened are performed. That is the caller
 * has to check it. For more details check the documentation of std::ofstream.
 *
 * Since all strings within Inviwo are utf-8 encoded, this function should be used to create a
 * stream when writing to files using streams.
 *
 * On Windows, the file name is first converted from a utf-8 string to std::wstring and then the
 * stream is created using the std::wstring as std::ofstream(std::string) does not support utf-8.
 *
 * @param filename   utf-8 encoded string
 * @param mode       mode to open the file (input or output)
 * @return stream for the given file, i.e. `std::ofstream(filename, mode);`
 *
 * \see std::ofstream
 */
[[deprecated("use std::ofstream")]] IVW_CORE_API std::ofstream ofstream(
    const std::filesystem::path& filename, std::ios_base::openmode mode = std::ios_base::out);

/**
 * Detects the UTF-8 byte order mark (BOM) and skips it if it exists.
 * Reads the first three characters to determine if the BOM exists.
 * Rewinds stream if no BOM exists and otherwise leaves the stream position
 * after the three BOM characters.
 * @param stream stream to check and potentially modify.
 * @return true if byte order mark was found, false otherwise
 */
IVW_CORE_API bool skipByteOrderMark(std::istream& stream);

/**
 * Get the working directory of the application.
 *
 * @note getBasePath should be used in the framework in general.
 * @see getBasePath
 * @return Full path to working directory.
 */
IVW_CORE_API std::filesystem::path getWorkingDirectory();

/**
 * Set the working directory of the application.
 */
IVW_CORE_API void setWorkingDirectory(const std::filesystem::path& path);

/**
 * Get full/path/to/executable running the application.
 * @return Full path to the executable if successful, empty string otherwise.
 */
IVW_CORE_API std::filesystem::path getExecutablePath();

IVW_CORE_API std::filesystem::path getInviwoBinDir();

IVW_CORE_API std::vector<std::filesystem::path> getLoadedLibraries();

IVW_CORE_API int getCurrentProcessId();


/**
 * Get path to the user settings / data folder for Inviwo,
 * i.e. a folder where we have write-access.
 * Will be:
 * - Windows: /AppData/Inviwo/
 * - Linux:   /home/.inviwo
 * - Mac:     /Library/Application Support/org.inviwo.network-editor
 *
 * @return Path to user settings folder
 */
IVW_CORE_API std::filesystem::path getInviwoUserSettingsPath();

/**
 * \brief Check if a file exists.
 * @see directoryExists for directories
 * @param filePath The path to the file
 * @return true if file exists, false otherwise
 */
[[deprecated("use std::filesystem::is_regular_file")]] IVW_CORE_API bool fileExists(
    const std::filesystem::path& filePath);

/**
 * Check if the directory exists
 *
 * @see fileExists for files
 * @param path Directory path
 * @return True if directory exists, false otherwise
 */
[[deprecated("use std::filesystem::is_directory")]] IVW_CORE_API bool directoryExists(
    const std::filesystem::path& path);

/**
 * \brief Get last time file was modified.
 * Error can occur if the file does not exist for example.
 * @param filePath The path to the file
 * @return Time of last modification, or 0 if an error occured (00:00, Jan 1 1970 UTC).
 */
[[deprecated("use std::filesystem::last_write_time")]] IVW_CORE_API std::time_t
fileModificationTime(const std::filesystem::path& filePath);

/**
 * \brief Copy an existing file to a new file. Overwrites existing file.
 * @param src Path to the file to the existing file
 * @param dst Path to the new file
 * @return True if source file exists and the copy is successful, false otherwise
 */
[[deprecated("use std::filesystem::copy")]] IVW_CORE_API bool copyFile(
    const std::filesystem::path& src, const std::filesystem::path& dst);

enum class ListMode {
    Files,
    Directories,
    FilesAndDirectories,
};
/**
 * Returns the file listing of a directory
 *
 * @param path Files are listed for this directory
 * @param mode What types of contents to return see ListMode
 * @return List of files residing in the given path
 */
IVW_CORE_API std::vector<std::filesystem::path> getDirectoryContents(
    const std::filesystem::path& path, ListMode mode = ListMode::Files);

/**
 * Recursively searches and returns full path to files/directories in specified directory and its
 * subdirectories.
 * @param path Files are listed for this directory and its subdirectories
 * @param mode What types of contents to return see ListMode
 * @return List of files residing in the given path and its subdirectories
 */
IVW_CORE_API std::vector<std::filesystem::path> getDirectoryContentsRecursively(
    const std::filesystem::path& path, ListMode mode = ListMode::Files);

/**
 * Checks whether a given string matches a pattern. The pattern
 * might contain '*' matching any string including the empty string
 * and '?' matching a single character.
 *
 * @param pattern  The pattern used for matching, might contain '*' and '?'
 * @param str      String which needs to be checked
 * @return true if the given string matches the pattern, false otherwise.
 */
IVW_CORE_API bool wildcardStringMatch(const std::string& pattern, const std::string& str);

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
 * @param pattern  The pattern used for matching, might contain a single
 *             sequence of '#' besides '*', and '?'
 * @param str      String which needs to be checked
 * @param index  if the match is successful, this index contains the extracted
 *             digit sequence indicated by '#'
 * @param matchLess   allows to match digit sequences shorter than defined by the number of '#'
 * (default false)
 * @param matchMore   allows to match longer digit sequences (default true)
 * @return True if the given string matches the pattern, false otherwise.
 */
IVW_CORE_API bool wildcardStringMatchDigits(const std::string& pattern, const std::string& str,
                                            int& index, bool matchLess = false,
                                            bool matchMore = true);

/**
 * Traverses all parent folders of path and returns the first directory matching the list of child
 * folders.
 *
 * @param path   directory where the search is started
 * @param childFolders   list of subfolders
 * @return path of parent directory holding all childFolders, otherwise std::nullopt
 */
IVW_CORE_API std::optional<std::filesystem::path> getParentFolderWithChildren(
    const std::filesystem::path& path, std::span<const std::filesystem::path> childFolders);

/**
 * Try to find the Inviwo base path containing subfolders "data/workspaces" and "modules".
 * If not found, try searching for path containing "modules". If neither are found, return the
 * executable path.
 *
 * @return Inviwo base path
 */
IVW_CORE_API std::filesystem::path findBasePath();

/**
 * Get basePath +  pathType + suffix.
 * @see PathType
 * @param pathType Enum for type of path
 * @param suffix Path extension
 * @param createFolder if true, will create the folder on disk if it does not exists.
 * @return basePath +  pathType + suffix
 */
IVW_CORE_API std::filesystem::path getPath(PathType pathType, const std::string& suffix = "",
                                           const bool createFolder = false);

[[deprecated("use std::filesystem::create_directories")]] IVW_CORE_API void
createDirectoryRecursively(const std::filesystem::path& path);

/**
 * \brief Adds the InviwoApplication base path before the url
 *
 * @see InviwoApplication::getBasePath
 * @param url Relative path
 * @return InviwoApplication base path + url
 */
IVW_CORE_API std::filesystem::path addBasePath(const std::filesystem::path& url);

[[deprecated("use std::filesystem::path::parent_path")]] IVW_CORE_API std::filesystem::path
getFileDirectory(const std::filesystem::path& url);
[[deprecated("use std::filesystem::path::filename")]] IVW_CORE_API std::filesystem::path
getFileNameWithExtension(const std::filesystem::path& url);
[[deprecated("use std::filesystem::path::stem")]] IVW_CORE_API std::filesystem::path
getFileNameWithoutExtension(const std::filesystem::path& url);
/**
 * \brief Returns the characters after last dot (.).
 *
 * @note Avoid using this function for extracting file extensions if you are not sure that the
 * expected extension only contains one dot. In general, there is no way to safely consider cases
 * with multiple dots. File extensions with multiple dots can exist for compressed files (for
 * example nii.gz). Instead, compare the url with registered extensions, see inviwo::FileExtension
 * and inviwo::DataReaderFactory
 *
 * @see DataReaderFactory
 * @see getFileNameWithExtension
 * @param url Path to extract extension from.
 * @return Extension excluding ., empty if no dot or filename are found.
 */
[[deprecated("use std::filesystem::path::extension note: this will include the dot")]] IVW_CORE_API
    std::string
    getFileExtension(const std::filesystem::path& url);

/**
 * Replace the last file extension to newFileExtension, if no extension exists append
 * newFileExtension. newFileExtension should not contain any leading "."
 */
IVW_CORE_API std::filesystem::path replaceFileExtension(const std::filesystem::path& url,
                                                        std::string_view newFileExtension);

/**
 * \brief Make a path relative to basePath.
 * Requirement: basePath and absulutePath has to be absolute paths.
 * basePath should point at directory.
 *
 * Example:
 * basePath = "C:/foo/bar"
 * absolutePath = "C:/foo/test/file.txt"
 * returns "../test/file.txt"
 */
[[deprecated("use std::filesystem::relative")]] IVW_CORE_API std::filesystem::path getRelativePath(
    const std::filesystem::path& basePath, const std::filesystem::path& absolutePath);
[[deprecated("use std::filesystem::canonical")]] IVW_CORE_API std::filesystem::path
getCanonicalPath(const std::filesystem::path& url);

[[deprecated("use std::filesystem::path::is_absolute")]] IVW_CORE_API bool isAbsolutePath(
    const std::filesystem::path& path);

/**
 * \brief Checks whether the second path is on the same drive as the first path
 *
 * If both paths are relative, this function returns true. If only refPath is relative
 * InviwoApplication::getBasePath is used instead as reference.
 *
 * @param refPath reference path, if relative then InviwoApplication::getBasePath
 * is used instead
 * @param queryPath path to be checked
 * @return true if queryPath and refPath are located on the same drive (on Windows), always true on
 * all other systems
 */
[[deprecated("use std::filesystem::path::root_name")]] IVW_CORE_API bool sameDrive(
    const std::filesystem::path& refPath, const std::filesystem::path& queryPath);

/**
 * \brief clean up path by replacing backslashes with forward slash and removing surrounding quotes
 *
 * @param path given path to be cleaned up
 * @return non-quoted path containing no backslashes as directory separators
 */
[[deprecated(
    "use std::filesystem::canonical/weakly_canonical or "
    "std::filesystem::path::lexically_normal")]] IVW_CORE_API std::filesystem::path
cleanupPath(std::string_view path);

}  // namespace filesystem

// Apple clang does not include std::hash<std::filesystem::path> as of writing
// was added first in Clang 15.
struct PathHash {
    std::size_t operator()(const std::filesystem::path& p) const noexcept {
        return std::filesystem::hash_value(p);
    }
};

}  // namespace inviwo
