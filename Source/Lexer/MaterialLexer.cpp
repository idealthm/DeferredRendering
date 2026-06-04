#include "MaterialLexer.h"

bool MaterialLexer::readLexeme() noexcept
{
	skipWhiteSpace();

	MaterialType nextMaterialType;
	bool peeked = peek(&nextMaterialType);
	if (!peeked) {
		return true;
	}

	const char* lexemeStart = m_Cursor;
	size_t line = getLine();
	size_t cursor = getCursor();
	switch (nextMaterialType) {
	case MaterialType::BLOCK:
		if (!readBlock()) {
			nextMaterialType = MaterialType::UNKNOWN;
			break;
		}
	case MaterialType::IDENTIFIER: readIdentifier(); break;
	case MaterialType::UNKNOWN:    readUnknown();    break;
	default:
		break;
	}
	m_Lexemes.emplace_back(nextMaterialType, lexemeStart, m_Cursor - 1, line, cursor);

	return nextMaterialType != UNKNOWN;
}

bool MaterialLexer::peek(MaterialType* type) const noexcept
{
	if (!HasMore()) return false;

	char c = *m_Cursor;
	if (isAlphaCharacter(c)) {
		*type = MaterialType::IDENTIFIER;
	} else if (c == '{') {
		*type = MaterialType::BLOCK;
	} else {
		*type = MaterialType::UNKNOWN;
	}

	return true;
}

bool MaterialLexer::readBlock() noexcept
{
	size_t braceCount = 0;
	while (HasMore()) {
		skipWhiteSpace();

		// This can occur if the block is malformed.
		if (m_Cursor >= m_End) {
			return false;
		}

		if (*m_Cursor == '{') {
			braceCount++;
		}  else if (*m_Cursor == '}') {
			braceCount--;
		}

		if (braceCount == 0) {
			Consume();
			return true;
		}

		Consume();
	}
	return true;
}

void MaterialLexer::readIdentifier() noexcept
{
	while (HasMore() && isAphaNumericCharacter(*m_Cursor)) {
		Consume();
	}
}

void MaterialLexer::readUnknown() noexcept
{
	Consume();
}
