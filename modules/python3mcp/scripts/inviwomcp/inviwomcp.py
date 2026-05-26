import asyncio, threading
from concurrent.futures import Future
import sys
import os
import time
import subprocess
import re
import webbrowser
import socket
#from mcp.server.fastmcp import FastMCP, Image, Context

# An mcp.json should be in the project with following content
#{
#    "servers": {
#        "inviwo": {
#            "type": "http",
#            "url": "http://127.0.0.1:8000/mcp"
#        }
#    }
#}

# If not, go to vs code in the inviwo folder, press ctrl + shift + P, write mcp and chose
# "MCP: open workspace folder MCP configuration" and create file and paste  in the above content

# When running Inviwo open cmd and type following command to open UI for mcp
# npx @modelcontextprotocol/inspector http://127.0.0.1:8000/mcp

# Change transport type to Streammable HTTP & URL to http://127.0.0.1:8000/mcp
# Change inspector proxy address to given location in cmd window
# Input the session token from cmd into proxy session token
# Connect

# Works as long a stdio isn't used (pywin32 is required which isn't installed/accessed properly)
# Tricks the system that pywin32 needed parts for mcp and fastmcp is already loaded
import unittest.mock

sys.modules['mcp.client.stdio'] = unittest.mock.MagicMock()
sys.modules['mcp.client.session_group'] = unittest.mock.MagicMock()
sys.modules['pywintypes'] = unittest.mock.MagicMock()
sys.modules['win32api'] = unittest.mock.MagicMock()
sys.modules['win32con'] = unittest.mock.MagicMock()
sys.modules['win32job'] = unittest.mock.MagicMock()

# Inviwo's OutputRedirector doesn't have isatty — add it
if not hasattr(sys.stdout, 'isatty'):
    sys.stdout.isatty = lambda: False
if not hasattr(sys.stderr, 'isatty'):
    sys.stderr.isatty = lambda: False

# Also disable uvicorn's color logging to avoid stdout issues
os.environ['NO_COLOR'] = '1'
os.environ['TERM'] = 'dumb'

import inviwopy
from fastmcp import FastMCP

WORKSPACE_PATH = r"C:\dev\inviwo-project\inviwo"
VS_CODE_PATH = r"C:\Users\oskar\AppData\Local\Programs\Microsoft VS Code\Code.exe"
MCP_URL = "http://127.0.0.1:8000/mcp"

_stop_event = asyncio.Event()
_thread = None

instructions = """"

You are an AI visualization assistant integrated within the inviwo scientific visualization framework through Model Context Protocol with FastMCP.

ROLE
Your task is to help users construct, modify, analyze and explain scientific visualization pipelines.

SYSTEM MODEL
The visualization system is processor-based. Pipelines consist of interconnected processors where:
- processors perform operations on data
- processors have input and output ports
- processors have configurable properties
- valid data flow and port compatibility must be considered

CAPABILITIES
You have access to:
- MCP resources which expose the current network state, available processors, processor metadata, port information (and later canvas output)
- MCP tools that can manipulate the visualization pipeline

PIPELINE GENERATION RULES
When generating or modifying pipelines:
- Ensure processor connections are compatible
- Prefer simple and valid pipeline structure before complexity
- Ensure required data sources exist before connecting dependent processors
- Configure processor properties when needed
- Avoid unnecessary changes to existing valid pipeline structure
- Apply modifications incrementally
- Do not assume the existence of processors, ports or properties that are not exposed through MCP resources.

MODIFICATION PROCEDURE
Before modifying a pipeline:
1. Read the current network state
2. Inspect relevant processors and their metadata
3. Determine whether an existing processor can be reused
4. Plan the minimal sequence of modifications required to fulfill the user query

USER INTERACTION
- Ask clarifying questions if the visualization goal is ambiguous
- Use terminology from scientific visualization and computer graphics when appropriate
- Focus on correctness and interpretability
- Prefer clear, reusable and well-structured pipeline layouts.

FAILURE HANDLING
If a request cannot be completed:
- explain why
- identify missing processors, incompatible ports or unavailable data
- suggest alternatives when possible

EXECUTION LOOP
You may iteratively:
- inspect resources
- reason about the network
- call tools
- re-evaluate the resulting pipeline state

OBJECTIVE
Your objective is to produce valid and meaningful visualization pipelines aligned with the user's analytical goals.

EXAMPLE PIPELINE
Volume source → Isosurface extraction → Mesh rendering → Canvas
"""


