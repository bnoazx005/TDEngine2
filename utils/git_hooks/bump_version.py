"""
The script is used to automatically increment current patch version of the engine during push to a server

author: Ildar Kasimov
date: 16.01.2022
"""
import sys, os
import re

def bump_version(match):
	return match.group(1) + str(int(match.group(4)) + 1)

def main():
	cmakeConfigData = ""

	# read TDEngine2's CMakeLists.txt
	with open("../../TDEngine2/CMakeLists.txt") as engineCMakeConfig:
		cmakeConfigData = engineCMakeConfig.read()
		engineCMakeConfig.close()

	# parse the version and bump it
	cmakeConfigData = re.sub(r'(project\s*\(\s*TDEngine2\s+VERSION\s+(\d+)\.(\d+)\.)(\d+).*?', bump_version, cmakeConfigData)

	# and save file back onto the storage
	with open("../../TDEngine2/CMakeLists.txt", 'w') as engineCMakeConfig:
		engineCMakeConfig.write(cmakeConfigData)
		engineCMakeConfig.close()

if __name__ == "__main__":
	main()