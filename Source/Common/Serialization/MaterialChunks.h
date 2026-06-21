#pragma once

#include "ChunkContainer.h"
#include "Common/Material/MaterialTypes.h"
#include "../../../Include/BufferInterfaceBlock.h"
#include "../../../Include/SamplerInterfaceBlock.h"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

struct FlatProperty { std::string name; uint8_t uniformType = 0; uint8_t propertyId = 0; std::string defaultValue; };
inline FArchive& operator<<(FArchive& ar, FlatProperty& v) { ar << v.name << v.uniformType << v.propertyId << v.defaultValue; return ar; }

inline FArchive& operator<<(FArchive& ar, BufferInterfaceBlock::FieldInfo& v)
{ ar << v.name << v.offset << v.stride << v.type << v.isArray << v.size << v.structName << v.sizeName; return ar; }

inline FArchive& operator<<(FArchive& ar, SamplerInterfaceBlock::SamplerInfo& v)
{ ar << v.name << v.uniformName << v.binding << v.type << v.format << v.multisample; return ar; }

inline FArchive& operator<<(FArchive& ar, VariableParam& v) { ar << v.name << v.type << v.location; return ar; }

// ── Multi-pass chunks ──────────────────────────────────────────────────

struct GlslEntry { std::string pass, vertexGlsl, fragmentGlsl; };
inline FArchive& operator<<(FArchive& ar, GlslEntry& v) { ar << v.pass << v.vertexGlsl << v.fragmentGlsl; return ar; }

struct ChunkGlsl : Chunk {
	static constexpr ChunkType Tag = ChunkType::MaterialGlsl;
	using Container = std::vector<GlslEntry>;
	Container data; explicit ChunkGlsl(Container c) : data(std::move(c)) {}
	ChunkType GetType() const override { return Tag; }
	void Serialize(FArchive& ar) override { ar << data; }
	static bool Serialize(FArchive& ar, Container& obj) { ar << obj; return true; }
};

struct SpirvEntry { std::string pass; std::vector<uint8_t> vertexSpirv, fragmentSpirv; };
inline FArchive& operator<<(FArchive& ar, SpirvEntry& v) { ar << v.pass << v.vertexSpirv << v.fragmentSpirv; return ar; }

struct ChunkSpirv : Chunk {
	static constexpr ChunkType Tag = ChunkType::MaterialSpirv;
	using Container = std::vector<SpirvEntry>;
	Container data; explicit ChunkSpirv(Container c) : data(std::move(c)) {}
	ChunkType GetType() const override { return Tag; }
	void Serialize(FArchive& ar) override { ar << data; }
	static bool Serialize(FArchive& ar, Container& obj) { ar << obj; return true; }
};

struct ChunkUib : Chunk {
	static constexpr ChunkType Tag = ChunkType::MaterialUib;
	using Container = BufferInterfaceBlock;
	Container data; explicit ChunkUib(Container c) : data(std::move(c)) {}
	ChunkType GetType() const override { return Tag; }
	void Serialize(FArchive& ar) override {
		std::string name = data.getName(); uint32_t sz = (uint32_t)data.getSize(); uint8_t al = (uint8_t)data.getAlignment();
		ar << name << sz << al;
		uint32_t n = (uint32_t)data.getFieldInfoList().size(); ar << n;
		for (auto& f : data.getFieldInfoList()) ar << const_cast<BufferInterfaceBlock::FieldInfo&>(f);
	}
	static bool Serialize(FArchive& ar, Container& obj) {
		std::string name; uint32_t sz = 0; uint8_t al = 0;
		if (!ar.IsLoading()) { name = obj.getName(); sz = (uint32_t)obj.getSize(); al = (uint8_t)obj.getAlignment(); }
		ar << name << sz << al;
		uint32_t n = ar.IsLoading() ? 0 : (uint32_t)obj.getFieldInfoList().size(); ar << n;
		if (ar.IsLoading()) {
			BufferInterfaceBlock::Builder b; b.name(name).alignment((BufferInterfaceBlock::Alignment)al);
			for (uint32_t i = 0; i < n; i++) { BufferInterfaceBlock::FieldInfo f; ar << f; b.add({{f.name,f.size,f.type,f.structName,f.stride,f.sizeName}}); }
			obj = std::move(b.build());
		} else { for (auto& f : obj.getFieldInfoList()) ar << const_cast<BufferInterfaceBlock::FieldInfo&>(f); }
		return true;
	}
};

