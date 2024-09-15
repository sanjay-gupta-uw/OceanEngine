#include "utility.h"
#include <filesystem>
#include <iostream>

std::string readFile(const char* filename) {
	std::string directory = "Shaders/";
	std::string fullPath = directory + filename;
	std::cout << "Reading file: " << fullPath << std::endl;
	std::ifstream in(fullPath, std::ios::binary);
	if (in)
	{
		std::string contents;
		in.seekg(0, std::ios::end);
		contents.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&contents[0], contents.size());
		in.close();
		return contents;
	}
	throw std::runtime_error("Failed to open file: " + std::string(filename));
}