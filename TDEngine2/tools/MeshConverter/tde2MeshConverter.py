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
	def __init__(self, x = 0.0, y = 0.0):
		self.x = x
		self.y = y

	def __str__(self):
		return "(%f, %f)" % (self.x, self.y)


class Vec3:
	def __init__(self, x = 0.0, y = 0.0, z = 0.0):
		self.x = x
		self.y = y
		self.z = z

	def __str__(self):
		return "(%f, %f, %f)" % (self.x, self.y, self.z)


class Vertex:
	def __init__(self, pos = Vec3(), uv = Vec2(), normal = Vec3()):
		self.pos    = pos
		self.normal = normal
		self.uv     = uv

	def __str__(self):
		return "pos: %s\tuv: %s\tnormal: %s" % (self.pos, self.uv, self.normal)


def parse_args():
	parser = argparse.ArgumentParser(description=
		"""
			TDEngine2 [FBX|OBJ] -> *.MESH Mesh Converter
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


"""
	FBX reader's description
"""

def extract_fbx_mesh_data(fbxMesh):
	polygonCount = fbxMesh.GetPolygonCount()
	controlPointsList = fbxMesh.GetControlPoints() # aka mesh's vertices

	vertices = []
	faces    = []

	for i in range(len(controlPointsList)):
		pos = controlPointsList[i]

		currVertex = Vertex(Vec3(pos[0], pos[1], pos[2]))

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
					uvId = fbxMesh.GetTextureUVIndex(i, 0) # FIXME

				currUVCoords = uvCoordsArray.GetAt(uvId)
				currVertex.uv = Vec2(currUVCoords[0], currUVCoords[1])

			# normals 
			normals = currLayer.GetNormals()
			if normals:
				normalsArray = normals.GetDirectArray()
				if (normals.GetMappingMode() == FbxLayerElement.eByControlPoint) and (normals.GetReferenceMode() == FbxLayerElement.eDirect):
					currNormal = normalsArray.GetAt(controlPointId)
					currVertex.normal = Vec3(currNormal[0], currNormal[1], currNormal[2])

		vertices.append(currVertex)

	# Fill up the faces array
	for i in range(polygonCount):
		assert fbxMesh.GetPolygonSize(i) == 3, 'Non-triangulated meshes are unacceptable'

		for j in range(3):
			faces.append(fbxMesh.GetPolygonVertex(i, j))
	
	return (fbxMesh.GetName(), vertices, faces)


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

	nodes = [(rootNode.GetChild(i), -1, i) for i in range(rootNode.GetChildCount())] # extract all nodes from the root

	meshConverter = FbxGeometryConverter(sdkManager) # the converter is used to triangulate the input mesh

	objects = []

	while len(nodes) > 0:
		currNodeEntry = nodes.pop(0)

		currNode   = currNodeEntry[0]
		parentId   = currNodeEntry[1]
		currNodeId = currNodeEntry[2]

		nodeType = (currNode.GetNodeAttribute().GetAttributeType())
		
		# Extract triangulated mesh's data
		nodeProcessorFunc = nodeProcessors.get(nodeType)
		meshData = nodeProcessorFunc(meshConverter.Triangulate(currNode.GetNodeAttribute(), False)) if nodeProcessorFunc else None

		objects.append((currNode.GetName(), parentId, meshData))

		for i in range(currNode.GetChildCount()):
			nodes.append((currNode.GetChild(i), currNodeId, nodes[-1][2] + i + 1))

	return objects


"""
	OBJ reader's description
"""

import copy

class IntermediateObjDataContext:
	class SubmeshDataDesc:
		def __init__(self, name):
			self.id = name
			self.currVertexOffset = 0
			self.currIndexOffset = 0
			self.numOfVertices = 0
			self.numOfIndices = 0

	class ObjVertexData:
		def __init__(self):
			self.pos = Vec3()
			self.normal = Vec3()
			self.texcoords = Vec2()

	def __init__(self):
		self.vertices = []
		self.normals = []
		self.texcoords = []
		self.indices = []
		self.finalVertices = []
		self.subMeshes = []
		self.numOfSubMeshes = 0
		self.currSubMeshDesc = self.SubmeshDataDesc("")
		self.numOfProcessedVertices = 0
		self.numOfProcessedIndices = 0
		self.verticesTable = dict()
		self.useCWOrder = True


def obj_process_vertices(tokens, sceneContext):
	assert len(tokens) >= 2, "[obj_process_vertices] Too few components"
	sceneContext.vertices.append(Vec3(float(tokens[0]), float(tokens[1]), float(tokens[2]) if tokens[2] else 0.0))
	return sceneContext


def obj_process_normals(tokens, sceneContext):
	assert len(tokens) == 3, "[obj_process_normals] Too few components"
	sceneContext.normals.append(Vec3(float(tokens[0]), float(tokens[1]), float(tokens[2]) if tokens[2] else 0.0))
	return sceneContext


def obj_process_texcoords(tokens, sceneContext):
	assert len(tokens) >= 2, "[obj_process_texcoords] Too few components"
	sceneContext.texcoords.append(Vec3(float(tokens[0]), float(tokens[1]), float(tokens[2]) if tokens[2] else 0.0))
	return sceneContext


def obj_process_triangulated_faces(tokens, sceneContext):
	assert len(tokens) == 3, "Invalid number of tokens"

	def parse_face_triplet(triplet):
		return tuple(map(lambda e : (int(e) - 1), triplet.split("/")))

	for currTriplet in tokens:
		faceData = parse_face_triplet(currTriplet)

		if currTriplet in sceneContext.verticesTable:
			sceneContext.indices.append(sceneContext.verticesTable[currTriplet])
			sceneContext.numOfProcessedIndices = sceneContext.numOfProcessedIndices + 1

			continue

		finalVertex = IntermediateObjDataContext.ObjVertexData()
		finalVertex.pos = sceneContext.vertices[faceData[0]]
		finalVertex.texcoords = sceneContext.texcoords[faceData[1]]
		finalVertex.normal = sceneContext.normals[faceData[2]]

		index = len(sceneContext.finalVertices)

		sceneContext.finalVertices.append(finalVertex)
		sceneContext.indices.append(index)

		sceneContext.verticesTable[currTriplet] = index

		sceneContext.numOfProcessedVertices = sceneContext.numOfProcessedVertices + 1
		sceneContext.numOfProcessedIndices = sceneContext.numOfProcessedIndices + 1

	return sceneContext


def obj_process_quad_faces(tokens, sceneContext):
	assert len(tokens) == 4, "Invalid number of tokens"

	if sceneContext.useCWOrder:
		obj_process_triangulated_faces(tokens[0:2], sceneContext)
		obj_process_triangulated_faces(list(tokens[0], tokens[2], tokens[3]), sceneContext)

		return sceneContext

	obj_process_triangulated_faces(tokens[0:2].reverse(), sceneContext)
	obj_process_triangulated_faces(list(tokens[0], tokens[2], tokens[3]).reverse(), sceneContext)

	return sceneContext


def obj_process_faces(tokens, sceneContext):
	return (obj_process_triangulated_faces if len(tokens) == 3 else obj_process_quad_faces)(tokens, sceneContext)


def obj_process_objects(tokens, sceneContext):

	if sceneContext.numOfSubMeshes < 1:
		sceneContext.numOfSubMeshes = sceneContext.numOfSubMeshes + 1
		sceneContext.currSubMeshDesc.id = tokens[0]

		return sceneContext

	currSubMeshDesc = sceneContext.currSubMeshDesc

	currSubMeshDesc.currVertexOffset += currSubMeshDesc.numOfVertices
	currSubMeshDesc.currIndexOffset += currSubMeshDesc.numOfIndices

	currSubMeshDesc.numOfVertices = sceneContext.numOfProcessedVertices
	currSubMeshDesc.numOfIndices = sceneContext.numOfProcessedIndices

	sceneContext.subMeshes.append(copy.deepcopy(currSubMeshDesc))

	sceneContext.currSubMeshDesc.id = tokens[0]

	sceneContext.numOfProcessedVertices = 0
	sceneContext.numOfProcessedIndices = 0

	sceneContext.numOfSubMeshes = sceneContext.numOfSubMeshes + 1

	return sceneContext


def read_obj_mesh_data(inputFilename):
	handlersMap = {
		"v" : obj_process_vertices,
		"vn" : obj_process_normals,
		"vt" : obj_process_texcoords,
		"f" : obj_process_faces,
		"g" : obj_process_objects,
	}

	tempSceneContext = IntermediateObjDataContext()

	try:
		objFileHandle = open(inputFilename, "r")

		for currLine in objFileHandle:
			if currLine.startswith('#'):
				continue

			tokens = currLine.split()
			if not tokens:
				continue

			if tokens[0] in handlersMap:
				tempSceneContext = handlersMap[tokens[0]](tokens[1:], tempSceneContext)
				continue

			print("[OBJ reader] Warning: Unknown block (%s)'s found" % tokens[0])

		objFileHandle.close()
	except IOError as err:
		print("Error: %s" % err)

	
	# Append information about last group
	tempSceneContext.currSubMeshDesc.currIndexOffset += tempSceneContext.currSubMeshDesc.numOfIndices;
	tempSceneContext.currSubMeshDesc.currVertexOffset += tempSceneContext.currSubMeshDesc.numOfVertices;

	tempSceneContext.currSubMeshDesc.numOfIndices = tempSceneContext.numOfProcessedIndices;
	tempSceneContext.currSubMeshDesc.numOfVertices = tempSceneContext.numOfProcessedVertices;
	
	tempSceneContext.subMeshes.append(tempSceneContext.currSubMeshDesc);
	
	objects = []

	for currSubMesh in tempSceneContext.subMeshes:
		# Gather vertices of the sub-mesh
		vertices = []

		for i in range(currSubMesh.currVertexOffset, currSubMesh.currVertexOffset + currSubMesh.numOfVertices):
			objVertexData = tempSceneContext.finalVertices[i]

			vertices.append(Vertex(objVertexData.pos, objVertexData.texcoords, objVertexData.normal))
		
		# Gather indices of the sub-mesh
		faces = []

		for i in range(currSubMesh.currIndexOffset, currSubMesh.currIndexOffset + currSubMesh.numOfIndices):
			faces.append(tempSceneContext.indices[i])

		print("verts : %d, faces %d" % (len(vertices), len(faces)))
		objects.append((currSubMesh.id, -1, (currSubMesh.id, vertices, faces)))

	return objects

"""
	MESH writer's description
"""

def write_mesh_header(file, offset):
	file.seek(0)
	file.write(struct.pack('<4s', 'MESH')) # signature
	file.write(struct.pack('<4B', 0, 1, 0, 0)) # version in the following format MAJOR.MINOR.ITERATION.UNUSED
	file.write(struct.pack('<I', offset)) # offset to scene's description
	file.write(struct.pack('<I', 0)) # padding to 16 bytes


def write_geometry_block(file, objectsData):
	def write_single_mesh_data(file, id, meshData):
		mesh  = meshData[1]
		faces = meshData[2]

		file.write(struct.pack('<2B', 77, 72)) # 0x4D, 0x48
		file.write(struct.pack('<H', id))
		file.write(struct.pack('<3I', len(mesh), len(faces), 0)) # 0 for padding to 16 bytes

		offset = 16

		# write vertices chunk
		file.write(struct.pack('<2B', 1, 205)) # 0x01CD
		offset += 2

		for currVertex in mesh:
			currPos = currVertex.pos

			# write in xyz order but each coordinate is stored as little-endian value
			file.write(struct.pack('<3f', currPos.x, currPos.y, currPos.z))
			file.write(struct.pack('<f', 1.0)) # w component, 1.0 - point, 0.0 - vector
			offset += 16

		# write textures chunk (uv0)
		file.write(struct.pack('<2B', 2, 240)) # 0x02F0, 
		offset += 2

		for currVertex in mesh:
			uv = currVertex.uv

			# write in xyz order but each coordinate is stored as little-endian value
			file.write(struct.pack('<2f', uv.x, uv.y))
			file.write(struct.pack('<2f', 0.0, 1.0)) # w component, 1.0 - point, 0.0 - vector
			offset += 16

		# write faces chunk (polygons)
		format = 2 if len(faces) < 65536 else 4

		file.write(struct.pack('<2B', 3, 255)) # 0x03FF
		file.write(struct.pack('<H', format)) # format
		file.write(struct.pack('<%d%s' % (len(faces), 'H' if format == 2 else 'I'), *faces))
		
		offset += 8 + struct.calcsize('<%d%s' % (len(faces), 'H' if format == 2 else 'I'))

		return offset

	file.seek(16) # size of the header
	file.write(struct.pack('<H', 47)) # 0x2F

	offset = 0

	for meshId, currObjectEntry in enumerate(objectsData):
		offset += write_single_mesh_data(file, meshId, currObjectEntry[2])

	return offset


def write_scene_desc_info(file, offset, objectsData):
	def write_single_object_entry(file, objectId, meshId, objectEntryData):
		file.write(struct.pack('<2B', 240, 205)) # 0xF0CD 
		file.write(struct.pack('<64s', str(objectEntryData[0]))) # Objects name
		file.write(struct.pack('<3H', objectId, objectEntryData[1] & 0xFFFF, meshId)) # Object ID, Parent ID, Mesh ID
		return

	file.seek(offset)
	file.write(struct.pack('<H', 18)) # 0x12 
	file.write(struct.pack('<H', len(objectsData)))

	meshIdCounter = 0

	for objectId, currObjectEntry in enumerate(objectsData):
		currMeshId = 0xFFFF

		if currObjectEntry[2]:
			currMeshId = meshIdCounter
			meshIdCounter += 1

		write_single_object_entry(file, objectId, currMeshId, currObjectEntry)

	return


def save_mesh_data(objectsData, outputFilename):
	assert outputFilename, "Output file path should be non empty"
	assert objectsData, "Empty objects data is not allowed"

	try:
		outputMeshFile = open(outputFilename, 'wb')

		offset = 16 + write_geometry_block(outputMeshFile, objectsData) # 16 plus for the header's size
		
		write_mesh_header(outputMeshFile, offset)
		write_scene_desc_info(outputMeshFile, offset, objectsData)

		outputMeshFile.close()
	except IOError as err:
		print("Error: %s" % err)

	return


def print_all_meshes_data(objects):
	for currObjectEntry in objects:
		print("Object Name: %s" % currObjectEntry[0])
		
		meshData = currObjectEntry[2]

		print("\nMesh Name: %s" % meshData[0])
		print("Vertices: ")

		for currVertex in meshData[1]:
			print("%s" % currVertex)

		print("\nFaces: ")

		faces = meshData[2]
		for i in xrange(0, len(faces), 3):
			print("%d %d %d" % (faces[i], faces[i + 1], faces[i + 2]))



FileTypeReaders = {
	FBX_TYPE : read_fbx_mesh_data,
	OBJ_TYPE : read_obj_mesh_data
}

"""
	Entry point
"""
def main():
	args = parse_args()
	
	fileReaderFunction = FileTypeReaders[get_filetype_from_path(args.input)]

	fileData = fileReaderFunction(args.input) # first step is to read information from original file

	if args.debug and args.debug == 'mesh-info':
		print_all_meshes_data(fileData)

	save_mesh_data(fileData, args.output if args.output else get_output_filename(args.input))

	return 0

main()
