#pragma once
#include <string>

template<typename T>
class Lexeme
{
public:
	Lexeme(T type, const char* start, const char* end, size_t line, size_t pos) :
			m_Start(start), m_End(end), m_LineNumber(line), m_Position(pos), m_Type(type) {
	}

	const char* getStart() const noexcept {
		return m_Start;
	}
	const char* getEnd() const noexcept {
		return m_End;
	}

	const size_t getSize() const noexcept {
		return m_End - m_Start + 1;
	}

	const T getType() const noexcept {
		return m_Type;
	}
	const size_t getLine() const noexcept {
		return m_LineNumber;
	}
	const size_t getLinePosition() const noexcept {
		return m_Position;
	}

	const std::string getStringValue() const noexcept {
		return std::string(m_Start, m_End - m_Start + 1);
	}

protected:
	const char* m_Start; // included
	const char* m_End;   // included
	size_t m_LineNumber;
	size_t m_Position; // The offset in the line.
	T m_Type;	
};