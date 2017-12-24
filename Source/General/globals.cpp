#include "Project.h"

#define instantiateArrayContentDeleter(type) template class ArrayContentDeleter<type>; template class Deleter<Array<type*>>;
#ifdef TRACK_OBJ_IDS
	//**/#define PRINT_OBJ_ADD_OR_REMOVE
#endif

/*
Array<Expression*> tokens;
int tlength;
Array<Function*> functions;
Array<MainFunction*> mainfunctions;

bool warnings = false;
int* rows;
int* cols;

int codesize;
int rdataRVA;
int rdatasize;
int dataRVA;
int datasize;
string data (
	//4 all-purpose variables
	"\0\0\0\0"
	"\0\0\0\0"
	"\0\0\0\0"
	"\0\0\0\0"
	//main process heap
	"\0\0\0\0"
	//current heap location
	"\0\0\0\0"
	//current spot in current heap
	"\0\0\0\0"
	//current heap size
	"\0\0\0\0"
	//new heap location
	"\0\0\0\0"
	//last try statement address
	"\0\0\0\0", 40);
Thunk TExitProcess ("ExitProcess", 0x9B);
Thunk TGetStdHandle ("GetStdHandle", 0x16A);
Thunk TWriteFile ("WriteFile", 0x2F7);
Thunk TGetProcessHeap ("GetProcessHeap", 0x156);
Thunk THeapAlloc ("HeapAlloc", 0x1BD);
Thunk THeapReAlloc ("HeapReAlloc", 0x1C4);
*/

template class Deleter<AbstractCodeBlock>;
template class Deleter<BigInt>;
template class Deleter<DirectiveTitle>;
template class Deleter<ExpressionStatement>;
template class Deleter<Identifier>;
template class Deleter<IntConstant>;
template class Deleter<LexToken>;
template class Deleter<Separator>;
template class Deleter<Token>;
template class Deleter<VariableInitialization>;
template class Deleter<Array<AVLNode<SourceFile*, bool>*>>;
template class Deleter<Array<string>>;
template class Deleter<PrefixTrie<char, CDirectiveReplace*>>;
template class Deleter<PrefixTrieUnion<char, CDirectiveReplace*>>;
instantiateArrayContentDeleter(CVariableDefinition);
instantiateArrayContentDeleter(Statement);
instantiateArrayContentDeleter(Token);

int Math::min(int a, int b) {
	return a < b ? a : b;
}
int Math::max(int a, int b) {
	return a > b ? a : b;
}
#ifdef DEBUG
	#ifdef TRACK_OBJ_IDS
		ObjCounter* ObjCounter::headObjCounter = nullptr;
		ObjCounter* ObjCounter::tailObjCounter = nullptr;
	#endif
	int ObjCounter::objCount = 0;
	int ObjCounter::untrackedObjCount;
	int ObjCounter::nextObjID = 0;
	ObjCounter::ObjCounter(onlyWhenTrackingIDs(char* pObjType))
	#ifdef TRACK_OBJ_IDS
		: objType(pObjType)
		, objID(nextObjID)
		, prev(tailObjCounter)
		, next(nullptr)
	#endif
	{
		objCount++;
		nextObjID++;

		#ifdef PRINT_OBJ_ADD_OR_REMOVE
			printf("  Added %s\t%d\t%p,    obj count: %d\n", objType, objID, this, objCount);
		#endif
		#ifdef TRACK_OBJ_IDS
			if (tailObjCounter != nullptr)
				tailObjCounter->next = this;
			tailObjCounter = this;
			if (headObjCounter == nullptr)
				headObjCounter = this;
		#endif
	}
	ObjCounter::~ObjCounter() {
		objCount--;
		#ifdef PRINT_OBJ_ADD_OR_REMOVE
			printf("Deleted %s\t%d\t%p,    obj count: %d\n", objType, objID, this, objCount);
		#endif
		#ifdef TRACK_OBJ_IDS
			if (next != nullptr)
				next->prev = prev;
			else if (this == tailObjCounter)
				tailObjCounter = prev;
			if (prev != nullptr)
				prev->next = next;
			else if (this == headObjCounter)
				headObjCounter = next;
		#endif
	}
	//assign the initial object ID to exclude any statically-allocated ObjCounters
	void ObjCounter::start() {
		untrackedObjCount = nextObjID;
		#ifdef TRACK_OBJ_IDS
			//reset the list, it's ok to leave any objects linked
			headObjCounter = nullptr;
			tailObjCounter = nullptr;
		#endif
	}
	//check for any non-deallocated objects
	void ObjCounter::end() {
		#ifdef TRACK_OBJ_IDS
			for (; headObjCounter != nullptr; headObjCounter = headObjCounter->next)
				printf("      Remaining object: %s\t%d\t%p\n", headObjCounter->objType, headObjCounter->objID, headObjCounter);
		#endif
		printf("Total remaining objects: %d\n", objCount - untrackedObjCount);
		printf("Total objects used: %d + %d untracked\n", (nextObjID - untrackedObjCount), untrackedObjCount);
	}
