import sys 
import os
import re
import colorama
colorama.init()

import refactoring # Note: refactoring.py need to be in the current working directory

paths = ["C:/inviwo-dev/vistinct"]

excludespatterns = ["*/ext/*", "*moc_*", "*cmake*", "*/proteindocking/*", "*/proteindocking2/*", "*/genetree/*", "*/vrnbase/*"];

files = refactoring.find_files(paths, ['*.h', '*.cpp'], excludes=excludespatterns)

def replace(pattern, replacement) :
	print("Matches:")
	matches = refactoring.find_matches(files, pattern)
	
	print("\n")
	print("Replacing:")
	refactoring.replace_matches(matches, pattern, replacement)


numericTypeReplacements = {
	"DataFormatEnums::NumericType" : "NumericType",
    "DataFormatEnums::NOT_SPECIALIZED_TYPE" : "NumericType::NotSpecialized",
    "DataFormatEnums::FLOAT_TYPE" : "NumericType::Float",
    "DataFormatEnums::UNSIGNED_INTEGER_TYPE" : "NumericType::UnsignedInteger",
    "DataFormatEnums::SIGNED_INTEGER_TYPE" : "NumericType::SignedInteger"
};

dataFormatIdReplacements = {
	"DataFormatEnums::Id"                : "DataFormatId",
	"DataFormatEnums::NOT_SPECIALIZED"   : "DataFormatId::NotSpecialized",

	"DataFormatEnums::FLOAT16"           : "DataFormatId::Float16",
	"DataFormatEnums::FLOAT32"           : "DataFormatId::Float32",
	"DataFormatEnums::FLOAT64"           : "DataFormatId::Float64",
	"DataFormatEnums::INT8"              : "DataFormatId::Int8",
	"DataFormatEnums::INT16"             : "DataFormatId::Int16",
	"DataFormatEnums::INT32"             : "DataFormatId::Int32",
	"DataFormatEnums::INT64"             : "DataFormatId::Int64",
	"DataFormatEnums::UINT8"             : "DataFormatId::UInt8",
	"DataFormatEnums::UINT16"            : "DataFormatId::UInt16",
	"DataFormatEnums::UINT32"            : "DataFormatId::UInt32",
	"DataFormatEnums::UINT64"            : "DataFormatId::UInt64",

	"DataFormatEnums::Vec2FLOAT16"       : "DataFormatId::Vec2Float16",
	"DataFormatEnums::Vec2FLOAT32"       : "DataFormatId::Vec2Float32",
	"DataFormatEnums::Vec2FLOAT64"       : "DataFormatId::Vec2Float64",
	"DataFormatEnums::Vec2INT8"          : "DataFormatId::Vec2Int8",
	"DataFormatEnums::Vec2INT16"         : "DataFormatId::Vec2Int16",
	"DataFormatEnums::Vec2INT32"         : "DataFormatId::Vec2Int32",
	"DataFormatEnums::Vec2INT64"         : "DataFormatId::Vec2Int64",
	"DataFormatEnums::Vec2UINT8"         : "DataFormatId::Vec2UInt8",
	"DataFormatEnums::Vec2UINT16"        : "DataFormatId::Vec2UInt16",
	"DataFormatEnums::Vec2UINT32"        : "DataFormatId::Vec2UInt32",
	"DataFormatEnums::Vec2UINT64"        : "DataFormatId::Vec2UInt64",

	"DataFormatEnums::Vec3FLOAT16"       : "DataFormatId::Vec3Float16",
	"DataFormatEnums::Vec3FLOAT32"       : "DataFormatId::Vec3Float32",
	"DataFormatEnums::Vec3FLOAT64"       : "DataFormatId::Vec3Float64",
	"DataFormatEnums::Vec3INT8"          : "DataFormatId::Vec3Int8",
	"DataFormatEnums::Vec3INT16"         : "DataFormatId::Vec3Int16",
	"DataFormatEnums::Vec3INT32"         : "DataFormatId::Vec3Int32",
	"DataFormatEnums::Vec3INT64"         : "DataFormatId::Vec3Int64",
	"DataFormatEnums::Vec3UINT8"         : "DataFormatId::Vec3UInt8",
	"DataFormatEnums::Vec3UINT16"        : "DataFormatId::Vec3UInt16",
	"DataFormatEnums::Vec3UINT32"        : "DataFormatId::Vec3UInt32",
	"DataFormatEnums::Vec3UINT64"        : "DataFormatId::Vec3UInt64",

	"DataFormatEnums::Vec4FLOAT16"       : "DataFormatId::Vec4Float16",
	"DataFormatEnums::Vec4FLOAT32"       : "DataFormatId::Vec4Float32",
	"DataFormatEnums::Vec4FLOAT64"       : "DataFormatId::Vec4Float64",
	"DataFormatEnums::Vec4INT8"          : "DataFormatId::Vec4Int8",
	"DataFormatEnums::Vec4INT16"         : "DataFormatId::Vec4Int16",
	"DataFormatEnums::Vec4INT32"         : "DataFormatId::Vec4Int32",
	"DataFormatEnums::Vec4INT64"         : "DataFormatId::Vec4Int64",
	"DataFormatEnums::Vec4UINT8"         : "DataFormatId::Vec4UInt8",
	"DataFormatEnums::Vec4UINT16"        : "DataFormatId::Vec4UInt16",
	"DataFormatEnums::Vec4UINT32"        : "DataFormatId::Vec4UInt32",
	"DataFormatEnums::Vec4UINT64"        : "DataFormatId::Vec4UInt64",

	"DataFormatEnums::NUMBER_OF_FORMATS" : "DataFormatId::NumberOfFormats"
}

