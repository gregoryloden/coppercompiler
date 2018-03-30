#include "../General/globals.h"

template <class KeyElement, class Value> class AVLTree;
template <class Type> class Array;

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

	virtual bool isEmpty();
	virtual Value set(const KeyElement* key, int keyLength, Value value);
	virtual Value get(const KeyElement* key, int keyLength);
	Array<Value>* getValues();
private:
	void addValues(Array<Value>* values);
};
template <class KeyElement, class Value> class PrefixTrieUnion: public PrefixTrie<KeyElement, Value> {
private:
	PrefixTrie<KeyElement, Value>* nextPrefixTrie;

public:
	PrefixTrieUnion(PrefixTrie<KeyElement, Value>* pNext);
	virtual ~PrefixTrieUnion();

	bool isEmpty();
	Value set(const KeyElement* key, int keyLength, Value value);
	Value get(const KeyElement* key, int keyLength);
	Value getFromParent(const KeyElement* key, int keyLength);
};
