# Parser-Project
This is a simple Tiff parser where you can read a tiff file after compiling code written inside src folder.

The sample data to test the output are kept inside data folder. It contains big endian and little endian TIFF files with single and multiple IFD entries.

list of supported documents related to tiff understanding and individual contribution are located inside docs folder.

Usage:

gcc -o TiffParser TiffParser.c

./TiffParser hub1.tif
