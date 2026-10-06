import asyncio, threading
from concurrent.futures import Future
import sys
import os
import time
import subprocess
import re
import webbrowser
import socket
import json
from pathlib import Path


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
from fastmcp.utilities.types import Image

import logging

logger = logging.getLogger("inviwomcp")
logger.setLevel(logging.DEBUG)
handler = logging.StreamHandler()
handler.setFormatter(logging.Formatter("%(asctime)s %(message)s"))
logger.addHandler(handler)

WORKSPACE_PATH = r"C:\dev\inviwo-project\inviwo"
VS_CODE_PATH = r"C:\Users\oskar\AppData\Local\Programs\Microsoft VS Code\Code.exe"
MCP_URL = "http://127.0.0.1:8000/mcp"

def MCP_server(approach: str = "command", launch_extra: bool = True):
    app = inviwopy.app
    network = app.network
    mcp = FastMCP("Inviwo MCP")

    if approach is not None:
        if approach == "command":
            import inviwomcp.tools.command_based as command_based
            command_based.register_tool(mcp, app, network, logger)
            inviwopy.log("Registered command based tools")
    
        elif approach == "xml":
            import inviwomcp.tools.xml_based as xml_based
            xml_based.register_tool(mcp, app, network, logger)
            inviwopy.log("Registered xml based tools")
    
        elif approach == "script":
            import inviwomcp.tools.script_based as script_based
            script_based.register_tool(mcp, app, network, logger)
            inviwopy.log("Registered script based tools")

    else:
        import inviwomcp.tools.command_based as command_based
        command_based.register_tool(mcp, app, network, logger)
        import inviwomcp.tools.xml_based as xml_based
        xml_based.register_tool(mcp, app, network, logger)
        import inviwomcp.tools.script_based as script_based
        script_based.register_tool(mcp, app, network, logger)

        print("Registered all tools")

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
            inviwopy.log("MCP Inspector launched")
        except Exception as e:
            inviwopy.log(f"Error: Could not start MCP Inspector: {e}")


    def launch_VS_code():
        wait_for_server()
        try:
            subprocess.Popen([VS_CODE_PATH, WORKSPACE_PATH])
            inviwopy.log("VS Code launched")
        except Exception as e:
            inviwopy.log(f"Error: Could not open VS Code: {e}")


    threading.Thread(target=run, daemon=True).start()
    if launch_extra:
        threading.Thread(target=launch_inspector, daemon=True).start()
        threading.Thread(target=launch_VS_code, daemon=True).start()

    inviwopy.log("MCP server started")
