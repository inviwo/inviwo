/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
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

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/util/stdextensions.h>
#include <inviwo/core/datastructures/representationconverterfactory.h>

namespace inviwo {

void RepresentationConverterFactory::registerObject(RepresentationConverter* converter) {
    if (!util::insert_unique(converters_, converter->getConverterID(), converter))
        throw(ConverterException("Converter with supplied ID already registered", IvwContext));
}

const RepresentationConverterPackage* RepresentationConverterFactory::getRepresentationConverter(
    ConverterID id) {
    RepresentationConverterPackage* res = nullptr;
    size_t steps = std::numeric_limits<size_t>::max();
    {
        std::unique_lock<std::mutex> lock(mutex_);
        auto range = packages_.equal_range(id);
        for (auto it = range.first; it != range.second; ++it) {
            if (it->second->steps() < steps) {
                res = it->second.get();
            }
        }
    }
    if (res) {
        return res;
    } else {
        return createConverterPackage(id);
    }
}

const RepresentationConverterPackage* RepresentationConverterFactory::getRepresentationConverter(
    std::type_index from, std::type_index to) {
    return getRepresentationConverter(ConverterID(from, to));
}

const RepresentationConverterPackage* RepresentationConverterFactory::createConverterPackage(
    ConverterID id) {
    /* Implementation of Dijkstra's algorithm following
     * https://en.wikipedia.org/wiki/Dijkstra's_algorithm#Pseudocode
     */
    std::type_index source = id.first;
    std::type_index target = id.second;

    std::unordered_set<std::type_index> verts;
    for (auto converter : converters_) {
        verts.insert(converter.first.first);
        verts.insert(converter.first.second);
    }

    std::unordered_map<std::type_index, size_t> dist;
    std::unordered_map<std::type_index, std::type_index> prev;

    dist[source] = 0;

    std::unordered_set<std::type_index> Q;
    for (auto v : verts) {
        if (v != source) {
            dist[v] = std::numeric_limits<size_t>::max();
        }
        Q.insert(v);
    }

    while (!Q.empty()) {
        std::size_t steps = std::numeric_limits<size_t>::max();
        std::type_index u = *Q.begin();
        for (auto t : Q)
            if (dist[t] < steps) {
                steps = dist[t];
                u = t;
            }
        Q.erase(u);

        if (u == target) break;

        for (auto converter : converters_) {
            if (converter.first.first == u) {
                auto v = converter.first.second;
                size_t alt = dist[u] + 1;
                if (alt < dist[v]) {
                    dist[v] = alt;
                    prev.emplace(std::make_pair(v, u));
                }
            }
        }
    }

    std::vector<const RepresentationConverter*> S;
    std::type_index u = target;
    while (util::has_key(prev, u)) {
        auto conv = converters_[ConverterID(prev.at(u), u)];
        S.push_back(conv);
        u = prev.at(u);
    }

    if (!S.empty() && S.back()->getConverterID().first == source &&
        S.front()->getConverterID().second == target) {
        auto package = util::make_unique<RepresentationConverterPackage>();
        for (auto it = S.crbegin(); it != S.crend(); it++) {
            package->addConverter(*it);
        }
        auto res = package.get();
        {
            std::unique_lock<std::mutex> lock(mutex_);
            packages_.emplace(std::make_pair(package->getConverterID(), std::move(package)));
        }
        return res;
    } else {
        return nullptr;
    }
}

}  // namespace