dataFormatTypeReplacements = {
	"DataFLOAT16"           : "DataFloat16",
	"DataFLOAT32"           : "DataFloat32",
	"DataFLOAT64"           : "DataFloat64",
	"DataINT8"              : "DataInt8",
	"DataINT16"             : "DataInt16",
	"DataINT32"             : "DataInt32",
	"DataINT64"             : "DataInt64",
	"DataUINT8"             : "DataUInt8",
	"DataUINT16"            : "DataUInt16",
	"DataUINT32"            : "DataUInt32",
	"DataUINT64"            : "DataUInt64",

	"DataVec2FLOAT16"       : "DataVec2Float16",
	"DataVec2FLOAT32"       : "DataVec2Float32",
	"DataVec2FLOAT64"       : "DataVec2Float64",
	"DataVec2INT8"          : "DataVec2Int8",
	"DataVec2INT16"         : "DataVec2Int16",
	"DataVec2INT32"         : "DataVec2Int32",
	"DataVec2INT64"         : "DataVec2Int64",
	"DataVec2UINT8"         : "DataVec2UInt8",
	"DataVec2UINT16"        : "DataVec2UInt16",
	"DataVec2UINT32"        : "DataVec2UInt32",
	"DataVec2UINT64"        : "DataVec2UInt64",

	"DataVec3FLOAT16"       : "DataVec3Float16",
	"DataVec3FLOAT32"       : "DataVec3Float32",
	"DataVec3FLOAT64"       : "DataVec3Float64",
	"DataVec3INT8"          : "DataVec3Int8",
	"DataVec3INT16"         : "DataVec3Int16",
	"DataVec3INT32"         : "DataVec3Int32",
	"DataVec3INT64"         : "DataVec3Int64",
	"DataVec3UINT8"         : "DataVec3UInt8",
	"DataVec3UINT16"        : "DataVec3UInt16",
	"DataVec3UINT32"        : "DataVec3UInt32",
	"DataVec3UINT64"        : "DataVec3UInt64",

	"DataVec4FLOAT16"       : "DataVec4Float16",
	"DataVec4FLOAT32"       : "DataVec4Float32",
	"DataVec4FLOAT64"       : "DataVec4Float64",
	"DataVec4INT8"          : "DataVec4Int8",
	"DataVec4INT16"         : "DataVec4Int16",
	"DataVec4INT32"         : "DataVec4Int32",
	"DataVec4INT64"         : "DataVec4Int64",
	"DataVec4UINT8"         : "DataVec4UInt8",
	"DataVec4UINT16"        : "DataVec4UInt16",
	"DataVec4UINT32"        : "DataVec4UInt32",
	"DataVec4UINT64"        : "DataVec4UInt64",
}

