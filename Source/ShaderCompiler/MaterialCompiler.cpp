#include "MaterialCompiler.h"
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include "Common/Material/MaterialBuilder.h"
#include "Common/Material/Package.h"
#include "IncludeCallbaks.h"
#include "Lexer/JsonishLexer.h"
#include "Lexer/MaterialLexeme.h"
#include "Lexer/MaterialLexer.h"
#include "ParameterProcessor.h"
#include "Parser/JsonishParser.h"

static constexpr const char* CK_MATERIAL  = "material";
static constexpr const char* CK_VERTEX    = "vertex";
static constexpr const char* CK_FRAGMENT  = "fragment";
static constexpr const char* CK_COMPUTE   = "compute";
static constexpr const char* CK_TOOL      = "tool";

MaterialCompiler::MaterialCompiler()
{
	mConfigProcessor[CK_MATERIAL]  = &MaterialCompiler::processMaterial;
	mConfigProcessor[CK_VERTEX]    = &MaterialCompiler::processVertexShader;
	mConfigProcessor[CK_FRAGMENT]  = &MaterialCompiler::processFragmentShader;
	mConfigProcessor[CK_COMPUTE]   = &MaterialCompiler::processComputeShader;
	mConfigProcessor[CK_TOOL]      = &MaterialCompiler::ignoreLexeme;
	mConfigProcessor["vertexCode"] = &MaterialCompiler::processVertexShader;
	mConfigProcessor["fragmentCode"] = &MaterialCompiler::processFragmentShader;

	mConfigProcessorJSON[CK_MATERIAL]  = &MaterialCompiler::processMaterialJSON;
	mConfigProcessorJSON[CK_VERTEX]    = &MaterialCompiler::processVertexShaderJSON;
	mConfigProcessorJSON[CK_FRAGMENT]  = &MaterialCompiler::processFragmentShaderJSON;
	mConfigProcessorJSON[CK_COMPUTE]   = &MaterialCompiler::processComputeShaderJSON;
	mConfigProcessorJSON[CK_TOOL]      = &MaterialCompiler::ignoreLexemeJSON;
}

bool MaterialCompiler::isValidJsonStart(const char* buffer, size_t size) noexcept {
	for (size_t i = 0; i < size; ++i) { char c = buffer[i]; if (c==' '||c=='\t'||c=='\r'||c=='\n') continue; return c == '{'; }
	return false;
}
bool MaterialCompiler::readFile(const std::string& path, std::string& out) const noexcept {
	std::ifstream f(path, std::ios::binary|std::ios::ate); if (!f.is_open()) return false;
	out.resize((size_t)f.tellg()); f.seekg(0); f.read(out.data(), out.size()); return true;
}
bool MaterialCompiler::writeFile(const std::string& path, const uint8_t* data, size_t size) const noexcept {
	std::ofstream f(path, std::ios::binary); if (!f.is_open()) return false;
	f.write(reinterpret_cast<const char*>(data), size); return true;
}

void MaterialCompiler::configureBuilder(MaterialBuilder& builder, const CompilerConfig& config) const noexcept {
	if (!config.includePaths.empty()) builder.includeCallback(makeIncludeCallback(config));
	if (config.compileSpirv) builder.spirvCompiler(makeSpirvCompiler(config));
}

std::function<bool(const std::string&, IncludeResult&)>
MaterialCompiler::makeIncludeCallback(const CompilerConfig& config) const noexcept {
	auto paths = config.includePaths;
	return [paths](const std::string&, IncludeResult& r)->bool {
		for (auto& d : paths) {
			std::string fp = d + "/" + r.includeName;
			std::ifstream f(fp, std::ios::binary|std::ios::ate); if (!f.is_open()) continue;
			r.text.resize((size_t)f.tellg()); f.seekg(0); f.read(r.text.data(), r.text.size()); r.name = fp; return true;
		}
		return false;
	};
}

std::function<bool(const std::string&, const std::string&, std::vector<uint8_t>&, std::vector<uint8_t>&)>
MaterialCompiler::makeSpirvCompiler(const CompilerConfig& config) const noexcept {
	std::string gp = config.glslcPath;
	return [gp](const std::string& vs, const std::string& fs, std::vector<uint8_t>& vspv, std::vector<uint8_t>& fspv)->bool {
		static int c = 0;
		auto comp = [&](const std::string& src, const std::string& stage, std::vector<uint8_t>& spv)->bool {
			int id = c++;
			std::string ti = "_matc_tmp_" + std::to_string(id) + "." + stage;
			std::string to = ti + ".spv";
			{ std::ofstream f(ti); if (!f) return false; f << src; }
			std::string cmd = gp + " -fshader-stage=" + stage + " --target-env=opengl " + ti + " -o " + to;
			int ret = system(cmd.c_str());
			if (ret != 0) std::cerr << "glslc[" << stage << "]: failed (exit " << ret << ")" << std::endl;
			if (ret == 0) { std::ifstream f(to, std::ios::binary|std::ios::ate); if (f) { spv.resize((size_t)f.tellg()); f.seekg(0); f.read((char*)spv.data(), spv.size()); } }
			std::remove(ti.c_str()); std::remove(to.c_str());
			return ret == 0 && !spv.empty();
		};
		return comp(vs, "vert", vspv) && comp(fs, "frag", fspv);
	};
}

