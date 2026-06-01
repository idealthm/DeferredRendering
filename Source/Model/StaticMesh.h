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
	Ref<RHI::RenderPrimitive> primitive;
	Ref<RHI::VertexBuffer> vb;
	Ref<RHI::IndexBuffer> ibo;
};

class StaticMesh
{
public:
	StaticMesh(const StaticMeshDesc& desc, std::vector<Ref<MeshSection>> sections, const Ref<MaterialInstance>& mi);

	std::vector<Ref<MeshSection>>& GetMeshSections() {return m_LoadedMeshes;}
	const std::vector<Ref<MeshSection>>& GetMeshSections() const {return m_LoadedMeshes;}

	const Ref<RHI::RenderPrimitive>& GetRenderPrimitive() const { return m_Desc.primitive; }
	const Ref<RHI::VertexBuffer>& GetVertexBuffer() const      { return m_Desc.vb; }
	const Ref<RHI::IndexBuffer>& GetIndexBuffer() const        { return m_Desc.ibo; }

	static Ref<StaticMesh> Create(const Asset& asset, const Ref<MaterialInstance>& mi);

	void SetMaterialInstance(Ref<MaterialInstance> instance);
	Ref<MaterialInstance> GetMaterial();

private:
	StaticMeshDesc                        m_Desc;
	std::vector<Ref<MeshSection>>         m_LoadedMeshes;
	Ref<MaterialInstance>                 m_Material;
};
