#include "Project.h"
#ifdef WIN32
	#include <Windows.h>
#endif

#define returnIfErrors() if (errorMessages->length > 0) return;
/*
//check if tpos is within the tokens
#define tinbounds() (tpos < tlength)
#define toutofbounds() (tpos >= tlength)

Array<AssemblyInstruction*> code;
string text;
string rdata;
string IAT;
string output;
Array<Thunk*> thunks;
Array<VariableData*> globals;
Function* fmain = NULL;
string types[] = {"void", "boolean", "byte", "int", "String", "Function", ""};
int contexts[] = {TVOID , TBOOLEAN , TBYTE , TINT , TSTRING , TFUNCTION , TNONE};
int tpos = 0;
Expression** initializations = NULL;
Array<AssemblySequence*> sequences;

template <class Key, class Value> void printNode(AVLNode<Key, Value>* node, int spaces) {
	if (node == nullptr)
		return;
	printNode(node->left, spaces + 1);
	for (int i = 0; i < spaces; i++)
		printf("  ");
	printf("%d -> %d\n", (int)(node->key), (int)(node->value));
	printNode(node->right, spaces + 1);
}
template <class Key, class Value> void validateHeights(AVLNode<Key, Value>* node) {
	if (node == nullptr)
		return;
	char leftHeight = AVLNode<Key, Value>::nodeHeight(node->left);
	char rightHeight = AVLNode<Key, Value>::nodeHeight(node->right);
	if (leftHeight < node->height - 2 || leftHeight >= node->height)
		puts("eror bad left height");
	if (rightHeight < node->height - 2 || rightHeight >= node->height)
		puts("eror bad left height");
	if (leftHeight != node->height - 1 && rightHeight != node->height - 1)
		puts("eror mismatched heights");
}
template <class Key, class Value> void setAndValidate(AVLTree<Key, Value>* tree, Key key, Value value) {
	tree->set(key, value);
	if (tree->get(key) != value)
		puts("eror bad value");
	validateHeights(tree->root);
	printNode(tree->root, 0);
	puts("");
}

template void validateHeights(AVLNode<int, int>* node);
template void setAndValidate(AVLTree<int, int>* tree, int key, int value);
template void printNode(AVLNode<int, int>* node, int spaces);
template void validateHeights(AVLNode<char, char>* node);
template void setAndValidate(AVLTree<char, char>* tree, char key, char value);
template void printNode(AVLNode<char, char>* node, int spaces);

int avltreetest = []() {
	AVLTree<int, int>* tree1 = new AVLTree<int, int>();
	setAndValidate(tree1, 1, 1);
	setAndValidate(tree1, 2, 1);
	setAndValidate(tree1, 3, 1);
	setAndValidate(tree1, 4, 1);
	setAndValidate(tree1, 5, 10);
	setAndValidate(tree1, 6, 1);
	setAndValidate(tree1, 7, 1);
	setAndValidate(tree1, 8, 1);
	setAndValidate(tree1, 9, 1);
	setAndValidate(tree1, 10, 15);
	setAndValidate(tree1, 11, 1);
	setAndValidate(tree1, 12, 1);
	setAndValidate(tree1, 13, 1);
	setAndValidate(tree1, 14, 1);
	setAndValidate(tree1, 15, 19);
	setAndValidate(tree1, 9, 19);
	AVLTree<char, char>* tree2 = new AVLTree<char, char>();
	setAndValidate(tree2, (char)15, (char)1);
	setAndValidate(tree2, (char)14, (char)1);
	setAndValidate(tree2, (char)13, (char)13);
	setAndValidate(tree2, (char)12, (char)1);
	setAndValidate(tree2, (char)11, (char)51);
	setAndValidate(tree2, (char)10, (char)1);
	setAndValidate(tree2, (char)9, (char)19);
	setAndValidate(tree2, (char)8, (char)1);
	setAndValidate(tree2, (char)7, (char)1);
	setAndValidate(tree2, (char)6, (char)1);
	setAndValidate(tree2, (char)5, (char)12);
	setAndValidate(tree2, (char)4, (char)1);
	setAndValidate(tree2, (char)3, (char)81);
	setAndValidate(tree2, (char)2, (char)1);
	setAndValidate(tree2, (char)1, (char)1);
	AVLTree<char, char>* tree3 = new AVLTree<char, char>();
	setAndValidate(tree3, (char)6, (char)1);
	setAndValidate(tree3, (char)7, (char)2);
	setAndValidate(tree3, (char)2, (char)3);
	setAndValidate(tree3, (char)1, (char)4);
	setAndValidate(tree3, (char)4, (char)5);
	setAndValidate(tree3, (char)5, (char)6);
	setAndValidate(tree3, (char)3, (char)7);
	printf("Heights: %d, %d, %d\n", (int)(tree1->root->height), (int)(tree2->root->height), (int)(tree3->root->height));
	return 0;
}();
*/
int main(int argc, char* argv[]) {
	puts("Pliers Copper Compiler v0.0");
	#ifdef WIN32
		HANDLE stdHandle = GetStdHandle(STD_INPUT_HANDLE);
		DWORD consoleMode = 0;
		if (GetConsoleMode(stdHandle, &consoleMode))
			SetConsoleMode(stdHandle, consoleMode & ~ENABLE_MOUSE_INPUT);
	#endif
	//Start compiling
	#ifdef DEBUG
		ObjCounter::start();
	#endif
	if (argc < 2) {
		puts("You need an input file");
		return -1;
	}
	Pliers* p = new Pliers(argv[1]);
	if (p->errorMessages->length > 0) {
		forEach(ErrorMessage*, errorMessage, p->errorMessages, ei) {
			errorMessage->printError();
		}
		printf("Quit with %d errors\n", p->errorMessages->length);
	}
//printAbstractCodeBlock(mainFile->abstractContents, 0);
	/*
	setRowsAndColumns();
	//prepare the error snippet
	snippet[SNIPPETCHARS] = '\n';
	memset(snippet + SNIPPETCHARS + 1, ' ', SNIPPETCHARS / 2);
	snippet[SNIPPETCHARS * 3 / 2 + 1] = '^';
	snippet[SNIPPETCHARS * 3 / 2 + 2] = 0;
	//build main functions
	buildMainFunctions(&mainfunctions);
	//start parsing
	parseCode();
	//build the output
	if (errors == 0) {
		buildAssembly();
		buildSections();
		buildExecutable();
		//search for the file name
		int period = 0;
		for (int i = 0; filename[i] != 0; i += 1) {
			if (filename[i] == '.')
				period = i;
		}
		//swap extensions, otherwise append the extension
		string name = period != 0 ? string(filename, period).append(".exe") : string(filename).append(".exe");
		//output the file
		puts("Writing file...\n");
		FILE* out = fopen(name.c_str(), "wb");
		if (out == NULL)
			puts("Unable to output file");
		else {
			fwrite(output.c_str(), 1, output.length(), out);
			fclose(out);
			puts("Compiled!");
		}
	}
	*/
	#ifdef DEBUG
		delete p;
		ObjCounter::end();
	#endif
while(true) {}
	return 0;
}
//upon initialization, run all the compilation steps
//if any of them fail, stop and leave this as it is
Pliers::Pliers(char* fileName)
: allFiles(nullptr)
, errorMessages(new Array<ErrorMessage*>()) {
	allFiles = Include::loadFiles(fileName, this);
	#ifdef DEBUG
		forEach(SourceFile*, s, allFiles, si1) {
			printf("Initial contents for \"%s\":\n", s->filename.c_str());
			Debug::printAbstractCodeBlock(s->abstractContents, 1);
		}
	#endif
	returnIfErrors();

	Replace::replaceCodeInFiles(allFiles);
	#ifdef DEBUG
		forEach(SourceFile*, s, allFiles, si2) {
			printf("Replaced contents for \"%s\":\n", s->filename.c_str());
			Debug::printAbstractCodeBlock(s->abstractContents, 1);
		}
	#endif
	returnIfErrors();

	ParseTypes::parseTypes(allFiles);
	returnIfErrors();

	ParseExpressions::parseExpressionsInFiles(allFiles);
	returnIfErrors();
	#ifdef DEBUG
		forEach(SourceFile*, s, allFiles, si3) {
			printf("Global definitions for \"%s\":\n", s->filename.c_str());
			forEach(VariableInitialization*, initialization, s->globalVariables, di) {
				Debug::printTokenTree(initialization, 0, false);
				printf(";\n");
			}
		}
	#endif
puts("Suspended until the rewrite is complete");
}
Pliers::~Pliers() {
	allFiles->deleteContents();
	delete allFiles;
	errorMessages->deleteContents();
	delete errorMessages;
}
/*
//get the contents of a file as a char*
char* getFile(char* filename) {
	printf("Reading %s...\n\n", filename);
	//get the file and file length
	FILE* file = fopen(filename, "rb");
	if (file == NULL)
		return NULL;
	fseek(file, 0, SEEK_END);
	clength = ftell(file);
	//put the file in a char*
	char* inputbytes = new char[clength + 1];
	rewind(file);
	fread(inputbytes, 1, clength, file);
	inputbytes[clength] = 0;
	fclose(file);
	return inputbytes;
}
//give each character a row and column
//pos location: clength
void setRowsAndColumns() {
	rows = new int[clength];
	cols = new int[clength];
	int row = 1;
	int col = 1;
	char c;
	char c2;
	for (; inbounds(); pos += 1) {
		rows[pos] = row;
		cols[pos] = col;
		c = contents[pos];
		//new line, go to next row
		if (c == '\n' || c == '\r') {
			if (pos + 1 < clength) {
				c2 = contents[pos + 1];
				//2-character newlines
				if ((c2 == '\n' || c2 == '\r') && c2 != c) {
					pos += 1;
					rows[pos] = row;
					cols[pos] = col + 1;
				}
			}
			col = 0;
			row += 1;
		}
		col += 1;
	}
}
//parse the code
void parseCode() {
	puts("Parsing the file...\n");
	try {
		//split the code into tokens
//		buildTokens();
		//find global variables
		for (tpos = 0; tinbounds(); tpos += 1) {
			//new variable
			if (newVariable(NULL, false, NULL) == NULL)
				makeError(0, "highest level must contain only variable declarations", tokens.inner[tpos]->contentpos);
		}
		//stop if there were errors
		if (errors != 0)
			throw NULL;
		//get initializations for global variables
		int glength = globals.length;
		VariableData** ginner = globals.inner;
		initializations = new Expression* [glength];
		for (int i = 0; i < glength; i += 1) {
			try {
				initializations[i] = getInitialization(ginner[i], NULL, NULL);
			} catch(...) {
			}
		}
		if (fmain == NULL) {
			puts("Error: could not find main function");
			errors += 1;
		}
		setTightLoose();
	} catch(...) {
	}
	if (errors > 1)
		printf("\nQuit with %d errors\n", errors);
	else if (errors == 1)
		puts("\nQuit with 1 error");
	else if (warnings)
		puts("");
}
//search for any type of variable declaration, return whether or not one was found
//tpos location: the token after the variable name (if one was found)
VariableData* newVariable(Function* owner, bool param, VariableStack* st) {
	int loc = tpos;
	//search for identifiers
	while (loc < tlength) {
//		if (isSpecificUnknownValue(tokens.inner[loc], "fixed"))
//			fixed = true;
//		else
			break;
		loc += 1;
	}
	//make sure that there are two variable/type names
	if (loc + 1 >= tlength || tokens.inner[loc]->etype != EUNKNOWNVALUE || tokens.inner[loc + 1]->etype != EUNKNOWNVALUE)
		return NULL;
	int varcontext = varType(((UnknownValue*)(tokens.inner[loc]))->val);
	if (varcontext == TNONE)
		return NULL;
	//check that the variable name is not a keyword
	loc += 1;
	size_t start = tokens.inner[loc]->contentpos;
	string name = ((UnknownValue*)(tokens.inner[loc]))->val;
	if (isReservedWord(name))
		makeError(0, "that word is reserved by the compiler", start);
	if (varType(name) != TNONE)
		makeError(0, "that name has already been defined as a data type", start);
	//add the varible to the list if it's not already defined
	VariableData* v;
	Array<VariableData*>* vs;
	//local variables or parameter variables
	if (owner != NULL) {
		//parameter variables
		if (param) {
			vs = &owner->params;
			//mem pointer will get set later
			v = new VariableData(name, varcontext, new MEMPTR(EBP, 0, -1, 0, false), loc);
		//local variables: check parameter variables too
		} else {
			vs = &owner->variables;
			v = new VariableData(name, varcontext, new MEMPTR(EBP, 0, -1, st != NULL ? (st->num + 1) * -4 : -4, false), loc);
			//search for previous definitions in the function
			for (VariableStack* vs = st; vs != NULL; vs = vs->next) {
				if (vs->val->name.compare(name) == 0) {
					delete v;
					makeError(0, "variable has already been defined in the function", start);
				}
			}
		}
		//search for previous definitions in the parameters
		ArrayIterator<VariableData*> p (&owner->params);
		for (VariableData* v2 = p.getFirst(); p.hasThis(); v2 = p.getNext()) {
			if (v2->name.compare(name) == 0) {
				delete v;
				makeError(0, "variable has already been defined as a parameter", start);
			}
		}
	//global variables: these go somewhere different from local variables
	} else {
		vs = &globals;
		v = new VariableData(name, varcontext, data.length(), loc);
		data.append(getbytesize(varcontext), '\0');
	}
	//search for previous definitions in the global variables
	ArrayIterator<VariableData*> aiv (&globals);
	for (VariableData* v2 = aiv.getFirst(); aiv.hasThis(); v2 = aiv.getNext()) {
		if (v2->name.compare(name) == 0) {
			delete v;
			makeError(0, "variable has already been defined globally", start);
		}
	}
	//possible fixed function
	if (varcontext == TFUNCTION)
		v = new FixedFunction(v);
	if (loc + 1 >= tlength) {
		delete v;
		if (param)
			makeError(1, "the rest of the parameters", start);
		else
			makeError(1, "the variable initialization", start);
	}
	//no more errors: proceed on
	vs->add(v);
	replace(loc, new Variable(v, start));
	tpos = loc + 1;
	//skip past the definition
	if (owner == NULL)
		advanceToSemicolon(start, true, true);
	return v;
}
//check if the string is a variable type
int varType(string s) {
	for (int i = 0; contexts[i] != TNONE; i += 1) {
		if (s.compare(types[i]) == 0)
			return contexts[i];
	}
	return TNONE;
}
//replace a spot in the tokens array with the given expression
Expression* replace(int loc, Expression* val) {
	delete tokens.inner[loc];
	return (tokens.inner[loc] = val);
}
//skip all characters, taking syntax into account if specified, until a semicolon is reached
//tpos location: the semicolon operator
void advanceToSemicolon(size_t start, bool track, bool includecomma) {
	int parentheses = 0;
	//two loops for speed
	while (tinbounds()) {
		if (tokens.inner[tpos]->etype == ESEPARATOR) {
			Separator* s = (Separator*)(tokens.inner[tpos]);
			if ((s->val == SSEMICOLON || (includecomma && s->val == SCOMMA)) && parentheses <= 0)
				return;
			else if (s->val == SLPAREN && track)
				parentheses += 1;
			else if (s->val == SRPAREN && track)
				parentheses -= 1;
		}
		tpos += 1;
	}
	makeError(1, "the end of the statement", start);
}
//get an expression that initializes the variable
//tpos location: the end semicolon operator
Expression* getInitialization(VariableData* v, Function* owner, VariableStack* st) {
	//tpos is assumed to be in range
	tpos = v->tpos + 1;
	//no initialization
	if (tokens.inner[tpos]->etype == ESEPARATOR) {
		if (((Separator*)(tokens.inner[tpos]))->val != SSEMICOLON)
			makeError(0, "expected a semicolon or assignment", tokens.inner[tpos]->contentpos);
		return NULL;
	//not followed by operator
	} else if (tokens.inner[tpos]->etype != EOPERATION)
		makeError(0, "expected a semicolon or assignment", tokens.inner[tpos]->contentpos);
	Operation* o = (Operation*)(tokens.inner[tpos]);
	//wrong operator
	if (o->oper != OASSIGN)
		makeError(0, "expected a semicolon or assignment", tokens.inner[tpos]->contentpos);
	tpos += 1;
	o->left = tokens.inner[tpos - 2];
	if ((o->right = getExpression(owner, false, true, st, NULL)) == NULL)
		makeError(0, "expected an expression to assign to the variable", o->contentpos);
	if (o->right->context != o->left->context && (o->right = implicitCast(o->right, o->left->context)) == NULL)
		makeError(0, "expression cannot be assigned to variable", o->contentpos);
	o->context = v->context;
	o->toplevel = true;
	if (v->name.compare("main") == 0 && v->context == TFUNCTION)
		fmain = ((ObjectConstant*)(o->right))->fval;
	//assign the function to the variable
	if (v->context == TFUNCTION) {
		if (o->right->etype == EOBJECTCONSTANT)
			((FixedFunction*)(v))->inner = ((ObjectConstant*)(o->right))->fval;
		else if (o->right->etype == EVAR)
			((FixedFunction*)(v))->inner = ((FixedFunction*)(((Variable*)(o->right))->inner))->inner;
	}
	return o;
}
//get an expression from the current position
//in addition to toplevel marking whether assignments need to be tracked, it also allows keywords to be used
//returns NULL for empty statements, returns a complete expression otherwise
//tpos location: the expression end character
Expression* getExpression(Function* owner, bool toplevel, bool semicolon, VariableStack* st, BlockStack* est) {
	Array<Expression*> ex;
	int oldtpos = tpos;
	bool assignment = false;
	Expression* e;
	//first round: put things in an array
	for (; tinbounds(); tpos += 1) {
		e = tokens.inner[tpos];
		if (e->etype == ESEPARATOR) {
			Separator* s = (Separator*)(e);
			if (s->val == SCOMMA || s->val == SSEMICOLON || s->val == SRPAREN) {
				if ((s->val == SSEMICOLON) == semicolon)
					break;
				if (semicolon)
					makeError(0, "expected a semicolon", s->contentpos);
				else
					makeError(0, "expected a comma or close parenthesis", s->contentpos);
			} else if (s->val == SLPAREN) {
				e = NULL;
				//cast
				if (tpos + 1 < tlength && tokens.inner[tpos + 1]->etype == EUNKNOWNVALUE) {
					int vt = varType(((UnknownValue*)(tokens.inner[tpos + 1]))->val);
					if (vt != TNONE && isSeparatorType(tokens.inner[tpos + 2], SRPAREN)) {
						e = replace(tpos + 1, new Cast(vt, tokens.inner[tpos]->contentpos));
						tpos += 2;
					}
				}
				if (e == NULL) {
					FunctionCall* f = NULL;
					if (ex.length > 0) {
						e = ex.inner[ex.length - 1];
						if (e->context == TFUNCTION)
							f = (FunctionCall*)(replace(tpos, new FunctionCall(e)));
					}
					tpos += 1;
					//function call
					if (f != NULL) {
						//get the parameters
						ex.pop();
						if (toutofbounds())
							makeError(1, "the function parameters", clength - 1);
						if (isSeparatorType(tokens.inner[tpos], SCOMMA))
							makeError(0, "expected an expression or close parenthesis", tokens.inner[tpos]->contentpos);
						//no comma and no close parenthesis- begin getting expressions
						if (!isSeparatorType(tokens.inner[tpos], SRPAREN)) {
							//won't be null- next token is neither a comma nor a close parenthesis
							f->params.add(getExpression(owner, false, false, st, NULL));
							//getExpression always leaves tpos inbounds and on a separator
							while (((Separator*)(tokens.inner[tpos]))->val == SCOMMA) {
								tpos += 1;
								if ((e = getExpression(owner, false, false, st, NULL)) == NULL)
									makeError(0, "expected an expression", tokens.inner[tpos]->contentpos);
								f->params.add(e);
							}
						}
						e = f;
					//subexpression
					} else {
						if ((e = getExpression(owner, false, false, st, NULL)) == NULL)
							makeError(0, "expected an expression", tokens.inner[tpos]->contentpos);
						//getExpression always leaves tpos inbounds and on a separator
						if (((Separator*)(tokens.inner[tpos]))->val != SRPAREN)
							makeError(0, "expected a close parenthesis", tokens.inner[tpos]->contentpos);
					}
				}
			}
		} else if (e->etype == EOPERATION) {
			Operation* o = (Operation*)(e);
			if (o->prec == PASSIGN) {
				//one assignment per expression/subexpression
				//this is only included for clarification purposes
				if (assignment)
					makeError(0, "only one assignment operator allowed per expression/subexpression", o->contentpos);
				//assignment must be the first operator
				if (tpos != oldtpos + 1)
					makeError(0, "assignment must be the first operator in an expression/subexpression", o->contentpos);
				//left must be a variable
				if (ex.inner[0]->etype != EVAR)
					makeError(0, "must assign to a variable", o->contentpos);
				VariableData* vd = ((Variable*)(ex.inner[0]))->inner;
				//no reassigning fixed functions
				if (vd->fixedfunction && ((FixedFunction*)(vd))->inner != NULL)
					makeError(0, "cannot reassign a fixed function", o->contentpos);
				assignment = true;
			//postfix operators- handle them now
			} else if (o->prec == PUNARYPOST) {
				if (tpos <= oldtpos)
					makeError(0, "a variable must precede this unary operator", o->contentpos);
				Expression* v = ex.pop();
				//only accept variables
				if (o->oper == OVARLNOT) {
					if (v->etype != EVAR || !isNumerical(v->context, true))
						makeError(0, "a number or boolean variable must precede this unary operator", o->contentpos);
				} else {
					if (v->etype != EVAR || !isNumerical(v->context, false))
						makeError(0, "a number variable must precede this unary operator", o->contentpos);
				}
				o->left = v;
				o->value = true;
				o->context = v->context;
			}
		} else if (e->etype == EUNKNOWNVALUE) {
			string s = ((UnknownValue*)(e))->val;
			int vt = varType(s);
			//function definition
			if (vt != TNONE) {
				Function* f = new Function(vt);
				functions.add(f);
				int ftpos = tpos;
				tpos += 1;
				if (toutofbounds())
					makeError(1, "the rest of the function definition", clength - 1);
				if (!isSeparatorType(tokens.inner[tpos], SLPAREN))
					makeError(0, "expected a parameters list", tokens.inner[tpos]->contentpos);
				tpos += 1;
				while (newVariable(f, true, st) != NULL) {
					if (tokens.inner[tpos]->etype == ESEPARATOR) {
						Separator* s = (Separator*)(tokens.inner[tpos]);
						if (s->val == SCOMMA)
							tpos += 1;
						else if (s->val != SRPAREN)
							makeError(0, "expected a comma or a close parenthesis", e->contentpos);
					} else
						makeError(0, "expected a comma or a close parenthesis", e->contentpos);
				}
				//assign the memory locations
				VariableData** pinner = f->params.inner;
				int dist = 8;
				for (int i = f->params.length - 1; i >= 0; i -= 1) {
					VariableData* v = pinner[i];
					((MEMPTR*)(v->ptr))->constant = dist;
					dist += getpopcount(v->context);
				}
				tpos += 2;
				if (toutofbounds())
					makeError(1, "the function statements", clength - 1);
				if (!isSeparatorType(tokens.inner[tpos - 1], SLPAREN))
					makeError(0, "expected an open parenthesis", tokens.inner[tpos - 1]->contentpos);
				//got the function first part- now, make the statements for it
				f->starttpos = tpos;
				addStatements(f);
				e = replace(ftpos, new ObjectConstant(f, "", e->contentpos));
} else if (s.compare("nop") == 0) {
AssemblySequence* as = new AssemblySequence(new NOP());
sequences.add(as);
return as;
			//keyword
			} else if (s.compare("return") == 0) {
				keywordErrors(s, !toplevel, tpos != oldtpos, e->contentpos);
				int ftpos = tpos;
				tpos += 1;
				//allowed to be null if function context is void
				e = getExpression(owner, false, true, st, NULL);
				if (owner->context != TVOID) {
					if (e == NULL)
						makeError(0, "expected an expression", tokens.inner[tpos]->contentpos);
					if (e->context != owner->context && (e = implicitCast(e, owner->context)) == NULL)
						makeError(0, "cannot return a value of this type", e->contentpos);
				}
				return replace(ftpos, new Return(e, owner));
			} else if (s.compare("if") == 0) {
				keywordErrors(s, !toplevel, tpos != oldtpos, e->contentpos);
				IfBranch* i = (IfBranch*)(replace(tpos, new IfBranch(e->contentpos)));
				//get condition part
				tpos += 2;
				if (toutofbounds())
					makeError(1, "the branch's condition", i->contentpos);
				if (!isSeparatorType(tokens.inner[tpos - 1], SLPAREN))
					makeError(0, "expected an open parenthesis", tokens.inner[tpos - 1]->contentpos);
				if ((i->condition = getExpression(owner, false, false, st, NULL)) == NULL)
					makeError(0, "expected an expression", tokens.inner[tpos]->contentpos);
				if (!isSeparatorType(tokens.inner[tpos], SRPAREN))
					makeError(0, "expected a close parenthesis", tokens.inner[tpos]->contentpos);
				//get if part
				getBranchStatements(owner, st, est, &i->ifpart, i->contentpos, "the if branch's statements");
				//get else part
				e = tokens.inner[tpos];
				if (isSpecificUnknownValue(e, "else"))
					getBranchStatements(owner, st, est, &i->elsepart, e->contentpos, "the else branch's statements");
				//back up to get to the finishing character
				tpos -= 1;
				return i;
			} else if (s.compare("for") == 0) {
				keywordErrors(s, !toplevel, tpos != oldtpos, e->contentpos);
				MultiLoop* m = (MultiLoop*)(replace(tpos, new MultiLoop(e->contentpos, false)));
				//get initialization part
				tpos += 2;
				if (toutofbounds())
					makeError(1, "the for loop initialization", m->contentpos);
				if (!isSeparatorType(tokens.inner[tpos - 1], SLPAREN))
					makeError(0, "expected an open parenthesis", tokens.inner[tpos - 1]->contentpos);
				VariableStack* vs = getSingleStatement(owner, st, NULL, &m->statements);
				//allowed to be missing- if not, mark it as toplevel
				if (m->statements.length > 0)
					(m->initialization = m->statements.pop())->toplevel = true;
				//get condition part
				tpos += 1;
				if (toutofbounds())
					makeError(1, "the for loop condition", m->contentpos);
				if ((m->condition = getExpression(owner, false, true, vs, NULL)) == NULL)
					makeError(0, "expected an expression", tokens.inner[tpos]->contentpos);
				//get the increment part
				tpos += 1;
				if (toutofbounds())
					makeError(1, "the for loop increment", m->contentpos);
				//allowed to be null- if not, mark it as toplevel
				if ((m->increment = getExpression(owner, false, false, vs, NULL)) != NULL)
					m->increment->toplevel = true;
				if (!isSeparatorType(tokens.inner[tpos], SRPAREN))
					makeError(0, "expected a close parenthesis", tokens.inner[tpos]->contentpos);
				BlockStack thisblock (m, est);
				//get the statements
				getBranchStatements(owner, vs, &thisblock, &m->statements, m->contentpos, "the for loop's statements");
				//give all continues the increment if it's an infinite loop
				if (m->condition->etype == EINTCONSTANT) {
					ArrayIterator<ControlFlow*> ac (&m->controlflows);
					for (ControlFlow* c = ac.getFirst(); ac.hasThis(); c = ac.getNext()) {
						if (c->val == CCONTINUE)
							c->code = m->increment;
					}
				}
				//back up to get to the finishing character
				tpos -= 1;
				//delete the stack pointer if it was new
				if (vs != st)
					delete vs;
				return m;
			} else if (s.compare("while") == 0) {
				keywordErrors(s, !toplevel, tpos != oldtpos, e->contentpos);
				MultiLoop* m = (MultiLoop*)(replace(tpos, new MultiLoop(e->contentpos, false)));
				//get condition part
				tpos += 2;
				if (toutofbounds())
					makeError(1, "the loop's condition", m->contentpos);
				if (!isSeparatorType(tokens.inner[tpos - 1], SLPAREN))
					makeError(0, "expected an open parenthesis", tokens.inner[tpos - 1]->contentpos);
				if ((m->condition = getExpression(owner, false, false, st, NULL)) == NULL)
					makeError(0, "expected an expression", tokens.inner[tpos]->contentpos);
				if (!isSeparatorType(tokens.inner[tpos], SRPAREN))
					makeError(0, "expected a close parenthesis", tokens.inner[tpos]->contentpos);
				BlockStack thisblock (m, est);
				//get statements
				getBranchStatements(owner, st, &thisblock, &m->statements, m->contentpos, "the while loop's statements");
				//back up to get to the finishing character
				tpos -= 1;
				return m;
			} else if (s.compare("do") == 0) {
				keywordErrors("do-while", !toplevel, tpos != oldtpos, e->contentpos);
				MultiLoop* m = (MultiLoop*)(replace(tpos, new MultiLoop(e->contentpos, true)));
				BlockStack thisblock (m, est);
				//get statements
				getBranchStatements(owner, st, &thisblock, &m->statements, m->contentpos, "the do-while loop's statements");
				if (!isSpecificUnknownValue(tokens.inner[tpos], "while"))
					makeError(0, "expected while keyword", tokens.inner[tpos]->contentpos);
				//get condition part
				tpos += 2;
				if (toutofbounds())
					makeError(1, "the loop's condition", m->contentpos);
				if (!isSeparatorType(tokens.inner[tpos - 1], SLPAREN))
					makeError(0, "expected an open parenthesis", tokens.inner[tpos - 1]->contentpos);
				if ((m->condition = getExpression(owner, false, false, st, NULL)) == NULL)
					makeError(0, "expected an expression", tokens.inner[tpos]->contentpos);
				if (!isSeparatorType(tokens.inner[tpos], SRPAREN))
					makeError(0, "expected a close parenthesis", tokens.inner[tpos]->contentpos);
				return m;
			//control flow
			} else if (s.compare("break") == 0 || s.compare("case") == 0 || s.compare("default") == 0 || s.compare("continue") == 0) {
				keywordErrors(s, !toplevel, tpos != oldtpos, e->contentpos);
				ControlFlow* c = (ControlFlow*)(replace(tpos, new ControlFlow(s, e->contentpos)));
				if (est == NULL)
					makeError(0, "needs to be in a loop", c->contentpos);
				e = est->val;
				if (e->etype == EMULTILOOP)
					((MultiLoop*)(e))->controlflows.add(c);
				return c;
			//variable
			} else {
				int oldpos = e->contentpos;
				e = NULL;
				//check for global variables
				ArrayIterator<VariableData*> g (&globals);
				for (VariableData* v = g.getFirst(); g.hasThis(); v = g.getNext()) {
					if (v->name.compare(s) == 0) {
						e = replace(tpos, new Variable(v, oldpos));
						break;
					}
				}
				if (e == NULL) {
					//check to see if the variable is one of the function's
					if (owner != NULL) {
						//check the parameter variables
						ArrayIterator<VariableData*> p (&owner->params);
						for (VariableData* v = p.getFirst(); p.hasThis(); v = p.getNext()) {
							if (v->name.compare(s) == 0) {
								e = replace(tpos, new Variable(v, oldpos));
								break;
							}
						}
						if (e == NULL) {
							//check the normal function variables
							for (VariableStack* vs = st; vs != NULL; vs = vs->next) {
								if (vs->val->name.compare(s) == 0) {
									e = replace(tpos, new Variable(vs->val, oldpos));
									break;
								}
							}
						}
					}
					if (e == NULL) {
						if (isReservedWord(s))
							makeError(0, "out-of-place keyword", oldpos);
						else
							makeError(0, "unknown value", oldpos);
					}
				}
			}
		}
else if (e->etype == EFLOATCONSTANT)
makeError(0, "floats are not supported yet", e->contentpos);
		ex.add(e);
	}
	int elength = ex.length;
	if (elength < 1)
		return NULL;
	if (toutofbounds())
		makeError(0, "the rest of the expression", tokens.inner[oldtpos]->contentpos);
	//second round: assign unary operators and casts
	bool value = true;
	Cast* c;
	Expression** einner = ex.inner;
	for (int i = elength - 1; i >= 0; i -= 1) {
		if (!einner[i]->value) {
			if (einner[i]->etype == EOPERATION) {
				Operation* o = (Operation*)(einner[i]);
				if (o->oper == OSUB && (i == 0 || !einner[i - 1]->value)) {
					o->oper = ONEG;
					o->prec = PUNARYPRE;
				}
				//prefix unary operator
				if (o->prec == PUNARYPRE) {
					i += 1;
					//only accept certain types
					if (o->oper == OLNOT) {
						if (i >= elength || !einner[i]->value || !isNumerical(einner[i]->context, true))
							makeError(0, "a number or boolean must follow this unary operator", o->contentpos);
					} else {
						if (i >= elength || !einner[i]->value || !isNumerical(einner[i]->context, false))
							makeError(0, "a number must follow this unary operator", o->contentpos);
					}
					//change int constants instead
					if (einner[i]->etype == EINTCONSTANT) {
						IntConstant* c = (IntConstant*)(einner[i]);
						//~
						if (o->oper == OBNOT)
							c->ival = ~c->ival;
						//!
						else if (o->oper == OLNOT)
							c->ival = (c->ival == 0) ? 1 : 0;
						//- -|
						else if (o->oper == ONEG)
							c->ival = -c->ival;
						i -= 1;
						ex.remove(i);
					} else {
						o->left = einner[i];
						o->value = true;
						o->context = o->left->context;
						ex.remove(i);
						i -= 1;
					}
					value = true;
					elength = ex.length;
				}
			//unbound cast
			} else if (einner[i]->etype == ECAST) {
				c = (Cast*)(einner[i]);
				i += 1;
				if (i >= elength || !einner[i]->value)
					makeError(0, "a value must follow this cast", c->contentpos);
				c->inner = einner[i];
				if (!c->validCast())
					makeError(0, "cannot cast expression to desired type", c->contentpos);
				c->value = true;
				ex.remove(i);
				i -= 1;
				value = true;
				elength = ex.length;
			}
		}
		//make sure that the syntax is correct
		if (einner[i]->value != value) {
			if (value)
				makeError(0, "was expecting a value", einner[i]->contentpos);
			else
				makeError(0, "was expecting an operator", einner[i]->contentpos);
		}
		value = !value;
	}
	if (!einner[0]->value)
		makeError(0, "expression must start with a value", einner[0]->contentpos);
	//third round: format the expression
	Expression* rval = createExpression(&ex, 0, elength, PASSIGN);
	rval->toplevel = toplevel;
	return rval;
}
//add statements to the function, including enter and leave assembly parts
//tpos location: the function's close parenthesis
void addStatements(Function* f) {
	//tpos is assumed to be at the function's start position
	getStatementBlock(f, NULL, NULL, &f->statements);
	int vnum = f->vnum * 4;
	int pnum = f->params.length * 4;
	//function-specific variables: create stack frame
	if (vnum > 0 || pnum > 0) {
		AssemblySequence* enter = new AssemblySequence(new PUSH(TREG32, EBP));
		sequences.add(enter);
		enter->inner.add(new MOV(TREG32, EBP, TREG32, ESP));
		AssemblySequence* leave = new AssemblySequence(new POP(TREG32, EBP));
		sequences.add(leave);
		if (vnum > 0) {
			enter->inner.add(new SUB(TREG32, ESP, TCONSTANT, vnum));
			leave->inner.add(new MOV(TREG32, ESP, TREG32, EBP), 0);
		}
		f->statements.add(enter, 0);
		f->statements.add(leave);
	}
	//return statement if needed
	if (f->statements.length < 1 || f->statements.inner[f->statements.length - 1]->etype != ERETURN) {
		AssemblySequence* as = new AssemblySequence(new RET(pnum));
		sequences.add(as);
		f->statements.add(as);
	}
}
//get a block of statements
//tpos location: close parenthesis | token after semicolon for single statement
void getStatementBlock(Function* f, VariableStack* st, BlockStack* est, Array<Expression*>* statements) {
	VariableStack* old = st;
	try {
		for (; tinbounds() && !isSeparatorType(tokens.inner[tpos], SRPAREN); tpos += 1)
			st = getSingleStatement(f, st, est, statements);
		while (st != old) {
			VariableStack* hold = st;
			st = st->next;
			delete hold;
		}
	} catch(...) {
		while (st != old) {
			VariableStack* hold = st;
			st = st->next;
			delete hold;
		}
		throw NULL;
	}
}
//get an expression or a new variable
//return the variable stack (different if there is a new variable, same if not)
//if an error occured and a semicolon was found, returns the stack
//if an error occured and a semicolon was not found, throws NULL and deletes any extra stack
//tpos location: the statement end character
VariableStack* getSingleStatement(Function* f, VariableStack* st, BlockStack* est, Array<Expression*>* statements) {
	VariableStack* old = st;
	try {
		VariableData* vd = newVariable(f, false, st);
		//found a new variable
		if (vd != NULL) {
			st = new VariableStack(vd, f, st);
			Expression* initialization = getInitialization(vd, f, st);
			if (initialization != NULL)
				statements->add(initialization);
		//not an empty statement- get an expression
		} else if (!isSeparatorType(tokens.inner[tpos], SSEMICOLON)) {
			//won't be null since the token isn't a semicolon
			Expression* e = getExpression(f, true, true, st, est);
			statements->add(e);
			//warnings about results
			if (e->etype == EINTCONSTANT || e->etype == EOBJECTCONSTANT || e->etype == EFLOATCONSTANT || e->etype == EVAR)
				makeWarning(0, "using a value as a statement", e->contentpos);
			else if (e->etype == EOPERATION) {
				Operation* o = (Operation*)(e);
				if (o->prec != PASSIGN && o->prec != PUNARYPOST)
					makeWarning(0, "operation result discarded", e->contentpos);
			} else if (e->etype == EBOOLEANOPER || e->etype == ESTRINGCONCAT)
				makeWarning(0, "operation result discarded", e->contentpos);
		}
	} catch(...) {
		try {
			//skip past errors
			if (tinbounds())
				advanceToSemicolon(tokens.inner[tpos]->contentpos, false, false);
		} catch(...) {
			if (st != old)
				delete st;
			throw NULL;
		}
	}
	return st;
}
//print out keyword errors if either boolean is true
void keywordErrors(string s, bool nottoplevel, bool notfirst, size_t loc) {
	char c [40];
	if (nottoplevel) {
		sprintf(c, "%s cannot be in a subexpression", s.c_str());
		makeError(0, c, loc);
	}
	if (notfirst) {
		sprintf(c, "%s must be the first operator", s.c_str());
		makeError(0, c, loc);
	}
}
//get statements for the branch (single or block)
//starting tpos location: close parenthesis of the branch | else keyword | do keyword
//tpos location: token after semicolon for single | token after close parenthesis for block
void getBranchStatements(Function* f, VariableStack* st, BlockStack* est, Array<Expression*>* statements, size_t start, char* message) {
	tpos += 1;
	if (toutofbounds())
		makeError(1, message, start);
	if (isSeparatorType(tokens.inner[tpos], SLPAREN)) {
		tpos += 1;
		if (toutofbounds())
			makeError(1, message, start);
		getStatementBlock(f, st, est, statements);
		tpos += 1;
	} else {
		VariableStack* vs = getSingleStatement(f, st, est, statements);
		tpos += 1;
		if (vs != st)
			delete vs;
	}
	if (toutofbounds())
		makeError(1, "the rest of the function", tokens.inner[tlength - 1]->contentpos);
}
//find all operations and create them
Expression* createExpression(Array<Expression*>* e, int start, int end, int prec) {
	Expression** einner = e->inner;
	if (start + 1 == end)
		return einner[start];
	if (prec == PASSIGN) {
		Operation* o = (Operation*)(einner[start + 1]);
		//assignment, only ever found on the second token/first operator
		if (o->prec == PASSIGN) {
			o->value = true;
			o->left = einner[start];
			o->right = createExpression(e, start + 2, end, PQMARK);
			//compare the results like the non-assign expression
			//make sure they're the same
			if (o->oper == OASSIGN) {
				if (o->right->context != o->left->context && (o->right = implicitCast(o->right, o->left->context)) == NULL)
					makeError(0, "expression cannot be assigned to variable", o->contentpos);
				if (o->left->context == TFUNCTION)
					((Variable*)(o->left))->inner->fixedfunction = false;
			//strictly numerical
			} else if (o->oper == OASSIGNSUB ||
				o->oper == OASSIGNMUL || o->oper == OASSIGNDIV || o->oper == OASSIGNMOD ||
				o->oper == OASSIGNAND || o->oper == OASSIGNXOR || o->oper == OASSIGNOR) {
				//always returns true if both contexts are numerical
				ensureSameContexts(o, true, false, false);
			//strictly numerical except for string adding
			} else if (o->oper == OASSIGNADD) {
				if (!ensureSameContexts(o, false, false, false))
					makeError(0, "expression cannot be assigned to variable", o->contentpos);
				//string add assign becomes assign a concatenation
				if (o->left->context == TSTRING) {
					StringConcatenation* s;
					if (o->right->etype == ESTRINGCONCAT) {
						s = (StringConcatenation*)(o->right);
						s->strings.add(o->left, 0);
					} else {
						s = new StringConcatenation(o->contentpos);
						s->strings.add(o->left);
						s->strings.add(o->right);
						tokens.add(s);
					}
					o->right = s;
					o->oper = OASSIGN;
				} else if (!isNumerical(o->left->context, false))
					makeError(0, "number or string expected", o->left->contentpos);
			//strictly numerical but they can be different
			} else if (o->oper == OASSIGNSHL || o->oper == OASSIGNSHR || o->oper == OASSIGNSAR || o->oper == OASSIGNROL || o->oper == OASSIGNROR) {
				if (!isNumerical(o->left->context, false))
					makeError(0, "number expected", o->left->contentpos);
				if (!isNumerical(o->right->context, false))
					makeError(0, "number expected", o->right->contentpos);
			//strictly boolean
			} else if (o->oper == OASSIGNBAND || o->oper == OASSIGNBXOR || o->oper == OASSIGNBOR)
				//always returns true if both contexts are boolean
				ensureSameContexts(o, false, true, false);
			o->context = o->left->context;
			return o;
		}
		prec = PQMARK;
	}
	//go down the list of precedences
	for (; prec <= PMULDIV; prec += 1) {
		//go backwards through the expression list looking for operators
		for (int i = end - 1; i >= start; i -= 1) {
			//found an operator
			if (einner[i]->etype == EOPERATION) {
				Operation* o = (Operation*)(einner[i]);
				//operator has the right precedence and is not a value
				if (o->prec == prec && !o->value) {
					o->value = true;
					//ternary question mark + colon pair
					if (prec == PQMARK) {
						//extra question mark
						if (o->oper == OQMARK)
							makeError(0, "question mark missing a colon", o->contentpos);
						int marks = 1;
						Operation* o2;
						int j = i;
						//pass any other question mark + colon pairs
						while (j >= start && marks > 0) {
							j -= 1;
							if (einner[j]->etype == EOPERATION) {
								o2 = (Operation*)(einner[j]);
								if (o2->oper == OQMARK)
									marks -= 1;
								else if (o2->oper == OCOLON)
									marks += 1;
							}
						}
						//extra question mark
						if (marks > 0)
							makeError(0, "colon missing a question mark", o->contentpos);
						o->testpart = createExpression(e, start, j, PQMARK);
						//needs a number result
						if (!isNumerical(o->testpart->context, true))
							makeError(0, "number or boolean expected", o->testpart->contentpos);
						o->left = createExpression(e, j + 1, i, PQMARK);
						o->right = createExpression(e, i + 1, end, PQMARK);
						//contexts must be the same
						if (!ensureSameContexts(o, false, false, true))
							makeError(0, "result types must match", o->contentpos);
					//all other operators
					} else {
						//get the values at the left and right, accept any context
						o->left = createExpression(e, start, i, prec);
						o->right = createExpression(e, i + 1, end, prec);
						//now look at the prec and compare the results as necessary
						//comparison: make sure both sides are equal, force numeric for integer comparisons
						if (prec == PCOMPARE) {
							if (o->oper == OLEQ || o->oper == OGEQ || o->oper == OLT || o->oper == OGT) {
								if (!ensureSameContexts(o, true, false, true))
									makeError(0, "need two numbers for number comparisons", o->contentpos);
							} else {
								if (!ensureSameContexts(o, false, false, true))
									makeError(0, "operand types do not match", o->contentpos);
							}
						//shifting: strictly numerical but they can be different
						} else if (prec == PSHIFT) {
							if (!isNumerical(o->left->context, false))
								makeError(0, "number expected", o->left->contentpos);
							if (!isNumerical(o->right->context, false))
								makeError(0, "number expected", o->right->contentpos);
						//or/xor/and/mul/div/mod: strictly numerical
						} else if (prec == POR || prec == PXOR || prec == PAND || prec == PMULDIV)
							//always returns true if both contexts are numerical
							ensureSameContexts(o, true, false, true);
						//boolean or/xor/and: strictly boolean
						else if (prec == PBOR || prec == PBXOR || prec == PBAND) {
							//always returns true if both contexts are boolean
							ensureSameContexts(o, false, true, true);
							if (prec == PBOR || prec == PBAND) {
								BooleanOperation* b;
								Expression* right = o->right;
								Expression* left = o->left;
								int oper = o->oper;
								if (isBooleanOperationType(left, oper)) {
									b = (BooleanOperation*)(left);
									if (isBooleanOperationType(right, oper))
										b->inner.add(&((BooleanOperation*)(right))->inner, false);
									else
										b->inner.add(right);
								} else if (isBooleanOperationType(right, oper)) {
									b = (BooleanOperation*)(right);
									b->inner.add(left, 0);
								} else {
									b = (BooleanOperation*)(replace(o->tpos, new BooleanOperation(oper, o->contentpos)));
									b->inner.add(left);
									b->inner.add(right);
								}
								return b;
							}
						//add/sub: strictly numerical except for string adding
						} else if (prec == PADDSUB) {
							if (o->left->context == TSTRING && o->oper == OADD) {
								//make sure the right one is a string too
								if (o->right->context != TSTRING && (o->right = implicitCast(o->right, TSTRING)) == NULL)
									makeError(0, "need a string", o->right->contentpos);
								StringConcatenation* s;
								Expression* right = o->right;
								Expression* left = o->left;
								if (left->etype == ESTRINGCONCAT)
									s = (StringConcatenation*)(left);
								//if both are constants, make a new constant instead
								else if (left->etype == EOBJECTCONSTANT && right->etype == EOBJECTCONSTANT) {
									((ObjectConstant*)(left))->sval.append(((ObjectConstant*)(right))->sval);
									return left;
								} else {
									s = (StringConcatenation*)(replace(o->tpos, new StringConcatenation(o->contentpos)));
									s->strings.add(left);
								}
								if (right->etype == ESTRINGCONCAT)
									s->strings.add(&((StringConcatenation*)(right))->strings, false);
								else
									s->strings.add(right);
								return s;
							} else
								//always returns true if both contexts are numerical
								ensureSameContexts(o, true, false, true);
						}
						//commutative operations, swap operands if it makes sense
						//integer comparison operators get swapped
						if (o->oper == OMUL || o->oper == OADD || o->oper == OAND || o->oper == OXOR || o->oper == OOR || o->prec == PCOMPARE) {
							bool swap = false;
							//left is a constant
							if (o->left->etype == EINTCONSTANT) {
								IntConstant* left = (IntConstant*)(o->left);
								//right is not constant, left is 1, or left is -1 and right is not 1
								if (o->right->etype != EINTCONSTANT || left->ival == 1 || (left->ival == -1 && ((IntConstant*)(o->right))->ival != 1))
									swap = true;
							//left is a variable
							} else if (o->left->etype == EVAR && o->right->etype != EINTCONSTANT && o->right->etype != EVAR)
								swap = true;
							if (swap) {
								Expression* hold = o->left;
								o->left = o->right;
								o->right = hold;
								if (o->prec == PCOMPARE) {
									if (o->oper == OLEQ)
										o->oper = OGEQ;
									else if (o->oper == OGEQ)
										o->oper = OLEQ;
									else if (o->oper == OLT)
										o->oper = OGT;
									else if (o->oper == OGT)
										o->oper = OLT;
								}
							}
						}
					}
					//resulting context equals the left context
					o->context = prec == PCOMPARE ? TBOOLEAN : o->left->context;
					return o;
				}
			}
		}
	}
	//should never get here
	puts("Oh no! If you see this, there's a bug!");
	throw NULL;
	return NULL;
}
//implicitly cast e to the given context
//returns NULL if it cannot
Expression* implicitCast(Expression* e, int context) {
	if (e->context == context)
		return e;
	else if (e->etype == EINTCONSTANT) {
//		castConstant((IntConstant*)(e), context);
		return e;
	}
	//numerical expand-cast
	if (context == TINT && e->context == TBYTE) {
		Cast* c = new Cast(context, e);
		tokens.add(c);
		return c;
	}
	return NULL;
}
//ensure that the left and right operands match contexts, return whether or not they are
bool ensureSameContexts(Operation* o, bool forcenumerical, bool forceboolean, bool cancastleft) {
	if (forcenumerical) {
		if (!isNumerical(o->left->context, false))
			makeError(0, "number expected", o->left->contentpos);
		if (!isNumerical(o->right->context, false))
			makeError(0, "number expected", o->right->contentpos);
	}
	if (forceboolean) {
		if (o->left->context != TBOOLEAN)
			makeError(0, "boolean expected", o->left->contentpos);
		if (o->right->context != TBOOLEAN)
			makeError(0, "boolean expected", o->right->contentpos);
		return true;
	}
	if (o->left->context != o->right->context) {
		Expression* e;
		if (cancastleft && (e = implicitCast(o->left, o->right->context)) != NULL)
			o->left = e;
		else if ((e = implicitCast(o->right, o->left->context)) != NULL)
			o->right = e;
		else
			return false;
	}
	return true;
}
//scan all the statements and assign tight- or loose-swappable functions
void setTightLoose() {
}
//first stage compiling, put all the stuff into the code list
void buildAssembly() {
	puts("Building assembly...");
	//add intro stuff to the code
	//get process heap
	code.add(new CALL(&TGetProcessHeap))
	->add(new MOV(TDATADWORDPTR, VDATAMAINHEAP, TREG32, EAX))
	//allocate heap
	->add(new PUSH(TCONSTANT, 0x100000))
	->add(new PUSH(TCONSTANT, 0))
	->add(new PUSH(TDATADWORDPTR, VDATAMAINHEAP))
	->add(new CALL(&THeapAlloc))
	//store heap info
	->add(new MOV(TDATADWORDPTR, VDATACURRENTHEAP, TREG32, EAX))
	->add(new MOV(TDATADWORDPTR, VDATACURRENTHEAPSPOT, TREG32, EAX))
	->add(new MOV(TDATADWORDPTR, VDATACURRENTHEAPSIZE, TCONSTANT, 0x100000));
	//global variable init
	int ilength = globals.length;
	for (int i = 0; i < ilength; i += 1) {
		if (initializations[i] != NULL)
			addAssembly(&code, initializations[i]->getAssembly(false, EAX), true);
	}
	//main function call
	code.add(new CALL(fmain))
	//exit the program with code 0
	->add(new MOV(TREG32, EAX, TCONSTANT, 0))
	->add(new RET());
	//add the functions to the code
	ArrayIterator<Function*> aif (&functions);
	for (Function* f = aif.getFirst(); aif.hasThis(); f = aif.getNext()) {
		f->startinstruction = code.length;
		addAssembly(&code, f->getInstructions(), true);
	}
}
//second stage of compiling, assign constants to representations
void buildSections() {
	puts("Formatting sections...");
	int sequencelength = code.length;
	AssemblyInstruction** instructions = code.inner;
	//jumps and conditional jumps have different sizes, so calculate these first
	//build a list of address locations
	int* locs = new int[sequencelength + 1];
	int offset = 0;
	int jcount = 0;
	AssemblyInstruction* instruction;
	int tag;
	//find all instructions with a tag
	for (int i = 0; i < sequencelength; i += 1) {
		locs[i] = offset;
		instruction = instructions[i];
		offset += instruction->bytes.length();
		tag = instruction->tag;
		//jump has a tag and will go in a list
		if (tag == TAGJMPINSTRUCTIONSHORT)
			jcount += 1;
		//modify the tags
		else if (tag == TAGJMPINSTRUCTIONSHORTRELATIVE) {
			jcount += 1;
			instruction->tag = TAGJMPINSTRUCTIONSHORT;
			instruction->tag2 += i;
		} else if (tag == TAGCALLTHUNK) {
			Thunk* tdest = (Thunk*)(instruction->tagp);
			if (!tdest->used) {
				tdest->offset = thunks.length * 4;
				thunks.add(tdest);
				tdest->used = true;
			}
		}
	}
	codesize = offset;
	//build the list of jumps
	locs[sequencelength] = offset;
	AssemblyInstruction** instructionjumps = new AssemblyInstruction*[jcount];
	int* jpositions = new int [jcount];
	jcount = 0;
	for (int i = 0; i < sequencelength; i += 1) {
		//jump to address
		if (instructions[i]->tag == TAGJMPINSTRUCTIONSHORT) {
			instructionjumps[jcount] = instructions[i];
			jpositions[jcount] = i;
			jcount += 1;
		}
	}
	//assign jumps
	int bytes;
	int locpos;
	int src;
	int dst;
	int jposition;
	while (offset != 0) {
		offset = 0;
		locpos = 0;
		//go through all jump instructions and assign
		for (int i = 0; i < jcount; i += 1) {
			instruction = instructionjumps[i];
			jposition = jpositions[i];
			//add offset to all locations
			if (offset == 0)
				locpos = jposition + 1;
			else {
				do {
					locs[locpos] += offset;
					locpos += 1;
				} while (locpos <= jposition);
			}
			//find the destination
			//dst = address of destination instruction
			//src = address of following instruction (jumps from here)
			//src has not been offset, dst might have been
			dst = instruction->tag2;
			src = jposition + 1;
			//dst has not been offset if it's greater than src
			//no need to offset src as well
			if (dst > src) {
				bytes = locs[dst] - locs[src];
				if (bytes > 127 && instruction->tag == TAGJMPINSTRUCTIONSHORT) {
					offset -= (int)(instruction->bytes.length());
					instruction->set(TCONSTANT, bytes);
					instruction->tag = TAGJMPINSTRUCTIONLONG;
					offset += (int)(instruction->bytes.length());
				} else
					instruction->set(TCONSTANT, bytes);
			//dst is before, so it has been offset
			//src needs to be offset too
			} else {
				bytes = locs[dst] - (locs[src] + offset);
				if (bytes < -128 && instruction->tag == TAGJMPINSTRUCTIONSHORT) {
					int diff = -(int)(instruction->bytes.length());
					//set the distance twice to account for the change in byte size
					instruction->set(TCONSTANT, -129);
					diff += (int)(instruction->bytes.length());
					instruction->set(TCONSTANT, bytes - diff);
					instruction->tag = TAGJMPINSTRUCTIONLONG;
					offset += diff;
				} else
					instruction->set(TCONSTANT, bytes);
			}
		}
		//offset the remaining instructions
		if (offset != 0) {
			for (; locpos <= sequencelength; locpos += 1)
				locs[locpos] += offset;
			codesize += offset;
		}
	}
	//build rdata before we do anything else
	//thunks are now set
	buildrdata();
	int rdataVA = IMAGEBASE + rdataRVA;
	dataRVA = rdataRVA + roundup(rdatasize, 0x1000);
	int dataVA = IMAGEBASE + dataRVA;
	int codeVA = IMAGEBASE + 0x1000;
	//go through all of the rest of the tags
	for (int i = 0; i < sequencelength; i += 1) {
		instruction = instructions[i];
		//clear tag
		tag = instruction->tag;
		instruction->tag = TAGNONE;
		//handle the tag
		if (tag != TAGNONE) {
			//function calls
			if (tag == TAGCALLFUNCTION)
				instruction->set(TCONSTANT, locs[((Function*)(instruction->tagp))->startinstruction] - locs[i + 1]);
			//thunk calls
			else if (tag == TAGCALLTHUNK)
				instruction->set(TDWORDPTR, (intptr_t)(new MEMPTR(-1, -1, -1, rdataVA + ((Thunk*)(instruction->tagp))->offset, true)));
			//address pushes
			else if (tag == TAGDATAADDRESS)
				instruction->set(TCONSTANT, dataVA + instruction->tag2);
			//data ptr/reg operators
			else if (tag == TAGDATAPTRREG)
				instruction->set(ptrfromreg(instruction->tag3), (intptr_t)(new MEMPTR(-1, -1, -1, dataVA + instruction->tag2, true)), instruction->tag3, instruction->tag4);
			//data ptr/constant operators
			else if (istagdataptrconstant(tag))
				instruction->set(ptrfromdataptr(dataptrfromtagconstant(tag)), (intptr_t)(new MEMPTR(-1, -1, -1, dataVA + instruction->tag2, true)), instruction->tag3, instruction->tag4);
			//reg/data ptr operators
			else if (tag == TAGREGDATAPTR)
				instruction->set(instruction->tag3, instruction->tag4, ptrfromreg(instruction->tag3), (intptr_t)(new MEMPTR(-1, -1, -1, dataVA + instruction->tag2, true)));
			//data ptr operators
			else if (istagdataptr(tag))
				instruction->set(ptrfromdataptr(dataptrfromtag(tag)), (intptr_t)(new MEMPTR(-1, -1, -1, dataVA + instruction->tag2, true)));
			//binary data address operators
			else if (tag == TAGOPXDATAADDRESS)
				instruction->set(instruction->tag3, instruction->tag4, TCONSTANT, dataVA + instruction->tag2);
			//function addresses
			else if (tag == TAGOPXCODEADDRESS)
				instruction->set(instruction->tag3, instruction->tag4, TCONSTANT, codeVA + locs[((Function*)(instruction->tagp))->startinstruction]);
			i -= 1;
		}
	}
	delete[] locs;
	delete[] instructionjumps;
	delete[] jpositions;
	text.reserve(codesize);
	//build output string
	for (int i = 0; i < sequencelength; i += 1)
		text.append(instructions[i]->bytes);
}
//build rdata
void buildrdata() {
	rdataRVA = roundup(0x1000 + codesize, 0x1000);
	int IATsize = thunks.length * 4 + 4;
	int ITsize = 0x28;
	int loc = IATsize * 2 + ITsize + 13;
	//add the thunks' RVAs
	ArrayIterator<Thunk*> ait (&thunks);
	for (Thunk* t = ait.getFirst(); ait.hasThis(); t = ait.getNext()) {
		t->rva.assign(to4bytes(rdataRVA + loc));
		loc += t->thunk.length();
		//0x(i*4): RVA for ?? Thunk
		IAT.append(t->rva);
	}
	//0x(IATsize*4-4): required blank space
	IAT.append("\0\0\0\0", 4);
	//Import Address Table
	rdata.append(IAT);
	//Import Table
	//0x00000000: Import Table Lookup RVA
	rdata.append(to4bytes(rdataRVA + ITsize + IATsize));
	rdata.append("\0\0\0\0" "\0\0\0\0", 8);
	//0x0000000A: Name RVA
	rdata.append(to4bytes(rdataRVA + IATsize * 2 + ITsize));
	//0x00000010: Thunk Table RVA
	rdata.append(to4bytes(rdataRVA));
	rdata.append("\0\0\0\0\0\0\0\0\0\0\0\0", 12);
	rdata.append("\0\0\0\0\0\0\0\0", 8);
	//Import Table Lookup
	rdata.append(IAT);
	//Name
	rdata.append("kernel32.dll\0", 13);
	//Thunks
	//assign the thunks their rdata location
	for (Thunk* t = ait.getFirst(); ait.hasThis(); t = ait.getNext()) {
		rdata.append(t->thunk);
	}
	rdatasize = rdata.length();
}
//third stage of compiling, make the file
void buildExecutable() {
	puts("Creating executable...\n");
	datasize = data.length();
	//********Main Header********
	output.append("MZ" "\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 16)
	.append("\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 16)
	.append("\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 16)
	.append("\0\0\0\0\0\0\0\0\0\0\0\0" "\x80\0\0\0", 16)
	.append("\x0E\x1F\xBA\x0E\0\xB4\x09\xCD\x21\xB8\x01\x4C\xCD\x21", 14)
	.append("This program cannot be run in DOS mode.")
	.append("\x0D\x0D\x0A\x24\0\0\0\0\0\0\0", 11)
	//********PE Header********
	.append("PE\0\0" "\x4C\x01" "\x03\0" "\0\0\0\0" "\0\0\0\0", 16)
	.append("\0\0\0\0" "\xE0\0" "\x0F\x01" "\x0B\x01" "\0" "\0", 12)
	//0x0000009C: size of code
	.append(to4bytes(roundup(codesize, 0x200)))
	//0x000000A0: size of initialized data
	.append(to4bytes(roundup(datasize, 0x200)))
	.append("\0\0\0\0" "\0\x10\0\0", 8)
	//0x000000AC: Base of code
	.append("\0\x10\0\0", 4)
	//0x000000B0: Base of data
	.append(to4bytes(rdataRVA))
	.append("\0\0\x40\0" "\0\x10\0\0" "\0\x02\0\0", 12)
	.append("\x04\0" "\0\0" "\0\0" "\0\0" "\x04\0" "\0\0" "\0\0\0\0", 16)
	//0x000000D0: Size of Image
	.append(to4bytes(dataRVA + roundup(datasize, 0x1000)))
	.append("\0\x02\0\0" "\0\0\0\0" "\x03\0" "\0\0", 12)
	.append("\0\0\x10\0" "\0\x10\0\0" "\0\0\x10\0" "\0\x10\0\0", 16)
	.append("\0\0\0\0" "\x10\0\0\0" "\0\0\0\0\0\0\0\0", 16)
	//0x00000100: Import Table location
	.append(to4bytes(rdataRVA + IAT.length()))
	//0x00000104: Import Table size
	.append(to4bytes(0x28))
	.append("\0\0\0\0\0\0\0\0", 8)
	.append("\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0", 16)
	.append("\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0", 16)
	.append("\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0", 16)
	.append("\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0", 16)
	.append("\0\0\0\0\0\0\0\0", 8)
	//0x00000158: Import Address Table location
	.append(to4bytes(rdataRVA))
	//0x0000015C: Import Address Table size
	.append(to4bytes(IAT.length()))
	.append("\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0", 16)
	.append("\0\0\0\0\0\0\0\0", 8)
	//********Section header: text********
	.append(".text\0\0\0", 8)
	//0x00000180: Size of section
	.append(to4bytes(codesize))
	//0x00000184: RVA
	.append("\0\x10\0\0", 4)
	//0x00000188: Size of raw data
	.append(to4bytes(roundup(codesize, 0x200)))
	//0x0000018C: Pointer to raw data
	.append("\0\x02\0\0", 4)
	.append("\0\0\0\0" "\0\0\0\0" "\0\0" "\0\0" "\x20\0\0\x60", 16)
	//********Section header: rdata********
	.append(".rdata\0\0", 8)
	//0x000001A8: Size of section
	.append(to4bytes(rdatasize))
	//0x000001AC: RVA
	.append(to4bytes(rdataRVA))
	//0x000001B0: Size of raw data
	.append(to4bytes(roundup(rdatasize, 0x200)))
	//0x000001B4: Pointer to raw data
	.append(to4bytes(0x200 + roundup(codesize, 0x200)))
	.append("\0\0\0\0" "\0\0\0\0", 8)
	.append("\0\0" "\0\0" "\x40\0\0\x40", 8)
	//********Section header: data********
	.append(".data\0\0\0", 8)
	//0x000001D0: Size of section
	.append(to4bytes(datasize))
	//0x000001D4: RVA
	.append(to4bytes(dataRVA))
	//0x000001D8: Size of raw data
	.append(to4bytes(roundup(datasize, 0x200)))
	//0x000001DC: Pointer to raw data
	.append(to4bytes(0x200 + roundup(codesize, 0x200) + roundup(rdatasize, 0x200)))
	.append("\0\0\0\0" "\0\0\0\0" "\0\0" "\0\0" "\x40\0\0\xC0", 16)
	.append("\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 16)
	//********Section: .text********
	.append(text)
	.append(roundup(codesize, 0x200) - codesize, 0)
	//********Section: .rdata********
	.append(rdata)
	.append(roundup(rdatasize, 0x200) - rdatasize, 0)
	//********Section: .data********
	.append(data)
	.append(roundup(datasize, 0x200) - datasize, 0);
}
//clean up memory
void cleanup() {
	delete[] contents;
	delete[] rows;
	delete[] cols;
	if (initializations != NULL)
		delete[] initializations;
	empty(&code);
	emptyV(&globals);
	ArrayIterator<MainFunction*> aim (&mainfunctions);
	for (MainFunction* m = aim.getFirst(); aim.hasThis(); m = aim.getNext()) {
		Function* f = m->fval;
		AssemblySequence* as = (AssemblySequence*)(f->statements.inner[0]);
		empty(&as->inner);
		delete as;
		if (!m->added) {
			emptyV(&f->params);
			emptyV(&f->variables);
			delete f;
		}
		delete m;
	}
	ArrayIterator<Function*> aif (&functions);
	for (Function* f = aif.getFirst(); aif.hasThis(); f = aif.getNext()) {
		emptyV(&f->params);
		emptyV(&f->variables);
		delete f;
	}
	empty(&tokens);
	ArrayIterator<AssemblySequence*> ais (&sequences);
	for (AssemblySequence* s = ais.getFirst(); ais.hasThis(); s = ais.getNext()) {
		empty(&s->inner);
		delete s;
	}
}
//empty the contents of an array
template <class Type> void empty(Array<type>* a) {
	ArrayIterator<type> ai (a);
	for (type t = ai.getFirst(); ai.hasThis(); t = ai.getNext()) {
		delete t;
	}
}
//empty the contents of a VariableData array
void emptyV(Array<VariableData*>* a) {
	ArrayIterator<VariableData*> ai (a);
	for (VariableData* v = ai.getFirst(); ai.hasThis(); v = ai.getNext()) {
		if (isptr(v->ptype))
			delete (MEMPTR*)(v->ptr);
		delete v;
	}
}
*/
