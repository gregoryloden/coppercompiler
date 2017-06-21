#include "Project.h"

AVLTree<Token*, EmptyToken*>* buildabc() {
	AVLTree<Token*, EmptyToken*>* abc = new AVLTree<Token*, EmptyToken*>();
	EmptyToken* a = new EmptyToken(0, 0, 0);
	abc->set(a, a);
	abc->remove(abc->get(a));
	return abc;
}
//AVLTree<Token*, EmptyToken*>* thing = buildabc();

template <class Key, class Value> AVLTree<Key, Value>::AVLTree()
: onlyInDebugWithComma(ObjCounter(onlyWhenTrackingIDs("AVLTREE")))
root(nullptr) {
}
template <class Key, class Value> AVLTree<Key, Value>::~AVLTree() {
	deleteTree(root);
}
template <class Key, class Value> void AVLTree<Key, Value>::deleteTree(AVLNode<Key, Value>* node) {
	if (node != nullptr) {
		deleteTree(node->left);
		deleteTree(node->right);
		delete node;
	}
}
template <class Key, class Value> void AVLTree<Key, Value>::set(Key key, Value value) {
AVLNode<Key, Value>* newRoot = new AVLNode<Key, Value>(key, value);
newRoot->left = root;
newRoot->right = nullptr;
newRoot->key = key;
newRoot->value = value;
newRoot->height = 1;
root = newRoot;
}
template <class Key, class Value> Value AVLTree<Key, Value>::get(Key key) {
return root->value;
}
template <class Key, class Value> Value AVLTree<Key, Value>::remove(Key key) {
return root->value;
}
template <class Key, class Value> AVLNode<Key, Value>::AVLNode(Key pKey, Value pValue)
: onlyInDebugWithComma(ObjCounter(onlyWhenTrackingIDs("AVLNODE")))
key(pKey)
, value(pValue)
, height(0)
, left(nullptr)
, right(nullptr) {
}
template <class Key, class Value> AVLNode<Key, Value>::~AVLNode() {
	//don't delete left or right, AVLTree calls deleteTree
}
