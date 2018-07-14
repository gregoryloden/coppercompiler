#include "Project.h"

//obtains tokens from the source file
//lexing mode can change based on which token came previously

#define inbounds() (pos < contentsLength)
#define outofbounds() (pos >= contentsLength)
#define goToOtherNewlineCharacterIfPresentAndStartRow() \
	char c2;\
	goToOtherNewlineCharacterIfPresentAndStartRowWithCharAndHandling(c2,)
#define goToOtherNewlineCharacterIfPresentAndStartRowWithCharAndHandling(c2, handling) \
	if (pos + 1 < contentsLength && ((c2 = contents[pos + 1]) == '\n' || c2 == '\r') && c2 != c) {\
		pos++;\
		handling;\
	}\
	rowStarts->add(pos + 1);
#define buildOperatorTrie(trieChar, trieType, baseTrie) {trieChar, trieType, sizeof(baseTrie) / sizeof(baseTrie[0]), baseTrie}

/*
MainFunction* nextMainFunction();
string variableName();
Expression* tonum(int base);
bool foundAndSkipped(string s);
void addDigit(BigInt* b, char d);
string stringVal();
char escapeSequenceCharacter();
int toint(int base, size_t loc, size_t end);
*/

struct OperatorTypeTrie {
	char operatorChar;
	OperatorType type;
	int count;
	OperatorTypeTrie* tries;
};
OperatorTypeTrie dotOperatorTries[] = {
	{'.', OperatorType::ObjectMemberAccess, 0, nullptr}
};
OperatorTypeTrie addOperatorTries[] = {
	{'+', OperatorType::Increment, 0, nullptr},
	{'=', OperatorType::AssignAdd, 0, nullptr}
};
OperatorTypeTrie subtractOperatorTries[] = {
	{'-', OperatorType::Decrement, 0, nullptr},
	{'=', OperatorType::AssignSubtract, 0, nullptr}
};
OperatorTypeTrie bitwiseNotOperatorTries[] = {
	{'!', OperatorType::VariableLogicalNot, 0, nullptr},
	{'~', OperatorType::VariableBitwiseNot, 0, nullptr},
	{'-', OperatorType::VariableNegate, 0, nullptr}
};
OperatorTypeTrie logicalNotOperatorTries[] = {
	{'=', OperatorType::NotEqual, 0, nullptr}
};
OperatorTypeTrie multiplyOperatorTries[] = {
	{'=', OperatorType::AssignMultiply, 0, nullptr}
};
OperatorTypeTrie divideOperatorTries[] = {
	{'=', OperatorType::AssignDivide, 0, nullptr}
};
OperatorTypeTrie modulusOperatorTries[] = {
	{'=', OperatorType::AssignModulus, 0, nullptr}
};
OperatorTypeTrie shiftLeftOperatorTries[] = {
	{'=', OperatorType::AssignShiftLeft, 0, nullptr}
};
OperatorTypeTrie lessThanOperatorTries[] = {
	buildOperatorTrie('<', OperatorType::ShiftLeft, shiftLeftOperatorTries),
	{'=', OperatorType::LessOrEqual, 0, nullptr}
};
OperatorTypeTrie shiftArithmeticRightOperatorTries[] = {
	{'=', OperatorType::AssignShiftArithmeticRight, 0, nullptr}
};
OperatorTypeTrie shiftRightOperatorTries[] = {
	buildOperatorTrie('>', OperatorType::ShiftArithmeticRight, shiftArithmeticRightOperatorTries),
	{'=', OperatorType::AssignShiftRight, 0, nullptr}
};
OperatorTypeTrie greaterThanOperatorTries[] = {
	buildOperatorTrie('>', OperatorType::ShiftRight, shiftRightOperatorTries),
	{'=', OperatorType::GreaterOrEqual, 0, nullptr}
};
OperatorTypeTrie bitwiseAndOperatorTries[] = {
	{'&', OperatorType::BooleanAnd, 0, nullptr},
	{'=', OperatorType::AssignBitwiseAnd, 0, nullptr}
};
OperatorTypeTrie bitwiseXorOperatorTries[] = {
	{'=', OperatorType::AssignBitwiseXor, 0, nullptr}
};
OperatorTypeTrie bitwiseOrOperatorTries[] = {
	{'|', OperatorType::BooleanOr, 0, nullptr},
	{'=', OperatorType::AssignBitwiseOr, 0, nullptr}
};
OperatorTypeTrie assignOperatorTries[] = {
	{'=', OperatorType::Equal, 0, nullptr}
};
OperatorTypeTrie baseOperatorTries[] = {
	buildOperatorTrie('.', OperatorType::Dot, dotOperatorTries),
	buildOperatorTrie('+', OperatorType::Add, addOperatorTries),
	buildOperatorTrie('-', OperatorType::Subtract, subtractOperatorTries),
	buildOperatorTrie('~', OperatorType::BitwiseNot, bitwiseNotOperatorTries),
	buildOperatorTrie('!', OperatorType::LogicalNot, logicalNotOperatorTries),
	buildOperatorTrie('*', OperatorType::Multiply, multiplyOperatorTries),
	buildOperatorTrie('/', OperatorType::Divide, divideOperatorTries),
	buildOperatorTrie('%', OperatorType::Modulus, modulusOperatorTries),
	buildOperatorTrie('<', OperatorType::LessThan, lessThanOperatorTries),
	buildOperatorTrie('>', OperatorType::GreaterThan, greaterThanOperatorTries),
	buildOperatorTrie('&', OperatorType::BitwiseAnd, bitwiseAndOperatorTries),
	buildOperatorTrie('^', OperatorType::BitwiseXor, bitwiseXorOperatorTries),
	buildOperatorTrie('|', OperatorType::BitwiseOr, bitwiseOrOperatorTries),
	buildOperatorTrie('=', OperatorType::Assign, assignOperatorTries),
	{':', OperatorType::Colon, 0, nullptr},
	{'?', OperatorType::QuestionMark, 0, nullptr}
};

