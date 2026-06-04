#include "JsonishLexer.h"

bool JsonishLexer::readLexeme() noexcept
{
	skipWhiteSpace();

	if (!HasMore()) {
		return true;
	}

	const char* lexemeStart = m_Cursor;
	size_t line = getLine();
	size_t cursor = getCursor();

	JsonType lexemeType;
	switch (char c = peek()) {
	case '"' :
		readString();
		lexemeType = STRING;
		break;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case '-':
		read_number();
		lexemeType = NUMBER;
		break;
	case ',':
	case ':':
	case '[':
	case ']':
	case '{':
	case '}':
		lexemeType = readPunctuation();
		break;
	default:
		if (isIdentifierCharacter(c)) {
			lexemeType = readIdentifier();
			break;
		} 
		return false;
	}

	JsonLexeme lexeme(lexemeType, lexemeStart, m_Cursor - 1, line, cursor);
	lexeme = lexeme.trim();
	m_Lexemes.push_back(lexeme);
	return true;
}

JsonType JsonishLexer::readIdentifier() noexcept
{
	const char* lexemeStart = m_Cursor;

	while (HasMore() && isIdentifierCharacter(*m_Cursor)) {
		Consume();
	}

	size_t lexemeSize = m_Cursor - lexemeStart;

	// Check what kind of keyword we got here.
	if (strncmp("true", lexemeStart, lexemeSize) == 0) {
		return BOOLEAN;
	} else if (strncmp("false", lexemeStart, lexemeSize) == 0) {
		return BOOLEAN;
	} else if (strncmp("null", lexemeStart, lexemeSize) == 0) {
		return NUll;
	} else {
		// Relax constraint where string MUST be surrounded by " characters.
		// All identifiers are treated as STRINGs.
		return STRING;
	}
}

void JsonishLexer::read_number() noexcept
{
	if (HasMore() && (isNumericCharacter(*m_Cursor) || *m_Cursor == '-')) {
		Consume();
	}

	while(HasMore() && isNumericCharacter(*m_Cursor)) {
		Consume();
	}

	if (!HasMore()) {
		return;
	}

	// Read fraction if present (TODO: Handle exp).
	if (*m_Cursor != '.') {
		return;
	}

	// There is a fractional part.
	Consume();
	while (HasMore() && isNumericCharacter(*m_Cursor)) {
		Consume();
	}
}

void JsonishLexer::readString() noexcept
{
	Consume(); // Skip first double quote character.
	while (HasMore() && *m_Cursor != '"') {
		Consume();
	}
	if (HasMore()) {
		Consume(); // Skip the last double quote character.
	}
}

JsonType JsonishLexer::readPunctuation() noexcept
{
	char punctuation = Consume();

	// Add lexeme.
	switch (punctuation) {
	case ',': return COMMA;
	case ':': return COLUMN;
	case '[': return ARRAY_START;
	case ']': return ARRAY_END;
	case '{': return BLOCK_START;
	case '}': return BLOCK_END;
	default : return readIdentifier(); // Treat everything else as an identifier.
	}
}