struct ChunkSib : Chunk {
	static constexpr ChunkType Tag = ChunkType::MaterialSib;
	using Container = SamplerInterfaceBlock;
	Container data; explicit ChunkSib(Container c) : data(std::move(c)) {}
	ChunkType GetType() const override { return Tag; }
	void Serialize(FArchive& ar) override {
		std::string name = data.getName(); uint8_t sf = (uint8_t)data.getStageFlags();
		ar << name << sf;
		uint32_t n = (uint32_t)data.getSamplerInfoList().size(); ar << n;
		for (auto& s : data.getSamplerInfoList()) ar << const_cast<SamplerInterfaceBlock::SamplerInfo&>(s);
	}
	static bool Serialize(FArchive& ar, Container& obj) {
		std::string name; uint8_t sf = 0;
		if (!ar.IsLoading()) { name = obj.getName(); sf = (uint8_t)obj.getStageFlags(); }
		ar << name << sf;
		uint32_t n = ar.IsLoading() ? 0 : (uint32_t)obj.getSamplerInfoList().size(); ar << n;
		if (ar.IsLoading()) {
			SamplerInterfaceBlock::Builder b; b.name(name).stageFlags((RHI::ShaderStageFlags)sf);
			for (uint32_t i = 0; i < n; i++) { SamplerInterfaceBlock::SamplerInfo s; ar << s; b.add(s.name, s.binding, s.type, s.format, s.multisample); }
			obj = std::move(b.build());
		} else { for (auto& s : obj.getSamplerInfoList()) ar << const_cast<SamplerInterfaceBlock::SamplerInfo&>(s); }
		return true;
	}
};

inline FArchive& operator<<(FArchive& ar, std::pair<std::string, uint8_t>& v) { ar << v.first << v.second; return ar; }

struct ChunkMaterialAttributesInfo : Chunk {
	static constexpr ChunkType Tag = ChunkType::MaterialAttributeInfo;
	using Container = std::vector<std::pair<std::string, uint8_t>>;
	Container data; explicit ChunkMaterialAttributesInfo(Container c) : data(std::move(c)) {}
	ChunkType GetType() const override { return Tag; }
	void Serialize(FArchive& ar) override { ar << data; }
	static bool Serialize(FArchive& ar, Container& obj) { ar << obj; return true; }
};

inline FArchive& operator<<(FArchive& ar, Descriptor& v) { ar << v.name << v.type << v.binding; return ar; }

struct ChunkMaterialDescriptorBindings : Chunk {
	static constexpr ChunkType Tag = ChunkType::MaterialDescriptorSetLayoutInfo;
	using Container = DescriptorSetInfo;
	Container data; explicit ChunkMaterialDescriptorBindings(Container c) : data(std::move(c)) {}
	ChunkMaterialDescriptorBindings(const SamplerInterfaceBlock& sib, const RHI::DescriptorSetLayout&) {
		for (auto& s : sib.getSamplerInfoList()) data[0].push_back({s.name, RHI::DescriptorType::SAMPLER, s.binding});
	}
	ChunkType GetType() const override { return Tag; }
	void Serialize(FArchive& ar) override { for (auto& b : data) { uint32_t n = (uint32_t)b.size(); ar << n; for (auto& d : b) ar << d; } }
	static bool Serialize(FArchive& ar, Container& obj) { for (auto& b : obj) { uint32_t n = ar.IsLoading()?0:(uint32_t)b.size(); ar << n; if (ar.IsLoading()) b.resize(n); for (auto& d : b) ar << d; } return true; }
};

inline FArchive& operator<<(FArchive& ar, RHI::DescriptorSetLayoutBinding& v) { ar << v.type << v.stageFlags << v.binding << v.flags << v.count; return ar; }

