#pragma once

#include "Config.h"

namespace engine {

namespace world {

struct Texture {
	unsigned int ID;
	Buffer<unsigned char> bytes;
	unsigned int width, height, components;
};

struct TextureHDR {
	unsigned int ID;
	Buffer<float> bytes;
	unsigned int width, height, components;
};

enum class TextureType {
	ALBEDO,
	NORMAL,
	METALLICNESS,
	NB_TEXTURE_TYPE
};

struct Material {
	Texture *texture[static_cast<unsigned int>(TextureType::NB_TEXTURE_TYPE)];
	geom::colorHDR color;
	float metallicness;
	float roughness;
};

struct Mesh {
	Material *material;
	Buffer<geom::point3> positions;
	Buffer<geom::norm3> normals;
	Buffer<geom::uv2> texcoords[static_cast<unsigned int>(TextureType::NB_TEXTURE_TYPE)];
	Buffer<geom::color32> colors;
	Buffer<unsigned int> indices;

	bool hasNormals() const { return normals.size() > 0; }
	bool hasTexcoords(TextureType type) const { return texcoords[static_cast<unsigned int>(type)].size() > 0; }
	bool hasColors() const { return colors.size() > 0; }
};

struct Node {
	Node();
	Mesh *mesh;				// Mesh of the node
	geom::mat4 transform;	// Local transform of the node

	Node *parent;			// Parent of the node

	// Get the model matrix computed from parents nodes.
	geom::mat4 getModel() const;
	// Is it a root node ?
	bool isRoot() const;
	// Draw the node (Do we need childrens ?)
	void draw(const geom::mat4 &model, const geom::mat4 &view, const geom::mat4 &projection);
};

struct Scene {
	Buffer<Texture> textures;
	Buffer<TextureHDR> texturesHDR;
	Buffer<Material> materials;
	Buffer<Mesh> meshes;

	Buffer<Node> nodes;

	void draw(const geom::mat4 &model, const geom::mat4 &view, const geom::mat4 &projection);
};

}

}