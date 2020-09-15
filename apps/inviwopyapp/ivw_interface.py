import os, sys, time
from pathlib import Path
import numpy as np
from PIL import Image

import inviwopy as ivw
from inviwopy.glm import vec4, dvec2, size2_t
from inviwopy.properties import InvalidationLevel
import inviwopyapp as qt

class InviwoInterface:
    def __init__(self, ivw_appname, ivw_network, ivw_poolsize=0):
        self.log_central = ivw.LogCentral()
        self.logger = ivw.ConsoleLogger()
        self.log_central.registerLogger(self.logger)

        # Create the inviwo application
        self.app = qt.InviwoApplicationQt(ivw_appname)
        self.app.registerModules()
        self.app.resizePool(ivw_poolsize)


        # load a workspace
        workspace_path = Path(self.app.getPath(ivw.PathType.Workspaces)) / ivw_network
        self.app.network.load(str(workspace_path))
        self.network = self.app.network
        self.proc_dict = {str(p): p for p in self.network.processors}
        print(f'Loaded workspace: {workspace_path}')

        self.injectors = {}

        # Get Background Job Number
        self.app.waitForNetwork(1)
        self.bgJobs = self.app.runningBackgroundJobs()

    def inject_volume_source(self, src_id):
        ''' Replaces VolumeSource processors with a VolumeInjector

        Args:
            src_id (str): Volume Source processor id

        Returns:
            function: Function that injects np.array volumes into the network
        '''
        inj = self.app.processorFactory.create('org.inviwo.VolumeInjector')
        self.injectors[src_id] = inj
        self.replace_processor(src_id, inj)
        def set_volume(array, invalidate=True):
            inj.setArray(array)
            if invalidate:
                inj.invalidate(InvalidationLevel.InvalidOutput)
        return set_volume

    ##### DO same for Image, Mesh?

    def replace_processor(self, old_id, new_proc, allow_partial_connections=True):
        ''' Replaces VolumeSource processors with a VolumeInjector

        Args:
            old_id (str): ID of the processor to be replaced
            new_proc (ivw.Processor): New processor
            allow_partial_connections (bool): Don't raise exception if not all ports can be matched properly.

        '''
        old_proc = self.proc_dict[old_id]
        if not allow_partial_connections:
            assert len(old_proc.outports) == len(new_proc.outports)
            for oldo, newo in zip(old_proc.outports, new_proc.outports):
                assert type(oldo) == type(newo)
        self.network.lock()
        self.network.addProcessor(new_proc)
        keep_old_proc = False
        for oldop, newop in zip(old_proc.outports, new_proc.outports):
            if type(oldop) == type(newop):
                old_cons = [(c.outport, c.inport) for c in self.network.connections if c.outport == oldop]
                new_cons = [(newop, dst) for src, dst in old_cons]
                for src, dst in old_cons:
                    print(f'Removing Connection {src.processor} -> {dst.processor}.')
                    self.network.removeConnection(src, dst)
                for src, dst in new_cons:
                    print(f'Adding Connection {src.processor} -> {dst.processor}.')
                    self.network.addConnection(src, dst)
            else:
                print(f'Non matching type: {oldop} ({type(oldop)})  vs.  {newop} ({type(newop)}).')
                keep_old_proc = True
        if not keep_old_proc: self.network.removeProcessor(old_proc)
        self.network.unlock()
