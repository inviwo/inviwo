# command script import <path>/glm_lldb.py
# type summary clear 
# type synthetic clear

import lldb

names = ["x", "y", "z", "w"]

def createSummary(size: int):
    def summary(valobj, internal_dict):
        try:
            elem = (str(valobj.GetChildMemberWithName(names[i]).GetValue()) for i in range(0, size))
            strRep = ", ".join(elem)
            return f"[{strRep}]"
        except Exception as e:
            return "Error Formatting: " + str(e)
    return summary

def createSyntheticProvider(size: int):
    class sp:
        def __init__(self, valobj, internal_dict):
           self.comps = [valobj.GetChildMemberWithName(names[i]) for i in range(0, size)]

        def num_children(self):
            return size

        def get_child_at_index(self, idx):
            if idx < size:
                return self.comps[idx]
            else:
                return None
    
        def get_child_index(self, name):
            if name in names:
                return names.index(name)
            return -1
    return sp

glmVec1 = createSummary(1)
glmVec2 = createSummary(2)
glmVec3 = createSummary(3)
glmVec4 = createSummary(4)

GLMVec1SyntheticProvider = createSyntheticProvider(1)
GLMVec2SyntheticProvider = createSyntheticProvider(2)
GLMVec3SyntheticProvider = createSyntheticProvider(3)
GLMVec4SyntheticProvider = createSyntheticProvider(4)

def __lldb_init_module(debugger, internal_dict):
    for i in [1, 2, 3, 4]:
        debugger.HandleCommand(
            f'type summary add -w glm -F glm_lldb.glmVec{i} -x "^glm::vec<{i},[^>]+>$"'
        )
        debugger.HandleCommand(
            f'type synthetic add -w glm --python-class glm_lldb.GLMVec{i}SyntheticProvider -x "^glm::vec<{i},[^>]+>$"'
        )
