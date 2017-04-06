import sys
import os.path

print(sys.version_info[1])
print(path_to_file)
print(os.path.isfile(path_to_file))



if sys.version_info[1] < 5:
    from importlib.machinery import SourceFileLoader
    inviwopy = SourceFileLoader("inviwopy", path_to_file).load_module()
else:
    import importlib.util
    spec = importlib.util.spec_from_file_location("inviwopy", path_to_file)
    inviwopy = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(inviwopy)
