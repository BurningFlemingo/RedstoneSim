import os
import subprocess

import subprocess
import platform

buildType = "debug"
projectName = "RedstoneSim"

buildPlatform = platform.system().lower();
buildArgument = f"{buildPlatform}-{buildType}"

rootDir = os.getcwd()

def buildPathFromRoot(*subdirs):
    return os.path.join(rootDir, *subdirs)

binDir = buildPathFromRoot("build", buildArgument)
binPath = buildPathFromRoot("build", buildArgument, projectName)

callingDir = os.getcwd();

os.chdir(binDir)

subprocess.run([projectName])

os.chdir(callingDir)
