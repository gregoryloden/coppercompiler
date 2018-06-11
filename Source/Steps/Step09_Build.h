#include "../Structure/Tokens.h"

class Pliers;
class CVariableData;
template <class KeyElement, class Value> class PrefixTrie;

class Build {
private:
	class FindUninitializedVariablesVisitor: public TokenVisitor {
	public:
		bool allVariablesAreInitialized; //copper: readonly
	private:
		PrefixTrie<char, CVariableData*>* variableData;
		bool errorForUninitializedVariables;

	public:
		FindUninitializedVariablesVisitor(
			PrefixTrie<char, CVariableData*>* pVariableData, bool pErrorForUninitializedVariables);
		virtual ~FindUninitializedVariablesVisitor();

		void handleExpression(Token* t);
	};

public:
	static void build(Pliers* pliers);
};
