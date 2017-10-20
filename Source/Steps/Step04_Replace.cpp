#include "Project.h"

//evaluates #if, #replace, and #replace-input

#define deleteArguments(arguments) \
	forEach(AbstractCodeBlock*, aa, arguments, aai) { aa->tokens->clear(); delete aa; } delete arguments;

//evaluate all replacements in all files, using the replacements available per file
void Replace::replaceCodeInFiles(Pliers* pliers) {
	forEach(SourceFile*, s, pliers->allFiles, si) {
		try {
			if (pliers->printProgress)
				printf("Replacing code in %s...\n", s->filename.c_str());

			//find all replace directives
			PrefixTrie<char, CDirectiveReplace*> replaces;
			Deleter<Array<AVLNode<SourceFile*, bool>*>> allIncludedEntries (s->includedFiles->entrySet());
			forEach(AVLNode<SourceFile* COMMA bool>*, includedEntry, allIncludedEntries.retrieve(), includedEntryi) {
				Array<CDirective*>* directives = includedEntry->key->abstractContents->directives;
				if (directives != nullptr)
					addReplacesToTrie(directives, &replaces);
			}

			replaceTokens(s->abstractContents->tokens, &replaces);
		} catch (...) {
		}
	}
}
//insert any replace directives from the list into the trie, erroring if it's already there
//may throw
void Replace::addReplacesToTrie(Array<CDirective*>* directives, PrefixTrie<char, CDirectiveReplace*>* replaces) {
	forEach(CDirective*, d, directives, di) {
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
	bool shouldBacktrack = false;
	for (int ti = 0; ti < tokens->length; ti++) {
		Token* t = tokens->get(ti);
		AbstractCodeBlock* a;
		Identifier* i;
		CDirectiveReplace* r;
		//if it's an abstract code block, we need to replace all its contents with its replaces considered
		//this is definitely not arguments for backtracking since it would have been used already
		if ((a = dynamic_cast<AbstractCodeBlock*>(t)) != nullptr) {
			Array<CDirective*>* directives = a->directives;
			if (directives != nullptr && directives->length > 0) {
				PrefixTrieUnion<char, CDirectiveReplace*> newReplaces (replaces);
				addReplacesToTrie(directives, &newReplaces);
				replaceTokens(a->tokens, &newReplaces);
			} else
				replaceTokens(a->tokens, replaces);
			continue;
		//if it's not an identifier to replace, don't do anything
		} else if ((i = dynamic_cast<Identifier*>(t)) == nullptr ||
			(r = replaces->get(i->name.c_str(), i->name.length())) == nullptr)
		{
			//if we were expecting an arguments list, we didn't get it
			if (shouldBacktrack)
				Error::makeError(ErrorType::Expected, "a replace-input argument list", t);
			continue;
		}

		//if the replace is already in use, that's an error
		if (r->inUse)
			Error::makeError(ErrorType::General, "cannot use replacement in its own body", i);
		Array<Token*>* tokensToInsert = new Array<Token*>();
		ArrayContentDeleter<Token> tokensToInsertDeleter (tokensToInsert);
		if (r->input == nullptr)
			buildReplacement(tokensToInsert, r->replacement, nullptr, nullptr, i);
		else {
			//start by collecting the arguments
			ti++;
			if (ti >= tokens->length)
				Error::makeError(ErrorType::ExpectedToFollow, "a replace-input argument list", i);
			t = tokens->get(ti);
			//we didn't see any arguments after, but maybe that's because they're behind another replace
			if ((a = dynamic_cast<AbstractCodeBlock*>(t)) == nullptr) {
				//if we already expected to backtrack, we already missed an input list so this is now really an error
				if (shouldBacktrack)
					Error::makeError(ErrorType::Expected, "a replace-input argument list", t);
				else {
					shouldBacktrack = true;
					ti--;
					continue;
				}
			}
			Array<AbstractCodeBlock*>* arguments = collectArguments(a, r->input->length, i);
			//replace
			buildReplacement(tokensToInsert, r->replacement, arguments, r->input, i);
			//cleanup the tokens
			deleteArguments(arguments);
		}
		r->inUse = true;
		try {
			replaceTokens(tokensToInsert, replaces);
		} catch (...) {
			r->inUse = false;
			throw;
		}
		//if we had arguments, save them away, making sure we get the full token
		if (a != nullptr)
			a->owningFile->replacedTokens->add(a);
		i->owningFile->replacedTokens->add(i);
		//insert them into the array, preserving the index relative to the end
		int offsetFromEnd = tokens->length - ti;
		if (r->input == nullptr)
			tokens->replace(ti, 1, tokensToInsert);
		else
			tokens->replace(ti - 1, 2, tokensToInsert);
		//if we were expecting replacement arguments, backtrack now
		if (shouldBacktrack) {
			ti -= 2;
			shouldBacktrack = false;
		} else
			ti = tokens->length - offsetFromEnd;
		tokensToInsert->clear();
		r->inUse = false;
	}
}
//get a list of arguments from the code block, splitting it around the commas
Array<AbstractCodeBlock*>* Replace::collectArguments(
	AbstractCodeBlock* argumentsCodeBlock, int expectedArgumentCount, Identifier* errorToken)
{
	Array<Token*>* nextTokens = new Array<Token*>();
	Array<AbstractCodeBlock*>* arguments = new Array<AbstractCodeBlock*>();
	int nextArgumentStartPos = argumentsCodeBlock->contentPos + 1;
	forEach(Token*, t, argumentsCodeBlock->tokens, ati) {
		Separator* s;
		if ((s = dynamic_cast<Separator*>(t)) != nullptr && s->separatorType == SeparatorType::Comma) {
			arguments->add(new AbstractCodeBlock(
				nextTokens, nullptr, nextArgumentStartPos, s->contentPos, argumentsCodeBlock->owningFile));
			nextArgumentStartPos = s->endContentPos;
			nextTokens = new Array<Token*>();
			ati.replaceThis(nullptr);
			delete s;
		} else
			nextTokens->add(t);
	}
	arguments->add(new AbstractCodeBlock(
		nextTokens, nullptr, nextArgumentStartPos, argumentsCodeBlock->endContentPos - 1, argumentsCodeBlock->owningFile));
	if (expectedArgumentCount > 0
		? arguments->length != expectedArgumentCount
		: (arguments->length != 1 || nextTokens->length != 0))
	{
		string message = to_string(expectedArgumentCount) + " arguments but got " + to_string(arguments->length);
		//cleanup the tokens
		deleteArguments(arguments);
		Error::makeError(ErrorType::Expected, message.c_str(), errorToken);
	}
	return arguments;
}
//recursively clone the replacement body, and substitute any other tokens under the parent token being replaced
//if there is input and a string or identifier matches one of the parameters, replace it
void Replace::buildReplacement(Array<Token*>* tokensOutput, AbstractCodeBlock* replacementBody,
	Array<AbstractCodeBlock*>* arguments, Array<string>* input, Identifier* replacementSource)
{
	bool hasInput = input != nullptr && input->length > 0;
	forEach(Token*, t, replacementBody->tokens, ti) {
		AbstractCodeBlock* a;
		//abstract code block, recursively replace it
		if ((a = dynamic_cast<AbstractCodeBlock*>(t)) != nullptr) {
			Array<Token*>* replacementTokens = new Array<Token*>();
			buildReplacement(replacementTokens, a, arguments, input, replacementSource);
			tokensOutput->add(new AbstractCodeBlock(replacementTokens, a, replacementSource));
			continue;
		}
		if (hasInput) {
			StringLiteral* s;
			Identifier* i;
			if ((s = dynamic_cast<StringLiteral*>(t)) != nullptr) {
				tokensOutput->add(replaceStringLiteral(s, arguments, input, replacementSource));
				continue;
			} else if ((i = dynamic_cast<Identifier*>(t)) != nullptr) {
				replaceIdentifier(tokensOutput, i, arguments, input, replacementSource);
				continue;
			}
		}
		LexToken* l = dynamic_cast<LexToken*>(t);
		assert(l != nullptr);
		tokensOutput->add(l->cloneWithReplacementSource(replacementSource));
	}
}
//replace any inputs with the entire argument including all whitespace and comments
//does not recursively replace
//searches in argument order, so if parameters have similar characters the first one that matches will be chosen
StringLiteral* Replace::replaceStringLiteral(
	StringLiteral* s, Array<AbstractCodeBlock*>* arguments, Array<string>* input, Identifier* replacementSource)
{
	string val = s->val; //copper: MutableString, initialized to null and gets set which will replace replacedParam
	int valLength = val.length();
	//search every character of the literal
	for (int i = 0; i < valLength; i++) {
		//check each input to see if it matches the current character
		for (int j = 0; j < input->length; j++) {
			string param = input->get(j);
			//we found a match, replace the parameter with the argument
			if (val.compare(i, param.length(), param) == 0) {
				int offsetFromEnd = valLength - i - param.length() + 1;
				AbstractCodeBlock* argument = arguments->get(j);
				char* contents = &argument->owningFile->contents[argument->contentPos];
				val.replace(i, param.length(), contents, argument->endContentPos - argument->contentPos);
				i = (valLength = val.length()) - offsetFromEnd;
				break;
			}
		}
	}
	s = new StringLiteral(val, s->contentPos, s->endContentPos, s->owningFile);
	s->replacementSource = replacementSource;
	return s;
}
//replace any sections of the identifier that match a parameter with the corresponding arguments
void Replace::replaceIdentifier(Array<Token*>* tokensOutput, Identifier* i, Array<AbstractCodeBlock*>* arguments,
	Array<string>* input, Identifier* replacementSource)
{
	int nameLength = i->name.length();
	Identifier* newI = i->cloneWithReplacementSource(replacementSource);
	bool cloneOfReplacementBody = true;
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
					? dynamic_cast<Identifier*>(argument->tokens->first())
					: nullptr;
				//first things first, check this special concatenation case
				//if we have no arguments or exactly one argument which is an identifier,
				//	we can join it with any prefix or suffix
				//we don't want to add anything to the tokens list yet
				if (argument->tokens->length == 0
					//if we have no argument, make sure we have a prefix or a suffix
					? ni > 0 || ni + (int)(param.length()) < nameLength
					: argument->tokens->length == 1 && firstI != nullptr)
				{
					string newName = newI->name.substr(0, ni) +
						(firstI != nullptr ? firstI->name : "") +
						newI->name.substr(ni + param.length());
					//if the identifier points to the replacement body, create a new one at the argument pointing back to it
					if (cloneOfReplacementBody) {
						Token* source = firstI != nullptr ? (Token*)firstI : (Token*)argument;
						Identifier* oldI = newI;
						newI = new Identifier(newName, source->contentPos, source->endContentPos, source->owningFile);
						newI->replacementSource = oldI;
						//make sure that the original one is tracked
						oldI->owningFile->replacedTokens->add(oldI);
						cloneOfReplacementBody = false;
					} else
						newI->name = newName;
				//if we have a prefix or suffix, they will not be part of the same identifier
				} else {
					//if there is prefix before the param we found but we will not be joining identifiers, add the resulting
					//	identifier to the beginning of the list
					if (ni > 0 && firstI == nullptr) {
						Identifier* prefix =
							new Identifier(newI->name.substr(0, ni), newI->contentPos, newI->endContentPos, newI->owningFile);
						prefix->replacementSource = newI->replacementSource;
						tokensOutput->add(prefix);
					}
					//insert all the argument tokens in the list
					int prevLength = tokensOutput->length;
					buildReplacement(
						tokensOutput, argument, nullptr, nullptr, cloneOfReplacementBody ? newI : newI->replacementSource);
					//if the first argument is an identifier and we have a prefix, prepend it to the name
					if (ni > 0 && firstI != nullptr) {
						firstI = dynamic_cast<Identifier*>(tokensOutput->get(prevLength));
						assert(firstI != nullptr);
						firstI->name = newI->name.substr(0, ni) + firstI->name;
					}
					//we have a suffix, prepend the name of an identifier if there is one
					if (ni + (int)(param.length()) < nameLength) {
						Identifier* lastI;
						//there is an identifier, join it
						if (argument->tokens->length > 1 &&
							(lastI = dynamic_cast<Identifier*>(argument->tokens->last())) != nullptr)
						{
							string newName = lastI->name + newI->name.substr(ni + param.length());
							//if newI points at the replacement body identifier, replace it with one pointing at the argument
							if (cloneOfReplacementBody) {
								Identifier* oldI = newI;
								newI = new Identifier(newName, lastI->contentPos, lastI->endContentPos, lastI->owningFile);
								oldI->owningFile->replacedTokens->add(oldI);
								cloneOfReplacementBody = false;
							} else
								newI->name = newName;
							delete tokensOutput->pop();
						//no identifier, just adjust what we have
						} else {
							string newName = newI->name.substr(ni + param.length());
							//if newI points at the argument identifier, replace it with one pointing at the replacement body
							if (!cloneOfReplacementBody) {
								delete newI;
								newI = i->cloneWithReplacementSource(replacementSource);
								cloneOfReplacementBody = true;
							}
							newI->name = newName;
						}
					//we have no suffix, we don't need to do anything except get rid of the identifier
					} else {
						newI->owningFile->replacedTokens->add(newI);
						newI = nullptr;
						ni = nameLength;
						break;
					}
				}
				ni = (nameLength = newI->name.length()) - offsetFromEnd;
				break;
			}
		}
	}
	if (newI != nullptr)
		tokensOutput->add(newI);
}
