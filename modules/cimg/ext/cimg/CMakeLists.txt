add_library(cimg INTERFACE)
add_library(inviwo::cimg ALIAS cimg)

target_include_directories(cimg INTERFACE include)
target_compile_definitions(cimg INTERFACE 
	cimg_verbosity=0 # Disable all cimg output
	cimg_display=0   # Do not use any gui stuff
)