const char* Lex::trueKeyword = "true";
const char* Lex::falseKeyword = "false";
thread_local SourceFile* Lex::sourceFile;
thread_local char* Lex::contents;
thread_local int Lex::contentsLength = 0;
thread_local int Lex::pos = 0;
thread_local Array<int>* Lex::rowStarts;
thread_local char Lex::c;

//prep lexing with the source file
void Lex::initializeLexer(SourceFile* newSourceFile) {
	sourceFile = newSourceFile;
	contents = sourceFile->contents;
	contentsLength = sourceFile->contentsLength;
	pos = 0;
	rowStarts = sourceFile->rowStarts;
	rowStarts->add(0);
}
//retrieve the next token from contents
//lex location: the first character after the next token | EOF
//may throw
LexToken* Lex::lex() {
	do {
		if (!skipWhitespace())
			return nullptr;
	} while (skipComment());
	LexToken* t;
//int oldPos = pos;
	if ((t = lexIdentifier()) != nullptr
			|| (t = lexNumber()) != nullptr
			|| (t = lexString()) != nullptr
			|| (t = lexCharacter()) != nullptr
			|| (t = lexSeparator()) != nullptr
			|| (t = lexOperator()) != nullptr
			|| (t = lexDirectiveTitle()) != nullptr)
//{
//char cp = contents[pos];
//contents[pos] = 0;
//printf("%s\n", contents + oldPos);
//contents[pos] = cp;
		return t;
//}
	EmptyToken errorToken (pos, sourceFile);
	pos++;
	Error::makeError(ErrorType::General, "unexpected character", &errorToken);
	return nullptr;
}
/*
//split the entire file into a bunch of token expressions
//pos location: clength
void buildTokens() {
	char c;
	size_t oldpos;
	size_t begin = 0;
	Expression* e = NULL;
	pos = 0;
	for (skipWhitespace(); inbounds(); skipWhitespace()) {
		oldpos = pos;
		c = contents[pos];
		//it's a main function
		//the work has already been done
		if ((e = nextMainFunction()) != NULL)
			;
		//variable or keyword
		else if (isalpha(c)) {
			string s = variableName();
			if (s.compare("true") == 0)
				e = new IntConstant(true, oldpos);
			else if (s.compare("false") == 0)
				e = new IntConstant(false, oldpos);
			else
				e = new UnknownValue(s, oldpos);
		//number
		} else if (isdigit(c)) {
			int base = 10;
			//different base
			if (c == '0' && pos + 1 < clength) {
				c = contents[pos + 1];
				//hexadecimal
				if (c == 'x' || c == 'h') {
					base = 16;
					begin = (pos += 2);
					if (outofbounds() || (!isalnum(c = contents[pos]) && c != '_'))
						makeError(1, "the number definition", oldpos);
				//binary
				} else if (c == 'b') {
					base = 2;
					begin = (pos += 2);
					if (outofbounds() || (!isalnum(c = contents[pos]) && c != '_'))
						makeError(1, "the number definition", oldpos);
				//octal
				} else if (c == 'o') {
					base = 8;
					begin = (pos += 2);
					if (outofbounds() || (!isalnum(c = contents[pos]) && c != '_'))
						makeError(1, "the number definition", oldpos);
				//other
				} else if (c == 'r') {
					pos += 2;
					if (outofbounds())
						makeError(1, "the number base", oldpos);
					c = contents[pos];
					if (c >= '2' && c <= '9')
						base = c - '0';
					else if (c >= 'a' && c <= 'z')
						base = c - 'a' + 10;
					else if (c >= 'A' && c <= 'Z')
						base = c - 'A' + 10;
					else
						makeError(0, "unknown base", pos);
					begin = (pos += 1);
					if (outofbounds() || (!isalnum(c = contents[pos]) && c != '_'))
						makeError(1, "the number definition", oldpos);
				//not different base
				} else
					begin = oldpos;
			} else
				begin = oldpos;
			e = tonum(base);
		//string literal
		} else if (c == '\"') {
			pos += 1;
			//don't make a new object until we know its arguments are valid
			string s = stringVal();
			e = new ObjectConstant(s, oldpos);
		//character literal
		} else if (c == '\'') {
			pos += 1;
			if (outofbounds())
				makeError(1, "the character definition", pos - 1);
			c = contents[pos];
			if (c == '\\')
				c = escapeSequenceCharacter();
			else if (c == '\'')
				c = 0;
			else
				pos += 1;
			if (outofbounds() || contents[pos] != '\'')
				makeError(0, "expected a single quote", pos);
			pos += 1;
			e = new IntConstant(c, oldpos);
			castConstant((IntConstant*)(e), TBYTE);
		//division or comment
		} else if (c == '/') {
			pos += 1;
			if (outofbounds())
				makeError(1, "the rest of the expression", pos - 1);
			c = contents[pos];
			//divide-equals
			if (c == '=') {
				pos += 1;
				e = new Operation("/=", oldpos, tokens.length);
			//single line comment
			} else if (c == '/') {
				pos += 1;
				while (inbounds() && contents[pos] != '\n' && contents[pos] != '\r')
					pos += 1;
				continue;
			//multiple line comment
			} else if (c == '*') {
				pos += 1;
				while (inbounds() && (contents[pos] != '/' || contents[pos - 1] != '*'))
					pos += 1;
				if (outofbounds())
					makeError(1, "the end of the comment", oldpos);
				pos += 1;
				continue;
			//regular division
			} else
				e = new Operation("/", oldpos, tokens.length);
		//separator
		} else if (c == '(' || c == ')' || c == ',' || c == ';') {
			pos += 1;
			e = new Separator(c, oldpos);
		//operator
		} else {
			begin = pos;
			pos += 1;
			if (outofbounds())
				makeError(1, "the rest of the expression", begin);
			switch (c) {
				case '~':
					if (contents[pos] == '-' || contents[pos] == '!') {
						pos += 1;
						break;
					}
				case '+':
					if (contents[pos] == c) {
						pos += 1;
						break;
					}
				case '*':
				case '%':
				case '=':
				case '!':
					if (contents[pos] == '=')
						pos += 1;
				case '?':
				case ':':
					break;
				case '-':
					if (contents[pos] == c || contents[pos] == '=' || contents[pos] == '|')
						pos += 1;
					break;
				case '&':
				case '^':
				case '|':
					if (contents[pos] == c)
						pos += 1;
					if (inbounds() && contents[pos] == '=')
						pos += 1;
					break;
				case '>':
				case '<':
					if (contents[pos] == c) {
						pos += 1;
						if (c == '>' && inbounds() && contents[pos] == '>')
							pos += 1;
					} else if (contents[pos] == '-' && pos + 1 < clength && contents[pos + 1] == c)
						pos += 2;
					if (inbounds() && contents[pos] == '=')
						pos += 1;
					break;
				default:
					makeError(0, "unexpected character", begin);
					break;
			}
			e = new Operation(string(contents + begin, pos - begin), oldpos, tokens.length);
		}
		tokens.add(e);
	}
	tlength = tokens.length;
}
*/
//skip all whitespace
//returns whether there are more characters left in contents
//lex location: the next non-whitespace character | EOF
bool Lex::skipWhitespace() {
	for (; inbounds(); pos++) {
		c = contents[pos];
		if (c == '\n' || c == '\r') {
			goToOtherNewlineCharacterIfPresentAndStartRow();
		} else if (c != ' ' && c != '\t')
			return true;
	}
	return false;
}
//skip the next comment if there is one
//returns whether a comment was skipped
//lex location: no change | the first character after the comment | EOF
//may throw
bool Lex::skipComment() {
	char c2;
	if (c != '/' || pos + 1 >= contentsLength || ((c2 = contents[pos + 1]) != '/' && c2 != '*'))
		return false;

	bool lineComment = c2 == '/';
	EmptyToken errorToken (pos, sourceFile);
	pos += 2;
	for (; inbounds(); pos++) {
		c = contents[pos];
		if (c == '\n' || c == '\r') {
			goToOtherNewlineCharacterIfPresentAndStartRow();
			if (lineComment) {
				pos++;
				return true;
			}
		} else if (!lineComment && c == '*' && pos + 1 < contentsLength && contents[pos + 1] == '/') {
			pos += 2;
			return true;
		}
	}
	//reached the end of the file before the comment terminator, that's ok for a line comment but not a block comment
	if (!lineComment)
		Error::makeError(ErrorType::EndOfFileWhileSearching, "the end of the block comment", &errorToken);
	return true;
}
//get a variable name, type, or keyword
//lex location: no change | the first character after the identifier
LexToken* Lex::lexIdentifier() {
	if (!isalpha(c))
		return nullptr;

	int begin = pos;
	pos++;
	//find the rest of the identifier characters
	for (; inbounds(); pos++) {
		c = contents[pos];
		if (!isalnum(c) && c != '_')
			break;
	}
	string s (contents + begin, pos - begin);

	if (s.compare(trueKeyword) == 0)
		return new BoolConstant(true, begin, pos, sourceFile);
	else if (s.compare(falseKeyword) == 0)
		return new BoolConstant(false, begin, pos, sourceFile);
	else
		return new Identifier(s, begin, pos, sourceFile);
}
//get a number constant, either int or float
//lex location: no change | the first character after the number
//may throw
LexToken* Lex::lexNumber() {
	if (!isdigit(c))
		return nullptr;

	EmptyToken errorToken (pos, sourceFile);
	int begin = pos;
	int base = 10;
	bool digitExpected = false;
	//different base
	char c2;
	if (c == '0' && pos + 1 < contentsLength && !isdigit(c2 = contents[pos + 1])) {
		switch (c2) {
			case 'x': base = 16; pos += 2; break;
			case 'o': base = 8; pos += 2; break;
			case 'b': base = 2; pos += 2; break;
		}
		if (outofbounds())
			Error::makeError(ErrorType::EndOfFileWhileReading, "the number definition", &errorToken);
		c = contents[pos];
		digitExpected = true;
	}

	BigInt* num = new BigInt(base);
	Deleter<BigInt> numDeleter (num);
	bool lexingExponent = false;
	bool isFloat = false;
	int fractionDigits = 0;
	int baseExponent = 0;
	bool negativeExponent = false;
	//lex number characters
	while (true) {
		if (c == '_')
			; //underscores can appear anywhere in the number definition after the first character
		else if (c == '.') {
			if (!isFloat && !digitExpected) {
				isFloat = true;
				digitExpected = true;
			} else
				break;
		} else if (c == '^') {
			if (isFloat && !lexingExponent && !digitExpected) {
				lexingExponent = true;
				digitExpected = true;
			} else
				break;
		} else if (c == '-') {
			if (lexingExponent && digitExpected && !negativeExponent)
				negativeExponent = true;
			else
				break;
		} else {
			char digit = cToDigit();
			if ((unsigned char)digit >= (unsigned char)base)
				break;
			if (lexingExponent) {
//				if (baseExponent < FloatConstant::FLOAT_TOO_BIG_EXPONENT)
					baseExponent = baseExponent * base + digit;
			} else {
				num->digit(digit);
				if (isFloat)
					fractionDigits++;
			}
			digitExpected = false;
		}
		pos++;
		if (outofbounds())
			break;
		c = contents[pos];
	}

	if (digitExpected) {
		if (outofbounds())
			Error::makeError(ErrorType::EndOfFileWhileReading, "the number definition", &errorToken);
		else
			makeLexError(ErrorType::Expected, "digit");
	}

	//we've now finished reading the number, turn it into the appropriate constant
	if (!isFloat)
		return new IntConstant(numDeleter.release(), begin, pos, sourceFile);
	else
		return new FloatConstant(
			numDeleter.release(), fractionDigits, negativeExponent ? -baseExponent : baseExponent, begin, pos, sourceFile);
/*
	//it's a float
	//first, adjust baseExponent
	baseExponent = (negativeExponent ? -baseExponent : baseExponent) - fractionDigits;

	//if we have no exponent, we're also done
	if (baseExponent == 0)
		return new FloatConstant(&num, num.highBit(), begin, pos, sourceFile);

	//it has an exponent- we want to divide or multiply num by base^abs(baseExponent)
	//we get this with the square-and-multiply trick
	//first, find the highest bit in the exponent
	int nextBaseExponentBit = 1;
	while (baseExponent >> 1 >= nextBaseExponentBit)
		nextBaseExponentBit <<= 1;

	//TODO: reduce precision to only the necessary bits and track how many bits were taken
	//next, square-and-multiply to get base^abs(baseExponent)
	BigInt exponentNum (base);
	//since baseExponent is not 0, the top bit is always set so we can stick two digits there
	exponentNum.digit(1);
	exponentNum.digit(0);
	//for any remaining bits, square before possibly multiplying
	for (nextBaseExponentBit >>= 1;
			nextBaseExponentBit > 0 && exponentNum.highBit() < FloatConstant::FLOAT_TOO_BIG_EXPONENT;
			nextBaseExponentBit >>= 1) {
		exponentNum.square();
		if ((nextBaseExponentBit & baseExponent) != 0)
			exponentNum.digit(0);
	}

	//we now have our big exponent number
	//if it's positive, just multiply it and that's our number
	if (baseExponent > 0) {
		num.multiply(&exponentNum);
		return new FloatConstant(&num, num.highBit(), begin, pos, sourceFile);
	//if it's negative, grow num before dividing
	} else {
		//provide at least 64 bits of precision
		int startingExponent = num.highBit();
		int toShift = exponentNum.highBit() - num.highBit() + 64;
		if (toShift > 0)
			num.lShift(toShift);
		int oldHighBit = num.highBit();
		num.longDiv(&exponentNum);
		return new FloatConstant(&num, startingExponent + num.highBit() - oldHighBit, begin, pos, sourceFile);
	}
	*/
}
//convert c from a character to the digit it represents
char Lex::cToDigit() {
	return
		(c >= '0' && c <= '9') ? c - '0' :
		(c >= 'a' && c <= 'z') ? c - 'a' + 10 :
		(c >= 'A' && c <= 'Z') ? c - 'A' + 10 :
		-1;
}
//get a string literal
//lex location: no change | the first character after the string
//may throw
StringLiteral* Lex::lexString() {
	if (c != '"')
		return nullptr;

	EmptyToken errorToken (pos, sourceFile);
	int begin = pos;
	string val;
	while (true) {
		pos++;
		if (outofbounds())
			Error::makeError(ErrorType::EndOfFileWhileReading, "the contents of the string", &errorToken);

		c = contents[pos];
		if (c == '"') {
			pos++;
			return new StringLiteral(val, begin, pos, sourceFile);
		}
		val += nextStringCharacter();
		if (c == '\n' || c == '\r') {
			char c2;
			goToOtherNewlineCharacterIfPresentAndStartRowWithCharAndHandling(c2, val += c2);
		}
	}
}
//returns the next character for the string, possibly from an escape sequence
//lex location: the location of the character | the last character of the escape sequence
//may throw
char Lex::nextStringCharacter() {
	if (c != '\\')
		return c;

	EmptyToken errorToken (pos, sourceFile);
	pos++;
	if (outofbounds())
		Error::makeError(ErrorType::EndOfFileWhileReading, "the escape sequence", &errorToken);
	c = contents[pos];
	switch (c) {
		case 'n': return '\n';
		case 'r': return '\r';
		case 't': return '\t';
		case 'b': return '\b';
		case '0': return '\0';
		//2-digit hex escape sequence
		case 'x': {
			if (pos + 2 >= contentsLength)
				Error::makeError(ErrorType::EndOfFileWhileReading, "the hex digit escape sequence", &errorToken);
			char c2 = 0;
			for (int max = pos + 2; pos < max;) {
				pos++;
				c = contents[pos];
				char digit = cToDigit();
				if (((unsigned char)digit) >= 16)
					makeLexError(ErrorType::General, "escape sequence requires 2 hex digits");
				c2 = c2 << 4 | digit;
			}
			return c2;
		}
		//these escape sequences just needed the backslash to be properly interpreted
		case '\\':
		case '\"':
		case '\'':
			return c;
		//anything else is an invalid escape sequence
		default:
			makeLexError(ErrorType::General, "invalid escape sequence");
			return 0;
	}
}
//get a character
//lex location: no change | the first character after the character
//may throw
IntConstant* Lex::lexCharacter() {
	if (c != '\'')
		return nullptr;

	EmptyToken errorToken (pos, sourceFile);
	int begin = pos;
	pos++;
	if (outofbounds())
		Error::makeError(ErrorType::EndOfFileWhileReading, "the character definition", &errorToken);

	c = contents[pos];
	c = nextStringCharacter();
	pos++;
	if (outofbounds())
		Error::makeError(ErrorType::EndOfFileWhileReading, "the character definition", &errorToken);
	if (contents[pos] != '\'')
		makeLexError(ErrorType::Expected, "a close quote");
	pos++;
	return new IntConstant(BigInt::createFrom((int)c), begin, pos, sourceFile);
}
//get a separator
//lex location: no change | the first character after the separator
Separator* Lex::lexSeparator() {
	Separator* val;
	switch (c) {
		case '(': val = new Separator(SeparatorType::LeftParenthesis, pos, pos + 1, sourceFile); break;
		case ')': val = new Separator(SeparatorType::RightParenthesis, pos, pos + 1, sourceFile); break;
		case ',': val = new Separator(SeparatorType::Comma, pos, pos + 1, sourceFile); break;
		case ';': val = new Separator(SeparatorType::Semicolon, pos, pos + 1, sourceFile); break;
		default: return nullptr;
	}

	pos++;
	return val;
}
//get an operator
//lex location: no change | the first character after the operator
Operator* Lex::lexOperator() {
	int begin = pos;
	OperatorType type = OperatorType::None;
	OperatorTypeTrie* tries = baseOperatorTries;
	int count = sizeof(baseOperatorTries) / sizeof(baseOperatorTries[0]);
	bool found = false;
	for (int i = 0; i < count; i++) {
		if (tries[i].operatorChar == c) {
			found = true;
			type = tries[i].type;
			pos++;
			if (outofbounds())
				break;
			c = contents[pos];
			count = tries[i].count;
			tries = tries[i].tries;
			i = -1;
		}
	}
	return found ? new Operator(type, begin, pos, sourceFile) : nullptr;
}
//get a directive title
//returns a token even if the directive title is invalid- ParseDirective will take care of it
//lex location: no change | the first character after the directive title
//may throw
DirectiveTitle* Lex::lexDirectiveTitle() {
	if (c != '#')
		return nullptr;

	EmptyToken errorToken (pos, sourceFile);
	pos++;
	int begin = pos;
	for (; inbounds(); pos++) {
		c = contents[pos];
		if (!isalpha(c) && c != '-')
			break;
	}

	if (outofbounds())
		Error::makeError(ErrorType::EndOfFileWhileReading, "the directive name", &errorToken);
	else if (pos == begin)
		makeLexError(ErrorType::Expected, "a directive name");

	return new DirectiveTitle(string(contents + begin, pos - begin), begin - 1, pos, sourceFile);
}
//throw an error at the current position
void Lex::makeLexError(ErrorType type, char* message) {
	EmptyToken errorToken (pos, sourceFile);
	pos++;
	Error::makeError(type, message, &errorToken);
}
/*
//check if the next statement is a Main. function
//pos location: the character after the close parenthesis of the Main. function
MainFunction* nextMainFunction() {
	ArrayIterator<MainFunction*> aio(&mainfunctions);
	for (MainFunction* m = aio.getFirst(); aio.hasThis(); m = aio.getNext()) {
		//check if the text matches a main function name
		size_t length = m->sval.length();
		if (strncmp(contents + pos, m->sval.c_str(), length) == 0 && pos + length < clength) {
			//check that the next character is a parenthesis or whitespace
			char c = contents[pos + length];
			if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '(' || c == ',') {
				pos += length;
				if (!m->added) {
					functions.add(m->fval);
					m->added = true;
				}
				return new MainFunction(m->fval, m->sval, m->contentpos);
			}
		}
	}
	return NULL;
}
//get the name of a variable
//pos location: clength | character after the variable name
string variableName() {
	size_t begin = pos;
	pos += 1;
	//find the rest of the variable name characters
	while (inbounds()) {
		char c = contents[pos];
		if (!isalnum(c) && c != '@' && c != '$' && c != '_' && c != '`'
&& c != '.'
)
			break;
		pos += 1;
	}
	return string(contents + begin, pos - begin);
}
//convert a string to a number
//return the IntConstant or FloatConstant that it represents- only makes an object if successful
//pos location: clength | the character after the number
Expression* tonum(int base) {
	size_t oldpos = pos;
	BigInt b (base);
	char d = ' ';
	//first stage: all numbers before the period
	for (; inbounds(); pos += 1) {
		if ((d = contents[pos]) == '_')
			continue;
		if (d >= '0' && d <= '9')
			addDigit(&b, d - '0');
		else if (d >= 'a' && d <= 'z')
			addDigit(&b, d - 'a' + 10);
		else if (d >= 'A' && d <= 'Z')
			addDigit(&b, d - 'A' + 10);
		else
			break;
	}
	if (outofbounds())
		makeError(1, "the number definition", oldpos);
	//not a period- the number is not a float
	if (d != '.') {
		IntConstant* i = new IntConstant(b.getInt(), oldpos);
		//assign a number a specific size
		if (foundAndSkipped("#b"))
			castConstant(i, TBYTE);
		return i;
	}
	//second stage: all numbers after the period
	int power = 0;
	pos += 1;
	//add digits normally, decrement power for every digit
	for (; inbounds(); pos += 1) {
		if ((d = contents[pos]) == '_')
			continue;
		if (d >= '0' && d <= '9') {
			addDigit(&b, d - '0');
			power -= 1;
		} else if (d >= 'a' && d <= 'z') {
			if (d == 'e' && base <= 14)
				break;
			addDigit(&b, d - 'a' + 10);
			power -= 1;
		} else if (d >= 'A' && d <= 'Z') {
			addDigit(&b, d - 'A' + 10);
			power -= 1;
		} else
			break;
	}
	if (outofbounds())
		makeError(1, "the number definition", oldpos);
	//stop if the BigInt is zero
	if (b.bitCount() == 0)
		return new FloatConstant(oldpos);
	int exp = b.bitCount() - 1;
	//third state: exponent
	if (d == '^' || d == 'e') {
		bool negexp = false;
		pos += 1;
		BigInt p (base);
		while (inbounds() && (d = contents[pos]) == '_')
			pos += 1;
		if (outofbounds())
			makeError(1, "the number definition", oldpos);
		if (d == '-') {
			negexp = true;
			pos += 1;
		}
		while (inbounds() && (d = contents[pos]) == '_')
			pos += 1;
		if (outofbounds())
			makeError(1, "the number definition", oldpos);
		//search for valid exponent characters
		for (; inbounds(); pos += 1) {
			if ((d = contents[pos]) == '_')
				continue;
			if (d >= '0' && d <= '9')
				addDigit(&p, d - '0');
			else if (d >= 'a' && d <= 'z')
				addDigit(&p, d - 'a' + 10);
			else if (d >= 'A' && d <= 'Z')
				addDigit(&p, d - 'A' + 10);
			else
				break;
		}
		//get the power, only use 16 bits
		if (negexp)
			power -= p.getInt() & 0xFFFF;
		else
			power += p.getInt() & 0xFFFF;
	}
	//multiply the BigInt by base to the power
	if (power > 0) {
		exp -= b.bitCount();
		for (int i = 0; i < power; i += 1)
			b.digit(0);
		exp += b.bitCount();
	} else if (power < 0) {
		int abc = b.getInt();
		BigInt p (base);
		p.digit(1);
		for (int i = 0; i > power; i -= 1)
			p.digit(0);
		//ensure b has at least 64 bits plus extra bits for p
		int shift = 64 - b.bitCount();
		b.lShift(shift > 0 ? shift + p.bitCount() : p.bitCount());
		exp -= b.bitCount();
		b.longDiv(&p);
		exp += b.bitCount();
	}
	return new FloatConstant(exp, &b, oldpos);
}
//check if contents compares with the input, skip it if so
//pos location: clength | determined by input
bool foundAndSkipped(string s) {
	int length = s.length();
	if (strncmp(contents, s.c_str(), length) == 0) {
		pos += length;
		return true;
	}
	return false;
}
//add a digit to the base
//make an error if it's not a character of the base
void addDigit(BigInt* b, char d) {
	if (d > b->base)
		makeError(0, "character is not a valid digit for the number", pos);
	b->digit(d);
}
//get a string from the input
//pos location: clength | the character afther the closing double quote
string stringVal() {
	string outputstring = "";
	char c;
	size_t loc = pos;
	while (inbounds()) {
		c = contents[pos];
		//escape character
		if (c == '\\')
			outputstring += escapeSequenceCharacter();
		//end of string
		else if (c == '\"') {
			pos += 1;
			return outputstring;
		//regular character
		} else {
			outputstring += c;
			pos += 1;
		}
	}
	//string never ended
	makeError(1, "the contents of the string", loc);
	//only here to satisfy return requirements
	return outputstring;
}
//get an escaped character
//pos location: the character after the escape sequence
char escapeSequenceCharacter() {
	pos += 2;
	if (outofbounds())
		makeError(1, "the escape sequence", pos - 2);
	char c = contents[pos - 1];
	if (c == 'n')
		return '\n';
	else if (c == 'r')
		return '\r';
	else if (c == '\\')
		return '\\';
	else if (c == '\'')
		return '\'';
	else if (c == '\"')
		return '\"';
	else if (c == 't')
		return '\t';
	else if (c == 'b')
		return '\b';
	else if (c == '0')
		return '\0';
	//2-digit hex escape sequence
	else if (c == 'x') {
		if (pos + 2 >= clength)
			makeError(1, "the escape sequence", pos - 2);
		if (isxdigit(contents[pos]) &&
			isxdigit(contents[pos + 1])) {
			pos += 2;
			return (char)(toint(16, pos - 2, pos));
		} else
			makeError(0, "escape sequence requires 2 hexadecimal digits", pos);
	}
	//invalid escape sequence
	makeError(0, "invalid escape sequence", pos);
	//only here to satisfy return requirements
	return NULL;
}
//convert string to int
int toint(int base, size_t loc, size_t end) {
	char d;
	BigInt b (base);
	for (; loc < end; loc += 1) {
		d = contents[loc];
		if (d >= '0' && d <= '9')
			d -= '0';
		else if (d >= 'a' && d <= 'y')
			d += 10 - 'a';
		else if (d >= 'A' && d <= 'Y')
			d += 10 - 'A';
		if (d < base)
			b.digit(d);
		else
			makeError(0, "character is not a valid digit for the number", loc);
	}
	return b.getInt();
}
//cast a constant to the given context
void castConstant(IntConstant* c, int context) {
	if (context == TVOID || c->context == context)
		return;
	//cast to byte
	else if (context == TBYTE) {
		//int constant shrinking
		if (c->context == TINT) {
			if (c->ival < -128 || c->ival > 127) {
				makeWarning(0, "int constant being resized to byte constant", c->contentpos);
				c->ival = (int)((char)(c->ival));
			}
			c->context = TBYTE;
		} else
			makeError(0, "value is not a byte value", c->contentpos);
	//cast to int
	} else if (context == TINT) {
		if (c->context == TBYTE)
			c->context = TINT;
		else
			makeError(0, "value is not an int value", c->contentpos);
	//cast to something else
	} else
		makeError(0, "value has wrong expression type", c->contentpos);
}
*/
