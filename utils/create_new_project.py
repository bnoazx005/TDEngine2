"""
	The script is used to generate a new projects using TDEngine2

	author: Ildar Kasimov
	date: 30.07.2022
"""

import sys, os, glob
import yaml
import shutil
from pathlib import Path


def copy_resources_dir(target_directory):
	resourcesDir = os.path.join(target_directory, "Resources/")

	if os.path.exists(resourcesDir):
		shutil.rmtree(resourcesDir)

	shutil.copytree("../Resources/", resourcesDir)


def copy_libraries(target_directory):
	destBinariesDirectory = os.path.join(target_directory, "bin/")

	if not os.path.exists(destBinariesDirectory):
		os.makedirs(destBinariesDirectory)

	for file in glob.glob("../bin/Debug/*.dll"):
		shutil.copy(file, destBinariesDirectory)

	for file in glob.glob("../bin/Debug/*.lib"):
		shutil.copy(file, destBinariesDirectory)

	if os.path.exists(os.path.join(target_directory, "TDEngine2/include/")):
		shutil.rmtree(os.path.join(target_directory, "TDEngine2/include/"))

	shutil.copytree("../TDEngine2/include/", os.path.join(target_directory, "TDEngine2/include/"))

	# copy plugins' includes
	pluginsDirectory = os.path.join(target_directory, "TDEngine2/plugins")
	
	if os.path.exists(pluginsDirectory):
		shutil.rmtree(pluginsDirectory)

	for filePath in Path("../TDEngine2/plugins/").glob("*/include"):
		destPath = os.path.join(pluginsDirectory, Path(filePath).relative_to("../TDEngine2/plugins/"))

		shutil.copytree(filePath, destPath)


def generate_project_files(project_name, target_directory):
	targetCmakeFile = os.path.join(target_directory, "CMakeLists.txt")

	shutil.copyfile("../Samples/TemplateApp/CMakeLists.txt", targetCmakeFile)
	
	# replace TemplateApp name within the file with a new name
	filedata = ""

	with open(targetCmakeFile, 'r') as f:
	  filedata = f.read()

	filedata = filedata.replace("TemplateApp", project_name)

	with open(targetCmakeFile, 'w') as f:
	  f.write(filedata)

	paths = [
		"include/", "source/"
	]

	#copy sources
	for currPath in paths:		
		if os.path.exists(os.path.join(target_directory, currPath)):
			shutil.rmtree(os.path.join(target_directory, currPath))

		shutil.copytree(os.path.join("../Samples/TemplateApp/", currPath), os.path.join(target_directory, currPath))

	shutil.copyfile("../Samples/TemplateApp/main.cpp.in", os.path.join(target_directory, "main.cpp.in"))



def generate_project_settings(project_name, target_directory):
	project_settings_data = {
		"meta" : {
			"resource-type": "project-settings",
			"version-tag": 1
		},

		"common_settings" : {
			"application_id" : project_name,
			"max_worker_threads_count" : 4,
			"total_preallocated_memory_size" : 134217728,
			"flags" : 20, # RESIZEABLE | ZBUFFER
			"additional_mounted_dirs" : [{
				"alias" : "ProjectResources/",
				"path" : "../../" + project_name + "/Resources/"
				}, {
					"alias" : "ProjectShaders/",
					"path" : "../../" + project_name + "/Resources/Shaders/"
				}
			],
			"user_plugins_list" : [ {
				"id" : "./TDE2LevelStreamingUtils"
			}
			]
		},

		"graphics_settings" : {
			"renderer_plugin" : "./D3D11GraphicsContext",
			"renderer_settings" : {
				"shadow_maps_size" : 1024,
				"shadow_maps_enabled" : True
			}
		},

		"audio_settings" : {
			"audio_plugin" : "./FmodAudioContext"
		},
	}

	f = open(os.path.join(target_directory, project_name + ".project"), 'w+')
	yaml.dump(project_settings_data, f, allow_unicode=True)


def generate_project_dirs(project_name, project_directory):
	# Create a new directory 
	if not os.path.exists(project_directory):
		os.makedirs(project_directory)

	# Create project's CMakeLists.txt
	generate_project_files(project_name, project_directory)

	# Copy Resources directory
	copy_resources_dir(project_directory)

	# Create a new project settings file
	generate_project_settings(project_name, project_directory)
	
	# Copy shared libraries and includes
	copy_libraries(project_directory)


def main(args):
	if len(args) < 2:
		print("The script creates a new directory for the project ")
		print("Usage: create_new_project <target_directory> <project_name>")

		return 0

	target_directory = args[0]
	project_name     = args[1]

	# Verify target_directory

	return generate_project_dirs(project_name, target_directory)


if __name__ == '__main__':
	sys.exit(main(sys.argv[1:]))