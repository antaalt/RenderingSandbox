#pragma once

#include "Buffer.h"
#include <fstream>

namespace engine {
namespace io {

struct BinaryReader
{
	BinaryReader(const unsigned char *bytes, size_t size);
	BinaryReader(const Buffer<unsigned char> &bytes);

	template <typename T>
	T read();
	template <typename T>
	void read(T* data, size_t size);

	const unsigned char * data() const;
	size_t size() const;
private:
	const unsigned char * m_bytes;
	const size_t m_size;
	size_t m_offset;
};

inline BinaryReader::BinaryReader(const unsigned char *bytes, size_t size) :
	m_bytes(bytes),
	m_size(size),
	m_offset(0)
{
}
inline BinaryReader::BinaryReader(const Buffer<unsigned char> &bytes) :
	m_bytes(bytes.data()),
	m_size(bytes.size()),
	m_offset(0)
{
}

template <typename T>
inline T BinaryReader::read() {
	T data;
	read(&data, 1);
	return data;
}

template <typename T>
inline void BinaryReader::read(T* data, size_t size) {
	size_t realSize = size * sizeof(T);
	if (realSize + offset > bytes.size())
		throw std::out_of_range();
	memcpy(data, &bytes[offset], realSize);
}

}
}