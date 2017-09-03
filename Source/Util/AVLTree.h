#include "../General/globals.h"

template <class Key, class Value> class AVLNode;

template <class Key, class Value> class AVLTree onlyInDebug(: public ObjCounter) {
public:
	AVLTree();
	virtual ~AVLTree();

	static const Value emptyValue;
private:
	static thread_local Value oldValue;
	AVLNode<Key, Value>* root;

	static void deleteTree(AVLNode<Key, Value>* node);
public:
	Value set(Key key, Value value);
private:
	static AVLNode<Key, Value>* setAndRebalance(AVLNode<Key, Value>* node, Key key, Value value);
public:
	void setAllFrom(AVLTree<Key, Value>* other);
private:
	void setAllFrom(AVLNode<Key, Value>* node);
public:
	Value get(Key key);
	Array<AVLNode<Key, Value>*>* entrySet();
private:
	static void addEntries(Array<AVLNode<Key, Value>*>* pEntrySet, AVLNode<Key, Value>* node);
};
template <class Key, class Value> class AVLNode onlyInDebug(: public ObjCounter) {
public:
	AVLNode(Key pKey, Value pValue);
	virtual ~AVLNode();

	Key key; //copper: private<readonly AVLTree>
	Value value; //copper: private<AVLTree>
	char height; //copper: private<AVLTree>
	AVLNode* left; //copper: private<AVLTree>
	AVLNode* right; //copper: private<AVLTree>

	static char nodeHeight(AVLNode<Key, Value>* node);
};
