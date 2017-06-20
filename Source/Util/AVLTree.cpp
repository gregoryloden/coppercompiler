#include "Project.h"

template <class Key, class Value> AVLTree<Key, Value>::AVLTree()
: onlyInDebugWithComma(ObjCounter("AVLTREE"))
, root(nullptr) {
}
template <class Key, class Value> AVLTree<Key, Value>::~AVLTree() {
	delete root;
}
template <class Key, class Value> AVLNode<Key, Value>::AVLNode(Key pKey, Value pValue)
: onlyInDebugWithComma(ObjCounter("AVLNODE"))
key(pKey)
, value(pValue)
, height(0)
, left(nullptr)
, right(nullptr) {
}
template <class Key, class Value> AVLNode<Key, Value>::~AVLNode() {
	//don't delete left or right, let AVLTree do that
}
