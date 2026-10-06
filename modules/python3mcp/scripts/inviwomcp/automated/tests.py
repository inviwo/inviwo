""" Connects to whichever server is already running (the normal
MCP_server(), port 8000, current APPROACH) doesn't touch server config at all yet. """
 
import asyncio
import json
from pathlib import Path
from datetime import timedelta

from copilot import CopilotClient
from copilot.session import PermissionHandler
from copilot.session_events import (
    AssistantMessageData,
    SessionIdleData,
    SessionShutdownData,
    SessionUsageInfoData,
    ToolExecutionCompleteData,
    ToolExecutionStartData,
    SessionErrorData
)

from inviwomcp.inviwomcp import *
 
MCP_URL = "http://127.0.0.1:8000/mcp"

SCENARIOS_PATH = Path(r"C:\Users\oskar\Dokument\Skola\Examensarbete\Projekt\automated\scenarios.json")
APPROACHES_PATH = Path(r"C:\Users\oskar\Dokument\Skola\Examensarbete\Projekt\automated\approaches.json")


#{ re-add to secenarios.json later
#    "id": "scenario_2",
#    "query": "Create a visualization pipeline based on the provided sources. It should be cuboid.",
#    "path": "C:\\Users\\oskar\\Dokument\\Skola\\Examensarbete\\Projekt\\automated\\workspace\\boron_source.inv"
#}

INSTRUCTIONS = Path(r"C:\Users\oskar\Dokument\Skola\Examensarbete\Projekt\.claude\agents\instructions-config\inviwo_agent_thorough.md").read_text(encoding="utf-8")

def load_scenarios() -> list[dict]:
    data = json.loads(SCENARIOS_PATH.read_text(encoding="utf-8"))

    scenarios = data["scenarios"]

    for scenario in scenarios:
        scenario["path"] = Path(scenario["path"])

    return scenarios

def load_approaches() -> dict[str, list[str]]:
    return json.loads(APPROACHES_PATH.read_text(encoding="utf-8"))


SCENARIOS = load_scenarios()
APPROACHES = load_approaches()
 

async def run_scenario(scenario: dict, approach: str, tools: list[str]) -> dict:
    final_message = None
    context_window = None
    session_error = None
    mcp_tool_calls = {}
    scenario_id = scenario["id"]
    done = asyncio.Event()
    t_start = time.monotonic()

    def on_event(event):
        nonlocal final_message, context_window, session_error

        #print(f"EVENT: {event}", flush=True)

        match event.data:
            case AssistantMessageData() as data:
                final_message = data.content

            case SessionErrorData() as data:
                session_error = {
                    "error_type": getattr(data, "error_type", None),
                    "message": getattr(data, "message", str(data)),
                }
                print(f"Copilot session error: {session_error}")

            case SessionUsageInfoData() as data:
                context_window = {
                    "token_limit": data.token_limit,
                    "current_tokens": data.current_tokens,
                    #"messages_length": data.messages_length,
                }

            case ToolExecutionStartData() as data:
                if data.mcp_server_name:
                    mcp_tool_calls[data.tool_call_id] = {
                        "tool_call_id": len(mcp_tool_calls) + 1,
                        "tool_name": data.mcp_tool_name,
                        "arguments": data.arguments,
                    }

            case ToolExecutionCompleteData() as data:
                if data.tool_call_id in mcp_tool_calls:
                    mcp_tool_calls[data.tool_call_id]["success"] = data.success
                    mcp_tool_calls[data.tool_call_id]["error"] = data.error

            case SessionIdleData():
                done.set()


    async with CopilotClient(use_logged_in_user=True) as client:
        session = None
        try:
            session = await client.create_session(
                on_permission_request=PermissionHandler.approve_all,
                model="claude-sonnet-4.6",
                reasoning_effort="low",
                memory={"enabled": False},
                system_message={
                    "mode": "replace",
                    "content": INSTRUCTIONS, #add instructions folder 
                },
                mcp_servers={
                    "inviwo": {
                        "type": "http",
                        "url": MCP_URL,
                        "tools": tools
                    }
                }
            )
            session.on(on_event)
            await session.send(scenario["query"])
            await asyncio.wait_for(done.wait(), timeout=300)

            tool_calls = list(mcp_tool_calls.values())

            failed_tool_calls = [
                call for call in tool_calls
                if call.get("success") is False
            ]
            result = {
                "scenario_id": f"{scenario_id}_{approach}",
                "error": session_error,
                "response": final_message,
                "duration_s": time.monotonic() - t_start,
                "context_window": context_window,
                "mcp_tool_call_count": len(mcp_tool_calls),
                "failed_mcp_tool_call_count": len(failed_tool_calls),
                "mcp_tool_calls": tool_calls,
    
            }
        except asyncio.TimeoutError: 
            result = {"scenario_id": f"{scenario_id}_{approach}", "status": "timeout", "mcp_tool_calls": list(mcp_tool_calls.values())}
        except Exception as e:
            result = {"scenario_id": f"{scenario_id}_{approach}", "status": "error", "error": str(e), "mcp_tool_calls": list(mcp_tool_calls.values())}
        finally:
            if session is not None:
                output_directory = Path("C:/Users/oskar/Dokument/Skola/Examensarbete/Projekt/automated/result")
                approach_directory = output_directory / scenario_id / approach
                timestamp = str(time.monotonic())
                file_path = approach_directory / timestamp


                workspace_result = await save_workspace(file_path)
                image_result = await get_canvas_image(file_path)
          
                await session.disconnect()
           
        return result
 
 
