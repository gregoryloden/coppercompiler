#include "general.h"
#include "Step01_Lex.h"
#include "Representation.h"

bool skipWhitespace();
bool skipComment();
Token* lexIdentifier();
Token* lexNumber();
char cToDigit();
StringLiteral* lexString();
char nextStringCharacter();
IntConstant2* lexCharacter();
Separator2* lexSeparator();
Operator* lexOperator();

MainFunction* nextMainFunction();
string variableName();
Expression* tonum(int base);
bool foundAndSkipped(string s);
void addDigit(BigInt* b, char d);
string stringVal();
char escapeSequenceCharacter();
int toint(int base, size_t loc, size_t end);

char c;

struct OperatorTypeTrie {
	char operatorChar;
	OperatorType type;
	int count;
	OperatorTypeTrie* tries;
};
const int baseOperatorTrieCount = 16;
OperatorTypeTrie baseOperatorTries[] = {
	{'.', Dot, 0, nullptr},
	{'+', Add, 2, new OperatorTypeTrie[] {
		{'+', Increment, 0, nullptr},
		{'=', AssignAdd, 0, nullptr}
	}},
	{'-', Subtract, 2, new OperatorTypeTrie[] {
		{'-', Decrement, 0, nullptr},
		{'=', AssignSubtract, 0, nullptr}
	}},
	{'~', BitwiseNot, 3, new OperatorTypeTrie[] {
		{'!', VariableLogicalNot, 0, nullptr},
		{'~', VariableBitwiseNot, 0, nullptr},
		{'-', VariableNegate, 0, nullptr}
	}},
	{'!', LogicalNot, 1, new OperatorTypeTrie[] {
		{'=', NotEqual, 0, nullptr}
	}},
	{'*', Multiply, 1, new OperatorTypeTrie[] {
		{'=', AssignMultiply, 0, nullptr}
	}},
	{'/', Divide, 1, new OperatorTypeTrie[] {
		{'=', AssignDivide, 0, nullptr}
	}},
	{'%', Modulus, 1, new OperatorTypeTrie[] {
		{'=', AssignModulus, 0, nullptr}
	}},
	{'<', LessThan, 2, new OperatorTypeTrie[] {
		{'<', ShiftLeft, 1, new OperatorTypeTrie[] {
			{'=', AssignShiftLeft, 0, nullptr}
		}},
		{'=', LessOrEqual, 0, nullptr}
	}},
	{'>', GreaterThan, 2, new OperatorTypeTrie[] {
		{'>', ShiftRight, 2, new OperatorTypeTrie[] {
			{'>', ShiftArithmeticRight, 1, new OperatorTypeTrie[] {
				{'=', AssignShiftArithmeticRight, 0, nullptr}
			}},
			{'=', AssignShiftRight, 0, nullptr}
		}},
		{'=', GreaterOrEqual, 0, nullptr}
	}},
	{'&', BitwiseAnd, 2, new OperatorTypeTrie[] {
		{'&', BooleanAnd, 0, nullptr},
		{'=', AssignBitwiseAnd, 0, nullptr}
	}},
	{'^', BitwiseXor, 1, new OperatorTypeTrie[] {
		{'=', AssignBitwiseXor, 0, nullptr}
	}},
	{'|', BitwiseOr, 1, new OperatorTypeTrie[] {
		{'|', BooleanOr, 0, nullptr},
		{'=', AssignBitwiseOr, 0, nullptr}
	}},
	{'=', Assign, 1, new OperatorTypeTrie[] {
		{'=', Equal, 0, nullptr}
	}},
	{'?', QuestionMark, 0, nullptr},
	{':', Colon, 0, nullptr}
};

