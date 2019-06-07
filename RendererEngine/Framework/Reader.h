#pragma once

#include "Buffer.h"
#include <fstream>

namespace engine {

namespace io {

struct BinaryReader
{
	BinaryReader(const char *path) :
		offset(0)
	{
		std::ifstream file(path, std::ios::binary);
		std::streampos size = file.tellg();
		bytes.resize(size);
		file.seekg(0);
		file.read(reinterpret_cast<char*>(bytes.data()), bytes.size());
	}
	BinaryReader(const Buffer<unsigned char> &bytes) :
		bytes(bytes),
		offset(0)
	{
	}

	template <typename T>
	T read() {
		T data;
		read(&data, 1);
		return data;
	}

	template <typename T>
	void read(T* data, size_t size) {
		size_t realSize = size * sizeof(T);
		if (realSize + offset > bytes.size())
			throw std::out_of_range();
		memcpy(data, &bytes[offset], realSize);
	}

	Buffer<unsigned char> bytes;
	size_t offset;
};

}

}