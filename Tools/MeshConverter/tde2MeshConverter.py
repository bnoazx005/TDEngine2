"""
File: tde2MeshConverter.py
Date: 09.12.2019
Author: Ildar Kasimov

The script is used to convert DCC files (FBX, OBJ) into *.MESH internal binary format
"""
import argparse
import os.path
import struct
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


class Vec2:
	x = 0.0
	y = 0.0

	def __init__(self, x = 0.0, y = 0.0):
		self.x = x
		self.y = y

	def __str__(self):
		return "(%.f, %.f)" % (self.x, self.y)


class Vec3:
	x = 0.0
	y = 0.0
	z = 0.0

	def __init__(self, x = 0.0, y = 0.0, z = 0.0):
		self.x = x
		self.y = y
		self.z = z

	def __str__(self):
		return "(%.f, %.f, %.f)" % (self.x, self.y, self.z)


class Vertex:
	pos = Vec3()
	normal = Vec3()
	uv = Vec2()

	def __init__(self, pos = Vec3(), uv = Vec2(), normal = Vec3()):
		self.pos = pos
		self.normal = normal
		self.uv = uv

	def __str__(self):
		return "pos: %s\nuv: %s\nnormal: %s" % (self.pos, self.uv, self.normal)


def parse_args():
	parser = argparse.ArgumentParser(description=
		"""
			TDEngine2 FBX -> *.MESH Mesh Converter
		""")
	parser.add_argument('input', type=str, help='Path to a file should be converted')
	parser.add_argument('-o', '--output', type=str, help='Output filename')
	parser.add_argument('-D', '--debug', type=str, action='store', help='Additional debug output')

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
	polygonCount = fbxMesh.GetPolygonCount()
	controlPointsList = fbxMesh.GetControlPoints() # aka mesh's vertices

	meshData = []

	for i in range(polygonCount):
		assert fbxMesh.GetPolygonSize(i) == 3, 'Non-triangulated meshes are unacceptable'

		for j in range(3):
			controlPointId = fbxMesh.GetPolygonVertex(i, j)
			currVertex = controlPointsList[controlPointId]

			currVertexInfo = Vertex()
			currVertexInfo.pos = Vec3(currVertex[0], currVertex[1], currVertex[2])

			for currLayerId in range(fbxMesh.GetLayerCount()):
				currLayer = fbxMesh.GetLayer(currLayerId)

				# first uv channel
				uvs = currLayer.GetUVs()
				if uvs:
					uvCoordsArray = uvs.GetDirectArray();

					uvId = 0

					if uvs.GetMappingMode() == FbxLayerElement.eByControlPoint:
						uvId = uvs.GetIndexArray().GetAt(controlPointId) if uvs.GetReferenceMode() == FbxLayerElement.eIndexToDirect else controlPointId						
					elif uvs.GetMappingMode() == FbxLayerElement.eByPolygonVertex:
						uvId = fbxMesh.GetTextureUVIndex(i, j)

					currUVCoords = uvCoordsArray.GetAt(uvId)
					currVertexInfo.uv = Vec2(currUVCoords[0], currUVCoords[1])

				# normals 
				normals = currLayer.GetNormals()
				if normals:
					normalsArray = normals.GetDirectArray()
					if (normals.GetMappingMode() == FbxLayerElement.eByControlPoint) and (normals.GetReferenceMode() == FbxLayerElement.eDirect):
						currNormal = normalsArray.GetAt(controlPointId)
						currVertexInfo.normal = Vec3(currNormal[0], currNormal[1], currNormal[2])

			meshData.append(currVertexInfo)
	
	return (fbxMesh.GetName(), meshData)


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

	meshConverter = FbxGeometryConverter(sdkManager) # the converter is used to triangulate the input mesh

	meshes = []

	while len(nodes) > 0:
		currNode = nodes.pop(0)

		print(currNode.GetName()) # display node's name
		nodeType = (currNode.GetNodeAttribute().GetAttributeType())
		
		# Extract triangulated mesh's data
		meshes.append(nodeProcessors[nodeType](meshConverter.Triangulate(currNode.GetNodeAttribute(), False)))

		for i in range(currNode.GetChildCount()):
			nodes.append(currNode.GetChild(i))


	print('read_fbx_mesh_data: %s' % inputFilename)
	return meshes


def read_obj_mesh_data(inputFilename):
	print('read_obj_mesh_data: %s' % inputFilename)
	return


def write_mesh_header(file, meshData):
	file.write(struct.pack('<4s', 'MESH'))

def write_mesh_scene_info(file, meshData):
	file.write(hex(18)) # 0x12
	return

def save_mesh_data(meshData, outputFilename):
	assert outputFilename, "Output file path should be non empty"
	assert meshData, "Empty mesh data is not allowed"

	try:
		outputMeshFile = open(outputFilename, 'wb')

		write_mesh_header(outputMeshFile, meshData)
		write_mesh_scene_info(outputMeshFile, meshData)

		outputMeshFile.close()
	except IOError as err:
		print("Error: %s" % err)

	return


def print_all_meshes_data(meshes):
	for currMesh in meshes:
		print("Mesh name: %s" % currMesh[0])
		for currVertex in currMesh[1]:
			print("%s\n" % currVertex)


FileTypeReaders = {
	FBX_TYPE : read_fbx_mesh_data,
	OBJ_TYPE : read_obj_mesh_data
}


def main():
	args = parse_args()
	
	fileReaderFunction = FileTypeReaders[get_filetype_from_path(args.input)]

	meshData = fileReaderFunction(args.input) # first step is to read information from original file

	if args.debug and args.debug == 'mesh-info':
		print_all_meshes_data(meshData)

	save_mesh_data(meshData, args.output if args.output else get_output_filename(args.input))

	return 0

main()
