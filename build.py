import sys
import subprocess

if len(sys.argv) < 2:
    print("Usage: build.py [--debug | --release | --clean]")
    sys.exit(1)

param = sys.argv[1]
exe_path = "build-debug/Project/Project"

try:
    if param == "--debug":
        print("Building debug...")
        # Build debug
        subprocess.run(["cmake", "--build", "."], cwd="build-debug", check=True)

    elif param == "--release":
        print("Building release...")
        # Build debug
        subprocess.run(["cmake", "--build", "."], cwd="build-debug", check=True)

    elif param == "--clean":
        print("Cleaning build...")
        subprocess.run(["rm", "-rf", "build-debug"])
        subprocess.run(["cmake", "-S", ".", "-B", "build-debug"])
        subprocess.run(["cmake", "--build", "."], cwd="build-debug", check=True)
    else:
        print("Unknown option:", param)

    subprocess.run([exe_path])
except subprocess.CalledProcessError:
    print("Build failed")
