#include "Project.h"

//evaluates #if, #replace and #replace-input, #line

thread_local PrefixTrie<char, CDirectiveReplace*>* Replace::replaces = nullptr;
thread_local Array<Token*>* Replace::resultContents = nullptr;

//replace all code in all files, using the replacements available per file
void Replace::replaceCodeInFiles(Array<SourceFile*>* files) {
	try {
		forEach(SourceFile*, s, files, si) {
			printf("Replacing code in %s...\n", s->filename.c_str());

			//find all replace directives
			replaces = new PrefixTrie<char, CDirectiveReplace*>();
			Deleter<PrefixTrie<char, CDirectiveReplace*>> replacesDeleter (replaces);
			Deleter<Array<AVLNode<SourceFile*, bool>*>> allIncludedEntries (s->includedFiles->entrySet());
			forEach(AVLNode<SourceFile* COMMA bool>*, includedEntry, allIncludedEntries.retrieve(), includedEntryi) {
				SourceFile* included = includedEntry->key;
				forEach(CDirective*, d, included->abstractContents->directives, di) {
					CDirectiveReplace* r;
					if ((r = dynamic_cast<CDirectiveReplace*>(d)) == nullptr)
						continue;

					if (replaces->set(r->toReplace->name.c_str(), r->toReplace->name.length(), r) !=
							PrefixTrie<char, CDirectiveReplace*>::emptyValue)
						Error::makeError(General, "replacement has already been defined", included, r->toReplace);
				}
			}

			resultContents = new Array<Token*>();
			Deleter<Array<Token*>> finalContentsDeleter (resultContents);
			replaceCode(s->abstractContents, s);
		}
	} catch (...) {
	}
}
//add the tokens of the abstract contents into the result list
//the owning file contains the tokens of the abstract contents
void Replace::replaceCode(AbstractCodeBlock* abstractContents, SourceFile* owningFile) {
	forEach(Token*, t, abstractContents->tokens, ti) {
		Identifier* i;
		CDirectiveReplace* r;
		//if it's not an identifier to replace, add it to the result list
		if ((i = dynamic_cast<Identifier*>(t)) == nullptr ||
			(r = replaces->get(i->name.c_str(), i->name.length())) == nullptr)
		{
			resultContents->add(t);
			continue;
		}

		//this identifier matches a replace directive
		//if it's already in use, that's an error
		if (r->inUse)
			Error::makeError(General, "cannot use replacement in its own body", owningFile, i);
		r->inUse = true;
		//if there's no input, just stick its contents into the result
		if (r->input == nullptr)
			replaceCode(r->replacement, r->owningFile);
		//if it takes input, ???????????????
		else {

		}
		r->inUse = false;
	}
}
