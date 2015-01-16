import sys
import inviwo_internal

class OutputRedirectStdout(object):
    def __init__(self,out):
        self.old = out
        self = out
        
    def write(self, string):
        inviwo_internal.ivwPrint(string, 0)
        self.old.write(string+'\n')

    def start(self):
        sys.stdout = self

    def stop(self):
        sys.stdout = self.old
        return self

class OutputRedirectStderr(object):
    def __init__(self,err):
        self.old = err
        self = err
        
    def write(self, string):
        inviwo_internal.ivwPrint(string, 1)
        self.old.write(string+'\n')

    def start(self):
        sys.stderr = self

    def stop(self):
        sys.stderr = self.old
        return self

        
sys.stdout = OutputRedirectStdout(sys.stdout)
sys.stderr = OutputRedirectStderr(sys.stderr)
