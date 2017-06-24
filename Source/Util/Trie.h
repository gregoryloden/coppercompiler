#include "../General/globals.h"

template <class Key, class Value> class AVLTree;

template <class Key, class Value> class Trie onlyInDebug(: public ObjCounter) {
public:
	Trie();
	~Trie();

private:
	bool hasValue;
	Value value;
};