# def test():
#     print("Hello Inviwo MCP!")

#     inviwopy.log("Hello Inviwo MCP!")

def MCP_server():
    app = inviwopy.app
    network = app.network
    mcp = FastMCP("Inviwo MCP")

#-------------------------------------------- Functions defined as tools, mainly for command based interaction ----------------------------------------------

    @mcp.tool()
    async def network_state() -> dict:
        """Lists all processors in the Inviwo network with their id and type."""
        loop = asyncio.get_event_loop()
        future = loop.create_future()
        try:
            def get_state():
                network_dict = {}
                processors = network.processors

                for p in processors:
                    processor_connections = []
                    connections = network.connections

                    for c in connections:
                        if c.outport.processor == p or c.inport.processor == p:
                            processor_connections.append({
                                "from": f"{c.outport.processor.identifier}.{c.outport.identifier}",
                                "to": f"{c.inport.processor.identifier}.{c.inport.identifier}"})

                    network_dict[p.identifier] = {
                        "classIdentifier": p.classIdentifier,
                        "identifier": p.identifier,
                        "connections": processor_connections}

                return {"sucess": True, "network dictionary" : network_dict}

        except Exception as e:
            return{"success": False, "error": str(e)}

        def call_dispatch():
            data = app.dispatch_front(get_state)
            loop.call_soon_threadsafe(future.set_result, data)

        threading.Thread(target=call_dispatch).start()
        return await future


    @mcp.tool()
    async def list_all_processors() -> dict:
        """Lists all processors available with their id and type."""
        loop = asyncio.get_event_loop()
        future = loop.create_future()

        def get_processors():
            processor_factory = app.processorFactory
            processor_dict = {}

            for key in processor_factory.keys:
                if not key.startswith("org.inviwo"):
                    continue

                try:
                    p = processor_factory.create(key)
                    info = p.getProcessorInfo()
                    if p.category == "Meta":
                        continue

                    processor_dict[p.identifier] = {
                        "identifier": p.identifier,
                        "classIdentifier": p.classIdentifier,
                        "displayName": p.displayName,
                        "category": p.category,
                        "tags": p.tags.getString(),
                        "codeState": p.codeState.name
                    }
                except Exception as e:
                    processor_dict[p.identifier] = {"identifier": p.identifier, "error": str(e)}

            return {"processors": processor_dict}

        def call_dispatch():
            data = app.dispatch_front(get_processors)
            loop.call_soon_threadsafe(future.set_result, data)

        threading.Thread(target=call_dispatch).start()
        return await future


    @mcp.tool()
    async def add_processor(classIdentifier: str, identifier: str = "", x_pos: int = 0, y_pos: int = 0) -> dict:
        """Add a processor to the Inviwo network by its class identifier, custom identifier can be assigned if seen as fit."""
        loop = asyncio.get_event_loop()
        future = loop.create_future()

        def create_processor():
            try:
                p = app.processorFactory.create(classIdentifier)
                if identifier:
                    p.identifier = identifier

                p.meta.position = inviwopy.glm.ivec2(x_pos, y_pos)
                app.network.addProcessor(p)
                return {"success": True, "classIdentifier": classIdentifier, "identifier": p.identifier }

            except Exception as e:
                return {"success": False, "error": str(e)}

        def call_dispatch():
            data = app.dispatch_front(create_processor)
            loop.call_soon_threadsafe(future.set_result, data)

        threading.Thread(target=call_dispatch).start()
        return await future


    @mcp.tool()
    async def remove_processor(identifier: str) -> dict:
        """Remove a processor based on identifier"""
        loop = asyncio.get_event_loop()
        future = loop.create_future()

        def find_processor():
            try:
                p = network.getProcessorByIdentifier(identifier)
                if p is None:
                    return {"success": False, "error": f"Processor '{identifier}' not found"}

                network.removeProcessor(p)
                return {"success": True, "Processor removed: ": identifier}

            except Exception as e:
                return {"success": False, "error: ": str(e)}

        def call_dispatch():
            data = app.dispatch_front(find_processor)
            loop.call_soon_threadsafe(future.set_result, data)

        threading.Thread(target=call_dispatch).start()
        return await future


    @mcp.tool()
    async def add_connection(sourceIdentifier: str, sourcePort: str, destIdentifier: str, destPort: str) -> dict:
        """Connect an outport of one processor to an inport of another"""
        loop = asyncio.get_event_loop()
        future = loop.create_future()

        def connect():
            try:
                sourceProcessor = network.getProcessorByIdentifier(sourceIdentifier)
                destProcessor = network.getProcessorByIdentifier(destIdentifier)
                outport = sourceProcessor.getOutport(sourcePort)
                inport = destProcessor.getInport(destPort)
                network.addConnection(outport, inport)
                return {"success": True}

            except Exception as e:
                return {"success": False, "error": str(e)}

        def call_dispatch():
            data = app.dispatch_front(connect)
            loop.call_soon_threadsafe(future.set_result, data)

        threading.Thread(target=call_dispatch).start()
        return await future


    @mcp.tool()
    async def remove_connection(sourceIdentifier: str, sourcePort: str, destIdentifier: str, destPort: str) -> dict:
        """Remove a connection between two processors"""
        loop = asyncio.get_event_loop()
        future = loop.create_future()

        def disconnect():
            try:
                sourceProcessor = network.getProcessorByIdentifier(sourceIdentifier)
                destProcessor = network.getProcessorByIdentifier(destIdentifier)
                outport = sourceProcessor.getOutport(sourcePort)
                inport = destProcessor.getInport(destPort)
                network.removeConnection(outport, inport)
                return {"success": True}

            except Exception as e:
                return {"success": False, "error": str(e)}

        def call_dispatch():
            data = app.dispatch_front(disconnect)
            loop.call_soon_threadsafe(future.set_result, data)

        threading.Thread(target=call_dispatch).start()
        return await future

    @mcp.tool()
    async def get_processor_ports_info(identifier: str) -> dict:
        """List a processors port information such as type of inports, outports, whether they are connected"""
        loop = asyncio.get_event_loop()
        future = loop.create_future()

        def get_info():
            try:
                p = network.getProcessorByIdentifier(identifier)
                if p is None:
                    return {"success": False, "error": f"Processor '{identifier}' not found"}

                inports = {}
                for port in p.inports:
                    connected_outports = []

                    for co in port.getConnectedOutports():
                        connected_outports.append({
                            "processor": co.processor.identifier,
                            "port": co.identifier})

                    inports[port.identifier] = {
                        "identifier": port.identifier,
                        "connected": port.isConnected(),
                        "optional": port.optional,
                        "connections": connected_outports
                    }

                outports = {}
                for port in p.outports:
                    connected_inports = []

                    for ci in port.getConnectedInports():
                        connected_inports.append({
                            "processor": ci.processor.identifier,
                            "port": ci.identifier})

                    outports[port.identifier] = {
                        "identifier": port.identifier,
                        "connected": port.isConnected(),
                        "connections": connected_inports
                    }

                port_info = {
                    "inports": inports,
                    "outports": outports}


                return {"success": True, f"Port info for {p.identifier}": port_info}

            except Exception as e:
                return {"success": False, "error": str(e)}

        def call_dispatch():
            data = app.dispatch_front(get_info)
            loop.call_soon_threadsafe(future.set_result, data)

        threading.Thread(target=call_dispatch).start()
        return await future


    @mcp.tool()
    async def get_processor_properties(identifier: str) -> dict:
        """Get properties of a processor"""
        loop = asyncio.get_event_loop()
        future = loop.create_future()

        def get_properties(prop):
            info = {
                "classIdentifier": prop.classIdentifier,
                "identifier": prop.identifier,
                "displayName": prop.displayName,
                #"description": prop.getDescription().str(),
            }

            # Optional semantic metadata
            if hasattr(prop, "path"):
                try:
                    info["path"] = prop.path
                except Exception:
                    pass

            if hasattr(prop, "semantics"):
                try:
                    info["semantics"] = str(prop.semantics)
                except Exception:
                    pass

            if hasattr(prop, "value"):
                try:
                    info["value"] = str(prop.value)
                except Exception:
                    pass

            if hasattr(prop, "minValue"):
                try:
                    info["minValue"] = str(prop.minValue)
                    info["maxValue"] = str(prop.maxValue)
                except Exception:
                    pass

            if hasattr(prop, "displayNames"):
                try:
                    info["options"] = list(prop.displayNames)
                    info["selectedOption"] = prop.selectedDisplayName
                except Exception:
                    pass

            if hasattr(prop, "properties") and len(prop.properties) > 0:
                sub_properties = {}

                for sub in prop.properties:
                    sub_properties[sub.identifier] = get_properties(sub)

                info["properties"] = sub_properties

            return info

        def get_info():
            try:
                p = network.getProcessorByIdentifier(identifier)
                if p is None:
                    return {"success": False, "error": f"Processor '{identifier}' not found"}

                properties = {}

                for prop in p.properties:
                    properties[prop.identifier] = get_properties(prop)

                processor_info = {
                    "identifier": p.identifier,
                    "classIdentifier": p.classIdentifier,
                    "displayName": p.displayName,
                    "properties": properties
                }

                return {"success": True, "processor": processor_info}

            except Exception as e:
                return {"success": False, "error": str(e)}

        def call_dispatch():
            data = app.dispatch_front(get_info)
            loop.call_soon_threadsafe(future.set_result, data)

        threading.Thread(target=call_dispatch).start()
        return await future


    @mcp.tool()
    async def set_processor_property(processorIdentifier: str, propertyIdentifier: str, value: str) -> dict:
        """Set a property value on a processor, if property is a drowdown menu selection, value is the index of the wanted property selection"""
        loop = asyncio.get_event_loop()
        future = loop.create_future()

        def set_info():
            try:
                prop = network.getProperty(f"{processorIdentifier}.{propertyIdentifier}")
                if prop is None:
                    return {"success": False, "error": "Property not found"}

                if hasattr(prop, "selectedIndex"):
                        prop.selectedIndex = int(value)

                else:
                    prop.value = eval(value)

                return {"success": True}

            except Exception as e:
                return {"success": False, "error": str(e)}

        def call_dispatch():
            data = app.dispatch_front(set_info)
            loop.call_soon_threadsafe(future.set_result, data)

        threading.Thread(target=call_dispatch).start()
        return await future
            
    
    #@mcp.tool()
    #async def save_network(path: str = "") -> dict:
    #    """Save the current Inviwo network to a file, leaving path empty will overwrite current save file"""
    #    loop = asyncio.get_event_loop()
    #    future = loop.create_future()

    #    def save():
    #        try:
    #            if path:
    #                save_path = path
    #
    #            else:
    #                workspaces_dir = app.getPath(inviwopy.PathType.Workspaces)
    #                #display_name = app.displayName
    #                #display_name = network.getFileName()
    #                if not display_name.endswith(".inv"):
    #                    display_name += ".inv"
    #                save_path = str(workspaces_dir / display_name)
    #            network.save(save_path)
    #            return {"success": True, "path": save_path}

    #        except Exception as e:
    #            return {"success": False, "error": str(e)}

    #    def call_dispatch():
    #        data = app.dispatch_front(save)
    #        loop.call_soon_threadsafe(future.set_result, data)

    #    threading.Thread(target=call_dispatch).start()
    #    return await future

