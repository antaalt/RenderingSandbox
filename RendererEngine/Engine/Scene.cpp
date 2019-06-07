#include "Scene.h"


namespace engine {

namespace world {

Node::Node()
{
}

geom::mat4 Node::getModel() const
{
	if (isRoot())
		return transform;
	return parent->getModel() * transform;
}

bool Node::isRoot() const
{
	return parent == nullptr;
}

void Node::draw(const geom::mat4 & model, const geom::mat4 & view, const geom::mat4 & projection)
{
}


}

}

