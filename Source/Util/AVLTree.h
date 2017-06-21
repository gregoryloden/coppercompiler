#include "globals.h"

template <class Key, class Value> class AVLNode;

template <class Key, class Value> class AVLTree onlyInDebug(: public ObjCounter) {
public:
	AVLTree();
	~AVLTree();

	void set(Key key, Value value);
	Value get(Key key);
	Value remove(Key key);
private:
	AVLNode<Key, Value>* root;

	void deleteTree(AVLNode<Key, Value>* node);
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
};
