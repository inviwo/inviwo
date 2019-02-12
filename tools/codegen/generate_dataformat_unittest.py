
print("/*********************************************************************************")
print(" *")
print(" * Inviwo - Interactive Visualization Workshop")
print(" *")
print(" * Copyright (c) 2014-2019 Inviwo Foundation")
print(" * All rights reserved.")
print(" *")
print(" * Redistribution and use in source and binary forms, with or without")
print(" * modification, are permitted provided that the following conditions are met:")
print(" *")
print(" * 1. Redistributions of source code must retain the above copyright notice, this")
print(" * list of conditions and the following disclaimer.")
print(" * 2. Redistributions in binary form must reproduce the above copyright notice,")
print(" * this list of conditions and the following disclaimer in the documentation")
print(" * and/or other materials provided with the distribution.")
print(" *")
print(' * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND')
print(" * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED")
print(" * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE")
print(" * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR")
print(" * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES")
print(" * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;")
print(" * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND")
print(" * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT")
print(" * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS")
print(" * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.")
print(" *")
print(" *********************************************************************************/")
print("")
print("#include <warn/push>")
print("#include <warn/ignore/all>")
print("#include <gtest/gtest.h>")
print("#include <warn/pop>")
print("")
print("#include <inviwo/core/common/inviwo.h>")
print("")
print("namespace inviwo {")
print("")



def dataformat(name,size,precision,numType):
    return {
        'name' : name ,
        'size' : size ,
        'precision' : str(precision) ,
        'numType' : numType
    }

formats = [
    dataformat("Float16",2,16,"Float") ,
    dataformat("Float32",4,32,"Float") ,
    dataformat("Float64",8,64,"Float") ,

    dataformat("Int8",1,8,"SignedInteger") ,
    dataformat("Int16",2,16,"SignedInteger") ,
    dataformat("Int32",4,32,"SignedInteger") ,
    dataformat("Int64",8,64,"SignedInteger") ,

    dataformat("UInt8",1,8,"UnsignedInteger") ,
    dataformat("UInt16",2,16,"UnsignedInteger") ,
    dataformat("UInt32",4,32,"UnsignedInteger") ,
    dataformat("UInt64",8,64,"UnsignedInteger")
]

tab = "    ";
for f in formats:
    sizeMulti = 1
    for v in ["","Vec2","Vec3","Vec4"]:
        Fname = f['name'] + v
        Ftype = v + f['name']
        print("TEST(DataFormatsTests, "+Fname+"Test) {")

        print(tab + "auto df = Data" + Ftype + "::get();")
        print(tab + "ASSERT_NE(nullptr, df);");
        print("");
        print(tab + "EXPECT_EQ("+str(sizeMulti*f['size'])+", df->getSize());");
        print(tab + "EXPECT_EQ("+f['precision']+", df->getPrecision());");
        print(tab + "EXPECT_EQ("+str(sizeMulti)+", df->getComponents());");
        string = v + f['name'].upper()
        print(tab + 'EXPECT_STREQ("'+string+'", df->getString());');
        print(tab + "EXPECT_EQ(NumericType::"+f['numType']+", df->getNumericType());");
        print(tab + "EXPECT_EQ(DataFormatId::"+Ftype+", df->getId());");
        

        print("}")
        print("")
        
        sizeMulti += 1;

print("}  // namespace")
