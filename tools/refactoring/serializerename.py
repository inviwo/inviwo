import sys 
import os
import re
import colorama
colorama.init()

import refactoring # Note: refactoring.py need to be in the current working directory

paths = ["C:/Users/petst55/Work/Inviwo/Inviwo-dev", "C:/Users/petst55/Work/Inviwo/Inviwo-research"]
excludespatterns = ["*/ext/*", "*moc_*", "*cmake*", "*/proteindocking/*", "*/proteindocking2/*", "*/genetree/*", "*/vrnbase/*"];

files = refactoring.find_files(paths, ['*.h', '*.cpp'], excludes=excludespatterns)

def replace(pattern, replacement) :
	print("Matches:")
	matches = refactoring.find_matches(files, pattern)
	
	print("\n")
	print("Replacing:")
	refactoring.replace_matches(matches, pattern, replacement)



serializerReplacements = {
	"IvwSerializeConstants::XML_VERSION" : "SerializeConstants::XmlVersion",
	"IvwSerializeConstants::INVIWO_TREEDATA" : "SerializeConstants::InviwoTreedata",
	"IvwSerializeConstants::INVIWO_VERSION" : "SerializeConstants::InviwoVersion",
	"IvwSerializeConstants::NETWORK_VERSION" : "SerializeConstants::NetworkVersion",
	"IvwSerializeConstants::VERSION" : "SerializeConstants::Version",
	"IvwSerializeConstants::EDIT_COMMENT" : "SerializeConstants::EditComment",
	"IvwSerializeConstants::ID_ATTRIBUTE" : "SerializeConstants::IDAttribute",
	"IvwSerializeConstants::REF_ATTRIBUTE" : "SerializeConstants::RefAttribute",
	"IvwSerializeConstants::VERSION_ATTRIBUTE" : "SerializeConstants::VersionAttribute",
	"IvwSerializeConstants::CONTENT_ATTRIBUTE" : "SerializeConstants::ContentAttribute",
	"IvwSerializeConstants::TYPE_ATTRIBUTE" : "SerializeConstants::TypeAttribute",
	"IvwSerializeConstants::KEY_ATTRIBUTE" : "SerializeConstants::KeyAttribute",
	"IvwSerializeConstants::VECTOR_ATTRIBUTES" : "SerializeConstants::VectorAttributes",
	"IvwSerializeConstants::PROPERTY_ATTRIBUTE_1" : "SerializeConstants::PropertyAttribute1",
	"IvwSerializeConstants::PROPERTY_ATTRIBUTE_2" : "SerializeConstants::PropertyAttribute2",
	"IvwSerializeConstants::PROCESSOR_ATTRIBUTE_1" : "SerializeConstants::ProcessorAttribute1",
	"IvwSerializeConstants::PROCESSOR_ATTRIBUTE_2" : "SerializeConstants::ProcessorAttribute2",
	"IvwSerializeConstants" : "SerializeConstants",
	"IvwSerializeBase" : "SerializeBase",
    "IvwDeserializer" : "Deserializer",
    "IvwSerializable" : "Serializable",
    "IvwSerializer"   : "Serializer",

    "inviwo/core/io/serialization/ivwdeserializer.h" : "inviwo/core/io/serialization/deserializer.h",
    "inviwo/core/io/serialization/ivwserializable.h" : "inviwo/core/io/serialization/serializable.h",  
    "inviwo/core/io/serialization/ivwserialization.h" : "inviwo/core/io/serialization/serialization.h",
    "inviwo/core/io/serialization/ivwserializebase.h" : "inviwo/core/io/serialization/serializebase.h",    
    "inviwo/core/io/serialization/ivwserializeconstants.h" : "inviwo/core/io/serialization/serializeconstants.h",    
    "inviwo/core/io/serialization/ivwserializer.h" : "inviwo/core/io/serialization/serializer.h"
}


for k,v in serializerReplacements.items():
	replace(r"\b"+k+r"\b", v)
