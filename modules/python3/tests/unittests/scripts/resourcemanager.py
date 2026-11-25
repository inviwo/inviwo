# ********************************************************************************
#
#  Inviwo - Interactive Visualization Workshop
#
#  Copyright (c) 2026 Inviwo Foundation
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#
#  1. Redistributions of source code must retain the above copyright notice, this
#  list of conditions and the following disclaimer.
#  2. Redistributions in binary form must reproduce the above copyright notice,
#  this list of conditions and the following disclaimer in the documentation
#  and/or other materials provided with the distribution.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
#  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
#  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
#  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
#  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
#  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
#  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
#  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# ********************************************************************************

import inviwopy
from inviwopy import resource
from inviwopy.glm import size4_t
from inviwopy.data.formats import DataFormatId

# Resource and ResourceMeta
res = inviwopy.Resource(dims=size4_t(4, 3, 2, 0), format=DataFormatId.FLOAT32, desc="Test")
assert res.dims == size4_t(4, 3, 2, 0)
assert res.format == DataFormatId.FLOAT32
assert res.desc == "Test"
assert res.meta is None
assert res.sizeInBytes() == 4 * 3 * 2 * 4

res.meta = inviwopy.ResourceMeta("file.dat")
assert res.meta == inviwopy.ResourceMeta(source="file.dat")
assert res.meta.source == "file.dat"
res.desc = "Other"
assert res.desc == "Other"

# Keys
assert resource.RAM.group == 0
assert resource.GL.group == 1
assert resource.PY.group == 2
assert resource.RAM.name == "RAM"
assert resource.RAM(1) == resource.RAM(1)
assert resource.RAM(1) != resource.RAM(2)
assert hash(resource.RAM(3)) == hash(resource.RAM(3))
assert resource.GL(1) in {resource.GL(1)}
obj = object()
assert resource.toPY(obj) == resource.PY(id(obj))

# A standalone manager with a python observer
events = []


class Observer(inviwopy.ResourceManagerObserver):
    def onDidAddResource(self, group, index, r):
        events.append(("add", group, index, r.desc))

    def onDidUpdateResource(self, group, index, r):
        events.append(("update", group, index, r.desc))

    def onDidRemoveResource(self, group, index, r):
        events.append(("remove", group, index, r.desc))


rm = inviwopy.ResourceManager()
observer = Observer()
rm.addObserver(observer)
assert rm.isObservedBy(observer)
assert list(rm.names) == ["RAM", "GL", "PY"]
assert rm.totalSize() == 0

rm.add(resource.RAM(1), inviwopy.Resource(size4_t(2, 2, 2, 0), DataFormatId.UINT8, "A"))
rm.add(resource.GL(1), inviwopy.Resource(size4_t(2, 2, 0, 0), DataFormatId.FLOAT32, "B"))
rm.add(resource.PY(1), inviwopy.Resource(size4_t(10, 0, 0, 0), DataFormatId.Vec4FLOAT32, "C"))
assert rm.totalSize() == 3
assert rm.size(resource.RAM.group) == 1
assert rm.totalByteSize(resource.RAM.group) == 8
assert rm.totalByteSize(resource.GL.group) == 16
assert rm.totalByteSize(resource.PY.group) == 160
assert rm.get(resource.PY.group, 0).desc == "C"
assert rm.get(resource.PY.group, 1) is None
assert rm.get(7, 0) is None
assert [r.desc for r in rm.resources(resource.RAM.group)] == ["A"]

# updating keeps old meta if none is given
rm.meta(resource.RAM(1), inviwopy.ResourceMeta("src"))
rm.add(resource.RAM(1), inviwopy.Resource(size4_t(4, 0, 0, 0), DataFormatId.UINT8, "A2"))
assert rm.get(resource.RAM.group, 0).meta.source == "src"
assert rm.totalByteSize(resource.RAM.group) == 4

rm.move(resource.GL(1), resource.GL(2), inviwopy.Resource(size4_t(1, 0, 0, 0), DataFormatId.UINT8, "B2"))
assert rm.size(resource.GL.group) == 1
removed = rm.remove(resource.GL(2))
assert removed is not None and removed.desc == "B2"
assert rm.remove(resource.GL(2)) is None

rm.clear()
assert rm.totalSize() == 0

assert ("add", 0, 0, "A") in events
assert ("update", 0, 0, "A") in events  # meta update
assert ("update", 0, 0, "A2") in events
assert ("remove", 1, 0, "B") in events  # move removes the old key
assert ("add", 1, 0, "B2") in events
assert ("remove", 1, 0, "B2") in events
assert events[-1][0] == "remove"

rm.removeObserver(observer)
assert not rm.isObservedBy(observer)

# The application resource manager
app = inviwopy.app
assert isinstance(app.resourceManager, inviwopy.ResourceManager)
assert isinstance(app.systemSettings, inviwopy.Settings)

totalEvents = len(events)
