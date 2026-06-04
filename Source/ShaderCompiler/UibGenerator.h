#pragma once
#include "BufferInterfaceBlock.h"
#include "UibStruct.h"

namespace UIB
{
	inline const BufferInterfaceBlock& GetPerRenderableUIB()
	{
		static BufferInterfaceBlock const uib = BufferInterfaceBlock::Builder()
			.name(PerRenderableUib::_name)
			.add({{"data", CONFIG_MAX_INSTANCES, BufferInterfaceBlock::Type::STRUCT, "PerRenderableData", sizeof(PerRenderableData)}})
			.build();
		return uib;
	}

	inline const BufferInterfaceBlock& GetPerViewUIB()
	{
		static BufferInterfaceBlock const uib = BufferInterfaceBlock::Builder()
			.name(PerViewUib::_name)
			.add({
				{"viewFromWorldMatrix", 0, BufferInterfaceBlock::Type::MAT4},
				{"worldFromViewMatrix", 0, BufferInterfaceBlock::Type::MAT4},
				{"clipFromViewMatrix", 0, BufferInterfaceBlock::Type::MAT4},
				{"viewFromClipMatrix", 0, BufferInterfaceBlock::Type::MAT4},
				{"clipFromWorldMatrix", 0, BufferInterfaceBlock::Type::MAT4},
				{"worldFromClipMatrix", 0, BufferInterfaceBlock::Type::MAT4},
				{"RenderMode", 0, BufferInterfaceBlock::Type::UINT},
				{"iblParams", 0, BufferInterfaceBlock::Type::FLOAT4},
			})
			.build();
		return uib;
	}
}