//retrieve the next token from contents
//pos location: the first character after the next token | clength
Token* lex() {
	do {
		if (!skipWhitespace())
			return nullptr;
	} while (skipComment());
	Token* t;
	if ((t = lexIdentifier()) != nullptr ||
		(t = lexNumber()) != nullptr ||
		(t = lexString()) != nullptr ||
		(t = lexCharacter()) != nullptr ||
		(t = lexSeparator()) != nullptr ||
		(t = lexOperator()))
		return t;
	makeError(0, "unexpected character", pos);
	return nullptr;
}
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
//skip all whitespace
//returns whether there are more characters left in contents
//pos location: the next non-whitespace character | clength
bool skipWhitespace() {
	for (; inbounds(); pos++) {
		c = contents[pos];
		if (c != ' ' && c != '\t' && c != '\n' && c != '\r')
			return true;
	}
	return false;
}
//skip the next comment if there is one
//returns whether a comment was skipped
//pos location: no change | the first character after the comment
bool skipComment() {
	char c2;
	if (c != '/' || pos + 1 >= clength || ((c2 = contents[pos + 1]) != '/' && c2 != '*'))
		return false;

	bool lineComment = c2 == '/';
	size_t oldPos = pos;
	pos += 2;
	for (; inbounds(); pos++) {
		c = contents[pos];
		if (lineComment ? c == '\n' : (c == '*' && pos + 1 < clength && contents[pos + 1] == '/')) {
			pos += lineComment ? 1 : 2;
			return true;
		}
	}
	//reached the end of the file before the comment terminator, that's ok for a line comment but not a stream comment
	if (!lineComment)
		makeError(1, "the end of the stream comment", oldPos);
	return true;
}
//get a variable name, type, or keyword
//pos location: no change | the first character after the identifier
Token* lexIdentifier() {
	if (!isalpha(c))
		return nullptr;

	size_t begin = pos;
	pos++;
	//find the rest of the identifier characters
	while (inbounds()) {
		c = contents[pos];
		if (!isalnum(c) && c != '_')
			break;
		pos++;
	}
	string s (contents + begin, pos - begin);

	if (s.compare("true") == 0)
		return new IntConstant2(1, begin);
	else if (s.compare("false") == 0)
		return new IntConstant2(0, begin);
	else
		return new Identifier(s, begin);
}
//get a number constant, either int or float
//pos location: no change | the first character after the number
Token* lexNumber() {
	if (!isdigit(c))
		return nullptr;

	size_t begin = pos;
	int base = 10;
	//different base
	char c2;
	if (c == '0' && pos + 1 < clength && !isdigit(c2 = contents[pos + 1])) {
		if (c2 == 'x') {
			base = 16;
			pos += 2;
		} else if (c2 == 'o') {
			base = 8;
			pos += 2;
		} else if (c2 == 'b') {
			base = 2;
			pos += 2;
		}
		if (outofbounds())
			makeError(1, "the number definition", begin);
		c = contents[pos];
	}

	BigInt2 num (base);
	bool lexingExponent = false;
	bool isFloat = false;
	bool digitExpected = false;
	int fractionDigits = 0;
	int baseExponent = 0;
	bool negativeExponent = false;
	//lex number characters
	while (true) {
		if (c == '_')
			;
		else if (c == '.') {
			if (!isFloat) {
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
			if (digit == -1 || digit > base)
				break;
			if (lexingExponent) {
				if (baseExponent < FloatConstant2::FLOAT_TOO_BIG_EXPONENT)
					baseExponent = baseExponent * base + digit;
			} else {
				num.digit(digit);
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

	if (digitExpected)
		makeError(0, "expected digit", pos);

	//we've now finished reading the number
	//turn it into the appropriate constant
	//if it's not a float, we're done
	if (!isFloat)
		return new IntConstant2(num.getInt(), begin);

	//it's a float
	//first, adjust baseExponent
	baseExponent = (negativeExponent ? -baseExponent : baseExponent) - fractionDigits;

	//if we have no exponent, we're also done
	if (baseExponent == 0)
		return new FloatConstant2(&num, num.highBit(), begin);

	//it has an exponent- we want to divide or multiply num by base^abs(baseExponent)
	//we get this with the square-and-multiply trick
	//first, find the highest bit in the exponent
	int nextBaseExponentBit = 1;
	while (baseExponent >> 1 >= nextBaseExponentBit)
		nextBaseExponentBit <<= 1;

	//next, square-and-multiply to get base^abs(baseExponent)
	BigInt2 exponentNum (base);
	//since baseExponent is not 0, the top bit is always set so we can stick two digits there
	exponentNum.digit(1);
	exponentNum.digit(0);
	//for any remaining bits, square before possibly multiplying
	for (nextBaseExponentBit >>= 1;
			nextBaseExponentBit > 0 && exponentNum.highBit() < FloatConstant2::FLOAT_TOO_BIG_EXPONENT;
			nextBaseExponentBit >>= 1) {
		exponentNum.square();
		if ((nextBaseExponentBit & baseExponent) != 0)
			exponentNum.digit(0);
	}

	//we now have our big exponent number
	//if it's positive, just multiply it and that's our number
	if (baseExponent > 0) {
		num.multiply(&exponentNum);
		return new FloatConstant2(&num, num.highBit(), begin);
	//if it's negative, grow num before dividing
	} else {
		//provide at least 64 bits of precision
		int startingExponent = num.highBit();
		int toShift = exponentNum.highBit() - num.highBit() + 64;
		if (toShift > 0)
			num.lShift(toShift);
		int oldHighBit = num.highBit();
		num.longDiv(&exponentNum);
		return new FloatConstant2(&num, startingExponent + num.highBit() - oldHighBit, begin);
	}
}
//convert c from a character to the digit it represents
char cToDigit() {
	if (c >= '0' && c <= '9')
		return c - '0';
	else if (c >= 'a' && c <= 'z')
		return c - 'a' + 10;
	else if (c >= 'A' && c <= 'Z')
		return c - 'A' + 10;
	else
		return -1;
}
//get a string literal
//pos location: no change | the first character after the string
StringLiteral* lexString() {
	if (c != '"')
		return nullptr;

	size_t oldPos = pos;
	string val;
	while (true) {
		pos++;
		if (outofbounds())
			makeError(1, "the contents of the string", oldPos);

		c = contents[pos];
		if (c == '"') {
			pos++;
			return new StringLiteral(val, oldPos);
		}
		val += nextStringCharacter();
	}
}
//returns the next character for the string, possibly from an escape sequence
//pos location: the location of the character | the last character of the escape sequence
char nextStringCharacter() {
	if (c != '\\')
		return c;

	pos++;
	if (outofbounds())
		makeError(1, "the escape sequence", pos - 1);
	c = contents[pos];
	if (c == 'n')
		return '\n';
	else if (c == 'r')
		return '\r';
	else if (c == 't')
		return '\t';
	else if (c == 'b')
		return '\b';
	else if (c == '0')
		return '\0';
	//2-digit hex escape sequence
	else if (c == 'x') {
		if (pos + 2 >= clength)
			makeError(1, "the escape sequence", pos);
		char c2 = 0;
		for (size_t max = pos + 2; pos < max;) {
			pos++;
			c = contents[pos];
			char digit = cToDigit();
			if (((unsigned char)digit) >= 16)
				makeError(0, "escape sequence requires 2 hex digits", pos);
			c2 = c2 << 4 | digit;
		}
		return c2;
	//these escape sequences just needed the backslash to be properly interpreted
	//anything else is an invalid escape sequence
	} else if (c != '\\' && c != '\"' && c != '\'')
		makeError(0, "invalid escape sequence", pos);
	//it's one of the above 3 escape sequences
	return c;
}
//get a character
//pos location: no change | the first character after the character
IntConstant2* lexCharacter() {
	if (c != '\'')
		return nullptr;

	size_t oldPos = pos;
	pos++;
	if (outofbounds())
		makeError(1, "the character definition", oldPos);

	c = contents[pos];
	c = nextStringCharacter();
	pos++;
	if (outofbounds())
		makeError(1, "the character definition", oldPos);
	if (contents[pos] != '\'')
		makeError(1, "expected a close quote", pos);
	pos++;
	return new IntConstant2((int)c, oldPos);
}
//get a separator
//pos location: no change | the first character after the separator
Separator2* lexSeparator() {
	Separator2* val;
	if (c == '(')
		val = new Separator2(LeftParenthesis, pos);
	else if (c == ')')
		val = new Separator2(RightParenthesis, pos);
	else if (c == ',')
		val = new Separator2(Comma, pos);
	else if (c == ';')
		val = new Separator2(Semicolon, pos);
	else
		return nullptr;

	pos++;
	return val;
}
//get an operator
//pos location: no change | the first character after the operator
Operator* lexOperator() {
	size_t oldPos = pos;
	OperatorType type = Dot;
	OperatorTypeTrie* tries = baseOperatorTries;
	int count = baseOperatorTrieCount;
	bool found = false;
	for (int i = 0; i < count; i++) {
		if (tries[i].operatorChar == c) {
			found = true;
			type = tries[i].type;
			pos++;
			if (outofbounds())
				break;
			c = contents[pos];
			tries = tries[i].tries;
			count = tries[i].count;
			i = -1;
		}
	}
	return found ? new Operator(type, oldPos) : nullptr;
}
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
