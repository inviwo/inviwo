/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2018 Inviwo Foundation
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

#ifndef IVW_INTEGRALLINE_H
#define IVW_INTEGRALLINE_H

#include <modules/vectorfieldvisualization/vectorfieldvisualizationmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/util/bufferutils.h>
#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>
#include <inviwo/core/util/interpolation.h>

namespace inviwo {
/**
 * \class IntegralLine
 *
 * \brief VERY_BRIEFLY_DESCRIBE_THE_CLASS
 *
 * DESCRIBE_THE_CLASS
 */
class IVW_MODULE_VECTORFIELDVISUALIZATION_API IntegralLine {
public:
    enum class TerminationReason {
        OutOfBounds, 
        ZeroVelocity, 
        Steps
    };

    IntegralLine() = default;
    IntegralLine(const IntegralLine &rhs) = default;
    IntegralLine(IntegralLine &&rhs) = default;

    IntegralLine &operator=(const IntegralLine &that) = default;
    IntegralLine &operator=(IntegralLine &&that) = default;

    virtual ~IntegralLine() = default;

    void setTerminationReason(TerminationReason terminationReason) {
        terminationReason_ = terminationReason;
    }

    const std::vector<dvec3> &getPositions() const;
    std::vector<dvec3> &getPositions();

    std::shared_ptr<const BufferBase> getMetaDataBuffer(const std::string &name) const;
    std::shared_ptr<BufferBase> getMetaDataBuffer(const std::string &name);
    
    template<typename T>
    std::shared_ptr<Buffer<T>> createMetaData(const std::string &name){
        if(hasMetaData(name)){
            throw  Exception("Meta data with name " + name + " already exists");
        }
        auto md = std::make_shared<Buffer<T>>();
        metaData_[name] = md;
        return md;
    }

    void reverse(){
        std::reverse(positions_.begin(),positions_.end());  
        for (auto &m : metaData_) {
            util::reverse(*m.second);
        }
    }

    template<typename T>
    const std::vector<T> &getMetaData(const std::string &name) const{
        auto it = metaData_.find(name);
        if (it == metaData_.end()) {
            throw Exception("No meta data with name: " + name, IvwContext);
        }
        auto askedDF = DataFormat<T>::get();
        auto isDF = it->second->getDataFormat();
        if ( isDF != askedDF ){
            std::ostringstream oss;
            oss << "Incorrect dataformat for metadata " << name << " asking for " << askedDF->getString() << " but is" << isDF->getString();
            throw Exception(oss.str() , IvwContext);
        }

        return static_cast<Buffer<T>*>(it->second.get())->getRAMRepresentation()->getDataContainer();
    }

    template<typename T>
    std::vector<T> &getMetaData(const std::string &name, bool create = false) {
        auto it = metaData_.find(name);
        if (it == metaData_.end() && !create) {
            throw Exception("No meta data with name: " + name, IvwContext);
        }else if(it == metaData_.end()){
            auto md = createMetaData<T>(name);
            return md->getEditableRAMRepresentation()->getDataContainer();
        }
        auto askedDF = DataFormat<T>::get();
        auto isDF = it->second->getDataFormat();
        if ( isDF != askedDF ){
            std::ostringstream oss;
            oss << "Incorrect dataformat for metadata " << name << " asking for " << askedDF->getString() << " but is " << isDF->getString();
            throw Exception(oss.str() , IvwContext);
        }

        return static_cast<Buffer<T>*>(it->second.get())->getEditableRAMRepresentation()->getDataContainer();
    }

    const std::map<std::string , std::shared_ptr<BufferBase>> &getMetaDataBuffers() const{return metaData_;}

    bool hasMetaData(const std::string &name) const;

    std::vector<std::string> getMetaDataKeys() const;


    double getLength()const;

    double distBetweenPoints(size_t a, size_t b) const;

    dvec3 getPointAtDistance(double d)const;

    template<typename T>
    T getMetaDataAtDistance(std::string md , double d)const{
        if (d<0 || d > getLength()) {
            return T(0);
        }

        if(!hasMetaData(md)){
            return T(0);
        }

        auto &metaData = getMetaData<T>(md);

        if (d == 0) {
            return metaData.front();
        }


        double distPrev = 0, distNext = 0;
        auto next = positions_.begin();
        auto prev = next;
        auto nextMD = metaData.begin();
        auto prevMD = nextMD;

        while (distNext < d) {
            prev = next++;
            prevMD = nextMD++;
            distPrev = distNext;
            distNext += glm::distance(*prev, *next);
        }

        double x = (d - distPrev) / (distNext - distPrev);
        //using F = typename std::conditional< std::is_same< typename util::value_type<T>::type,float >::value , float , double >::type;
        //using TV = typename std::conditional<util::extent<T>::value==1 , double , Vector<util::extent<T>::value ,double> >::type;
        using TV = typename util::same_extent<T,double>::type;    
            
        return static_cast<T>( Interpolation<TV, double>::linear( static_cast<TV>(*prevMD), static_cast<TV>(*nextMD), x));
    }

    size_t getIndex()const { return idx_; }
    void setIndex(size_t idx ) { idx_  = idx; }

private:
    double calcLength( std::vector<dvec3>::const_iterator start ,   std::vector<dvec3>::const_iterator end ) const;

    std::vector<dvec3> positions_;
    //std::map<std::string, std::vector<dvec3>> metaData_;
    std::map<std::string , std::shared_ptr<BufferBase>> metaData_;
    TerminationReason terminationReason_ = TerminationReason::Steps;

    mutable double length_ = -1;

    size_t idx_;



};

}  // namespace

#endif  // IVW_INTEGRALLINE_H