struct ChunkMaterialDescriptorSetLayout : Chunk {
	static constexpr ChunkType Tag = ChunkType::MaterialDescriptorSetLayout;
	using Container = std::array<RHI::DescriptorSetLayout, 2>;
	Container data; explicit ChunkMaterialDescriptorSetLayout(Container c) : data(std::move(c)) {}
	ChunkMaterialDescriptorSetLayout(const SamplerInterfaceBlock& sib, const RHI::DescriptorSetLayout&) {
		for (auto& s : sib.getSamplerInfoList()) { RHI::DescriptorSetLayoutBinding b{}; b.type = RHI::DescriptorType::SAMPLER; b.binding = s.binding; b.stageFlags = RHI::ShaderStageFlags::ALL_SHADER_STAGE_FLAGS; b.count = 1; data[0].bindings.push_back(b); }
	}
	ChunkType GetType() const override { return Tag; }
	void Serialize(FArchive& ar) override { for (auto& l : data) { uint32_t n = (uint32_t)l.bindings.size(); ar << n; for (uint32_t i=0;i<n;i++) ar << l.bindings[i]; } }
	static bool Serialize(FArchive& ar, Container& obj) { for (auto& l : obj) { uint32_t n = ar.IsLoading()?0:(uint32_t)l.bindings.size(); ar << n; if (ar.IsLoading()) l.bindings.resize(n); for (uint32_t i=0;i<n;i++) ar << l.bindings[i]; } return true; }
};

struct ChunkAttributeInfo : Chunk {
	static constexpr ChunkType Tag = ChunkType::MaterialAttributeInfo;
	struct Container { std::vector<VariableParam> inputs, outputs; };
	Container data; explicit ChunkAttributeInfo(Container c) : data(std::move(c)) {}
	ChunkType GetType() const override { return Tag; }
	void Serialize(FArchive& ar) override { ar << data.inputs << data.outputs; }
	static bool Serialize(FArchive& ar, Container& obj) { ar << obj.inputs << obj.outputs; return true; }
};

using ChunkName              = TChunk<ChunkType::MaterialName,             std::string>;
using ChunkVersion           = TChunk<ChunkType::MaterialVersion,          uint32_t>;
using ChunkShading           = TChunk<ChunkType::MaterialShading,          std::string>;
using ChunkDomain            = TChunk<ChunkType::MaterialDomain,           uint8_t>;
using ChunkRequiredAttrs     = TChunk<ChunkType::MaterialRequiredAttributes, uint32_t>;
using ChunkProperties        = TChunk<ChunkType::MaterialProperties,       std::vector<FlatProperty>>;
using ChunkConstants         = TChunk<ChunkType::MaterialConstants,        std::vector<std::string>>;
using ChunkBlendingMode      = TChunk<ChunkType::MaterialBlendingMode,     uint8_t>;
using ChunkDoubleSided       = TChunk<ChunkType::MaterialDoubleSided,      bool>;
using ChunkColorWrite        = TChunk<ChunkType::MaterialColorWrite,       bool>;
using ChunkDepthWrite        = TChunk<ChunkType::MaterialDepthWrite,       bool>;
using ChunkDepthWriteSet     = TChunk<ChunkType::MaterialDepthWriteSet,    bool>;
using ChunkDepthTest         = TChunk<ChunkType::MaterialDepthTest,        bool>;
using ChunkCullingMode       = TChunk<ChunkType::MaterialCullingMode,      uint8_t>;
using ChunkBlendFunction     = TChunk<ChunkType::MaterialBlendFunction,    uint32_t>;
using ChunkMaskThreshold     = TChunk<ChunkType::MaterialMaskThreshold,    float>;
using ChunkShadowMultiplier  = TChunk<ChunkType::MaterialShadowMultiplier, bool>;

using ChunkDescriptorSetBindings  = ChunkMaterialDescriptorBindings;
using ChunkDescriptorSetLayout    = ChunkMaterialDescriptorSetLayout;
using MaterialAttributesInfoChunk        = ChunkMaterialAttributesInfo;
using MaterialUniformInterfaceBlockChunk = ChunkUib;
using MaterialDescriptorBindingsChuck    = ChunkMaterialDescriptorBindings;
using MaterialDescriptorSetLayoutChunk   = ChunkMaterialDescriptorSetLayout;
