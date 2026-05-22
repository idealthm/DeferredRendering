#pragma once
#include <string>
#include <vector>

#include "MeshSection.h"
#include "Asset.h"
#include "Shader/Program.h"

namespace RHI
{
	class VertexBuffer;
	class IndexBuffer;
	class RenderPrimitive;
}

struct StaticMeshDesc
{
	std::string importPath;
};

class StaticMesh
{
public:
	StaticMesh(const StaticMeshDesc& desc, std::vector<Ref<MeshSection>> sections,
	           Ref<RHI::RenderPrimitive> primitive, Ref<RHI::VertexBuffer> vb, Ref<RHI::IndexBuffer> ibo);

	std::vector<Ref<MeshSection>>& GetMeshSections() {return m_LoadedMeshes;}
	const std::vector<Ref<MeshSection>>& GetMeshSections() const {return m_LoadedMeshes;}

	const Ref<RHI::RenderPrimitive>& GetRenderPrimitive() const { return m_RenderPrimitive; }
	const Ref<RHI::VertexBuffer>& GetVertexBuffer() const      { return m_VertexBuffer; }
	const Ref<RHI::IndexBuffer>& GetIndexBuffer() const        { return m_IndexBuffer; }

	static Ref<StaticMesh> Create(const Asset& asset);

private:
	StaticMeshDesc                        m_Desc;
	Ref<RHI::RenderPrimitive>             m_RenderPrimitive;
	Ref<RHI::VertexBuffer>                m_VertexBuffer;
	Ref<RHI::IndexBuffer>                 m_IndexBuffer;
	std::vector<Ref<MeshSection>>         m_LoadedMeshes;
};
