#pragma once
#include "Common/Utils/Bitset.h"
#include "RHI/BindingMap.h"
#include "RHI/RHIDriver.h"
#include "Shader/Program.h"


namespace RHI
{
	class OpenGLContext;
}

namespace RHI
{
	class GLDriver;
}

class GLProgram : public RHI::HwProgram
{
public:
	GLProgram() = default;
	GLProgram(RHI::GLDriver* driver, Program&& program) noexcept;

	bool use(RHI::GLDriver* driver, RHI::OpenGLContext* context);

	void InitializeProgramState(RHI::OpenGLContext& context, GLuint program, DescriptorSetInfo& info);

	GLuint getBufferBinding(descriptor_set_t set, descriptor_binding_t binding) const noexcept;

	GLuint getTextureUnit(descriptor_set_t set, descriptor_binding_t binding) const noexcept;

	Util::bitset64 getActiveDescriptors(descriptor_set_t set) const;
public:
	struct {
		GLuint program = 0;
	} gl;  
private:
	BindingMap m_BindingMap;

	
};
