tests = {}

points = {}

nets = {}

print("Reading tests...")
with open("board/test.txt", "r") as fin:
    for line in fin.readlines():
        tokens = line.split("\n")[0].split(" ")
        tests[tokens[1]] = False

print("Reading result trace...")
with open("results/testTrace.txt") as fin:
    for line in fin.readlines():
        tokens = line.split("\n")[0].split(" ")
        for t in tokens:
            if t in tests:
                tests[t] = True
            else:
                print("ERROR: %s not in tests!!!!!!" % t)

print("------------------------")
print("Checking for unperformed tests...")
for t in tests:
    if not tests[t]:
        print("ERROR: %s has not been analyzed!!!!")

print("------------------------")
print("Reading board...")
with open("board/board.txt") as fin:
    for line in fin.readlines():
        tokens = line.split("\n")[0].split("\r")[0].split(" ")
        if tokens[0] == ".point":
            point = {
                "name": tokens[1],
                "net": ""
            }
            points[tokens[1]] = point
        elif tokens[0] == ".net":
            net = {
                "name": tokens[1],
                "points": []
            }
            for i in range(3, len(tokens)):
                net["points"].append(tokens[i])
                points[tokens[i]]["net"] = net["name"]
            nets[tokens[1]] = net