bool MaterialCompiler::Run(const CompilerConfig& config) {
	if (config.inputFile.empty()) { std::cerr << "matc: no input file" << std::endl; return false; }
	std::string src;
	if (!readFile(config.inputFile, src)) { std::cerr << "matc: cannot open " << config.inputFile << std::endl; return false; }
	std::cout << "matc: reading " << config.inputFile << " (" << src.size() << " bytes)" << std::endl;

	MaterialBuilder builder; builder.fileName(config.inputFile.c_str());
	configureBuilder(builder, config);

	bool ok = isValidJsonStart(src.data(), src.size())
		? parseMaterialAsJSON(src.data(), src.size(), builder, config)
		: parseMaterial(src.data(), src.size(), builder, config);
	if (!ok) { std::cerr << "matc: failed to parse material" << std::endl; return false; }

	Package pkg = Package::invalidPackage();
	try { pkg = builder.build(); } catch (const std::exception& e) { std::cerr << "matc: exception: " << e.what() << std::endl; return false; }
	if (!pkg.isValid()) { std::cerr << "matc: failed to build material" << std::endl; return false; }

	auto deriveName = [](const std::string& path)->std::string {
		std::string n = path; auto p = n.find_last_of("/\\"); if (p!=std::string::npos) n=n.substr(p+1);
		p = n.rfind('.'); if (p!=std::string::npos) n=n.substr(0,p);
		while (!n.empty() && n.front()=='_') n.erase(0,1);
		while (!n.empty() && n.back()=='_') n.pop_back();
		return n.empty() ? "material" : n;
	};
	std::string matbPath = (config.outputDir.empty()?"CompiledMaterials":config.outputDir) + "/" + deriveName(config.inputFile) + ".matb";
	if (!writeFile(matbPath, pkg.getData(), pkg.getSize())) { std::cerr << "matc: cannot write " << matbPath << std::endl; return false; }
	std::cout << "matc: wrote " << matbPath << " (" << pkg.getSize() << " bytes)" << std::endl;
	return true;
}

bool MaterialCompiler::Build(MaterialBuilder& builder, const CompilerConfig& config) {
	configureBuilder(builder, config);
	Package pkg = Package::invalidPackage();
	try { pkg = builder.build(); } catch (const std::exception& e) { std::cerr << "matc: exception: " << e.what() << std::endl; return false; }
	if (!pkg.isValid()) { std::cerr << "matc: failed to build material" << std::endl; return false; }
	auto deriveName = [](const std::string& path)->std::string {
		std::string n = path; auto p = n.find_last_of("/\\"); if (p!=std::string::npos) n=n.substr(p+1);
		p = n.rfind('.'); if (p!=std::string::npos) n=n.substr(0,p);
		while (!n.empty() && n.front()=='_') n.erase(0,1);
		while (!n.empty() && n.back()=='_') n.pop_back();
		return n.empty() ? "material" : n;
	};
	std::string matbPath = (config.outputDir.empty()?"CompiledMaterials":config.outputDir) + "/" + deriveName(config.inputFile) + ".matb";
	if (!writeFile(matbPath, pkg.getData(), pkg.getSize())) { std::cerr << "matc: cannot write " << matbPath << std::endl; return false; }
	std::cout << "matc: wrote " << matbPath << " (" << pkg.getSize() << " bytes)" << std::endl;
	return true;
}

// ── Block-format parsing ────────────────────────────────────────────────────

bool MaterialCompiler::parseMaterial(const char* buf, size_t sz, MaterialBuilder& b, const CompilerConfig&) const noexcept {
	MaterialLexer lx; lx.Lex(buf, sz); auto& lex = lx.getLexemes();
	for (auto& l : lex) if (l.getType() == MaterialType::UNKNOWN) return false;
	if (lex.size() < 2) return false;
	for (size_t i = 0; i < lex.size(); i += 2) {
		if (i == lex.size()-1) return false;
		auto& id = lex[i]; auto& bl = lex[i+1];
		if (id.getType() != MaterialType::IDENTIFIER || bl.getType() != MaterialType::BLOCK) return false;
		auto it = mConfigProcessor.find(id.getStringValue());
		if (it == mConfigProcessor.end()) { std::cerr << "matc: unknown '" << id.getStringValue() << "'" << std::endl; return false; }
		if (!(*this.*(it->second))(bl, b)) { std::cerr << "matc: error in block '" << id.getStringValue() << "'" << std::endl; return false; }
	}
	return true;
}
bool MaterialCompiler::parseMaterialAsJSON(const char* buf, size_t sz, MaterialBuilder& b, const CompilerConfig&) const noexcept {
	JsonishLexer jl; jl.Lex(buf, sz, 1); JsonishParser p(jl.getLexemes()); auto j = p.parse();
	if (!j) { std::cerr << "matc: JSON parse failed" << std::endl; return false; }
	for (auto& e : j->getEntries()) {
		auto it = mConfigProcessorJSON.find(e.first);
		if (it == mConfigProcessorJSON.end()) { std::cerr << "matc: unknown key '" << e.first << "'" << std::endl; return false; }
		if (!(*this.*(it->second))(e.second, b)) { std::cerr << "matc: error in key '" << e.first << "'" << std::endl; return false; }
	}
	return true;
}

