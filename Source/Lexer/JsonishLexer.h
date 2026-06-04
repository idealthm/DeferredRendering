#pragma once
#include "JsonLexme.h"
#include "Lexer.h"

class JsonishLexer final: public Lexer<JsonLexeme> {
public:
	virtual bool readLexeme() noexcept override;
private:
	JsonType readIdentifier() noexcept;
	void read_number() noexcept;
	void readString() noexcept;
	JsonType readPunctuation() noexcept;
};

