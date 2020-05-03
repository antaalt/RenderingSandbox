#pragma once
#include <vector>
#include <string>
#include <map>

/*
 * TODO
 * - ios compat for debug (operator<< operator>>)
 * - parser
 * - serializer
 * - test
 *
 */

namespace json {

class Exception : public std::exception {
public:
	Exception(const char *str) : exception(str) {}
	Exception(const std::string &str) : exception(str.c_str()) {}
};

class InvalidValue : public Exception {
public:
	InvalidValue(const char *str) : Exception(str) {}
	InvalidValue(const std::string &str) : Exception(str) {}
};

class Variable;
class Name;

struct Object {
	Object();

	Variable &operator[](const Name &index);
	const Variable &operator[](const Name &index) const;
	void add(const Name &name, const Variable &variable);
	void add(const Name &name, Variable &&variable);
	void remove(const Name &name);
	size_t size() const;
	
	using iterator = std::map<Name, Variable>::iterator; // TODO use custom iterator to use Node
	using const_iterator = std::map<Name, Variable>::const_iterator;
	iterator begin() { return m_nodes.begin(); }
	const_iterator begin() const { return m_nodes.begin(); }
	iterator end() { return m_nodes.end(); }
	const_iterator end() const { return m_nodes.end(); }
private:
	std::map<Name, Variable> m_nodes; // TODO custom map ?
};
    
struct Array {
	Array();
	Array(const std::initializer_list<Variable> &list);
	Variable &operator[](size_t index);
	const Variable &operator[](size_t index) const;
	void add(const Variable &variable);
	void add(Variable &&variable);
	void insert(size_t index, const Variable &variable);
	void remove(size_t index);
	size_t size() const;

	using vector = std::vector<Variable>;
	using iterator = vector::iterator;
	using const_iterator = vector::const_iterator;
	iterator begin() { return m_data.begin(); }
	const_iterator begin() const { return m_data.begin(); }
	iterator end() { return m_data.end(); }
	const_iterator end() const { return m_data.end(); }
private:
	std::vector<Variable> m_data;
};

struct Null {
	Null();
};

class Name {
public:
	Name() {}
	Name(const char *str) : m_name(str) {}
	Name(const std::string &str) : m_name(str) {}
	// Is it a valid name (no invalid character)
	std::string &name() { return m_name; }
	const std::string &name() const { return m_name; }
	bool operator<(const Name &name) const { return m_name < name.m_name; }
private:
	std::string m_name;
};
    
class Variable {
public:
	enum class Type {
		eOBJECT,
		eARRAY,
		eNUMBER,
		eSTRING,
		eBOOL,
		eNULL
	};
	Variable() : m_type(Type::eNULL) {}
	Variable(const Object &object) : m_type(Type::eOBJECT), m_value(object) {}
	Variable(const Array &array) : m_type(Type::eARRAY), m_value(array) {}
	Variable(const std::initializer_list<Variable> &list) : m_type(Type::eARRAY),  m_value(list) {}
	Variable(int integer) : m_type(Type::eNUMBER), m_value(static_cast<double>(integer)) {}
	Variable(double real) : m_type(Type::eNUMBER), m_value(real) {}
	Variable(const char *str) : m_type(Type::eSTRING), m_value(str) {}
	Variable(const std::string &str) : m_type(Type::eSTRING), m_value(str) {}
	Variable(bool boolean) : m_type(Type::eBOOL), m_value(boolean) {}

	Variable &operator=(const Object &value) { m_type = Type::eOBJECT; m_value.object = value; }
	Variable &operator=(const Array &value) { m_type = Type::eARRAY; m_value.array = value; }
	Variable &operator=(double value) { m_type = Type::eNUMBER; m_value.number = value; }
	Variable &operator=(const char *value) { m_type = Type::eSTRING; m_value.string = value; }
	Variable &operator=(const std::string &value) { m_type = Type::eSTRING; m_value.string = value; }
	Variable &operator=(bool value) { m_type = Type::eBOOL; m_value.boolean = value; }

