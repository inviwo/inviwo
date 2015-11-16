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
	r"\bDataReaderFactory::getPtr\(\)" : r"InviwoApplication::getPtr()->getDataReaderFactory()",
	r"\bDataWriterFactory::getPtr\(\)" : r"InviwoApplication::getPtr()->getDataWriterFactory()",
	r"\bDialogFactory::getPtr\(\)" : r"InviwoApplication::getPtr()->getDialogFactory()",
	r"\bMeshDrawerFactory::getPtr\(\)" : r"InviwoApplication::getPtr()->getMeshDrawerFactory()",
	r"\bMetaDataFactory::getPtr\(\)" : r"InviwoApplication::getPtr()->getMetaDataFactory()",
	r"\bPortFactory::getPtr\(\)" : r"InviwoApplication::getPtr()->getPortFactory()",
	r"\bPortInspectorFactory::getPtr\(\)" : r"InviwoApplication::getPtr()->getPortInspectorFactory()",
	r"\bProcessorFactory::getPtr\(\)" : r"InviwoApplication::getPtr()->getProcessorFactory()",
	r"\bProcessorWidgetFactory::getPtr\(\)" : r"InviwoApplication::getPtr()->getProcessorWidgetFactory()",
	r"\bPropertyFactory::getPtr\(\)" : r"InviwoApplication::getPtr()->getPropertyFactory()",
	r"\bPropertyWidgetFactory::getPtr\(\)" : r"InviwoApplication::getPtr()->getPropertyWidgetFactory()",
	r"\bPropertyConverterManager::getPtr\(\)" : r"InviwoApplication::getPtr()->getPropertyConverterManager()",
	r"\bRepresentationConverterFactory::getPtr\(\)" : r"InviwoApplication::getPtr()->getRepresentationConverterFactory()"
};

# order matters here...
for k,v in factoryReplacements.items():
	replace(k, v)