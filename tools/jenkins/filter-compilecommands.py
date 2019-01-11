import re
import json

with open("compile_commands.json", "r") as f:
    data = json.load(f)

with open("compile_commands.json", "w") as f:
    data = [x for x in data if re.fullmatch(".*/ext/.*", x['file']) == None]
    data = [x for x in data if re.fullmatch(".*/build/.*", x['file']) == None]
    json.dump(data, f, sort_keys=True, indent=4)