#endif
template <class Type> Deleter<Type>::Deleter(Type* pToDelete)
: onlyInDebug(ObjCounter(onlyWhenTrackingIDs("DELETER")) COMMA)
toDelete(pToDelete) {
}
template <class Type> Deleter<Type>::~Deleter() {
	delete toDelete;
}
//return the held object without releasing it
template <class Type> Type* Deleter<Type>::retrieve() {
	return toDelete;
}
//return the held object and release it so it won't be deleted
template <class Type> Type* Deleter<Type>::release() {
	Type* val = toDelete;
	toDelete = nullptr;
	return val;
}
template <class Type> ArrayContentDeleter<Type>::ArrayContentDeleter(Array<Type*>* pToDelete)
: Deleter<Array<Type*>>(pToDelete) {
}
template <class Type> ArrayContentDeleter<Type>::~ArrayContentDeleter() {
	if (toDelete != nullptr)
		toDelete->deleteContents();
}
//log an error and throw
void Error::makeError(ErrorType type, const char* message, Token* token) {
	logError(type, message, token);
	throw 0;
}
//add an error to the error list without throwing
void Error::logError(ErrorType type, const char* message, Token* token) {
	token->owningFile->owningPliers->errorMessages->add(buildErrorMessage(type, message, token));
}
//build an error message list from the given input
ErrorMessage* Error::buildErrorMessage(ErrorType type, const char* message, Token* token) {
	return new ErrorMessage(
		type,
		message,
		token->owningFile,
		type == ErrorType::ExpectedToFollow ? token->endContentPos : token->contentPos,
		token->replacementSource != nullptr
			? buildErrorMessage(ErrorType::Continuation, "", token->replacementSource)
			: nullptr);
}
ErrorMessage::ErrorMessage(
	ErrorType pType, const char* pMessage, SourceFile* pOwningFile, int pContentPos, ErrorMessage* pContinuation)
