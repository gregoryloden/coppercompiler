#include "Project.h"

//build the final executable file
//TODO: finalize byte sizes for primitive types
//TODO: circular definitions
//TODO: unitialized variables (all uninitialized variable errors will be handled here)

//variable data: global variable data stored in tree
//each scope has a PrefixTrieUnion with the data
//when adding variable data:
//use the PrefixTrie check to see if it's got one already
//if not: make a new one, and if the parent has one (getFromParent), port the data from that one to the new one

Build::FindUninitializedVariablesVisitor::FindUninitializedVariablesVisitor(
	PrefixTrie<char, CVariableData*>* pVariableData, bool pErrorForUninitializedVariables)
: TokenVisitor(onlyWhenTrackingIDs("FUVVTR"))
, allVariablesAreInitialized(true)
, errorForUninitializedVariables(pErrorForUninitializedVariables)
, variableData(pVariableData) {
}
Build::FindUninitializedVariablesVisitor::~FindUninitializedVariablesVisitor() {}
//go through the expression and make sure all of the variables it uses are initialized
void Build::FindUninitializedVariablesVisitor::handleExpression(Token* t) {
	Operator* o;
	Identifier* i;
	if (let(Operator*, o, t)) {
		if (o->operatorType == OperatorType::Assign) {
			handleExpression(o->right);
			if (!allVariablesAreInitialized)
				return;
			VariableDeclarationList* v;
			if (let(VariableDeclarationList*, v, o->left)) {
				forEach(CVariableDefinition*, c, v->variables, ci) {
					CVariableData::addToVariableData(variableData, c, CVariableData::isInitialized);
				}
			} else if (let(Identifier*, i, o->left))
				CVariableData::addToVariableData(variableData, i->variable, CVariableData::isInitialized);
			else
				Error::logError(ErrorType::CompilerIssue, "resulting in an assignment with a non-variable left side", o);
		} else if (o->operatorType == OperatorType::QuestionMark) {
			handleExpression(o->left);
			Operator* ternaryResult;
			if (!let(Operator*, ternaryResult, o->right)) {
				Error::logError(ErrorType::CompilerIssue, "resulting in a question mark without a corresponding colon", o);
				return;
			}
			//save any variable data in separate tries
			PrefixTrie<char, CVariableData*>* oldVariableData = variableData;
			PrefixTrieUnion<char, CVariableData*> leftVariableData (oldVariableData);
			variableData = &leftVariableData;
			handleExpression(ternaryResult->left);
			PrefixTrieUnion<char, CVariableData*> rightVariableData (oldVariableData);
			variableData = &rightVariableData;
			handleExpression(ternaryResult->right);
			variableData = oldVariableData;
			//any variable data that appears in both tries will go in the original trie
			Array<CVariableData*>* leftVariableDataList = leftVariableData.getValues();
			forEach(CVariableData*, c, leftVariableDataList, ci) {
				string& name = c->variable->name->name;
				CVariableData* other = rightVariableData.get(name.c_str(), name.length());
				if (other == nullptr) {
					delete c;
					continue;
				}
				//TODO: variable overloading
				if (c->variable != other->variable) {
					Error::logError(ErrorType::CompilerIssue, "resulting in mismatched variables", other->variable->name);
					delete c;
					continue;
				}
				unsigned short combinedBitmask = c->dataBitmask & other->dataBitmask;
				if (combinedBitmask != 0)
					CVariableData::addToVariableData(variableData, c->variable, combinedBitmask);
				delete c;
			}
			delete leftVariableDataList;
			rightVariableData.deleteValues();
		} else if (o->operatorType == OperatorType::BooleanAnd || o->operatorType == OperatorType::BooleanOr) {
			handleExpression(o->left);
			PrefixTrie<char, CVariableData*>* oldVariableData = variableData;
			PrefixTrieUnion<char, CVariableData*> rightVariableData (oldVariableData);
			variableData = &rightVariableData;
			handleExpression(o->right);
			variableData = oldVariableData;
			rightVariableData.deleteValues();
		} else
			o->visitSubtokens(this);
	} else if (let(Identifier*, i, t)) {
		if (!CVariableData::variableDataContains(variableData, i->name, CVariableData::isInitialized)) {
			allVariablesAreInitialized = false;
			if (errorForUninitializedVariables) {
				string errorMessage = "\"" + i->name + "\" may not have been initialized";
				Error::logError(ErrorType::General, errorMessage.c_str(), i);
			}
		}
	} else
		t->visitSubtokens(this);
}
//build the final executable file
void Build::build(Pliers* pliers) {
	if (pliers->printProgress)
		puts("Building executable...");

	//first: find out the order that global variables are initialized
	//gather all the variable initializations, initialize them if possible and save them for later if not
	PrefixTrie<char, CVariableData*> globalVariableData;
	Array<Operator*> initializationOrder;
	Array<Operator*> initializationsNotReady;
	forEach(SourceFile*, s, pliers->allFiles, si) {
		forEach(Token*, t, s->globalVariables, ti) {
			Operator* o;
			if (!let(Operator*, o, t))
				continue;
			FindUninitializedVariablesVisitor visitor (&globalVariableData, false);
			visitor.handleExpression(o);
			(visitor.allVariablesAreInitialized ? &initializationOrder : &initializationsNotReady)->add(o);
		}
	}

	//next go through the initializations not ready and see if they're ready
	bool foundInitializedVariable = true;
	while (foundInitializedVariable) {
		foundInitializedVariable = false;
		for (int i = initializationsNotReady.length - 1; i >= 0; i--) {
			Operator* o = initializationsNotReady.get(i);
			FindUninitializedVariablesVisitor visitor (&globalVariableData, false);
			visitor.handleExpression(o);
			//if the variables are initialized now, add them to the ordered list
			if (visitor.allVariablesAreInitialized) {
				initializationsNotReady.remove(i);
				initializationOrder.add(o);
				foundInitializedVariable = true;
			}
		}
	}

	//error if there are any uninitialized global variables
	forEach(Operator*, o, &initializationsNotReady, oi) {
		FindUninitializedVariablesVisitor(&globalVariableData, true).handleExpression(o);
	}

	globalVariableData.deleteValues();

	//TODO: build
}

/*
	if (!CVariableData::variableDataContains(variableData, i->name, CVariableData::isInitialized)) {
		string errorMessage = "\"" + i->name + "\" may not have been initialized";
		Error::logError(ErrorType::General, errorMessage.c_str(), i);
	}
*/
