import sys
import os
import glob
import subprocess

shader_files = []
for exts in ('*.vert', '*.frag', '*.comp', '*.geom', '*.tesc', '*.tese'):
    shader_files.extend(glob.glob(os.path.join("./", exts)))

failed_files = []
for shader_file in shader_files:
    print("compiling %s\n" % shader_file)
    if subprocess.call("glslangvalidator -V %s -o ../bin/%s.spv" % (shader_file, shader_file), shell=True) != 0:
        failed_files.append(shader_file)

for failed_file in failed_files:
    print("Failed to compile " + failed_file + "\n")
