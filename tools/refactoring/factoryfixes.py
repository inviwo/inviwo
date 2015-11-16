import sys 
import os
import re
import colorama
colorama.init()

import refactoring # Note: refactoring.py need to be in the current working directory

paths = ["/Users/petst/Work/Projects/Inviwo-Developent/Private/Inviwo-dev", "/Users/petst/Work/Projects/Inviwo-Developent/Private/Inviwo-research"]

excludespatterns = ["*/ext/*", "*moc_*", "*cmake*", "*/proteindocking/*", "*/proteindocking2/*", "*/genetree/*", "*/vrnbase/*"];

files = refactoring.find_files(paths, ['*.h', '*.cpp'], excludes=excludespatterns)

def replace(pattern, replacement) :
	print("Matches: "  + pattern)
	matches = refactoring.find_matches(files, pattern)
	
	print("\n")
	print("Replacing: " + replacement)
	refactoring.replace_matches(matches, pattern, replacement)

factoryReplacements = {
	"DataReaderFactory::getPtr()" : "InviwoApplication::getPtr()->DataReaderFactory()",
	"DataWriterFactory::getPtr()" : "InviwoApplication::getPtr()->DataWriterFactory()",
	"DialogFactory::getPtr()" : "InviwoApplication::getPtr()->DialogFactory()",
	"MeshDrawerFactory::getPtr()" : "InviwoApplication::getPtr()->MeshDrawerFactory()",
	"MetaDataFactory::getPtr()" : "InviwoApplication::getPtr()->MetaDataFactory()",
	"PortFactory::getPtr()" : "InviwoApplication::getPtr()->PortFactory()",
	"PortInspectorFactory::getPtr()" : "InviwoApplication::getPtr()->PortInspectorFactory()",
	"ProcessorFactory::getPtr()" : "InviwoApplication::getPtr()->ProcessorFactory()",
	"ProcessorWidgetFactory::getPtr()" : "InviwoApplication::getPtr()->ProcessorWidgetFactory()",
	"PropertyFactory::getPtr()" : "InviwoApplication::getPtr()->PropertyFactory()",
	"PropertyWidgetFactory::getPtr()" : "InviwoApplication::getPtr()->PropertyWidgetFactory()",
	"PropertyConverterManager::getPtr()" : "InviwoApplication::getPtr()->PropertyConverterManager()",
	"RepresentationConverterFactory::getPtr()" : "InviwoApplication::getPtr()->RepresentationConverterFactory()"
};

# order matters here...
for k,v in factoryReplacements.items():
	replace(r"\b"+k+r"\b", v)