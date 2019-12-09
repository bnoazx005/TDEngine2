"""
File: tde2MeshConverter.py
Date: 09.12.2019
Author: Ildar Kasimov

The script is used to convert DCC files (FBX, OBJ) into *.MESH internal binary format
"""
import argparse


def parse_args():
	parser = argparse.ArgumentParser(description=
		"""
			TDEngine2 FBX -> *.MESH Mesh Converter
		""")
	parser.add_argument('input', type=str, help='Path to a file should be converted')
	parser.add_argument('-o', '--output', type=str, help='Output filename')

	return parser.parse_args()


def main():
	args = parse_args()
	print(args)

	return 0

main()
