import re
import json

with open("compile_commands.json", "r") as f:
    data = json.load(f)

with open("compile_commands.json", "w") as f:
    fdata = [x for x in data if re.fullmatch(".*/ext/.*", x['file']) == None]
    json.dump(fdata, f, sort_keys=True, indent=4)
