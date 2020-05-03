#pragma once

namespace engine {


template <typename T, size_t size>
class Array
{
public:
	Array();
	~Array();
private:

};

template<typename T, size_t size>
inline Array<T, size>::Array()
{
}

template<typename T, size_t size>
inline Array<T, size>::~Array()
{
}

}