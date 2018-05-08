/*********************************************************************
 *  Author  : Anke Friederici & Tino Weinkauf
 *  Init    : Monday, December 04, 2017 - 11:44:12
 *
 *  Project : KTH Inviwo Modules
 *
 *  License : Follows the Inviwo BSD license model
 *********************************************************************
 */

#pragma once

template <typename T>
T* inviwo::ChannelIterator<T>::operator*() {
    ivwAssert(Parent, "No channel to iterate is set.");

    // If our parent is a buffer, return a direct data pointer.
    if (dynamic_cast<BufferChannel<T>*>(Parent))
        return dynamic_cast<BufferChannel<T>*>(Parent)->get(Index);

    // No buffer, evaluate analytics.
    // Is the data up to date?
    if (DataIndex != Index) {
        delete[] Data;
        Data = new T[Parent->getNumComponents()];
        // Note: we could use the old memory again, but numCOmponents might change (improbable).
        // Also, we want segfaults when old data is accessed.

        Parent->fill(Data, Index);
        DataIndex = Index;
    }

    // Always return data.
    // If the iterator is changed and dereferenced, the pointer becomes invalid.
    return Data;
};
