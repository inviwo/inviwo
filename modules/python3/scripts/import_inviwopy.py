import sys
if sys.version_info[1] < 5:
    from importlib.machinery import SourceFileLoader
    inviwopy = SourceFileLoader("inviwopy", path_to_file).load_module()
else:
    import importlib.util
    spec = importlib.util.spec_from_file_location("inviwopy", path_to_file)
    inviwopy = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(inviwopy)
