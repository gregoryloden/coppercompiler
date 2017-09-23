#include "Project.h"

//comletely parses all directives, evaluates some
//	(builds #replace, evaluates #buildSetting, groups code for #if, #enable, etc.)

thread_local SourceFile* ParseDirectives::sourceFile;
thread_local Token* ParseDirectives::searchOrigin = nullptr;

//get the list of tokens and directives
//parse location: EOF
void ParseDirectives::parseDirectives(SourceFile* newSourceFile) {
	printf("Parsing directives for %s...\n", newSourceFile->filename.c_str());
	sourceFile = newSourceFile;
	Lex::initializeLexer(newSourceFile);
	newSourceFile->abstractContents = parseAbstractCodeBlock(false, 0);
}
//get the list of tokens and maybe directives
//parse location: EOF (endsWithParenthesis == false) | the next token after the right parenthesis (endsWithParenthesis == true)
AbstractCodeBlock* ParseDirectives::parseAbstractCodeBlock(bool endsWithParenthesis, int contentPos) {
	Array<Token*>* tokens = new Array<Token*>();
	Array<CDirective*>* directives = nullptr;
	int endContentPos = sourceFile->contentsLength;
	try {
		while (true) {
			Token* next = Lex::lex();
			if (next == nullptr) {
				if (endsWithParenthesis) {
					EmptyToken errorToken (contentPos, sourceFile);
					Error::makeError(ErrorType::EndOfFileWhileSearching, "a matching right parenthesis", &errorToken);
				}
				break;
			}

			DirectiveTitle* dt;
			Separator* s;
			if ((dt = dynamic_cast<DirectiveTitle*>(next)) != nullptr) {
				Deleter<DirectiveTitle> dtDeleter (dt);
				if (directives == nullptr)
					directives = new Array<CDirective*>();
				directives->add(dt->directive = completeDirective(dt));
				dtDeleter.release();
			} else if ((s = dynamic_cast<Separator*>(next)) != nullptr) {
				if (s->type == SeparatorType::LeftParenthesis) {
					next = parseAbstractCodeBlock(true, s->contentPos + 1);
					delete s;
				} else if (s->type == SeparatorType::RightParenthesis) {
					Deleter<Separator> sDeleter (s);
					if (!endsWithParenthesis)
						Error::makeError(
							ErrorType::General, "found a right parenthesis without a matching left parenthesis", s);
					endContentPos = s->contentPos;
					break;
				}
			}

			//whatever it was, put it in tokens to make sure it's semantically okay to be where it is
			//if it's a directive, tokens were parsed to complete it
			tokens->add(next);
		}
	} catch (...) {
		//try to balance parentheses
		if (endsWithParenthesis) {
			try {
				for (int parentheses = 1; parentheses > 0;) {
					LexToken* next = Lex::lex();
					Separator* s;
					if ((s = dynamic_cast<Separator*>(next)) != nullptr) {
						if (s->type == SeparatorType::LeftParenthesis)
							parentheses++;
						else if (s->type == SeparatorType::RightParenthesis)
							parentheses--;
					}
					delete next;
				}
			} catch (...) {
				// well we tried, just return what we've got
			}
		}
	}
	return new AbstractCodeBlock(tokens, directives, contentPos, endContentPos, sourceFile);
}
//get the definition of a directive
//parse location: the next token after the directive
//may throw
CDirective* ParseDirectives::completeDirective(DirectiveTitle* dt) {
	if (dt->title == "replace")
		return completeDirectiveReplace(false, dt);
	else if (dt->title == "replace-input")
		return completeDirectiveReplace(true, dt);
	else if (dt->title == "include")
		return completeDirectiveInclude(false, dt);
	else if (dt->title == "include-all")
		return completeDirectiveInclude(true, dt);
	//other directives may change the lexing mode

	Error::makeError(ErrorType::General, "unknown directive type", dt);
	return nullptr;
}
//get the definition of a replace directive
//parse location: the next token after the replace directive
//may throw
CDirectiveReplace* ParseDirectives::completeDirectiveReplace(bool replaceInput, DirectiveTitle* endOfFileErrorToken) {
	Deleter<Identifier> toReplace(parseToken<Identifier>("an identifier to replace", endOfFileErrorToken));
	Deleter<Array<string>> input(replaceInput ? parseReplaceParameters(toReplace.retrieve()) : nullptr);
	int parenthesisContentPos = parseSeparator(
		SeparatorType::LeftParenthesis, "a starting left parenthesis for the replacement body", endOfFileErrorToken);
	return new CDirectiveReplace(
		toReplace.release(), input.release(), parseAbstractCodeBlock(true, parenthesisContentPos + 1), sourceFile);
}
//get the definition of an include directive
//parse location: the next token after the include directive
//may throw
CDirectiveInclude* ParseDirectives::completeDirectiveInclude(bool includeAll, DirectiveTitle* endOfFileErrorToken) {
	StringLiteral* filenameLiteral = parseToken<StringLiteral>("a filename", endOfFileErrorToken);
	CDirectiveInclude* directive = new CDirectiveInclude(filenameLiteral->val, includeAll);
	delete filenameLiteral;
	return directive;
}
//lex a token and make sure that it's the right type
//parse location: the next token after this one
//may throw
template <class TokenType> TokenType* ParseDirectives::parseToken(
	const char* expectedTokenTypeName, Token* endOfFileErrorToken)
{
	LexToken* l = Lex::lex();
	if (l == nullptr)
		Error::makeError(ErrorType::EndOfFileWhileSearching, expectedTokenTypeName, endOfFileErrorToken);
	TokenType* t;
	if ((t = dynamic_cast<TokenType*>(l)) == nullptr) {
		Deleter<LexToken> lDeleter (l);
		makeUnexpectedTokenError(expectedTokenTypeName, l);
	}
	return t;
}
//lex a token and make sure it's a separator of the right type
//parse location: the next token after the separator
//may throw
int ParseDirectives::parseSeparator(SeparatorType type, const char* expectedTokenTypeName, Token* endOfFileErrorToken) {
	Separator* s = parseToken<Separator>(expectedTokenTypeName, endOfFileErrorToken);
	Deleter<Separator> sDeleter (s);
	if (s->type != type)
		makeUnexpectedTokenError(expectedTokenTypeName, s);
	return s->contentPos;
}
//lex a comma-separated list of identifiers
//parse location: the next token after the right parenthesis of the list
//may throw
Array<string>* ParseDirectives::parseReplaceParameters(Identifier* endOfFileErrorToken) {
	Array<string>* names = new Array<string>();
	Deleter<Array<string>> namesDeleter (names);
	int parenthesisPos = parseSeparator(
		SeparatorType::LeftParenthesis, "a left parenthesis for the replace-input parameters", endOfFileErrorToken);
	LexToken* initial = Lex::lex();
	if (initial == nullptr)
		Error::makeError(ErrorType::EndOfFileWhileSearching, "the replace-input parameters", endOfFileErrorToken);
	Identifier* identifier;
	if ((identifier = dynamic_cast<Identifier*>(initial)) == nullptr) {
		Separator* s;
		if ((s = dynamic_cast<Separator*>(initial)) == nullptr || s->type != SeparatorType::RightParenthesis) {
			Deleter<LexToken> initialDeleter (initial);
			makeUnexpectedTokenError("an identifier or right parenthesis", initial);
		}
		delete s;
		return namesDeleter.release();
	}
	char* expectedTokenTypeName = "a comma or right parenthesis";
	while (true) {
		names->add(identifier->name);
		delete identifier;
		Separator* s = parseToken<Separator>(expectedTokenTypeName, endOfFileErrorToken);
		Deleter<Separator> sDeleter (s);
		if (s->type == SeparatorType::RightParenthesis)
			return namesDeleter.release();
		else if (s->type != SeparatorType::Comma)
			makeUnexpectedTokenError(expectedTokenTypeName, s);
		identifier = parseToken<Identifier>("a replace-input parameter", endOfFileErrorToken);
	}
}
//throw an error about an unexpected token
void ParseDirectives::makeUnexpectedTokenError(const char* expectedTokenTypeName, Token* t) {
	string message = string("expected ") + expectedTokenTypeName;
	Error::makeError(ErrorType::General, message.c_str(), t);
}
