import os, sys, time
from pathlib import Path
from contextlib import contextmanager
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
        print('Registering modules')
        self.app.registerModules()
        self.app.resizePool(ivw_poolsize)
        print(f'Resized pool to {self.app.getPoolSize()}')


        # load a workspace
        workspace_path = Path(self.app.getPath(ivw.PathType.Workspaces)) / ivw_network
        self.app.network.load(str(workspace_path))
        self.network = self.app.network
        self.proc_dict = {p.identifier: p for p in self.network.processors}
        print(f'Loaded workspace: {workspace_path}')

        self.set_canvas_hidden_evaluation(True)

        self.injectors = {}

        # Get Background Job Number
        self.app.waitForNetwork(1)
        self.bgJobs = self.app.runningBackgroundJobs()

    def set_canvas_hidden_evaluation(self, value):
        ''' Sets the "evaluateWhenHidden" property of all canvases in the network

        Args:
            value (bool): The value to set "evaluateWhenHidden" to. Usually you want this to be True when working in Python
        '''
        for p in self.network.processors:
            if p.classIdentifier == 'org.inviwo.CanvasGL':
                print(p.properties)
                p.getPropertyByIdentifier('evaluateWhenHidden').value = value

    @contextmanager
    def network_locked(self, wait_after_unlock=False):
        ''' Context manager within which the network is locked

        Args:
            wait_after_unlock (bool, optional): Wait for network evaluation after unlocking. Defaults to False.
        '''
        try:
            self.network.lock()
            yield
        finally:
            self.network.unlock()
            if wait_after_unlock: self.app.waitForNetwork()

    def eval(self):
        ''' Waits for the network evaluation, if the network is unlocked. (Prints warning otherwise) '''
        if self.network.isLocked():
            self.logger.warn("Cannot evaluate network while it is locked! Check InviwoInterface.network.isLocked().")
        else:
            self.app.waitForNetwork()

    def inject_volume_source(self, src_id):
        ''' Replaces VolumeSource processors with a VolumeInjector.

        Args:
            src_id (str): Volume Source processor id

        Returns:
            function: Function that injects np.array volumes into the network
        '''
        inj = self.app.processorFactory.create('org.inviwo.VolumeInjector')
        self.injectors[src_id] = inj
        self.replace_processor(src_id, inj)
        def set_volume(array):
            inj.setArray(array)
        return set_volume

    def inject_image_source(self, src_id):
        ''' Replaces ImageSource Processors with an ImageInjector

        Args:
            src_id (str): Image Source processor id

        Returns:
            function: Function that injects np.array images into the network
        '''
        inj = self.app.processorFactory.create('org.inviwo.ImageInjector')
        self.injectors[src_id] = inj
        self.replace_processor(src_id, inj)
        def set_image(color, depth=None, picking=None):
            inj.setArray(color, depth, picking)
        return set_image

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
