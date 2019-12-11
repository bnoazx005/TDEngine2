"""
File: tde2MeshConverter.py
Date: 09.12.2019
Author: Ildar Kasimov

The script is used to convert DCC files (FBX, OBJ) into *.MESH internal binary format
"""
import argparse
import os.path
try:
	from FbxCommon import *
except ImportError:
	print("Error: module FbxCommon failed to import.\n")
	print("Copy the files located in the compatible sub-folder lib/python<version> into your python interpreter site-packages folder.")
	import platform
	if platform.system() == 'Windows' or platform.system() == 'Microsoft':
		print('For example: copy ..\\..\\lib\\Python27_x64\\* C:\\Python27\\Lib\\site-packages')
	elif platform.system() == 'Linux':
		print('For example: cp ../../lib/Python27_x64/* /usr/local/lib/python2.7/site-packages')
	elif platform.system() == 'Darwin':
		print('For example: cp ../../lib/Python27_x64/* /Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/site-packages')
	sys.exit(1)


FBX_TYPE = 0
OBJ_TYPE = 1


def parse_args():
	parser = argparse.ArgumentParser(description=
		"""
			TDEngine2 FBX -> *.MESH Mesh Converter
		""")
	parser.add_argument('input', type=str, help='Path to a file should be converted')
	parser.add_argument('-o', '--output', type=str, help='Output filename')

	return parser.parse_args()


def get_output_filename(inputFilename):
	return os.path.splitext(inputFilename)[0] + '.mesh'


def get_filetype_from_path(filename):
	fileExtension =  os.path.splitext(filename)[1]

	extension2fileType = {
		'.fbx' : FBX_TYPE,
		'.obj' : OBJ_TYPE
	}

	return extension2fileType[fileExtension.lower()]


def extract_fbx_mesh_data(fbxMesh):
	print("extract_fbx_mesh_data")
	return


def read_fbx_mesh_data(inputFilename):
	sdkManager, scene = InitializeSdkObjects()
	
	if not LoadScene(sdkManager, scene, inputFilename):
		print('Error: some problem has occurred during processing of FBX file [%s] ...' % inputFilename)
		return None

	nodeProcessors = {
		FbxNodeAttribute.eMesh : extract_fbx_mesh_data
	}

	# Traverse scene's hierarchy
	rootNode = scene.GetRootNode()

	nodes = [rootNode.GetChild(i) for i in range(rootNode.GetChildCount())] # extract all nodes from the root

	while len(nodes) > 0:
		currNode = nodes.pop(0)

		print(currNode.GetName()) # display node's name
		nodeType = (currNode.GetNodeAttribute().GetAttributeType())
		
		nodeMesh = nodeProcessors[nodeType](currNode.GetNodeAttribute())

		for i in range(currNode.GetChildCount()):
			nodes.append(currNode.GetChild(i))


	print('read_fbx_mesh_data: %s' % inputFilename)
	return


def read_obj_mesh_data(inputFilename):
	print('read_obj_mesh_data: %s' % inputFilename)
	return


def save_mesh_data(mesh_data, outputFilename):
	print('save_mesh_data: %s' % outputFilename)
	return


FileTypeReaders = {
	FBX_TYPE : read_fbx_mesh_data,
	OBJ_TYPE : read_obj_mesh_data
}


def main():
	args = parse_args()
	
	fileReaderFunction = FileTypeReaders[get_filetype_from_path(args.input)]

	mesh_data = fileReaderFunction(args.input) # first step is to read information from original file
	save_mesh_data(mesh_data, args.output if args.output else get_output_filename(args.input))

	return 0

main()
