#include "Project.h"

template class PrefixTrie<char, SourceFile*>;
template class PrefixTrie<char, CDirectiveReplace*>;

SourceFile* const PrefixTrie<char, SourceFile*>::emptyValue = nullptr;
CDirectiveReplace* const PrefixTrie<char, CDirectiveReplace*>::emptyValue = nullptr;

template <class KeyElement, class Value> PrefixTrie<KeyElement, Value>::PrefixTrie(
	KeyElement* pCommonPrefix,
	int pCommonPrefixLength,
	Value pValue,
	AVLTree<KeyElement, PrefixTrie<KeyElement, Value>*>* pNextTree)
: onlyInDebug(ObjCounter(onlyWhenTrackingIDs("PRETRIE")) COMMA)
commonPrefix(pCommonPrefix)
, commonPrefixLength(pCommonPrefixLength)
, value(pValue)
, nextTree(pNextTree) {
}
template <class KeyElement, class Value> PrefixTrie<KeyElement, Value>::PrefixTrie(
	const KeyElement* key,
	int keyOffset,
	int pCommonPrefixLength,
	Value pValue,
	AVLTree<KeyElement, PrefixTrie<KeyElement, Value>*>* pNextTree)
: PrefixTrie(new KeyElement[pCommonPrefixLength], pCommonPrefixLength, pValue, pNextTree) {
	memcpy(commonPrefix, key + keyOffset, pCommonPrefixLength);
}
template <class KeyElement, class Value> PrefixTrie<KeyElement, Value>::~PrefixTrie() {
	delete commonPrefix;
	delete nextTree;
}
//set the vaue in the trie
template <class KeyElement, class Value> Value PrefixTrie<KeyElement, Value>::set(
	const KeyElement* key, int keyLength, Value value)
{
	PrefixTrie<KeyElement, Value>* next = this;
	int keyIndex = 0;
	//search through the tries
	while (true) {
		int commonPrefixKeyIndexStart = keyIndex;
		int commonPrefixKeyIndexEnd = commonPrefixKeyIndexStart + next->commonPrefixLength;
		//search through the common prefix on our next trie
		while (true) {
			//we have more key to compare
			if (keyIndex < keyLength) {
				PrefixTrie<KeyElement, Value>* nextNext;
				//and we have more common prefix to compare
				if (keyIndex < commonPrefixKeyIndexEnd) {
					//same, keep going
					if (key[keyIndex] == next->commonPrefix[keyIndex - commonPrefixKeyIndexStart]) {
						keyIndex++;
						continue;
					}
				//we have no more common prefix to compare, see if we have a next tree
				} else if (next->nextTree != nullptr && (nextNext = next->nextTree->get(key[keyIndex])) !=
					AVLTree<KeyElement, PrefixTrie<KeyElement, Value>*>::emptyValue)
				{
					keyIndex++;
					next = nextNext;
					break; //break to set the bounds of the next common prefix
				}
			}

			//we are no longer comparing the key and the common prefix, rearrange things as necessary
			//first, if the common prefix has been cut short, split this trie into a new one
			if (keyIndex < commonPrefixKeyIndexEnd) {
				PrefixTrie<KeyElement, Value>* child = new PrefixTrie<KeyElement, Value>(
					next->commonPrefix,
					keyIndex + 1 - commonPrefixKeyIndexStart,
					commonPrefixKeyIndexEnd - keyIndex - 1,
					next->value,
					next->nextTree);
				next->value = emptyValue;
				next->commonPrefixLength = keyIndex - commonPrefixKeyIndexStart;
				(next->nextTree = new AVLTree<KeyElement, PrefixTrie<KeyElement, Value>*>())
					->set(next->commonPrefix[keyIndex - commonPrefixKeyIndexStart], child);
			}
			//now find out where the key goes
			//if there was more key, assign the new value to a new tree
			if (keyIndex < keyLength) {
				if (next->nextTree == nullptr)
					next->nextTree = new AVLTree<KeyElement, PrefixTrie<KeyElement, Value>*>();
				next->nextTree->set(key[keyIndex],
					new PrefixTrie<KeyElement, Value>(key, keyIndex + 1, keyLength - keyIndex - 1, value, nullptr));
				return emptyValue;
			//if there was no more key, we actually want to set the value here
			} else {
				Value oldValue = next->value;
				next->value = value;
				return oldValue;
			}
		}
	}
}
//get the vaue from the trie
template <class KeyElement, class Value> Value PrefixTrie<KeyElement, Value>::get(const KeyElement* key, int keyLength) {
	PrefixTrie<KeyElement, Value>* next = this;
	int keyIndex = 0;
	//search throgh the tries
	while (true) {
		int commonPrefixKeyIndexStart = keyIndex;
		int commonPrefixKeyIndexEnd = commonPrefixKeyIndexStart + next->commonPrefixLength;
		//we definitely don't have it if the common prefix goes further
		if (commonPrefixKeyIndexEnd > keyLength)
			return emptyValue;
		//search through the common prefix on our next trie
		while (true) {
			//we reached the end of the common prefix
			if (keyIndex == commonPrefixKeyIndexEnd) {
				//we also reached the key end, return the value
				if (keyIndex == keyLength)
					return next->value;
				//we have more key to go, find the next trie if it's there
				else if (next->nextTree != nullptr && (next = next->nextTree->get(key[keyIndex])) !=
					AVLTree<KeyElement, PrefixTrie<KeyElement, Value>*>::emptyValue)
				{
					keyIndex++;
					break; //break to set the bounds of the next common prefix
				}
			//we didn't reach the end but the key elements are the same, keep going
			} else if (key[keyIndex] == next->commonPrefix[keyIndex - commonPrefixKeyIndexStart]) {
				keyIndex++;
				continue;
			}

			//we ran out of tries or found a differing key element
			return emptyValue;
		}
	}
}

