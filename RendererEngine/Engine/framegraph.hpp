#pragma once

#include <functional>
#include <vector>
#include <map>
#include <stdint.h>

// http://themaister.net/blog/2017/08/15/render-graphs-and-vulkan-a-deep-dive/

namespace app {

namespace render {
using DeviceSize = size_t;
class Context {};
}


namespace graph {

class Framegraph;
class Resource;
class RenderTask;

template <typename Type, typename Tag>
struct IndexType {
private:
	uint32_t value;
};
// Or use string ?
struct ResourceIDTag {};
//using ResourceID = IndexType<uint32_t, ResourceIDTag>;
struct ResourceID {
	explicit ResourceID(const char* );
	explicit ResourceID(const std::string &str);
};


class Renderer final {
public:
	void create();
	void destroy();
private:
	render::Context m_context;
	Framegraph m_framegraph;
};

class Framegraph final {
public:
	// Setup the timeline (must be called after compile.)
	void setup(render::Context &context);
	// Destroy the timeline
	void destroy(render::Context &context);
	// Compile the timeline
	void compile() {
		// Find all rendertask that do not rely on any resources.
		// Put them first in the timeline. 
		// Make them async if necessary, sort them by queue.
		// Barrier only for one queue
		// 
	}
	// Execute the timeline.
	void execute(render::Context &context) {
		for (std::pair<std::string, RenderTask*> renderTask : m_renderTasks)
			renderTask.second->execute(context);
	}

	/*ComputeTask &addComputeTask(const std::string &renderTaskName);
	GraphicTask &addGraphicTask(const std::string &renderTaskName);
	RaytracingTask &addRaytracingTask(const std::string &renderTaskName);*/

	// Create a resource for this render task.
	template <typename ResourceType>
	ResourceType &create(const ResourceID &resourceID, const ResourceType::CreateInfo &info) {
		static_assert(std::is_convertible<ResourceType*, Resource*>::value, "Type must inherit Resource class");
		ResourceType *resource = ResourceType::create(info);
		m_resources.insert(std::make_pair(resourceID, resource));
		return *resource;
	}

	Resource * getResource(const ResourceID &resourceID);
	Image * getImage(const ResourceID &resourceID);
	Buffer * getBuffer(const ResourceID &resourceID);

	void addRenderTask(const std::string &renderTaskName, RenderTask * renderTask);

private:
	std::map<ResourceID, Resource*> m_resources;
	std::map<std::string, RenderTask*> m_renderTasks;
};

// --- Resources
// Use template for creation ? (pass info struct)
// static assert child has member CreateInfo
class Resource {
public:
	virtual void setup(render::Context &context) = 0;
	virtual void destroy(render::Context &context) = 0;

	friend class RenderTask;
public:
	ResourceID getID() const { return m_id; }
protected:
	ResourceID m_id; // & name ? (use map)
	const RenderTask *m_creator; // task which create and manage the resource ? use dedicated class
	std::vector<const RenderTask*> m_reader; // which task read to resource
	std::vector<const RenderTask*> m_writer; // which task write to resource
	std::size_t m_refCount; // Number of references
};

class Buffer : public Resource {
public:
	struct CreateInfo {
		size_t size;
	};

	static Buffer create(const CreateInfo &createInfo);

	void setup(render::Context &context) override {}
	void destroy(render::Context &context) override {}
	CreateInfo m_createInfo;
};

class Image : public Resource {
public:
	struct CreateInfo {
		uint32_t width;
		uint32_t height;
	};

	static Image create(const CreateInfo &createInfo);