#---------------------------- Functions for launching VS code, MCP Inspector and MCP server, as well as keeping it running as long as Inviwo is running -------------------
    def run():
        loop = asyncio.new_event_loop()
        asyncio.set_event_loop(loop)
        loop.run_until_complete(mcp.run_async(transport="http", host="127.0.0.1", port=8000))

    
    def wait_for_server(host="127.0.0.1", port=8000, timeout=30):
        start = time.time()
        while time.time() - start < timeout:
            try:
                with socket.create_connection((host, port), timeout=1):
                    return True
            except OSError:
                time.sleep(0.5)
        return False

    
    def launch_inspector():
        wait_for_server()
        print("Launching MCP Inspector...")
        try:
            subprocess.run(
                ["npx", "kill-port", "6277", "6274"],
                shell=True,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL)
            
            subprocess.Popen(
                f"npx @modelcontextprotocol/inspector {MCP_URL}",
                shell=True,
                env={**os.environ, "DANGEROUSLY_OMIT_AUTH": "true"})
            print("MCP Inspector launched")
        except Exception as e:
            print(f"Error: Could not start MCP Inspector: {e}")


    def launch_VS_code():
        wait_for_server()
        try:
            subprocess.Popen([VS_CODE_PATH, WORKSPACE_PATH])
            print("VS Code launched")
        except Exception as e:
            print(f"Error: Could not open VS Code: {e}")

    threading.Thread(target=run, daemon=True).start()
    threading.Thread(target=launch_inspector, daemon=True).start()
    threading.Thread(target=launch_VS_code, daemon=True).start()
    inviwopy.log("MCP server started")
