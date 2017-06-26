#include "../General/globals.h"

template <class KeyElement, class Value> class AVLTree;

template <class KeyElement, class Value> class Trie onlyInDebug(: public ObjCounter) {
public:
	Trie();
	~Trie();

	static const Value emptyValue;
private:
	Value value;
	AVLTree<KeyElement, Trie<KeyElement, Value>*>* nextTree;

public:
/*
	Value set(KeyElement* key, int keyLength, Value value);
	Value get(KeyElement* key, int keyLength);
*/
	Value getOrSet(const KeyElement* key, int keyLength, Value(*value)());
};
