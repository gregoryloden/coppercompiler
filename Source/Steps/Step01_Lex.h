class Token;
class IntConstant;

Token* lex();
void buildTokens();
void castConstant(IntConstant* c, int context);
