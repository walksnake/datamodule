import os
import re
import sys
import subprocess
import platform

for root,dirs,files in os.walk(os.getcwd()):
    for dir in dirs:
        if re.search("Build", dir) != None:
            builddir = os.path.join(root, dir)
            os.system('rm -rf %s/*' % builddir)
