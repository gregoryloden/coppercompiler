#include "Project.h"

//comletely parses all directives (builds #replace, evaluates #buildSetting, groups code for #if, #enable, etc.)

thread_local SourceFile* ParseDirectives::sourceFile;

//get the list of tokens and directives
//parse location: EOF
void ParseDirectives::parseDirectives(SourceFile* newSourceFile) {
	sourceFile = newSourceFile;
	Lex::initializeLexer(newSourceFile);
	newSourceFile->abstractContents = parseAbstractCodeBlock(false);
}
//get the list of tokens and maybe directives
//parse location: EOF (endsWithParenthesis == false) | the next token after the right parenthesis (endsWithParenthesis == true)
AbstractCodeBlock* ParseDirectives::parseAbstractCodeBlock(bool endsWithParenthesis) {
	Array<Token*>* tokens = new Array<Token*>();
	Array<CDirective*>* directives = new Array<CDirective*>();
	try {
		while (true) {
			Token* next = Lex::lex();
			if (next == nullptr) {
				if (endsWithParenthesis)
					Lex::makeLexError(EndOfFileWhileSearching, "a right parenthesis");
				break;
			}

			DirectiveTitle* dt;
			Separator2* s;
			if ((dt = dynamic_cast<DirectiveTitle*>(next)) != nullptr) {
				Deleter<DirectiveTitle> dtDeleter (dt);
				directives->add(completeDirective(dt));
				dtDeleter.release();
			} else if ((s = dynamic_cast<Separator2*>(next)) != nullptr) {
				if (s->type == LeftParenthesis) {
					delete s;
					next = parseAbstractCodeBlock(true);
				} else if (s->type == RightParenthesis) {
					Deleter<Separator2> sDeleter (s);
					if (!endsWithParenthesis)
						Error::makeError(General, "found a right parenthesis without a matching left parenthesis", sourceFile, s);
					break;
				}
			}

			//whatever it was, put it in tokens to make sure it's semantically okay to be where it is
			//if it's a directive, tokens were parsed until after the semicolon
			tokens->add(next);
		}
	} catch (...) {
		//try to balance parentheses
		if (endsWithParenthesis) {
			try {
				for (int parentheses = 1; parentheses > 0;) {
					LexToken* next = Lex::lex();
					Separator2* s;
					if ((s = dynamic_cast<Separator2*>(next)) != nullptr) {
						if (s->type == LeftParenthesis)
							parentheses++;
						else if (s->type == RightParenthesis)
							parentheses--;
					}
					delete next;
				}
			} catch (...) {
				// well we tried, just return what we've got
			}
		}
	}
	return new AbstractCodeBlock(tokens, directives);
}
//get the definition of a directive
//parse location: the next token after the directive
CDirective* ParseDirectives::completeDirective(DirectiveTitle* dt) {
	CDirective* directive;
	if (dt->title == "replace")
		directive = completeDirectiveReplace(false);
	else if (dt->title == "replace-input")
		directive = completeDirectiveReplace(true);
	else if (dt->title == "include")
		directive = completeDirectiveInclude(false);
	else if (dt->title == "include-all")
		directive = completeDirectiveInclude(true);
	else
		Error::makeError(General, "unknown directive type", sourceFile, dt);
	dt->directive = directive;
	return directive;
}
//get the definition of a replace directive
//parse location: the next token after the replace directive
CDirectiveReplace* ParseDirectives::completeDirectiveReplace(bool replaceInput) {
	Deleter<Identifier> toReplace(parseIdentifier());
	Deleter<Array<string>> input(replaceInput ? parseParenthesizedCommaSeparatedIdentifierList() : nullptr);
	parseSeparator(LeftParenthesis);
	//use retrieve() so that toReplace deletes the identifier
	return new CDirectiveReplace(toReplace.retrieve()->name, input.release(), parseAbstractCodeBlock(true));
}
//get the definition of an include directive
//parse location: the next token after the include directive
CDirectiveInclude* ParseDirectives::completeDirectiveInclude(bool includeAll) {
	StringLiteral* filenameLiteral = parseToken<StringLiteral>("a filename");
	CDirectiveInclude* directive = new CDirectiveInclude(filenameLiteral->val, includeAll);
	delete filenameLiteral;
	return directive;
}
//lex a token and make sure that it's the right type
//parse location: the next token after this one
template <class TokenType> TokenType* ParseDirectives::parseToken(char* expectedTokenTypeName) {
	LexToken* l;
	if ((l = Lex::lex()) == nullptr)
		Lex::makeLexError(EndOfFileWhileSearching, expectedTokenTypeName);
	TokenType* t;
	if ((t = dynamic_cast<TokenType*>(l)) == nullptr) {
		Deleter<LexToken> lDeleter (l);
		makeUnexpectedTokenError(expectedTokenTypeName, l);
	}
	return t;
}
//lex a token and make sure it's an identifier
//parse location: the next token after the identifier
Identifier* ParseDirectives::parseIdentifier() {
	return parseToken<Identifier>("an identifier");
}
//lex a token and make sure it's a separator
//parse location: the next token after the separator
Separator2* ParseDirectives::parseSeparator() {
	return parseToken<Separator2>("a separator");
}
//lex a token and make sure it's a separator of the right type
//parse location: the next token after the separator
void ParseDirectives::parseSeparator(SeparatorType type) {
	char* expectedTokenTypeName;
	switch (type) {
		case Semicolon: expectedTokenTypeName = "a semicolon"; break;
		case LeftParenthesis: expectedTokenTypeName = "a left parenthesis"; break;
		case RightParenthesis: expectedTokenTypeName = "a right parenthesis"; break;
		default: expectedTokenTypeName = "a comma"; break;
	}
	Separator2* s = parseToken<Separator2>(expectedTokenTypeName);
	Deleter<Separator2> sDeleter (s);
	if (s->type != type)
		makeUnexpectedTokenError(expectedTokenTypeName, s);
}
//lex a comma-separated list of identifiers
//parse location: the next token after the right parenthesis of the list
Array<string>* ParseDirectives::parseParenthesizedCommaSeparatedIdentifierList() {
	Array<string>* names = new Array<string>();
	parseSeparator(LeftParenthesis);
	while (true) {
		Identifier* identifier = parseIdentifier();
		names->add(identifier->name);
		delete identifier;
		Separator2* s = parseSeparator();
		Deleter<Separator2> sDeleter (s);
		if (s->type == RightParenthesis)
			break;
		else if (s->type != Comma)
			makeUnexpectedTokenError("a comma or right parenthesis", s);
	}
	return names;
}
//throw an error about an unexpected token
void ParseDirectives::makeUnexpectedTokenError(char* expectedTokenTypeName, Token* t) {
	sprintf_s(allPurposeStringBuffer, ALL_PURPOSE_STRING_BUFFER_SIZE, "expected %s", expectedTokenTypeName);
	Error::makeError(General, allPurposeStringBuffer, sourceFile, t);
}
