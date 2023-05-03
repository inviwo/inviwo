# ********************************************************************************
#
# Inviwo - Interactive Visualization Workshop
#
# Copyright (c) 2023 Inviwo Foundation
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

help('inviwopy')
app = inviwopy.getApp()
network = app.getProcessorNetwork()
processor = network.getProcessors()[0]
prop = processor.getProperties()[0]

print(processor.getClassIdentifier())
print(processor.getDisplayName())
print(processor.getCategory())
print(processor.setIdentifier("testing asdf"))
print(processor.getIdentifier())
print(processor.hasProcessorWidget())
print(processor.getNetwork())
print(processor.hasProcessorWidget())
print(processor.getPath())

print(prop.setIdentifier("setIdentifier"))
print(prop.getIdentifier())
print(prop.getPath())
print(prop.setDisplayName("setDisplayName"))
print(prop.getDisplayName())
print(prop.getClassIdentifierForWidget())
print(prop.setReadOnly(True))
print(prop.getReadOnly())
print(prop.updateWidgets())
print(prop.setCurrentStateAsDefault())
print(prop.resetToDefaultState())
print(prop.get())

prop.set(0.51)
print(prop.get())

print(type(prop))
