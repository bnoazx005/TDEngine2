"""
	The script is used to generate a new projects using TDEngine2

	author: Ildar Kasimov
	date: 30.07.2022
"""

import sys, os, glob
import yaml
import shutil


def copy_resources_dir(target_directory):
	shutil.copytree("../Resources/", os.path.join(target_directory, "Resources/"))


def copy_libraries(target_directory):
	destBinariesDirectory = os.path.join(target_directory, "bin/")

	for file in glob.glob("../bin/Debug/*.dll"):
		shutil.copy(file, destBinariesDirectory)

	shutil.copytree("../TDEngine2/include/", os.path.join(target_directory, "TDEngine2/include/"))


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

	#copy sources
	shutil.copyfile("../Samples/TemplateApp/main.cpp.in", os.path.join(target_directory, "main.cpp.in"))
	shutil.copytree("../Samples/TemplateApp/include/", os.path.join(target_directory, "include/"))
	shutil.copytree("../Samples/TemplateApp/source/", os.path.join(target_directory, "source/"))



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