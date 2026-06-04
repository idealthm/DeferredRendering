#pragma once
#include "Lexeme.h"

enum JsonType {
	BOOLEAN,
	NUll,
	STRING,
	NUMBER,
	BLOCK_START,
	BLOCK_END,
	COMMA,
	ARRAY_START,
	ARRAY_END,
	COLUMN,
};


class JsonLexeme : public Lexeme<JsonType>
{
public:
	static const char* GetTypeString(JsonType type)
	{
		switch (type) {
		case BOOLEAN: return "BOOLEAN";
		case NUll: return "NULL";
		case STRING: return "STRING";
		case NUMBER: return "NUMBER";
		case BLOCK_START: return "BLOCK_START";
		case BLOCK_END: return "BLOCK_END";
		case COMMA: return "COMMA";
		case ARRAY_START: return "ARRAY_START";
		case ARRAY_END: return "ARRAY_END";
		case COLUMN: return "COLUMN";
		}
		return "UNKNOWN";
	}

	JsonLexeme(JsonType type, const char* start, const char* end, size_t line, size_t pos) :
			Lexeme(type, start, end, line, pos)
	{
	}

	// Remove double quote around the lexeme if it is a STRING.
	JsonLexeme trim() {
		if (m_Type != STRING) {
			return *this;
		}
		const char *start = (*m_Start == '"') ? m_Start + 1 : m_Start;
		const char *end = (*m_End == '"') ? m_End - 1 : m_End;
		return { m_Type, start, end, m_LineNumber, m_Position };
	}
};