	void setup(render::Context &context) override {}
	void destroy(render::Context &context) override {}


};
// Others resources :
// Color image, depth image, stencil image, buffer, export buffer...

// --- Render task
class RenderTask {
public:
	virtual void setup(render::Context &context) = 0;
	virtual void execute(render::Context &context) = 0;
	virtual void destroy(render::Context &context) = 0; // do we need this

public:
	// Is the task dependant on another
	bool isDependent() const { return m_reads.size() != 0; }

public:
	// Create a resource for this render task.
	/*template <typename ResourceType>
	void create(const ResourceID &resourceID, const ResourceType::CreateInfo &info) {
		static_assert(std::is_convertible<ResourceType*, Resource*>::value, "Type must inherit Resource class");
		m_created.push_back(std::make_pair(resourceID, ResourceType::create(info)));
	}*/
	// Add a resource that get read from by this task
	void addReadResource(Resource* resource) {
		auto it = m_reads.insert(std::make_pair(resource->getID(), resource));
		if (it.second)
			resource->m_reader.push_back(this);
	}
	// Add a resource that get written to by this task
	void addWriteResource(Resource* resource) {
		m_writes.insert(std::make_pair(resource->getID(), resource));
		resource->m_writer.push_back(this);
	}

public:
	/*Resource * getResource(const ResourceID &resourceID);
	Image * getImage(const ResourceID &resourceID);
	Buffer * getBuffer(const ResourceID &resourceID);*/

protected:
	Image *getReadImage(const ResourceID &resource) const;
	Image *getWriteImage(const ResourceID &resource) const;
private:
	//std::map<ResourceID, const Resource*> m_created; // which resources are created by renderTask. Only used for tracking.
	std::map<ResourceID, const Resource*> m_reads; // which resources are read by renderTask
	std::map<ResourceID, const Resource*> m_writes; // which resources are written by renderTask
};

// This or inheritance
enum class RenderTaskType {
	COMPUTE_TASK,
	RAYTRACING_TASK,
	GRAPHIC_TASK
};

class ComputeTask : public RenderTask {

};

class RaytracingTask : public RenderTask {

};

class GraphicTask : public RenderTask {

};

} // graph


// --- Example
// Basic hybrid renderer

class RasterTask : public graph::GraphicTask {
public:
	void setup(render::Context &context) override {}
	void execute(render::Context &context) override {}
	void destroy(render::Context &context) override {}
};

class IndirectLightPass : public graph::RaytracingTask {
public:
	void setup(render::Context &context) override {}
	void execute(render::Context &context) override {}
	void destroy(render::Context &context) override {}
};

class DirectLightPass : public graph::RaytracingTask {
public:
	void setup(render::Context &context) override {}
	void execute(render::Context &context) override {}
	void destroy(render::Context &context) override {}
};

class ComposePass : public graph::ComputeTask {
public:
	void setup(render::Context &context) override {}
	void execute(render::Context &context) override {}
	void destroy(render::Context &context) override {}
};

class TonemapperPass : public graph::ComputeTask {
public:
	void setup(render::Context &context) override {}
	void execute(render::Context &context) override {
		graph::Image * imageCompose = getReadImage(graph::ResourceID("imageCompose"));
		graph::Image * imageOutput = getWriteImage(graph::ResourceID("imageOutput"));
		// Do some stuff with this
	}
	void destroy(render::Context &context) override {}
};


void test() {
	graph::Framegraph fg;
	RasterTask rasterTask;
	IndirectLightPass indirectLightTask;
	DirectLightPass directLightTask;
	ComposePass composeTask;
	TonemapperPass tonemapperTask;

	// --- Declare resources
	graph::ResourceID imageAlbedoID("imageAlbedo");
	graph::ResourceID imageNormalID("imageNormal");
	graph::ResourceID imageIndirectLightID("imageIndirectLight");
	graph::ResourceID imageDirectLightID("imageDirectLight");
	graph::ResourceID imageComposeID("imageCompose");
	graph::ResourceID imageOutputID("imageOutput");

	// --- Declare resources to framegraph
	graph::Image &imageAlbedo = fg.create<graph::Image>(imageAlbedoID, graph::Image::CreateInfo{
		1280, 
		720
	});
	graph::Image &imageNormal = fg.create<graph::Image>(imageNormalID, graph::Image::CreateInfo{
		1280,
		720
	});
	graph::Image &imageIndirectLighting = fg.create<graph::Image>(imageIndirectLightID, graph::Image::CreateInfo{
		1280,
		720
	});
	graph::Image &imageDirectLighting = fg.create<graph::Image>(imageDirectLightID, graph::Image::CreateInfo{
		1280,
		720
	});
	graph::Image &imageCompose = fg.create<graph::Image>(imageComposeID, graph::Image::CreateInfo{
		1280,
		720
	});
	graph::Image &imageOutput = fg.create<graph::Image>(imageOutputID, graph::Image::CreateInfo{
		1280,
		720
	});

	// --- Link render task to framegraph
	fg.addRenderTask("rasterTask", &rasterTask);
	fg.addRenderTask("indirectLightTask", &indirectLightTask);
	fg.addRenderTask("directLightTask", &directLightTask);
	fg.addRenderTask("composeTask", &composeTask);
	fg.addRenderTask("tonemapperTask", &tonemapperTask);

	// --- Link resources
	rasterTask.addWriteResource(&imageAlbedo);
	rasterTask.addWriteResource(&imageNormal);

	indirectLightTask.addWriteResource(fg.getResource(imageIndirectLightID));

	directLightTask.addWriteResource(fg.getResource(imageDirectLightID));

	composeTask.addReadResource(&imageAlbedo);
	composeTask.addReadResource(&imageNormal);
	composeTask.addReadResource(fg.getResource(imageIndirectLightID));
	composeTask.addReadResource(fg.getResource(imageDirectLightID));
	composeTask.addWriteResource(fg.getResource(imageComposeID));
	
	tonemapperTask.addReadResource(fg.getResource(imageComposeID));
	tonemapperTask.addWriteResource(fg.getResource(imageOutputID));

	// --- Compile graph
	fg.compile();
	render::Context context;

	// --- Setup resources of timeline
	fg.setup(context);

	bool quit = false;
	while (!quit)
	{
		fg.execute(context);
	}
}

}