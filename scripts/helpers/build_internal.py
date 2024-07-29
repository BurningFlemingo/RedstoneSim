import subprocess
import platform
import os

buildType = "release"

buildPlatform = platform.system().lower();
buildArgument = f"{buildPlatform}-{buildType}"

rootDir = os.getcwd()

def buildPathFromRoot(*subdirs):
    return os.path.join(rootDir, *subdirs)


subprocess.run(["python", buildPathFromRoot("scripts", "build.py"), buildArgument])