	operator Null() const { if (m_type != Type::eNULL) throw InvalidValue("Null type");  return Null(); }
	operator Object&() { if (m_type != Type::eOBJECT) throw InvalidValue("Object type");  return m_value.object; }
	operator const Object&() const { if (m_type != Type::eOBJECT) throw InvalidValue("Object type");  return m_value.object; }
	operator Array&() { if (m_type != Type::eARRAY) throw InvalidValue("Array type");  return m_value.array; }
	operator const Array&() const { if (m_type != Type::eARRAY) throw InvalidValue("Array type");  return m_value.array; }
	operator double&() { if (m_type != Type::eNUMBER) throw InvalidValue("Number type");  return m_value.number; }
	operator const double&() const { if (m_type != Type::eNUMBER) throw InvalidValue("Number type");  return m_value.number; }
	operator std::string&() { if (m_type != Type::eSTRING) throw InvalidValue("String type");  return m_value.string; }
	operator const std::string&() const { if (m_type != Type::eSTRING) throw InvalidValue("String type");  return m_value.string; }
	operator bool&() { if (m_type != Type::eBOOL) throw InvalidValue("Boolean type");  return m_value.boolean; }
	operator const bool&() const { if (m_type != Type::eBOOL) throw InvalidValue("Boolean type");  return m_value.boolean; }

	bool isNull() const { return m_type == Type::eNULL; }
	bool isObject() const { return m_type == Type::eOBJECT; }
	bool isArray() const { return m_type == Type::eARRAY; }
	bool isNumber() const { return m_type == Type::eNUMBER; }
	bool isString() const { return m_type == Type::eSTRING; }
	bool isBoolean() const { return m_type == Type::eBOOL; }

private:
	struct Value {
		Value() {}
		Value(const Object &object) : object(object) {}
		Value(const Array &array) : array(array) {}
		Value(double number) : number(number) {}
		Value(const char *str) : string(str) {}
		Value(const std::string &str) : string(str) {}
		Value(bool boolean) : boolean(boolean) {}
		Object object;
		Array array;
		double number;
		std::string string;
		bool boolean;
	} m_value;
	Type m_type;
};
    
struct JSON {
	JSON() {}
	operator Object&() { return m_root; }
	operator const Object&() const { return m_root; }
private:
	Object m_root;
};
// Into serializer
struct Writer {
	Writer(std::string &string) : m_string(string) {}
	void set(char c) {
		m_string += c;
	}
	size_t size() { return m_string.size(); }
private:
	std::string &m_string;
};

class Serializer {
public:
	Serializer();

	std::string operator()(const json::JSON &json);
	std::string operator()(const json::Name &name);
	std::string operator()(const json::Variable &variable);
	std::string operator()(const json::Object &object);
	std::string operator()(const json::Array &array);
	std::string operator()(const json::Null &null);
	std::string operator()(const std::string &string);
	std::string operator()(double number);
	std::string operator()(bool boolean);
private:
	bool m_prettify;
	unsigned int m_depth;
};
// Into parser
struct Reader {
	Reader(const std::string &string) : m_string(string), m_offset(0) {}
	// get next char
	const char &get();
	// get next char & skip everything until this.
	const char &get(char c);
	// Get next valid character
	const char &getValid();
	// peek next char
	const char &peek();
	// Skip n char
	void skip(size_t count = 1);
private:
	const std::string &m_string;
	size_t m_offset;
};

inline const char &Reader::get() {
	return m_string[m_offset++];
}
inline const char &Reader::get(char c) {
	while (peek() != c)
		m_offset++;
	return get();
}
inline bool isWhitespace(char c) {
	return  c == '\r' || c == '\n' || c == '\t' || c == ' ';
}
inline const char &Reader::getValid() {
	while (isWhitespace(peek()))
		m_offset++;
	return get();
}
inline const char &Reader::peek() {
	return m_string[m_offset + 1];
}
inline void Reader::skip(size_t count) {
	m_offset += count;
}

class Parser {
public:
	Parser();

	enum class Token {
		UNINTIALIZED, 
		BOOLEAN_TRUE,
		BOOLEAN_FALSE,
		NULL_VALUE,
		STRING_VALUE,
		NUMBER_VALUE, // TODO split as int, uint, float
		ARRAY_START,      // [
		ARRAY_END,        // {
		OBJECT_START,     // ]
		OBJECT_END,       // }
		NAME_SEPARATOR,   // :
		VALUE_SEPARATOR,  // ,
	};

