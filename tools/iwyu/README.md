

From the bild dir

	cmake -S ../../inviwo -B . --preset iwyu
	../../inviwo/tools/iwyu/run.sh mymodule

you need to rerun cmake to make any change to the mappingfile (.imp) to take affect 