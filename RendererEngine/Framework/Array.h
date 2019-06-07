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

template <typename T>
class FixedArray
{
public:
	Array(size_t size);
	~Array();
private:

};

}