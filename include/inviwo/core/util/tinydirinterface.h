/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#ifndef IVW_TINYDIR_INTERFACE_H
#define IVW_TINYDIR_INTERFACE_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <ext/tinydir/tinydir.h>
#include <functional>
#include <string>
#include <vector>


namespace inviwo {

/** \class TinyDirInteface
 *
 * Interface for tinydir responsible for listing files in a directory. Depending
 * on the list mode (default ListMode::FilesOnly), the result contains files, directories or both.
 */
class IVW_CORE_API TinyDirInterface {
public:
    enum class ListMode {
        FilesOnly,
        DirectoriesOnly,
        FilesAndDirectories,
    };

    TinyDirInterface();
    TinyDirInterface(TinyDirInterface const&) = delete;
    TinyDirInterface& operator=(TinyDirInterface const&) = delete;
    
    virtual ~TinyDirInterface();

    /** 
     * \brief Opens the given path as directory resource.
     *
     * \see close isOpen
     * 
     * @param path Path of the directory whose contents will be queried.
     * @return True if opening the directory resource is successful.
     */
    bool open(const std::string &path);
    /** 
     * \brief Closes any open directory resource. Directory contents can 
     * no longer be listed. The resource will automatically be closed on
     * deconstruction of this object.
     *
     * \see open
     */
    void close();

    /** 
     * \brief Returns whether a directory resource is open, i.e. available, 
     * for querying.
     *
     * @return True if resource is open. False otherwise.
     */
    bool isOpen() const;

    /**
    * \brief Set the current mode used for listing the directory.
    *
    * @param mode new listing mode
    */
    void setListMode(ListMode mode);

    /**
    * \brief Returns the current mode used for listing the directory.
    *
    * @param
    * @return current listing mode
    */
    ListMode getListMode() const;

    /**
    * \brief Returns whether the directory resource can be queried for
    * another entry.
    *
    * @return True if directory listing contains at least one entry.
    */
    bool isNextEntryAvailable() const;
    
    /** 
     * \brief Queries the directory for the next entry. The current ListMode 
     *  determines what contents will be returned. An empty string is 
     *  returned in case the resource is closed or no further file is available.
     *  Querying the last file automatically closes the directory resource.
     *
     * \see close getNextEntryWithBasePath
     *
     * @return next entry in directory. If none is available or the directory 
     *         was not open before, an empty string is returned.
     * @throws FileException
     */
    std::string getNextEntry();
    /**
     * \brief Convenience function for getNextEntry including the base path.
     *
     * \see close getNextEntry
     *
     * @return next entry in directory. If none is available or the directory
     *         was not open before, an empty string is returned.
     * @throws FileException
     */
    std::string getNextEntryWithBasePath();

    /**
     * \brief Queries the directory for all entries. The current ListMode
     *  determines what contents will be returned. An empty vector is
     *  returned in case the resource is closed. This function will close
     *  the directory resource.
     *
     * \see close getNextEntry getFileNamesWithBasePath
     *
     * @return directory listing based on ListMode setting.
     * @throws FileException
     */
    std::vector<std::string> getContents();

    /**
     * \brief Convenience function for getContents including the base path.
     *
     * \see close getNextEntry getContents
     *
     * @return directory listing based on ListMode setting.
     * @throws FileException
     */
    std::vector<std::string> getContentsWithBasePath();

protected:
    std::string getNextEntry(bool includeBasePath);

private:
    bool isOpen_;
    ListMode mode_;
    std::string path_;
    tinydir_dir resource_;
};

}; // namespace inviwo

#endif // IVW_TINYDIR_INTERFACE_H
