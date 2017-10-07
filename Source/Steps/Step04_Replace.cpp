#include "Project.h"

//evaluates #if, #replace, and #replace-input

#define deleteArguments(arguments) \
	forEach(AbstractCodeBlock*, aa, arguments, aai) { aa->tokens->clear(); delete aa; } delete arguments;

//evaluate all replacements in all files, using the replacements available per file
void Replace::replaceCodeInFiles(Array<SourceFile*>* files) {
	forEach(SourceFile*, s, files, si) {
		try {
			printf("Replacing code in %s...\n", s->filename.c_str());

			//find all replace directives
			PrefixTrie<char, CDirectiveReplace*> replaces;
			Deleter<Array<AVLNode<SourceFile*, bool>*>> allIncludedEntries (s->includedFiles->entrySet());
			forEach(AVLNode<SourceFile* COMMA bool>*, includedEntry, allIncludedEntries.retrieve(), includedEntryi)
				addReplacesToTrie(includedEntry->key->abstractContents, &replaces);

			replaceTokens(s->abstractContents->tokens, &replaces);
		} catch (...) {
		}
	}
}
//insert any replace directives from the list into the trie, erroring if it's already there
//may throw
void Replace::addReplacesToTrie(AbstractCodeBlock* abstractContents, PrefixTrie<char, CDirectiveReplace*>* replaces) {
	forEach(CDirective*, d, abstractContents->directives, di) {
		CDirectiveReplace* r;
		if ((r = dynamic_cast<CDirectiveReplace*>(d)) == nullptr)
			continue;

		if (replaces->set(r->toReplace->name.c_str(), r->toReplace->name.length(), r) !=
				PrefixTrie<char, CDirectiveReplace*>::emptyValue)
			Error::makeError(ErrorType::General, "replacement has already been defined", r->toReplace);
	}
}
//search through all of the tokens in the file to look for ones to replace
//if we find any, run its replacement
//may throw
void Replace::replaceTokens(Array<Token*>* tokens, PrefixTrie<char, CDirectiveReplace*>* replaces) {
	for (int ti = 0; ti < tokens->length; ti++) {
		Token* fullToken = tokens->get(ti);
		//find the real token
		Token* t = Token::getResultingToken(fullToken);

		AbstractCodeBlock* a;
		Identifier* i;
		CDirectiveReplace* r;
		//if it's an abstract code block, we need to replace all its contents with its replaces considered
		if ((a = dynamic_cast<AbstractCodeBlock*>(t)) != nullptr) {
			if (a->directives != nullptr && a->directives->length > 0) {
				PrefixTrieUnion<char, CDirectiveReplace*> newReplaces (replaces);
				addReplacesToTrie(a, &newReplaces);
				replaceTokens(a->tokens, &newReplaces);
			} else
				replaceTokens(a->tokens, replaces);
			continue;
		//if it's not an identifier to replace, don't do anything
		} else if ((i = dynamic_cast<Identifier*>(t)) == nullptr ||
				(r = replaces->get(i->name.c_str(), i->name.length())) == nullptr)
			continue;

		//if the replace is already in use, that's an error
		if (r->inUse)
			Error::makeError(ErrorType::General, "cannot use replacement in its own body", fullToken);
		Array<Token*>* tokensToInsert;
		if (r->input == nullptr)
			tokensToInsert = buildReplacement(nullptr, r->replacement, nullptr, nullptr, fullToken);
		else {
			//start by collecting the arguments
			ti++;
			if (ti >= tokens->length)
				Error::makeError(ErrorType::General, "expected an input list to follow", fullToken);
			t = tokens->get(ti);
			if ((a = dynamic_cast<AbstractCodeBlock*>(Token::getResultingToken(t))) == nullptr)
				Error::makeError(ErrorType::General, "expected an input list", t);
			Array<AbstractCodeBlock*>* arguments = collectArguments(a, r->input->length, fullToken);
			//replace
			tokensToInsert = buildReplacement(nullptr, r->replacement, arguments, r->input, fullToken);
			//cleanup the tokens
			deleteArguments(arguments);
		}
		r->inUse = true;
		try {
			replaceTokens(tokensToInsert, replaces);
		} catch (...) {
			tokensToInsert->deleteContents();
			delete tokensToInsert;
			r->inUse = false;
			throw;
		}
		//if we had arguments, save them away, making sure we get the full token
		if (a != nullptr)
			a->owningFile->replacedArguments->add(t);
		//insert them into the array, preserving the index relative to the end
		int offsetFromEnd = tokens->length - ti;
		if (r->input == nullptr)
			tokens->replace(ti, 1, tokensToInsert);
		else
			tokens->replace(ti - 1, 2, tokensToInsert);
		ti = tokens->length - offsetFromEnd;
		delete fullToken;
		delete tokensToInsert;
		r->inUse = false;
	}
}
//get a list of arguments from the code block, splitting it around the commas
Array<AbstractCodeBlock*>* Replace::collectArguments(
	AbstractCodeBlock* argumentsCodeBlock, int expectedArgumentCount, Token* errorToken)
{
	Array<Token*>* nextTokens = new Array<Token*>();
	Array<AbstractCodeBlock*>* arguments = new Array<AbstractCodeBlock*>();
	int nextArgumentStartPos = argumentsCodeBlock->contentPos;
	forEach(Token*, at, argumentsCodeBlock->tokens, ati) {
		Separator* s;
		if ((s = dynamic_cast<Separator*>(Token::getResultingToken(at))) != nullptr && s->type == SeparatorType::Comma) {
			arguments->add(new AbstractCodeBlock(
				nextTokens, nullptr, nextArgumentStartPos, s->contentPos, argumentsCodeBlock->owningFile));
			nextArgumentStartPos = s->contentPos + 1;
			nextTokens = new Array<Token*>();
			ati.replaceThis(nullptr);
			delete at;
		} else
			nextTokens->add(at);
	}
	arguments->add(new AbstractCodeBlock(
		nextTokens, nullptr, nextArgumentStartPos, argumentsCodeBlock->endContentPos, argumentsCodeBlock->owningFile));
	if (expectedArgumentCount > 0
		? arguments->length != expectedArgumentCount
		: (arguments->length != 1 || nextTokens->length != 0))
	{
		string message = "expected " + to_string(expectedArgumentCount) + " arguments but got " + to_string(arguments->length);
		//cleanup the tokens
		deleteArguments(arguments);
		Error::makeError(ErrorType::General, message.c_str(), errorToken);
	}
	return arguments;
}
//create a chain of substituted tokens above the original token, based on the parent token being replaced
SubstitutedToken* Replace::substituteTokens(Token* tokenBeingReplaced, Token* resultingToken, bool deleteResultingToken) {
	SubstitutedToken* s;
	if ((s = dynamic_cast<SubstitutedToken*>(tokenBeingReplaced)) != nullptr)
		return new SubstitutedToken(substituteTokens(s->resultingToken, resultingToken, deleteResultingToken), true, s);

	return new SubstitutedToken(resultingToken, deleteResultingToken, tokenBeingReplaced);
}
//recursively clone the replacement body, and substitute any other tokens under the parent token being replaced
//if there is input and a string or identifier matches one of the parameters, replace it
Array<Token*>* Replace::buildReplacement(Array<Token*>* tokensOutput, AbstractCodeBlock* replacementBody,
	Array<AbstractCodeBlock*>* arguments, Array<string>* input, Token* tokenBeingReplaced)
{
	if (tokensOutput == nullptr)
		tokensOutput = new Array<Token*>();
	bool hasInput = input != nullptr && input->length > 0;
	forEach(Token*, t, replacementBody->tokens, ti) {
		AbstractCodeBlock* a;
		StringLiteral* s;
		Identifier* i;
		//abstract code block, recursively replace it
		if ((a = dynamic_cast<AbstractCodeBlock*>(t)) != nullptr) {
			tokensOutput->add(
				substituteTokens(
					tokenBeingReplaced,
					new AbstractCodeBlock(
						buildReplacement(nullptr, a, arguments, input, tokenBeingReplaced),
						nullptr, a->contentPos, a->endContentPos, a->owningFile),
					true));
			continue;
		}
		if (hasInput) {
			if ((s = dynamic_cast<StringLiteral*>(t)) != nullptr) {
				StringLiteral* newS = replaceStringLiteral(s, arguments, input);
				tokensOutput->add(substituteTokens(tokenBeingReplaced, newS, newS != s));
				continue;
			} else if ((i = dynamic_cast<Identifier*>(t)) != nullptr) {
				replaceIdentifier(tokensOutput, i, arguments, input, tokenBeingReplaced);
				continue;
			}
		}
		tokensOutput->add(substituteTokens(tokenBeingReplaced, Token::getResultingToken(t), false));
	}
	return tokensOutput;
}
//replace any inputs with the entire argument including all whitespace and comments
//does not recursively replace
//searches in argument order, so if parameters have similar characters the first one that matches will be chosen
StringLiteral* Replace::replaceStringLiteral(StringLiteral* s, Array<AbstractCodeBlock*>* arguments, Array<string>* input) {
	bool replacedParam = false;
	string val = s->val; //copper: MutableString, initialized to null and gets set which will replace replacedParam
	int valLength = val.length();
	//search every character of the literal
	for (int i = 0; i < valLength; i++) {
		//check each input to see if it matches the current character
		for (int j = 0; j < input->length; j++) {
			string param = input->get(j);
			//we found a match, replace the parameter with the argument
			if (val.compare(i, param.length(), param) == 0) {
				replacedParam = true;
				int offsetFromEnd = valLength - i - param.length() + 1;
				AbstractCodeBlock* argument = arguments->get(j);
				char* contents = &argument->owningFile->contents[argument->contentPos];
				val.replace(i, param.length(), contents, argument->endContentPos - argument->contentPos);
				i = (valLength = val.length()) - offsetFromEnd;
				break;
			}
		}
	}
	return replacedParam ? new StringLiteral(val, s->contentPos, s->endContentPos, s->owningFile) : s;
}
//replace any sections of the identifier that match a parameter with the corresponding arguments
void Replace::replaceIdentifier(Array<Token*>* tokensOutput, Identifier* i, Array<AbstractCodeBlock*>* arguments,
	Array<string>* input, Token* tokenBeingReplaced)
{
	int nameLength = i->name.length();
	SubstitutedToken* newTokenBeingReplaced = substituteTokens(tokenBeingReplaced, i, false);
	Identifier* newI = i;
	//search every character of the identifier
	for (int ni = 0; ni < nameLength; ni++) {
		//check each input to see if it matches the current character
		for (int j = 0; j < input->length; j++) {
			string param = input->get(j);
			//we found a match, replace the parameter with the argument
			if (newI->name.compare(ni, param.length(), param) == 0) {
				int offsetFromEnd = nameLength - ni - param.length() + 1;
				AbstractCodeBlock* argument = arguments->get(j);
				//if the first token in the argument is an identifier, save it
				Identifier* firstI = argument->tokens->length >= 1
					? dynamic_cast<Identifier*>(Token::getResultingToken(argument->tokens->first()))
					: nullptr;
				//first things first, check this special concatenation case
				//if we have no arguments or exactly one argument which is an identifier,
				//	we can join it with any prefix or suffix
				//we need at least 1 of those 3 though- if they're all empty, the regular case will handle it
				//we don't want to add anything to the tokens list yet
				if (argument->tokens->length == 0
						? ni > 0 || ni + (int)(param.length()) < nameLength
						: argument->tokens->length == 1 && firstI != nullptr)
					newTokenBeingReplaced->replaceResultingToken(
						newI = new Identifier(
							newI->name.substr(0, ni) +
								(firstI != nullptr ? firstI->name : "") +
								newI->name.substr(ni + param.length()),
							newI->contentPos, newI->endContentPos, newI->owningFile));
				//if we have a prefix or suffix, they will not be part of the same identifier
				else {
					//if there is prefix before the param we found but we will not be joining identifiers, add the resulting
					//	identifier to the beginning of the list
					if (ni > 0 && firstI == nullptr)
						tokensOutput->add(substituteTokens(
							tokenBeingReplaced,
							new Identifier(newI->name.substr(0, ni), newI->contentPos, newI->endContentPos, newI->owningFile),
							true));
					//insert all the argument tokens in the list
					int prevLength = tokensOutput->length;
					buildReplacement(tokensOutput, argument, nullptr, nullptr, newTokenBeingReplaced);
					//if we have a prefix and an identifier as the first argument, join them
					if (ni > 0 && firstI != nullptr) {
						delete tokensOutput->get(prevLength);
						tokensOutput->set(prevLength, substituteTokens(
							tokenBeingReplaced,
							new Identifier(newI->name.substr(0, ni) + firstI->name,
								newI->contentPos, newI->endContentPos, newI->owningFile),
							true));
					}
					//we have a suffix, join it with an identifier if there is one
					if (ni + (int)(param.length()) < nameLength) {
						Identifier* lastI;
						string lastName = "";
						//there's an identifier, join it
						if (argument->tokens->length > 1 &&
							(lastI = dynamic_cast<Identifier*>(Token::getResultingToken(argument->tokens->last()))) != nullptr)
						{
							delete tokensOutput->pop();
							lastName = lastI->name;
						}
						newTokenBeingReplaced->replaceResultingToken(
							newI = new Identifier(lastName + newI->name.substr(ni + param.length()),
								newI->contentPos, newI->endContentPos, newI->owningFile));
					//we have no suffix, we don't need to do anything except get rid of the identifier
					} else {
						delete newTokenBeingReplaced;
						newTokenBeingReplaced = nullptr;
					}
				}
				ni = (nameLength = newI->name.length()) - offsetFromEnd;
				break;
			}
		}
	}
	if (newTokenBeingReplaced != nullptr)
		tokensOutput->add(newTokenBeingReplaced);
}
