#include "../General/globals.h"

template <class KeyElement, class Value> class AVLTree;

template <class KeyElement, class Value> class PrefixTrie onlyInDebug(: public ObjCounter) {
public:
	PrefixTrie(
		KeyElement* pCommonPrefix,
		int pCommonPrefixLength,
		Value pValue,
		AVLTree<KeyElement, PrefixTrie<KeyElement, Value>*>* pNextTree);
	PrefixTrie(): PrefixTrie(nullptr, 0, emptyValue, nullptr) {}
	PrefixTrie(
		const KeyElement* key,
		int keyOffset,
		int pCommonPrefixLength,
		Value pValue,
		AVLTree<KeyElement, PrefixTrie<KeyElement, Value>*>* pNextTree);
	~PrefixTrie();

	static const Value emptyValue;
private:
	KeyElement* commonPrefix;
	int commonPrefixLength;
	Value value;
	AVLTree<KeyElement, PrefixTrie<KeyElement, Value>*>* nextTree;

public:
	Value set(const KeyElement* key, int keyLength, Value value);
	Value get(const KeyElement* key, int keyLength);
};
