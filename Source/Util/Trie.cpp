#include "Project.h"

template class Trie<char, SourceFile*>;

SourceFile* const Trie<char, SourceFile*>::emptyValue = nullptr;

template <class KeyElement, class Value> Trie<KeyElement, Value>::Trie()
: onlyInDebugWithComma(ObjCounter(onlyWhenTrackingIDs("TRIE")))
value(emptyValue)
, nextTree(nullptr) {
}
template <class KeyElement, class Value> Trie<KeyElement, Value>::~Trie() {
	delete value;
	delete nextTree;
}
//if the value is present in the tree, return it, otherwise, generate a value, set it, and return it
template <class KeyElement, class Value>
Value Trie<KeyElement, Value>::getOrSet(const KeyElement* key, int keyLength, Value(*generateValue)()) {
	if (keyLength == 0)
		return value;

	Trie<KeyElement, Value>* next = this;
	int nextKeyElementIndex = 0;
	//try to find the existing element
	while (true) {
		Trie<KeyElement, Value>* nextNext;
		//in copper this and the below conditionals will be a switch statement with fallthrough gotos
		#define assignTrieAndAdvanceAndBreak() \
			Trie<KeyElement, Value>* prev = next;\
			prev->nextTree->set(key[nextKeyElementIndex], next = new Trie<KeyElement, Value>());\
			nextKeyElementIndex++;\
			break;
		//next does not have any subvalues
		if (next->nextTree == nullptr) {
			next->nextTree = new AVLTree<KeyElement, Trie<KeyElement, Value>*>();
			assignTrieAndAdvanceAndBreak();
		//next has subvalues but not the one we want
		} else if ((nextNext = nextTree->get(key[nextKeyElementIndex])) != AVLTree<KeyElement, Trie<KeyElement, Value>*>::emptyValue) {
			assignTrieAndAdvanceAndBreak();
		}
		//we found our next trie node
		next = nextNext;
		nextKeyElementIndex++;
		//and we reached the end
		if (nextKeyElementIndex == keyLength)
			return next->value;
	}
	//the fact that we got here means our value is missing
	//we've advanced to the earliest node without any subvalues
	//assign trees and tries until we get to the one we want, then generate, set, and return the value
	for (; nextKeyElementIndex < keyLength; nextKeyElementIndex++) {
		AVLTree<KeyElement, Trie<KeyElement, Value>*>* nextNextTree =
			(next->nextTree = new AVLTree<KeyElement, Trie<KeyElement, Value>*>());
		nextNextTree->set(key[nextKeyElementIndex], (next = new Trie<KeyElement, Value>()));
	}
	return (next->value = generateValue());
}
/*
//set the vaue in the trie
template <class KeyElement, class Value> Value Trie<KeyElement, Value>::set(KeyElement* key, int keyLength, Value value) {
	Trie<KeyElement, Value>* next = this;
	for (int nextKeyElementIndex = 0; nextKeyElementIndex < keyLength; nextKeyElementIndex++) {
		KeyElement nextKeyElement = key[nextKeyElementIndex];
		do {
			if (next->nextTree == nullptr)
				next->nextTree = new AVLTree<KeyElement, Trie<KeyElement, Value>*>();
			else if ((next = next->nextTree->get(nextKeyElement)) != AVLTree<KeyElement, Trie<KeyElement, Value>*>::emptyValue)
				break;
			next->nextTree->set(nextKeyElement, next = new Trie<KeyElement, Value>());
		} while (false);
	}
	Value oldValue = next->value;
	next->value = value;
	return oldValue;
}
//get the vaue from the trie
template <class KeyElement, class Value> Value Trie<KeyElement, Value>::get(KeyElement* key, int keyLength) {
	Trie<KeyElement, Value>* next = this;
	for (int nextKeyElementIndex = 0; nextKeyElementIndex < keyLength; nextKeyElementIndex++) {
		if (next->nextTree == nullptr ||
				(next = nextTree->get(key[nextKeyElementIndex])) == AVLTree<KeyElement, Trie<KeyElement, Value>*>::emptyValue)
			return emptyValue;
	}
	return next->value;
}
*/
