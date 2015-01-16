/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#ifndef IVW_REPRESENTATIONCONVERTER_H
#define IVW_REPRESENTATIONCONVERTER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <vector>

namespace inviwo {

class DataRepresentation;

class IVW_CORE_API RepresentationConverter {

public:
    RepresentationConverter();
    virtual ~RepresentationConverter();

    /**
     * Checks if it is possible to convert from the data representation.
     * @param source is the DataRepresentation to test for conversion possibility.
     * @return boolean True if possible, false otherwise.
     */
    virtual bool canConvertFrom(const DataRepresentation* source) const = 0;
    virtual bool canConvertTo(const DataRepresentation* destination) const = 0;

    virtual bool isConverterReverse(RepresentationConverter*) { return false; }

    virtual DataRepresentation* createFrom(const DataRepresentation* source) = 0;
    virtual void update(const DataRepresentation* source, DataRepresentation* destination) = 0;
};

template <typename TO>
class RepresentationConverterType : public RepresentationConverter {
public:
    virtual ~RepresentationConverterType() {};

    bool canConvertTo(const DataRepresentation* destination) const {
        return dynamic_cast<const TO*>(destination) != NULL;
    }
};


template <typename T>
class RepresentationConverterPackage : public RepresentationConverter {
public:
    RepresentationConverterPackage() : RepresentationConverter() {
        converters_ = new std::vector<RepresentationConverter*>();
    };
    ~RepresentationConverterPackage() {
        for (std::vector<RepresentationConverter*>::iterator it = converters_->begin() ; it != converters_->end(); ++it)
            delete(*it);

        delete converters_;
    }
    bool canConvertFrom(const DataRepresentation* source) const {
        for (std::vector<RepresentationConverter*>::const_iterator it = converters_->begin() ; it != converters_->end(); ++it) {
            if ((*it)->canConvertFrom(source))
                return true;
        }

        return false;
    }
    bool canConvertTo(const DataRepresentation* destination) const {
        for (std::vector<RepresentationConverter*>::const_iterator it = converters_->begin() ; it != converters_->end(); ++it) {
            if ((*it)->canConvertTo(destination))
                return true;
        }

        return false;
    }
    DataRepresentation* createFrom(const DataRepresentation* source) {
        for (std::vector<RepresentationConverter*>::iterator it = converters_->begin() ; it != converters_->end(); ++it) {
            if ((*it)->canConvertFrom(source))
                return (*it)->createFrom(source);
        }

        return NULL;
    }
    virtual void update(const DataRepresentation* source, DataRepresentation* destination) {
        for (std::vector<RepresentationConverter*>::iterator it = converters_->begin() ; it != converters_->end(); ++it) {
            if ((*it)->canConvertFrom(source))
                (*it)->update(source, destination);
        }
    }

    void addConverter(RepresentationConverter* converter) { converters_->push_back(converter); }
    size_t getNumberOfConverters() { return converters_->size(); }

    const std::vector<RepresentationConverter*>* getConverters() const { return converters_; }
private:
    std::vector<RepresentationConverter*>* converters_;
};

} // namespace

#endif // IVW_REPRESENTATIONCONVERTER_H