	json::JSON operator()(const std::string &str);
	bool validate(const std::string &str);
private:
	json::Name parseName(Reader &str);
	json::Variable parseVariable(Reader &str);
	json::Object parseObject(Reader &str);
	json::Array parseArray(Reader &str);
	json::Variable parseNull(Reader &str);
	std::string parseString(Reader &str);
	double parseNumber(Reader &str);
	bool parseBoolean(Reader &str);
	const char &get();
private:
	bool m_prettify;
	unsigned int m_depth;
};


// ------ Declaration
// --- Object
inline Object::Object()
{
}
inline Variable &Object::operator[](const Name &index)
{
	return m_nodes.at(index);
} // TODO do not throw
inline const Variable &Object::operator[](const Name &index) const
{
	return m_nodes.at(index);
} // TODO do not throw
inline void Object::add(const Name &name, const Variable &variable)
{
	m_nodes.insert(std::make_pair(name, variable));
}
inline void Object::add(const Name &name, Variable &&variable)
{
	m_nodes.insert(std::make_pair(name, variable));
}
inline void Object::remove(const Name &name)
{

}
inline size_t Object::size() const
{
	return m_nodes.size();
}
// --- Array
inline Array::Array()
{
}
inline Array::Array(const std::initializer_list<Variable> &list) : m_data(list)
{
}
inline Variable &Array::operator[](size_t index)
{
	return m_data[index];
}
inline const Variable &Array::operator[](size_t index) const
{
	return m_data[index];
}
inline void Array::add(const Variable &variable)
{
	m_data.push_back(variable);
}
inline void Array::add(Variable &&variable)
{
	m_data.push_back(variable);
}
inline void Array::insert(size_t index, const Variable &variable)
{
}
inline void Array::remove(size_t index)
{
}
inline size_t Array::size() const
{
	return m_data.size();
}
// --- Null
inline Null::Null()
{
}


// --------------------------------

inline Serializer::Serializer() :
	m_prettify(true),
	m_depth(0)
{

}

inline std::string Serializer::operator()(const JSON &json)
{
	return this->operator()(static_cast<json::Object>(json));
}

inline std::string Serializer::operator()(const json::Name & name)
{
	return this->operator()(name.name());
}
inline std::string Serializer::operator()(const json::Variable &variable)
{
	if (variable.isObject())
	{
		return this->operator()(static_cast<json::Object>(variable));
	}
	else if (variable.isArray())
	{
		return this->operator()(static_cast<json::Array>(variable));
	}
	else if (variable.isNumber())
	{
		return this->operator()(static_cast<double>(variable));
	}
	else if (variable.isString())
	{
		return this->operator()(static_cast<std::string>(variable));
	}
	else if (variable.isBoolean())
	{
		return this->operator()(static_cast<bool>(variable));
	}
	else // Default is null
	{
		return this->operator()(static_cast<json::Null>(variable));
	}
}
inline std::string Serializer::operator()(const json::Object &object)
{
	std::string str;
	str += '{';
	if(m_prettify)
		str += '\n';
	size_t elemCount = 0;
	m_depth++;
	for(const std::pair<json::Name, json::Variable> &element : object)
	{
		bool lastElement = (++elemCount == object.size());
		if (m_prettify)
			for (unsigned int i = 0; i < m_depth; i++)
				str += '\t';
		str += this->operator()(element.first);
		str += ": ";
		str += this->operator()(element.second);
		if(!lastElement)
			str += ',';
		if (m_prettify)
			str += '\n';
	}
	m_depth--;
	if(m_prettify)
	for (unsigned int i = 0; i < m_depth; i++)
		str += '\t';
	str += '}';
	return str;
}
inline std::string Serializer::operator()(const json::Array &array)
{
	std::string str;
	str += '[';
	for (size_t iVar = 0; iVar < array.size(); iVar++)
	{
		const json::Variable &var = array[iVar];
		str += (*this)(var);
		if (iVar != (array.size() - 1)) {
			str += ',';
			if (m_prettify)
				str += ' ';
		}
	}
	str += ']';
	return str;
}
inline std::string Serializer::operator()(double number)
{
	// TODO optimizer formatting
	// TODO support e & E for exponent
	std::string s(16, '\0');
	int written = std::snprintf(&s[0], s.size(), "%.2f", number);
	s.resize(written);
	return s;
}
inline std::string Serializer::operator()(const std::string &string)
{
	// is u supported
	// TODO sanitize string
	return '"' + string + '"';
}
inline std::string Serializer::operator()(bool boolean)
{
	if (boolean) return "true";
	return "false";
}
inline std::string Serializer::operator()(const Null &null)
{
	return "null";
}

inline Parser::Parser()
{
}

inline json::JSON Parser::operator()(const std::string & str)
{
	JSON json;
	Object &obj = json;
	Reader reader(str);
	obj = parseObject(reader);
	return json;
}

inline json::Name Parser::parseName(Reader& reader)
{
	if (reader.getValid() != '"')
		throw InvalidValue("");
	char c;
	std::string name;
	while ((c = reader.get()) != '"') // TODO check for '\"'
	{
		name += c;
		c = reader.get();
	}
	// TODO sanitize string
	return json::Name(name);
}

inline json::Variable Parser::parseVariable(Reader& reader)
{
	Variable::Type type = Variable::Type::eNULL;
	switch (reader.peek())
	{
	case '[':
		return parseArray(reader);
	case '{':
		return parseObject(reader);
	case '"':
		return parseString(reader);
	case 't':
	case 'f':
		return parseBoolean(reader);
	case 'n':
	case 'N':
		return parseNull(reader);
	default:
		return parseNumber(reader);
		throw InvalidValue("Invalid value");
	}
}

inline json::Object Parser::parseObject(Reader& reader)
{
	json::Object object;
	if (reader.getValid() != '{')
		throw InvalidValue("");
	while (true)
	{
		json::Name name = parseName(reader);
		if (reader.getValid() != ':')
			throw InvalidValue("");
		json::Variable var = parseVariable(reader);
		object.add(std::move(name), std::move(var));
		switch (reader.getValid())
		{
		case '}':
			return object;
		case ',':
			continue;
		default:
			throw InvalidValue("");
		}
	}
}

inline json::Array Parser::parseArray(Reader& reader)
{
	if (reader.getValid() != '[')
		throw InvalidValue("");
	json::Array array;
	char c;
	std::string value;
	while ((c = reader.get()) != '"') // TODO check for '\"'
	{
		//name += c;
		c = reader.get();
	}
	return json::Array();
}

inline json::Variable Parser::parseNull(Reader& reader)
{
	if (reader.getValid() != 'n')
		throw InvalidValue("");
	return json::Variable();
}

inline std::string Parser::parseString(Reader& reader)
{
	if (reader.getValid() != '"')
		throw InvalidValue("");
	return std::string();
}

inline double Parser::parseNumber(Reader& reader)
{
	if (reader.getValid() != '[')
		throw InvalidValue("");
	return 0.0;
}

inline bool Parser::parseBoolean(Reader& reader)
{
	if (reader.getValid() != '[')
		throw InvalidValue("");
	return false;
}

}


