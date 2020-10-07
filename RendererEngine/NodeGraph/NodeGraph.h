#pragma once

#include <map>
#include <vector>

namespace node {

struct NodeID {
	static int generate() { return id++; }
private:
	static int id;
};

struct NodeLink {
	virtual void draw() = 0;
protected:
	int m_id;
};

struct TextNodeLink : NodeLink {
	std::string text;
};

struct InputTextNodeLink : NodeLink {
	std::string text;
};

// --- Nodes
struct Node {
	Node() : m_id(NodeID::generate()) {}
	void draw();
protected:
	virtual void drawInside() = 0;
protected:
	int m_id;
	std::map<std::string, int> m_inputs;
	std::map<std::string, int> m_outputs;
};

struct NodeModel : Node {
	NodeModel() : m_buffer("") {
		m_outputs.insert(std::make_pair("Model", NodeID::generate()));
	}
	void drawInside() override;
private:
	char m_buffer[256];
};

struct NodeBuffer : Node {
	NodeBuffer() {
		m_outputs.insert(std::make_pair("Buffer", NodeID::generate()));
		m_outputs.insert(std::make_pair("Memory", NodeID::generate()));
	}
	void drawInside() override;
};

struct NodeImage : Node {
	NodeImage() {
		m_outputs.insert(std::make_pair("Image", NodeID::generate()));
		m_outputs.insert(std::make_pair("View", NodeID::generate()));
	}
	void drawInside() override;
};

struct NodeStage : Node {
	NodeStage() {
		m_outputs.insert(std::make_pair("Image", NodeID::generate()));
		m_outputs.insert(std::make_pair("View", NodeID::generate()));
	}
	void drawInside() override;
private:

};

struct NodeAttribute {

};

class NodeGraph
{
public:
	NodeGraph();
	~NodeGraph();

	void draw();
private:
	std::vector<Node*> m_nodes;
	std::vector<std::pair<int, int>> m_links;
};

}