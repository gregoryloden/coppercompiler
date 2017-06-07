#include "Project.h"

//comletely parses all directives (builds #replace, evaluates #buildSetting, groups code for #if, #enable, etc.)

//Token* addToIdentifier(Identifier* base, Token* next);
AbstractCodeBlock* parseAbstractCodeBlock(bool endsWithParenthesis);
CDirective* completeDirective(DirectiveTitle* dt);
template <class TokenType> TokenType* parseToken(char* errorMessage);
Identifier* parseIdentifier();
Separator2* parseSeparator();
void parseSeparator(SeparatorType type);
Array<string>* parseParenthesizedCommaSeparatedIdentifierList();

//get the list of tokens and directives
//parse location: EOF
AbstractCodeBlock* parseDirectives() {
	return parseAbstractCodeBlock(false);
}
//get the list of tokens and maybe directives
//parse location: EOF (endsWithParenthesis == false) | the next token after the right parenthesis (endsWithParenthesis == true)
AbstractCodeBlock* parseAbstractCodeBlock(bool endsWithParenthesis) {
	Array<Token*>* tokens = new Array<Token*>();
	Array<CDirective*>* directives = new Array<CDirective*>();
	while (true) {
		Token* next = lex();
		if (next == nullptr) {
			if (endsWithParenthesis)
				makeError(0, "expected a right parenthesis", pos);
			break;
		}

		DirectiveTitle* dt;
		Separator2* s;
		if ((dt = dynamic_cast<DirectiveTitle*>(next)) != nullptr)
			directives->add(completeDirective(dt));
		else if ((s = dynamic_cast<Separator2*>(next)) != nullptr) {
			if (s->type == LeftParenthesis) {
				delete s;
				next = parseAbstractCodeBlock(true);
			} else if (s->type == RightParenthesis) {
				delete s;
				break;
			}
		}

		//whatever it was, put it in tokens to make sure it's syntactically & semantically okay to be where it is
		//if it's a directive and buildDirectives == true, tokens were parsed until after the semicolon
		tokens->add(next);
	}
	return new AbstractCodeBlock(tokens, directives, tokens->length >= 1 ? tokens->inner[0]->contentPos : 0);
}
//get the definition of a directive
//parse location: the next token after the semicolon for the directive
CDirective* completeDirective(DirectiveTitle* dt) {
	CDirective* directive;
	if (dt->title == "replace" || dt->title == "replace-input") {
		Identifier* toReplace = parseIdentifier();
		Array<string>* input = dt->title == "replace-input" ? parseParenthesizedCommaSeparatedIdentifierList() : nullptr;
		parseSeparator(LeftParenthesis);
		AbstractCodeBlock* replacement = parseAbstractCodeBlock(true);
		directive = new CDirectiveReplace(toReplace->name, input, replacement);
		delete toReplace;
	} else
		makeError(0, "unexpected directive type", dt->contentPos);
	parseSeparator(Semicolon);
	dt->directive = directive;
	return directive;
}
//lex a token and make sure that it's the right type
//parse location: the next token after this one
template <class TokenType> TokenType* parseToken(char* errorMessage) {
	LexToken* l;
	if ((l = lex()) == nullptr)
		makeError(0, errorMessage, pos);
	TokenType* t;
	if ((t = dynamic_cast<TokenType*>(l)) == nullptr)
		makeError(0, errorMessage, l->contentPos);
	return t;
}
//lex a token and make sure it's an identifier
//parse location: the next token after the identifier
Identifier* parseIdentifier() {
	return parseToken<Identifier>("expected an identifier");
}
//lex a token and make sure it's a separator
//parse location: the next token after the separator
Separator2* parseSeparator() {
	return parseToken<Separator2>("expected a separator");
}
//lex a token and make sure it's a separator of the right type
//parse location: the next token after the separator
void parseSeparator(SeparatorType type) {
	char* errorMessage;
	switch (type) {
		case Semicolon: errorMessage = "expected a semicolon"; break;
		case LeftParenthesis: errorMessage = "expected a left parenthesis"; break;
		case RightParenthesis: errorMessage = "expected a right parenthesis"; break;
		default: errorMessage = "expected a comma"; break;
	}
	Separator2* s = parseToken<Separator2>(errorMessage);
	if (s->type != type)
		makeError(0, errorMessage, s->contentPos);
	delete s;
}
//lex a comma-separated list of identifiers
//parse location: the next token after the right parenthesis of the list
Array<string>* parseParenthesizedCommaSeparatedIdentifierList() {
	Array<string>* names = new Array<string>();
	parseSeparator(LeftParenthesis);
	while (true) {
		Identifier* identifier = parseIdentifier();
		names->add(identifier->name);
		delete identifier;
		Separator2* separator = parseSeparator();
		if (separator->type == Comma)
			delete separator;
		else if (separator->type == RightParenthesis) {
			delete separator;
			break;
		} else
			makeError(0, "expected a comma or right parenthesis", separator->contentPos);
	}
	return names;
}
