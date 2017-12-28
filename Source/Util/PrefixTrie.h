#include "../General/globals.h"

template <class KeyElement, class Value> class AVLTree;

template <class KeyElement, class Value> class PrefixTrie onlyInDebug(: public ObjCounter) {
public:
	static const Value emptyValue;
private:
	KeyElement* commonPrefix;
	int commonPrefixLength;
	Value value;
	AVLTree<KeyElement, PrefixTrie<KeyElement, Value>*>* nextTree;

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
	virtual ~PrefixTrie();

	virtual Value set(const KeyElement* key, int keyLength, Value value);
	virtual Value get(const KeyElement* key, int keyLength);
};
template <class KeyElement, class Value> class PrefixTrieUnion: public PrefixTrie<KeyElement, Value> {
private:
	PrefixTrie<KeyElement, Value>* next;

public:
	PrefixTrieUnion(PrefixTrie<KeyElement, Value>* pNext);
	virtual ~PrefixTrieUnion();

	Value set(const KeyElement* key, int keyLength, Value value);
	Value get(const KeyElement* key, int keyLength);
};
