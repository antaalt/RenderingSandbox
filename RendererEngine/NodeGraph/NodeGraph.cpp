#include "NodeGraph.h"

#include <imgui.h>
#include <examples\imgui_impl_glfw.h>
#include <examples\imgui_impl_vulkan.h>
#include <imnodes.h>

namespace node {

int NodeID::id = 0;

NodeGraph::NodeGraph()
{
}


NodeGraph::~NodeGraph()
{
}

void NodeGraph::draw()
{
	if (ImGui::Begin("Node editor", 0, ImGuiWindowFlags_MenuBar))
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Add"))
			{
				if (ImGui::MenuItem("NodeModel"))
				{
					NodeModel *nodeModel = new NodeModel;
					m_nodes.push_back(nodeModel);
				}
				if (ImGui::MenuItem("NodeBuffer"))
				{
					NodeBuffer *nodeBuffer = new NodeBuffer;
					m_nodes.push_back(nodeBuffer);
				}
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		imnodes::BeginNodeEditor();

		for (int i = 0; i < m_links.size(); ++i)
		{
			const std::pair<int, int> &p = m_links[i];
			// in this case, we just use the array index of the link
			// as the unique identifier
			imnodes::Link(i, p.first, p.second);
		}

		for (Node *node : m_nodes)
			node->draw();

		imnodes::EndNodeEditor();

		int start_attr, end_attr;
		if (imnodes::IsLinkCreated(&start_attr, &end_attr))
		{
			m_links.push_back(std::make_pair(start_attr, end_attr));
		}
		int node_id;
		if (imnodes::IsNodeHovered(&node_id))
		{
			//node_hovered = node_id;
		}
		// Note that since many nodes can be selected at once, we first need to query the number of
		// selected nodes before getting them.
		const int num_selected_nodes = imnodes::NumSelectedNodes();
		if (num_selected_nodes > 0)
		{
			std::vector<int> selected_nodes;
			selected_nodes.resize(num_selected_nodes);
			imnodes::GetSelectedNodes(selected_nodes.data());
		}
	}
	ImGui::End();
}

void Node::draw()
{
	imnodes::PushColorStyle(
		imnodes::ColorStyle_TitleBar, IM_COL32(11, 109, 191, 255));
	imnodes::PushColorStyle(
		imnodes::ColorStyle_TitleBarSelected, IM_COL32(81, 148, 204, 255));
	imnodes::BeginNode(m_id);
	for (const std::pair<std::string, int> &attr : m_inputs)
	{
		imnodes::BeginInputAttribute(attr.second);
		ImGui::Text(attr.first.c_str());
		imnodes::EndInputAttribute();
	}
	for (const std::pair<std::string, int> &attr : m_outputs)
	{
		imnodes::BeginOutputAttribute(attr.second);
		ImGui::Text(attr.first.c_str());
		imnodes::EndOutputAttribute();
	}
	drawInside();
	imnodes::EndNode();
	imnodes::PopColorStyle();
	imnodes::PopColorStyle();
}

void NodeModel::drawInside()
{
	ImGui::SameLine(); ImGui::Separator();
	ImGui::InputText("Path", m_buffer, 256);
	if (ImGui::Button("Load"))
	{

	}
}

void NodeBuffer::drawInside()
{
}

void NodeImage::drawInside()
{
}

void NodeStage::drawInside()
{
}

}