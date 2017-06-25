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
template <class KeyElement, class Value> Value Trie<KeyElement, Value>::get(KeyElement* key, int keyLength) {
	Trie<KeyElement, Value>* next = this;
	for (int nextKeyElementIndex = 0; nextKeyElementIndex < keyLength; nextKeyElementIndex++) {
		if (next->nextTree == nullptr ||
				(next = nextTree->get(key[nextKeyElementIndex])) == AVLTree<KeyElement, Trie<KeyElement, Value>*>::emptyValue)
			return emptyValue;
	}
	return next->value;
}
