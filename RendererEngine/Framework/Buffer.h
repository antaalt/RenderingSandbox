#pragma once

#include <vector>

namespace engine {

template <typename T>
using Buffer = std::vector<T>;

template <typename T>
class TBuffer
{
public:
	TBuffer();
	~TBuffer();
};

}