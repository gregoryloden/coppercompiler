#include "Project.h"

//evaluates #if, #replace and #replace-input, #line

void Replace::replaceCode(Array<SourceFile*>* files) {
	try {
		forEach(SourceFile*, s, files, si) {
			printf("Replacing code in %s...\n", s->filename.c_str());

			//find all replace directives
			PrefixTrie<char, CDirectiveReplace*> replaces;
			Deleter<Array<AVLNode<SourceFile*, bool>*>> allIncludedEntries (s->includedFiles->entrySet());
			forEach(AVLNode<SourceFile* COMMA bool>*, includedEntry, allIncludedEntries.retrieve(), includedEntryi) {
				SourceFile* included = includedEntry->key;
				forEach(CDirective*, d, included->abstractContents->directives, di) {
					CDirectiveReplace* r;
					if ((r = dynamic_cast<CDirectiveReplace*>(d)) == nullptr)
						continue;

					if (replaces.set(r->toReplace->name.c_str(), r->toReplace->name.length(), r) !=
							PrefixTrie<char, CDirectiveReplace*>::emptyValue)
						Error::makeError(General, "replacement has already been defined", included, r->toReplace);
				}
			}

			searchContents(&replaces, s->abstractContents);
		}
	} catch (...) {
	}
}
void Replace::searchContents(PrefixTrie<char, CDirectiveReplace*>* replaces, AbstractCodeBlock* abstractContents) {
	Array<Token*>* tokens = abstractContents->tokens;
	for (int ti = 0; ti < tokens->length; ti++) {
		Identifier* i;
		if ((i = dynamic_cast<Identifier*>(tokens->inner[ti])) == nullptr)
			continue;

		//TODO
	}
}
