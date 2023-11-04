from glob import glob
from collections import deque

lib_name = "jiko"

def find_deps(file_path, already_found = set([])):
    res = set(already_found)
    f = open(file_path, "r")
    for l in f.readlines():
        l = l.strip()
        if l.startswith("#include \""):
            inc = l.split('"')[1]
            if not inc in res:
                res.add(inc)
                res.union(find_deps(inc), res)
            else:
                res.add(inc)
    f.close()
    return res

def order_files(deps):
    queue = deque(deps.keys())
    res = []

    while len(queue) > 0:
        e = queue.popleft()
        if all([d in res for d in deps[e]]):
            res.append(e)
        else:
            queue.append(e)
    return res

sources = set(glob("*.c")).union(glob("*.h"))
sources.remove("main.c")
deps = {source : find_deps(source) for source in sources}
files = order_files(deps)
print(deps)
    

print(files)

with open(f"amalgamation/{lib_name}.h", "w") as output:
    for path in files:
        output.write("/* " + 74 * "*" + " */\n")
        output.write(f"/* {path} */\n")
        output.write("/* " + 74 * "*" + " */\n")
        if path.endswith(".c"):
            output.write(f"#ifdef {lib_name.upper()}_IMPLEMENTATION\n")
        f = open(path, "r")
        for line in f.readlines():
            if not line.strip().startswith('#include "'):
                output.write(line)
        output.write("\n")
        if path.endswith(".c"):
            output.write("#endif\n")
        f.close()

with open("main.c", "r") as main_in:
    with open("amalgamation/main.c", "w") as main_out:
        for line in main_in:
            if line.strip() == f"#include \"{lib_name}.h\"":
                main_out.write(f"#define {lib_name.upper()}_IMPLEMENTATION\n")
            main_out.write(line)