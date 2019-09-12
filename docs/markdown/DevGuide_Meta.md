# Generating files for Processors, Properties and Modules with Inviwo Meta

To create a new module for Inviwo there is a helper program which creates the folder structure,
the CMake files, basic module files and if needed example processors and shaders. To use the
helper, you need to build the inviwo-meta-tools target. After building them they can be found
in the same directory as the Inviwo executable.

Note that the same functionality as provided by this command line utility is also available inside the GUI through `Tools > Create Sources > Create Processor`.
Make sure to also write any module and processor names in PascalCase and without file extension (Example `MyNewProcessor`).

The following commands work on Windows. On Linux
and Mac, simply omit the `.exe` from the commands.

1. If you have built Inviwo in Release mode open the Inviwo build folder at `bin/Release` (Visual Studio) or just `bin/` (Unix) with
a terminal.

2. Create a new module by entering\
`./inviwo-meta-cli.exe -m <inviwo-dir>/modules/ModuleNameInPascalCase -o modules`\
**Note**: The module name needs to be globally unique.

3. Create a processor by entering\
`./inviwo-meta-cli.exe -p <module-dir>/src/processor/ProcessorNameInPascalCase`\
A small example processor is created which can then be modified. The processor is automatically registered in the module and the CMake files.

4. To add, for example, a helper file in `utils` you can use\
`./inviwo-meta-cli.exe -f <module-dir>/src/utils/<file-name>`

A few more notes:
- When creating a module, you might also want to change the `depends.cmake` to depend on other Inviwo modules. One
common dependency is for example the `InviwoOpenGLModule`.
Dependencies must be set, otherwise it is not possible to include files from other modules.
As all modules depend on the Inviwo core, files from the core can always be included.
- Do not simply rename the created processors. In order for a processor to work properly, it must be registered in its module and the files need to be included in CMake's source files. The Meta module does all that for you. If you change a processors name manually afterwards, you have to adapt in multiple places.