ShadingFunctionReplacements = {

    "ShadingFunctionEnum::Enum" : "ShadingFunctionKind",
    "ShadingFunctionEnum::HENYEY_GREENSTEIN" : "ShadingFunctionKind::HenyeyGreenstein",
    "ShadingFunctionEnum::SCHLICK" : "ShadingFunctionKind::Schlick",
    "ShadingFunctionEnum::BLINN_PHONG" : "ShadingFunctionKind::BlinnPhong",
    "ShadingFunctionEnum::WARD" : "ShadingFunctionKind::Ward",
    "ShadingFunctionEnum::COOK_TORRANCE" : "ShadingFunctionKind::CookTorrance",
    "ShadingFunctionEnum::ABC_MICROFACET" : "ShadingFunctionKind::AbcMicrofacet",
    "ShadingFunctionEnum::ASHIKHMIN" : "ShadingFunctionKind::Ashikhmin",
    "ShadingFunctionEnum::MIX" : "ShadingFunctionKind::Mix",
    "ShadingFunctionEnum::ISOTROPIC" : "ShadingFunctionKind::Isotropic"
}

usageModeReplacements =  {
    "APPLICATION" : "UsageMode::Application",
    "DEVELOPMENT" : "UsageMode::Development"
}

drawModeReplacements = {
    "DrawMode::NOT_SPECIFIED" : "DrawMode::NotSpecified",
    "DrawMode::POINTS" : "DrawMode::Points",
    "DrawMode::LINES" : "DrawMode::Lines",
    "DrawMode::LINE_STRIP" : "DrawMode::LineStrip",
    "DrawMode::LINE_LOOP" : "DrawMode::LineLoop",
    "DrawMode::LINES_ADJACENCY" : "DrawMode::LinesAdjacency",
    "DrawMode::LINE_STRIP_ADJACENCY" : "DrawMode::LineStripAdjacency",
    "DrawMode::TRIANGLES" : "DrawMode::Triangles",
    "DrawMode::TRIANGLE_STRIP" : "DrawMode::TriangleStrip",
    "DrawMode::TRIANGLE_FAN" : "DrawMode::TriangleFan",
    "DrawMode::TRIANGLES_ADJACENCY" : "DrawMode::TrianglesAdjacency",
    "DrawMode::TRIANGLE_STRIP_ADJACENCY" : "DrawMode::TriangleStripAdjacency",
    "DrawMode::NUMBER_OF_DRAW_MODES" : "DrawMode::NumberOfDrawModes"    
}

interactionEventTypeReplacements = {
    "NONE_SUPPORTED" : "InteractionEventType::NoneSupported",
    "MOUSE_INTERACTION_EVENT" : "InteractionEventType::MouseInteraction",
    "TOUCH_INTERACTION_EVENT" : "InteractionEventType::TouchInteraction"
}

glVendorReplacements = {
 	"VENDOR_NVIDIA" : "GlVendor::Nvidia",
 	"VENDOR_AMD" : "GlVendor::Amd",
 	"VENDOR_INTEL" : "GlVendor::Intel",
 	"VENDOR_UNKNOWN" : "GlVendor::Unknown"
}


gLFormatsNormalizationReplacements = {
    "GLFormats::NONE": "GLFormats::Normalization::None",
    "GLFormats::NORMALIZED": "GLFormats::Normalization::Normalized",
    "GLFormats::SIGN_NORMALIZED": "GLFormats::Normalization::SignNormalized"
}

cLFormatsNormalizationReplacements = {
    "CLFormats::NONE": "CLFormats::Normalization::None",
    "CLFormats::NORMALIZED": "CLFormats::Normalization::Normalized",
    "CLFormats::SIGN_NORMALIZED": "CLFormats::Normalization::SignNormalized"
}

invalidationLevelReplacements = {
    "VALID" : "InvalidationLevel::Valid",
    "INVALID_OUTPUT" : "InvalidationLevel::InvalidOutput",
    "INVALID_RESOURCES" : "InvalidationLevel::InvalidResources"
}

# order matters here...
for k,v in numericTypeReplacements.items():
	replace(k,v)

for k,v in dataFormatIdReplacements.items():
	replace(k,v)

for k,v in dataFormatTypeReplacements.items():
	replace(k,v)

for k,v in ShadingFunctionReplacements.items():
	replace(k,v)

for k,v in usageModeReplacements.items():
	replace(r"\b"+k+r"\b", v)

for k,v in drawModeReplacements.items():
	replace(r"\b"+k+r"\b", v)

for k,v in interactionEventTypeReplacements.items():
	replace(r"\b"+k+r"\b", v)

for k,v in glVendorReplacements.items():
	replace(r"\b"+k+r"\b", v)

for k,v in gLFormatsNormalizationReplacements.items():
	replace(r"\b"+k+r"\b", v)

for k,v in cLFormatsNormalizationReplacements.items():
	replace(r"\b"+k+r"\b", v)

for k,v in invalidationLevelReplacements.items():
	replace(r"\b"+k+r"\b", v)