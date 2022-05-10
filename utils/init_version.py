"""
The script is retrieve current version from CMakeLists.txt and creates environment variable with its value

author: Ildar Kasimov
date: 10.05.2022
"""
import sys, os
import re

CMAKE_CONFIG_FILENAME = os.environ['GITHUB_WORKSPACE'] + "/TDEngine2/CMakeLists.txt"

cmakeConfigData = ""

# read TDEngine2's CMakeLists.txt
with open(CMAKE_CONFIG_FILENAME) as engineCMakeConfig:
	cmakeConfigData = engineCMakeConfig.read()
	engineCMakeConfig.close()

# parse the version and bump it
match = re.search(r'project\s*\(\s*TDEngine2\s+VERSION\s+((\d+)\.(\d+)\.(\d+)).*?', cmakeConfigData)

envFile = os.getenv('GITHUB_ENV')

with open(envFile, 'a') as file:
	file.write("TDE2_VERSION=%s" % str(match.group(1)))