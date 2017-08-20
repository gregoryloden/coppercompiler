#include "Project.h"

//evaluates #if, #replace and #replace-input, #line

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

			replaceAbstractContents(s->abstractContents, &replaces);
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
//evaluate the replaces in the abstract contents
//the resulting array of tokens replaces the old array in the abstract contents list
void Replace::replaceAbstractContents(AbstractCodeBlock* abstractContents, PrefixTrie<char, CDirectiveReplace*>* replaces) {
	Array<Token*>* result = new Array<Token*>();
	try {
		replaceTokens(abstractContents->tokens, result, replaces, nullptr);
		delete abstractContents->tokens; //don't delete the contents since they're in the new array
		abstractContents->tokens = result;
	} catch (...) {
		result->deleteSelfAndContents();
	}
}
//add the tokens of the abstract contents into the result list
//the owning file contains the tokens of the abstract contents
void Replace::replaceTokens(Array<Token*>* source, Array<Token*>* result, PrefixTrie<char, CDirectiveReplace*>* replaces,
	SubstitutedToken* substitutions)
{
	int length = source->length;
	Token** sourceInner = source->inner;
	for (int ti = 0; ti < length; ti++) {
		Token* t = sourceInner[ti];
		//erase entries in the file's source token list so that they aren't deleted twice on an error
		if (substitutions == nullptr)
			sourceInner[ti] = nullptr;
		AbstractCodeBlock* a;
		Identifier* i;
		CDirectiveReplace* r;
		//if it's an abstract code block, we have to do one of two things depending on whether it's top level or not
		if ((a = dynamic_cast<AbstractCodeBlock*>(t)) != nullptr) {
			//if it's not part of a replacement, we need to replace all its contents, and also consider its replaces
			if (substitutions == nullptr) {
				PrefixTrieUnion<char, CDirectiveReplace*> newReplaces (replaces);
				addReplacesToTrie(a, &newReplaces);
				replaceAbstractContents(a, &newReplaces);
				result->add(a);
			//if it is, we need to clone the abstract code block to add to the result
			} else {
				Deleter<Array<Token*>> tokens (new Array<Token*>());
				replaceTokens(a->tokens, tokens.retrieve(), replaces, substitutions);
				result->add(new AbstractCodeBlock(tokens.release(), nullptr, a->contentPos, a->endContentPos, a->owningFile));
			}
			continue;
		//if it's not an identifier to replace, add it to the result list
		} else if ((i = dynamic_cast<Identifier*>(t)) == nullptr ||
			(r = replaces->get(i->name.c_str(), i->name.length())) == nullptr)
		{
			result->add(cloneSubstitutions(substitutions, t));
			continue;
		}

		//this identifier matches a replace directive
		//for starters, ensure it will be deleted if it's top level, but not if it's from a replacement
		Deleter<Identifier> iDeleter (i);
		if (substitutions != nullptr)
			iDeleter.release();

		//if the replace is already in use, that's an error
		if (r->inUse)
			Error::makeError(General, "cannot use replacement in its own body", i);
		r->inUse = true;
		//no input, just stick its contents into the result
		if (r->input == nullptr) {
			try {
				SubstitutedToken newSubstitution (substitutions, i);
				newSubstitution.shouldDelete = false;
				replaceTokens(r->replacement->tokens, result, replaces, &newSubstitution);
			} catch (...) {
				r->inUse = false;
				Error::makeError(Continuation, nullptr, i);
			}
		//if it takes input, ???????????????
		} else {

		}
		r->inUse = false;
	}
}
Token* Replace::cloneSubstitutions(SubstitutedToken* substitutions, Token* token) {
	return (substitutions == nullptr)
		? token
		: new SubstitutedToken(
			cloneSubstitutions(dynamic_cast<SubstitutedToken*>(substitutions->parent), token),
			substitutions);
}
