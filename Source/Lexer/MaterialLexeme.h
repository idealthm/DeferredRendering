#pragma once
#include "Lexeme.h"

enum MaterialType {
	BLOCK,
	IDENTIFIER,
	UNKNOWN
};

class MaterialLexeme final: public Lexeme<MaterialType> {
public:
	MaterialLexeme(MaterialType type, const char* start, const char* end, size_t line, size_t pos) :
			Lexeme(type, start, end, line, pos) {
	}

	MaterialLexeme trimBlockMarkers() const {
		return { m_Type, m_Start + 1, m_End - 1, m_LineNumber, m_Position };
	}
};