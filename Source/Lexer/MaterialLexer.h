#pragma once
#include "Lexer.h"
#include "MaterialLexeme.h"

class MaterialLexer final: public Lexer<MaterialLexeme> {
public:
private:

	virtual bool readLexeme() noexcept override;

	bool peek(MaterialType* type) const noexcept;

	bool readBlock() noexcept;
	void readIdentifier() noexcept;
	void readUnknown() noexcept;
};