def run_test() -> None:
    def start():
        time.sleep(10) #Letting Inviwo initialize
        MCP_server(approach=None, launch_extra=False)

        async def run_all_scenarios():
            for approach, tools in APPROACHES.items():
                for scenario in SCENARIOS:
                    load_result = await load_workspace(Path(scenario["path"]))

                    if not load_result["success"]:
                        print(f"Failed to load {scenario['id']}: "f"{load_result['error']}")
                        continue

                    result = await run_scenario(scenario=scenario, approach=approach, tools=tools)

                    result_path = Path("C:/Users/oskar/Dokument/Skola/Examensarbete/Projekt/automated/result") / scenario["id"] / approach / "result.json"

                    result_path.write_text(
                        json.dumps(result,indent=2,default=json_default),
                        encoding="utf-8",
                        
                    )

        asyncio.run(run_all_scenarios())

    threading.Thread(target=start, daemon=True).start()


def json_default(value):
    if isinstance(value, timedelta):
        return value.total_seconds() * 1000  # milliseconds

    if hasattr(value, "model_dump"):
        return value.model_dump()

    return str(value)


async def save_workspace(path: Path) -> dict:
    loop = asyncio.get_running_loop()
    future = loop.create_future()

    app = inviwopy.app
    network = app.network

    def save():
        try:
            app.network.save(path.with_suffix(".inv"))

            return {
                "success": True,
                "path": path
            }

        except Exception as e:
            return {
                "success": False,
                "error": str(e)
            }

    def call_dispatch():
        data = app.dispatch_front(save)
        loop.call_soon_threadsafe(future.set_result, data)

    threading.Thread(target=call_dispatch).start()

    return await future


async def load_workspace(path: Path) -> dict:

    app = inviwopy.app
    loop = asyncio.get_running_loop()
    future = loop.create_future()

    def load():
        try:
            workspace_path = Path(path).expanduser().resolve()

            if workspace_path.suffix.lower() != ".inv":
                return {
                    "success": False,
                    "error": "The workspace must be an .inv file"
                }

            if not workspace_path.is_file():
                return {
                    "success": False,
                    "error": f"Workspace not found: {workspace_path}"
                }

                   
            app.network.load(str(workspace_path))

            return {
                "success": True,
                "path": str(workspace_path)
            }

        except Exception as e:
            return {
                "success": False,
                "error": str(e)
            }

    def call_dispatch():
        data = app.dispatch_front(load)
        loop.call_soon_threadsafe(future.set_result, data)

    threading.Thread(target=call_dispatch).start()

    return await future


async def get_canvas_image(filename: Path) -> Image:
    loop = asyncio.get_event_loop()
    future = loop.create_future()
    app = inviwopy.app
    network = app.network
        
    def get_snapshot():
        try:
            canvas = None

            for processor in network.processors:
                if isinstance(processor, inviwopy.CanvasProcessor):
                    canvas = processor
                    break

            if canvas is None:
                return "No canvas processor found in the network"

            canvas.snapshot(str(filename.with_suffix(".png")))

        except Exception as e:
            return str(e)

    def call_dispatch():
        data = app.dispatch_front(get_snapshot)
        loop.call_soon_threadsafe(future.set_result, data)

    threading.Thread(target=call_dispatch).start()
    return await future
