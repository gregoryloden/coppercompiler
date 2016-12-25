class MainFunction;
class Expression;
class IntConstant;
template <class type> class Array;

void buildTokens();
void skipWhitespace();
MainFunction* nextMainFunction();
string variableName();
Expression* tonum(int base);
bool foundAndSkipped(string s);
void addDigit(BigInt* b, char d);
string stringVal();
char escapeSequenceCharacter();
int toint(int base, size_t loc, size_t end);
void castConstant(IntConstant* c, int context);
