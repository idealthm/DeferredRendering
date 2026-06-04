#pragma once
#include <vector>

template<typename T>
class Lexer
{
public:
	virtual ~Lexer() = default;

	void Lex(const char* text, size_t size, size_t lineOffset = 1)
	{
		m_Cursor = text;
		m_End = text + size;
		m_LineCounter = lineOffset;
		while (HasMore())
		{
			if (!readLexeme())
			{
				return;
			}
		}
	}

	const std::vector<T>& getLexemes() const noexcept { return m_Lexemes; }

protected:
	bool HasMore() const noexcept
	{
		return m_Cursor < m_End;
	}

	size_t getLine() const noexcept {
		return m_LineCounter;
	}

	size_t getCursor() const noexcept {
		return m_LinePosition;
	}

	inline bool isWhiteSpaceCharacter(char c) const noexcept {
		return c < 33 || c > 127;
	}

	inline bool isAlphaCharacter(char c) const noexcept {
		return (c >= 'A' && c <= 'Z') ||  (c >= 'a' && c <= 'z');
	}

	inline bool isNumericCharacter(char c) const noexcept {
		return (c >= '0' && c <= '9');
	}

	inline bool isAphaNumericCharacter(char c) const noexcept {
		return  isAlphaCharacter(c) || isNumericCharacter(c);
	}

	inline bool isIdentifierCharacter(char c) const noexcept {
		return isAphaNumericCharacter(c) || (c == '_');
	}

	inline void skipUntilEndOfLine() {
		while (HasMore() && *m_Cursor != '\n') {
			Consume();
		}
	}

	inline char Consume() noexcept {
		if (*m_Cursor == '\n') {
			m_LineCounter++;
			m_LinePosition = 0;
		}
		m_LinePosition++;
		return *m_Cursor++;
	}

	inline char peek() const noexcept {
		return *m_Cursor;
	}

	// Read the next Lexeme. It will be of the type peek() returned just before
	// this method was called. This method can return false if it wishes lexing to stop.
	virtual bool readLexeme() noexcept = 0;

	void skipWhiteSpace() {
		while (m_Cursor < m_End && isWhiteSpaceCharacter(*m_Cursor)) {
			Consume();
		}

		// If a comment marker is detected "//", skip until the next return
		// carriage is encountered.
		if (HasMore() && *m_Cursor == '/' && m_Cursor < (m_End - 1) && *(m_Cursor + 1) == '/') {
			skipUntilEndOfLine();
			skipWhiteSpace();
		}
	}

	const char* m_Cursor = nullptr;
	const char* m_End = nullptr;

	std::vector<T> m_Lexemes;

	size_t m_LineCounter = 0;
	size_t m_LinePosition = 0;
};
