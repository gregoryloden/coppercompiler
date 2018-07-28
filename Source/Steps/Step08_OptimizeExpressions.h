class Pliers;
class Operator;
class Token;
class Statement;
class Group;
template <class Type> class Array;

class OptimizeExpressions {
public:
	static void optimizeExpressions(Pliers* pliers);
private:
	static Token* optimizeExpression(Token* t);
	static Token* optimizeOperator(Operator* o);
	static void optimizeFunctionCall(FunctionCall* f);
	static void optimizeStatementList(Array<Statement*>* statements);
	static void optimizeGroup(Group* g);
};