: onlyInDebug(ObjCounter(onlyWhenTrackingIDs("ERORMSG")) COMMA)
type(pType)
, message(nullptr)
, owningFile(pOwningFile)
, contentPos(pContentPos)
, continuation(pContinuation) {
	int messageLength = strlen(pMessage) + 1;
	char* val = new char[messageLength];
	memcpy((void*)val, pMessage, messageLength);
	val[messageLength - 1] = '\0';
	message = val;
}
ErrorMessage::~ErrorMessage() {
	delete message;
	delete continuation;
}
char* ErrorMessage::snippet = []() -> char* {
	char* val = new char[SNIPPET_PREFIX_SPACES + SNIPPET_CHARS + 1 + SNIPPET_PREFIX_SPACES + SNIPPET_CHARS / 2 + 2];
	memset(val, ' ', SNIPPET_PREFIX_SPACES);
	val[SNIPPET_PREFIX_SPACES + SNIPPET_CHARS] = '\n';
	memset(val + SNIPPET_PREFIX_SPACES + SNIPPET_CHARS + 1, ' ', SNIPPET_PREFIX_SPACES + SNIPPET_CHARS / 2);
	val[SNIPPET_PREFIX_SPACES + SNIPPET_CHARS + 1 + SNIPPET_PREFIX_SPACES + SNIPPET_CHARS / 2] = '^';
	val[SNIPPET_PREFIX_SPACES + SNIPPET_CHARS + 1 + SNIPPET_PREFIX_SPACES + SNIPPET_CHARS / 2 + 1] = '\0';
	return val;
}();
//print the error of this error message
void ErrorMessage::printError() {
	int row = owningFile->getRow(contentPos);
	//print the error
	char* errorPrefix;
	switch (type) {
		case ErrorType::Continuation: errorPrefix = "  --- in \"%s\" at line %d char %d\n"; break;
		default:                      errorPrefix = "Error in \"%s\" at line %d char %d: "; break;
	}
	printf(errorPrefix, owningFile->filename.c_str(), row + 1, contentPos - owningFile->rowStarts->get(row) + 1);
	switch (type) {
		case ErrorType::General: puts(message); break;
		case ErrorType::EndOfFileWhileSearching: printf("reached the end of the file while searching for %s\n", message); break;
		case ErrorType::EndOfFileWhileReading: printf("reached the end of the file while reading %s\n", message); break;
		case ErrorType::Expected: printf("expected %s\n", message); break;
		case ErrorType::ExpectedToFollow: printf("expected %s to follow\n", message); break;
		case ErrorType::Continuation: break;
	}
	showSnippet();
	if (continuation != nullptr)
		continuation->printError();
}
//show the snippet where the error/warning is
void ErrorMessage::showSnippet() {
	int targetStart = contentPos - SNIPPET_CHARS / 2;
	int targetEnd = targetStart + SNIPPET_CHARS;
	int start = Math::max(targetStart, 0);
	int end = Math::min(targetEnd, owningFile->contentsLength);
	memset(snippet + SNIPPET_PREFIX_SPACES, ' ', start - targetStart);
	memset(snippet + SNIPPET_PREFIX_SPACES + end - targetStart, ' ', targetEnd - end);
	for (int i = start; i < end; i++) {
		char c = owningFile->contents[i];
		//make sure c is in the printable character range
		snippet[i - targetStart + SNIPPET_PREFIX_SPACES] = (c >= '!' && c <= '~') ? c : ' ';
	}
	puts(snippet);
}
#ifdef DEBUG
	//recursively print the contents of the abstract code block
	void Debug::printAbstractCodeBlock(AbstractCodeBlock* codeBlock, int tabsCount) {
		if (codeBlock->directives != nullptr && codeBlock->directives->length > 0)
			printf(" -- %d directives\n", codeBlock->directives->length);
		else
			printf("\n");
		bool printedSpaces = false;
		forEach(Token*, t, codeBlock->tokens, ti) {
			DirectiveTitle* dt;
			if ((dt = dynamic_cast<DirectiveTitle*>(t)) != nullptr && printedSpaces) {
				printf("\n");
				printedSpaces = false;
			}
			if (printedSpaces)
				printf(" ");
			else {
				for (int j = 0; j < tabsCount; j++)
					printf("    ");
				printedSpaces = true;
			}
			AbstractCodeBlock* a;
			if ((a = dynamic_cast<AbstractCodeBlock*>(t)) != nullptr) {
				if (a->tokens->length == 0) {
					printf("()");
				} else {
					printf("(");
					printAbstractCodeBlock(a, tabsCount + 1);
					for (int i = 0; i < tabsCount; i++)
						printf("    ");
					printf(")");
				}
			} else {
				printLexToken(t);
				Separator* separator;
				if ((separator = dynamic_cast<Separator*>(t)) != nullptr &&
					separator->separatorType == SeparatorType::Semicolon)
				{
					printf("\n");
					printedSpaces = false;
				}
			}
			if (dt != nullptr) {
				printf("\n");
				printedSpaces = false;
			}
		}
		if (printedSpaces)
			printf("\n");
	}
	//recursively print the contents of the token tree
	void Debug::printTokenTree(Token* t, int tabsCount, bool printOperatorParentheses) {
		VariableInitialization* v;
		Operator* o;
		ParenthesizedExpression* p;
		FunctionCall* fc;
		FunctionDefinition* fd;
		if ((v = dynamic_cast<VariableInitialization*>(t)) != nullptr) {
			bool printComma = false;
			CDataType* lastDataType = nullptr;
			forEach(CVariableDefinition*, d, v->variables, di) {
				if (printComma)
					printf(", ");
				else
					printComma = true;
				if (d->type != lastDataType) {
					printf(d->type->name.c_str());
					printf(" ");
					lastDataType = d->type;
				}
				printf(d->name->name.c_str());
			}
			if (v->initialization != nullptr) {
				printf(" = ");
				printTokenTree(v->initialization, tabsCount, false);
			}
		} else if ((o = dynamic_cast<Operator*>(t)) != nullptr) {
			Cast* c;
			if ((c = dynamic_cast<Cast*>(t)) != nullptr) {
				printf("(");
				if (c->isRaw)
					printf("raw ");
				printf(c->dataType->name.c_str());
				printf(")(");
				printTokenTree(c->right, tabsCount, false);
				printf(")");
			} else {
				if (printOperatorParentheses)
					printf("(");
				if (o->left != nullptr)
					printTokenTree(o->left, tabsCount, true);
				if (o->precedence < OperatorTypePrecedence::Prefix) {
					printf(" ");
					printLexToken(o);
					printf(" ");
				} else
					printLexToken(o);
				if (o->right != nullptr)
					printTokenTree(o->right, tabsCount, true);
				if (printOperatorParentheses)
					printf(")");
			}
		} else if ((p = dynamic_cast<ParenthesizedExpression*>(t)) != nullptr)
			printTokenTree(p->expression, tabsCount, true);
		else if ((fc = dynamic_cast<FunctionCall*>(t)) != nullptr) {
			printTokenTree(fc->function, tabsCount, true);
			printf("(");
			bool printComma = false;
			forEach(Token*, argument, fc->arguments, ai) {
				if (printComma)
					printf(", ");
				else
					printComma = true;
				printTokenTree(argument, tabsCount, false);
			}
			printf(")");
		} else if ((fd = dynamic_cast<FunctionDefinition*>(t)) != nullptr) {
			printf(fd->returnType->name.c_str());
			printf("(");
			bool printComma = false;
			forEach(CVariableDefinition*, parameter, fd->parameters, pi) {
				if (printComma)
					printf(", ");
				else
					printComma = true;
				printf(parameter->type->name.c_str());
				printf(" ");
				printf(parameter->name->name.c_str());
			}
			printf(") (\n");
			forEach(Statement*, s, fd->body, si) {
				printStatement(s, tabsCount + 1);
			}
			for (int i = 0; i < tabsCount; i++)
				printf("    ");
			printf(")");
		} else
			printLexToken(t);
	}
	//print the contents of the statement list
	//if a statement continuation follows, add appropriate whitespace so that it can immediately follow
	//if not, the statement is over and we need to end on a new line
	void Debug::printStatementList(Array<Statement*>* a, int tabsCount, bool statementContinuationFollows) {
		if (a->length == 1 &&
			(dynamic_cast<ExpressionStatement*>(a->get(0)) != nullptr ||
				dynamic_cast<ReturnStatement*>(a->get(0)) != nullptr ||
				dynamic_cast<LoopControlFlowStatement*>(a->get(0)) != nullptr))
		{
			printf("\n");
			printStatement(a->get(0), tabsCount + 1);
			if (statementContinuationFollows) {
				for (int i = 0; i < tabsCount; i++)
					printf("    ");
			}
		} else {
			printf(" (\n");
			forEach(Statement*, s, a, si) {
				printStatement(s, tabsCount + 1);
			}
			for (int i = 0; i < tabsCount; i++)
				printf("    ");
			printf(")");
			if (statementContinuationFollows)
				printf(" ");
			else
				printf("\n");
		}
	}
	//print the contents of the statement, including a newline after
	void Debug::printStatement(Statement* s, int tabsCount) {
		for (int i = 0; i < tabsCount; i++)
			printf("    ");
		ExpressionStatement* e;
		ReturnStatement* r;
		IfStatement* i;
		LoopStatement* l;
		LoopControlFlowStatement* c;
		if ((e = dynamic_cast<ExpressionStatement*>(s)) != nullptr) {
			printTokenTree(e->expression, tabsCount, false);
			printf(";\n");
		} else if ((r = dynamic_cast<ReturnStatement*>(s)) != nullptr) {
			printf("return");
			if (r->expression) {
				printf(" ");
				printTokenTree(r->expression, tabsCount, false);
			}
			printf(";\n");
		} else if ((i = dynamic_cast<IfStatement*>(s)) != nullptr) {
			//loop for multiple if statements
			while (true) {
				printf("if (");
				printTokenTree(i->condition, tabsCount, false);
				printf(")");
				Array<Statement*>* elseBody = i->elseBody;
				printStatementList(i->thenBody, tabsCount, elseBody != nullptr);
				if (elseBody != nullptr) {
					printf("else");
					if (i->elseBody->length == 1 && (i = dynamic_cast<IfStatement*>(elseBody->get(0))) != nullptr) {
						printf(" ");
						continue;
					} else
						printStatementList(i->elseBody, tabsCount, false);
				}
				break;
			}
		} else if ((l = dynamic_cast<LoopStatement*>(s)) != nullptr) {
			if (l->initialization != nullptr || l->increment != nullptr) {
				printf("for (");
				if (l->initialization != nullptr)
					printTokenTree(l->initialization->expression, tabsCount, false);
				printf("; ");
				printTokenTree(l->condition, tabsCount, false);
				printf("; ");
				if (l->increment != nullptr)
					printTokenTree(l->increment, tabsCount, false);
				printf(")");
				printStatementList(l->body, tabsCount, false);
			} else if (l->initialConditionCheck) {
				printf("while (");
				printTokenTree(l->condition, tabsCount, false);
				printf(")");
				printStatementList(l->body, tabsCount, false);
			} else {
				printf("do");
				printStatementList(l->body, tabsCount, true);
				printf("while (");
				printTokenTree(l->condition, tabsCount, false);
				printf(")\n");
			}
		} else if ((c = dynamic_cast<LoopControlFlowStatement*>(s)) != nullptr) {
			printf(c->continueLoop ? "continue" : "break");
			if (c->levels != nullptr) {
				printf(" ");
				printLexToken(c->levels);
			}
			printf(";\n");
		} else if (dynamic_cast<EmptyStatement*>(s) != nullptr)
			;
		else {
			assert(false);
		}
	}
	//if it's an identifier or a string, it might have come from a replace, so print its contents
	//if it's anything else, use the contentPos and endContentPos to print this token
	void Debug::printLexToken(Token* t) {
		assert(dynamic_cast<LexToken*>(t) != nullptr);
		Identifier* i;
		StringLiteral* s;
		if ((i = dynamic_cast<Identifier*>(t)) != nullptr)
			printf(i->name.c_str());
		else if ((s = dynamic_cast<StringLiteral*>(t)) != nullptr)
			printf("\"%s\"", s->val.c_str());
		else {
			char* contents = t->owningFile->contents;
			char old = contents[t->endContentPos];
			contents[t->endContentPos] = '\0';
			printf(contents + t->contentPos);
			contents[t->endContentPos] = old;
		}
	}
	void Debug::crashProgram() {
		*((char*)(nullptr)) = 0;
		std::exit(0);
		while (true) {}
	}