// ── Block processors ────────────────────────────────────────────────────────

bool MaterialCompiler::processMaterial(const MaterialLexeme& lex, MaterialBuilder& b) const noexcept {
	auto t = lex.trimBlockMarkers(); std::string raw = t.getStringValue();
	// Pre-process unquoted keys to proper JSON
	std::string js = "{"; size_t i = 0;
	auto skipWs = [&]{ while(i<raw.size()&&(raw[i]==' '||raw[i]=='\t'||raw[i]=='\r'||raw[i]=='\n'))++i; };
	auto isSpec = [](char c){ return c==':'||c==','||c=='{'||c=='}'||c=='['||c==']'; };
	auto isBool = [](const std::string& tk){ return tk=="true"||tk=="false"; };
	auto isNum  = [](const std::string& tk){ if(tk.empty())return false; for(char c:tk)if(!isdigit((unsigned char)c)&&c!='.'&&c!='-')return false; return true; };
	while (i < raw.size()) {
		skipWs(); if (i>=raw.size()) break;
		if (raw[i]==',') { size_t p=i+1; while(p<raw.size()&&(raw[p]==' '||raw[p]=='\t'||raw[p]=='\r'||raw[p]=='\n'))++p; if(p<raw.size()&&(raw[p]==']'||raw[p]=='}')){i=p;continue;} js+=raw[i++]; continue; }
		if (raw[i]=='"'||raw[i]=='\'') { char q=raw[i]; js+=raw[i++]; while(i<raw.size()&&raw[i]!=q)js+=raw[i++]; if(i<raw.size())js+=raw[i++]; continue; }
		if (isSpec(raw[i])) { js+=raw[i++]; continue; }
		size_t s=i; while(i<raw.size()&&!isSpec(raw[i])&&raw[i]!=' '&&raw[i]!='\t'&&raw[i]!='\r'&&raw[i]!='\n')++i;
		std::string tk=raw.substr(s,i-s); if(tk.empty()){++i;continue;}
		size_t pk=i; while(pk<raw.size()&&(raw[pk]==' '||raw[pk]=='\t'||raw[pk]=='\r'||raw[pk]=='\n'))++pk;
		if (pk<raw.size()&&raw[pk]==':') js += "\""+tk+"\""; else if(isBool(tk)||isNum(tk)) js+=tk; else js += "\""+tk+"\"";
	}
	js += "}";
	JsonishLexer jl; jl.Lex(js.c_str(), js.size(), t.getLine()); JsonishParser p(jl.getLexemes()); auto j = p.parse(); if(!j)return false;
	auto* o = j->toJsonObject(); if(!o)return false;
	ParametersProcessor pp; return pp.process(b, *o);
}
bool MaterialCompiler::processVertexShader(const MaterialLexeme& lex, MaterialBuilder& b) const noexcept
	{ auto t=lex.trimBlockMarkers(); b.materialVertex(t.getStringValue().c_str(), t.getLine()); return true; }
bool MaterialCompiler::processFragmentShader(const MaterialLexeme& lex, MaterialBuilder& b) const noexcept
	{ auto t=lex.trimBlockMarkers(); b.material(t.getStringValue().c_str(), t.getLine()); return true; }
bool MaterialCompiler::processComputeShader(const MaterialLexeme& lex, MaterialBuilder& b) const noexcept
	{ auto t=lex.trimBlockMarkers(); b.material(t.getStringValue().c_str(), t.getLine()); return true; }

// ── JSON processors ─────────────────────────────────────────────────────────

bool MaterialCompiler::processMaterialJSON(const JsonishValue* v, MaterialBuilder& b) const noexcept {
	if (!v || v->getType()!=JsonishValue::OBJECT) return false;
	ParametersProcessor pp; return pp.process(b, *v->toJsonObject());
}
bool MaterialCompiler::processVertexShaderJSON(const JsonishValue* v, MaterialBuilder& b) const noexcept
	{ if(!v)return false; auto* s=v->toJsonString(); if(!s)return false; b.materialVertex(s->getString().c_str()); return true; }
bool MaterialCompiler::processFragmentShaderJSON(const JsonishValue* v, MaterialBuilder& b) const noexcept
	{ if(!v)return false; auto* s=v->toJsonString(); if(!s)return false; b.material(s->getString().c_str()); return true; }
bool MaterialCompiler::processComputeShaderJSON(const JsonishValue* v, MaterialBuilder& b) const noexcept
	{ if(!v)return false; auto* s=v->toJsonString(); if(!s)return false; b.material(s->getString().c_str()); return true; }
