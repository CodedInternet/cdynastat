import json

pos = {}

for row in range(10):
    pos[row] = {}
    for col in range(16):
        pos[row][col] = {
            "zeroValue": 0,
            "halfValue": 127,
            "fullValue": 255
        }

out = {
    "type": "matrix",
    "base_address": 0,
    "positions": pos
}

print(json.dumps([out], indent=4))