#endif
/*
//make a warning
void makeWarning(int type, char* message, size_t loc) {
	printf("Warning at line %d column %d: ", rows[loc], cols[loc]);
	if (type == 0)
		printf("%s\n", message);
	warnings = true;
	showSnippet(loc);
}
//build all of the Main. functions
void buildMainFunctions(Array<MainFunction*>* mainfunctions) {
	//Main.exit(int);
	Function* f = new Function(TVOID);
	f->params.add(new VariableData("", TINT, new MEMPTR(ESP, 0, -1, 4, false), 0));
	AssemblySequence* as = new AssemblySequence(new PUSH(TDWORDPTR, f->params.inner[0]->ptr));
	as->inner.add(new CALL(&TExitProcess));
	f->statements.add(as);
	mainfunctions->add(new MainFunction(f, "Main.exit", 0));
	//Main.print(String);
	f = new Function(TVOID);
	f->params.add(new VariableData("", TSTRING, new MEMPTR(ESP, 0, -1, 4, false), 0));
	as = new AssemblySequence(new MOV(TREG32, EAX, TDWORDPTR, f->params.inner[0]->ptr));
	as->inner.add(new PUSH(TCONSTANT, 0))
		->add(new PUSH(TDATAADDRESS, VDATAGENERAL))
		//string size
		->add(new PUSH(TDWORDPTR, (intptr_t)(new MEMPTR(EAX, 0, -1, STRING_LENGTH, true))))
		//string address
		->add(new PUSH(TDWORDPTR, (intptr_t)(new MEMPTR(EAX, 0, -1, STRING_ADDR, true))))
		//push STD_OUTPUT_HANDLE
		->add(new PUSH(TCONSTANT, -0x0B))
		->add(new CALL(&TGetStdHandle))
		->add(new PUSH(TREG32, EAX))
		->add(new CALL(&TWriteFile))
		->add(new RET(4));
	f->statements.add(as);
	mainfunctions->add(new MainFunction(f, "Main.print", 0));
	//Main.str(int)
	f = new Function(TSTRING);
	f->params.add(new VariableData("", TINT, new MEMPTR(ESP, 0, -1, 4, false), 0));
	as = new AssemblySequence(new MOV(TREG32, EAX, TDWORDPTR, f->params.inner[0]->ptr));
	as->inner.add(new MOV(TREG32, ESI, TREG32, ESP))
		//load 10 in the divisor
		->add(new MOV(TREG32, ECX, TCONSTANT, 10))
		//check if the number is negative
		->add(new CMP(TREG32, EAX, TCONSTANT, 0))
		->add(new JGE(TINSTRUCTIONRELATIVE, 7))
		//indicate negative
		->add(new MOV(TREG8, BL, TCONSTANT, 1))
		//get the lowest digit, negate everything, then jump to regular processing
		->add(new CDQ())
		->add(new IDIV(TREG32, ECX))
		->add(new NEG(TREG32, EAX))
		->add(new NEG(TREG8, DL))
		->add(new JMP(TINSTRUCTIONRELATIVE, 4))
		//indicate positive
		->add(new MOV(TREG8, BL, TCONSTANT, 0))
		//begin converting
		->add(new CDQ())
		->add(new IDIV(TREG32, ECX))
		->add(new ADD(TREG8, DL, TCONSTANT, 48))
		->add(new DEC(TREG32, ESI))
		->add(new MOV(TBYTEPTR, (intptr_t)(new MEMPTR(ESI, 0, -1, 0, true)), TREG8, DL))
		//jump back if it's greater than 0
		->add(new CMP(TREG32, EAX, TCONSTANT, 0))
		->add(new JG(TINSTRUCTIONRELATIVE, -6))
		//add a minus sign if it's negative
		->add(new CMP(TREG8, BL, TCONSTANT, 0))
		->add(new JE(TINSTRUCTIONRELATIVE, 3))
		->add(new DEC(TREG32, ESI))
		->add(new MOV(TBYTEPTR, (intptr_t)(new MEMPTR(ESI, 0, -1, 0, true)), TCONSTANT, 45))
		//begin moving the string
		//get the heap address
		->add(new MOV(TREG32, EAX, TDATADWORDPTR, VDATACURRENTHEAPSPOT))
		//load the byte array address onto the heap
		->add(new LEA(TREG32, EDI, TDWORDPTR, (intptr_t)(new MEMPTR(EAX, 0, -1, 8, true))))
		->add(new MOV(TDWORDPTR, (intptr_t)(new MEMPTR(EAX, 0, -1, STRING_ADDR, true)), TREG32, EDI))
		//load the string size
		->add(new MOV(TREG32, ECX, TREG32, ESP))
		->add(new SUB(TREG32, ECX, TREG32, ESI))
		->add(new MOV(TDWORDPTR, (intptr_t)(new MEMPTR(EAX, 0, -1, STRING_LENGTH, true)), TREG32, ECX))
		//bump up the heap
		->add(new ADD(TDATADWORDPTR, VDATACURRENTHEAPSPOT, TCONSTANT, 8))
		->add(new ADD(TDATADWORDPTR, VDATACURRENTHEAPSPOT, TREG32, ECX))
		//move the string
		->add(new REPMOVSB())
		->add(new RET(4));
	f->statements.add(as);
	mainfunctions->add(new MainFunction(f, "Main.str", 0));
}
*/
