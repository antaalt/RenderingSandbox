#include "Model.h"


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


}

}