inline void testParse() {
	std::string str;
	json::Parser parser;
	const json::JSON j = parser(str);
	const json::Object &obj = j;
	if (!obj["hash"].isString())
		throw std::runtime_error("error");
	if (!obj["data"].isObject())
		throw std::runtime_error("error");
	const json::Object &obj1 = obj["data"];
	const json::Object &obj2 = obj1["scene"];
}

inline void testSerialize() {
	// TODO add initializer list
	json::Object cameraTransform;
	cameraTransform.add("rotation", { 0.f, 0.f, 0.f, 1.f });
	cameraTransform.add("position", { 5.f, 6.f, 7.f });
	json::Object camera;
	camera.add("fov", 60.f);
	camera.add("transform", cameraTransform);

	json::Object sceneTransform;
	sceneTransform.add("rotation", { 0.f, 0.f, 0.f, 1.f });
	sceneTransform.add("position", { 0.f, 0.f, 0.f });
	sceneTransform.add("scale", { 1.f, 1.f, 1.f });
	json::Object scene;
	scene.add("transform", sceneTransform);

	json::Object image;
	image.add("width", 800);
	image.add("height", 600);

	json::Object data;
	data.add("clay", false);
	data.add("alpha", false);
	data.add("tod", 12.f);
	data.add("camera", camera);
	data.add("scene", scene);
	data.add("image", image);


	json::JSON j;
	json::Object &command = j;
	command.add("hash", "blablabla");
	command.add("data", data);
	command.add("nulltest", json::Variable());

	json::Serializer serializer;
	std::string str = serializer(j);

	int ss = 0;
}