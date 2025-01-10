/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024-2025 Inviwo Foundation
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

#include <modules/json/jsonmoduledefine.h>

#include <modules/json/jsonconverter.h>
#include <modules/json/jsonconverterregistry.h>

#include <modules/json/json.h>
#include <vector>

namespace inviwo {

template <typename Base, template <typename...> typename Traits>
class JSONSupplier {
public:
    JSONSupplier(JSONConverterRegistry<Base>& pjc) : registry_{pjc} {}
    JSONSupplier(const JSONSupplier&) = delete;
    JSONSupplier& operator=(const JSONSupplier&) = delete;
    ~JSONSupplier() { unregisterJSONConverters(); }

    void registerJSONConverter(std::unique_ptr<JSONConverter<Base>> converter) {
        if (registry_.registerObject(converter.get())) {
            converters_.push_back(std::move(converter));
        }
    }
    template <typename Derived>
        requires std::is_base_of_v<Base, Derived> && JSONConvertable<Derived>
    void registerJSONConverter() {
        registerJSONConverter(std::make_unique<TemplateJSONConverter<Base, Derived, Traits>>());
    }

    void unregisterJSONConverters() {
        for (auto& elem : converters_) {
            registry_.unRegisterObject(elem.get());
        }
        converters_.clear();
    }

private:
    JSONConverterRegistry<Base>& registry_;
    std::vector<std::unique_ptr<JSONConverter<Base>>> converters_;
};

}  // namespace inviwo
