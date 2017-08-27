#include "Project.h"

//evaluates #if, #replace, and #replace-input

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
void Replace::addReplacesToTrie(AbstractCodeBlock* abstractContents, PrefixTrie<char, CDirectiveReplace*>* replaces) {
	forEach(CDirective*, d, abstractContents->directives, di) {
		CDirectiveReplace* r;
		if ((r = dynamic_cast<CDirectiveReplace*>(d)) == nullptr)
			continue;

		if (replaces->set(r->toReplace->name.c_str(), r->toReplace->name.length(), r) !=
				PrefixTrie<char, CDirectiveReplace*>::emptyValue)
			Error::makeError(General, "replacement has already been defined", r->toReplace);
	}
}
//search through all of the tokens in the file to look for ones to replace
//if we find any, run its replacement
void Replace::replaceTokens(Array<Token*>* tokens, PrefixTrie<char, CDirectiveReplace*>* replaces) {
	for (int ti = 0; ti < tokens->length; ti++) {
		Token* fullToken = tokens->get(ti);
		//find the real token
		Token* t = fullToken;
		SubstitutedToken* s;
		while ((s = dynamic_cast<SubstitutedToken*>(t)) != nullptr)
			t = s->resultingToken;

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
			Error::makeError(General, "cannot use replacement in its own body", fullToken);
		r->inUse = true;
		//no input, just stick its contents into the result
		if (r->input == nullptr) {
			//get the tokens we need
			Array<Token*>* tokensToInsert = simpleReplace(r->replacement, fullToken);
			try {
				replaceTokens(tokensToInsert, replaces);
			} catch (...) {
				tokensToInsert->deleteSelfAndContents();
				r->inUse = false;
				throw;
			}
			//insert them into the array, preserving the index relative to the end
			int offsetFromEnd = tokens->length - ti;
			tokens->replace(ti, 1, tokensToInsert);
			ti = tokens->length - offsetFromEnd;
			delete fullToken;
			delete tokensToInsert;
		//if it takes input, ???????????????
		} else {
			//start by collecting the arguments
			try {
				//make sure we have an argument list
				ti++;
				if (ti >= tokens->length)
					Error::makeError(General, "expected an input list to follow", fullToken);
				t = tokens->get(ti);
				if ((a = dynamic_cast<AbstractCodeBlock*>(t)) == nullptr)
					Error::makeError(General, "expected an input list", t);
				//split up the arguments around the commas
				Array<Token*>* nextTokens = nullptr;
				Array<Array<Token*>*>* arguments = new Array<Array<Token*>*>();
				forEach(Token*, at, a->tokens, ati) {
					Separator2* s;
					if ((s = dynamic_cast<Separator2*>(at)) != nullptr && s->type == Comma) {
						delete s;
						arguments->add(nextTokens);
						nextTokens = nullptr;
					} else {
						if (nextTokens == nullptr)
							nextTokens = new Array<Token*>();
						nextTokens->add(at);
					}
				}
				if (nextTokens != nullptr)
					arguments->add(nextTokens);
				a->tokens->clear();
				delete a;
				//?????????????????????
				//behavior:
				//identifiers: normal replace, concat after removing leading and trailing whitespace
				//strings: full replace including whitespace and comments
			} catch (...) {
				r->inUse = false;
				throw;
			}
			//?????????????????????
		}
		r->inUse = false;
	}
}
//recursively clone the abstract code block, and substitute any other tokens using the parent token
Array<Token*>* Replace::simpleReplace(AbstractCodeBlock* abstractContents, Token* tokenBeingReplaced) {
	Array<Token*>* tokens = new Array<Token*>();
	forEach(Token*, t, abstractContents->tokens, ti) {
		AbstractCodeBlock* a;
		if ((a = dynamic_cast<AbstractCodeBlock*>(t)) != nullptr)
			tokens->add(new AbstractCodeBlock(
				simpleReplace(a, tokenBeingReplaced), nullptr, a->contentPos, a->endContentPos, a->owningFile));
		else
			tokens->add(substituteTokens(tokenBeingReplaced, t));
	}
	return tokens;
}
//create a chain of substituted tokens above the original token, based on the parent token being replaced
SubstitutedToken* Replace::substituteTokens(Token* tokenBeingReplaced, Token* resultingToken) {
	SubstitutedToken* s;
	if ((s = dynamic_cast<SubstitutedToken*>(tokenBeingReplaced)) != nullptr)
		return new SubstitutedToken(substituteTokens(s->resultingToken, resultingToken), true, s);
	else
		return new SubstitutedToken(resultingToken, false, tokenBeingReplaced);
}
