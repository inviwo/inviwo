# ********************************************************************************
#
# Inviwo - Interactive Visualization Workshop
#
# Copyright (c) 2026 Inviwo Foundation
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# ********************************************************************************

import inviwopy
from inviwopy.glm import vec3, ivec2, dvec2

# 'volume' and 'prop' are passed in from C++

for owner in (volume, prop):
    assert isinstance(owner, inviwopy.MetaDataOwner)

assert not volume.hasMetaData("int")
assert volume.getMetaData("int") is None
assert volume.getMetaData("int", 7) == 7

volume.setMetaData("int", 5)
volume.setMetaData("bool", True)
volume.setMetaData("double", 1.5)
volume.setMetaData("string", "hello")
volume.setMetaData("vec3", vec3(1, 2, 3))
volume.setMetaData("strings", ["a", "b"])
volume.setMetaData("dict", {"key": "value"})
volume.setMetaData("float", inviwopy.FloatMetaData(2.5))

assert volume.hasMetaData("int")
assert volume.hasMetaData("int", inviwopy.IntMetaData)
assert not volume.hasMetaData("int", inviwopy.FloatMetaData)
assert volume.getMetaData("int") == 5
assert volume.getMetaData("bool") is True
assert volume.getMetaData("double") == 1.5
assert volume.getMetaData("string") == "hello"
assert volume.getMetaData("vec3") == vec3(1, 2, 3)
assert list(volume.getMetaData("strings")) == ["a", "b"]
assert volume.getMetaData("dict") == {"key": "value"}
assert volume.getMetaData("float") == 2.5

# updating keeps the existing type when the value is convertible
volume.setMetaData("float", 3)
assert volume.hasMetaData("float", inviwopy.FloatMetaData)
assert volume.getMetaData("float") == 3.0

# otherwise the type is replaced
volume.setMetaData("string", 10)
assert volume.hasMetaData("string", inviwopy.IntMetaData)
assert volume.getMetaData("string") == 10
volume.setMetaData("string", "hello")
assert volume.hasMetaData("string", inviwopy.StringMetaData)

# access via the MetaDataMap
md = volume.metaData
assert "int" in md
assert "missing" not in md
assert len(md) == 8
assert set(md.keys()) == {"int", "bool", "double", "string", "vec3", "strings", "dict", "float"}
assert set(md) == set(md.keys())
assert isinstance(md["int"], inviwopy.IntMetaData)
assert isinstance(md["vec3"], inviwopy.FloatVec3MetaData)
assert md["int"] == inviwopy.IntMetaData(5)
assert md["int"] != inviwopy.IntMetaData(6)
assert md["int"].classIdentifier == "org.inviwo.IntMetaData"
assert md.get("missing") is None
assert isinstance(md["int"].clone(), inviwopy.IntMetaData)

md["int"].value = 6
assert volume.getMetaData("int") == 6

md["ivec2"] = inviwopy.IntVec2MetaData(ivec2(1, 2))
assert volume.getMetaData("ivec2") == ivec2(1, 2)

del md["ivec2"]
assert "ivec2" not in md
try:
    md["ivec2"]
    assert False, "expected KeyError"
except KeyError:
    pass

assert volume.unsetMetaData("bool")
assert not volume.unsetMetaData("bool")
assert not volume.hasMetaData("bool")

try:
    volume.setMetaData("bad", object())
    assert False, "expected TypeError"
except TypeError:
    pass

# copy between owners of different types
prop.copyMetaDataFrom(volume)
assert prop.getMetaData("string") == "hello"
prop.setMetaData("dvec2", dvec2(1.0, 2.0))

copy = volume.clone()
assert copy.getMetaData("int") == 6
assert copy.metaData == volume.metaData

repr(md)
