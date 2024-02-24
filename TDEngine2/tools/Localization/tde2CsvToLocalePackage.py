"""
File: tde2CsvToLocalePackage.py
Date: 23.02.2024
Author: Ildar Kasimov

The script converts CSV tables into *.localization specific views. Each column except first one generates
corresponding localization file

## Change log:

- **[23.02.2024]**: 
  ####Added: Add basic functionality of conversion from CSV files into *.localization packages
"""
import argparse
import csv
import os
import io
from ruamel.yaml import YAML


def parse_args():
	parser = argparse.ArgumentParser(description=
		"""
			TDEngine2 csv -> *.localization converter
		""")

	parser.add_argument('input', type=str, help='Path to a file should be converted')
	parser.add_argument('-o', '--output_dir', type=str, help='Output directory')

	return parser.parse_args()


def generate_tables(filename):
	with open(filename, encoding="utf-8") as csv_file:
		csv_reader = csv.DictReader(csv_file, delimiter=';')
		locales_map = {}

		for curr_row in csv_reader:
			curr_key_id = ""

			if not locales_map:
				for curr_locale_id in curr_row:
					if curr_locale_id == "ID":
						continue

					locales_map[curr_locale_id] = {}

			for curr_locale_id in curr_row:
				if curr_locale_id == "ID":
					curr_key_id = curr_row[curr_locale_id]
					continue

				locales_map[curr_locale_id][curr_key_id] = curr_row[curr_locale_id]

		return locales_map

	return {}


def write_output_packages(locales_map, output_directory):
	if not os.path.exists(output_directory):
		os.makedirs(output_directory)

	yaml = YAML()
	yaml.indent(mapping=4, sequence=6, offset=3)

	for curr_locale_id in locales_map:
		output_filename = os.path.join(output_directory, f'{curr_locale_id}.localization')

		dictionary = []

		package_data = {
			"dictionary" : dictionary
		}

		for curr_key in locales_map[curr_locale_id]:
			dictionary.append({ "locale_entity" : {
				"key" : curr_key,
				"value" : locales_map[curr_locale_id][curr_key]
				}})

		with io.open(output_filename, 'w', encoding='utf-8') as outfile:
			yaml.dump(package_data, outfile)


def process(args):
	write_output_packages(generate_tables(args.input), args.output_dir if args.output_dir else os.getcwd())
	return 0


"""
	Entry point
"""
def main():
	args = parse_args()
	return process(args)

if __name__ == "__main__":
	main()
