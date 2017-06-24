#include "globals.h"

template <class Key, class Value> class AVLNode;

template <class Key, class Value> class AVLTree onlyInDebug(: public ObjCounter) {
public:
	AVLTree();
	~AVLTree();

	Value set(Key key, Value value);
	Value get(Key key);
private:
	static thread_local Value nextReturnValue;
	static const Value emptyValue;
	AVLNode<Key, Value>* root;

	void deleteTree(AVLNode<Key, Value>* node);
	static AVLNode<Key, Value>* setAndRebalance(AVLNode<Key, Value>* node, Key key, Value value);
	static Value get(AVLNode<Key, Value>*, Key key);
};
template <class Key, class Value> class AVLNode onlyInDebug(: public ObjCounter) {
public:
	AVLNode(Key pKey, Value pValue);
	~AVLNode();

	Key key; //private<AVLTree>
	Value value; //private<AVLTree>
	char height; //private<AVLTree>
	AVLNode* left; //private<AVLTree>
	AVLNode* right; //private<AVLTree>

	static char nodeHeight(AVLNode<Key, Value>* node);
};
