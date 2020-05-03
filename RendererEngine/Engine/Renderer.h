#pragma once

#include "RenderingSession.h"


namespace engine {

class Renderer
{
public:
	Renderer();
	~Renderer();

	void commit(const RenderingSession &session);

	void prepare(const RenderingSession &session);
	void render(const RenderingSession &session);
	void finish(const RenderingSession &session);
};

}