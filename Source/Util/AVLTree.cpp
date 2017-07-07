#include "Project.h"

//template class AVLTree<int, int>;
//template class AVLTree<char, char>;
//template class AVLNode<int, int>;
//template class AVLNode<char, char>;
template class AVLTree<char, PrefixTrie<char, SourceFile*>*>;
template class AVLTree<SourceFile*, bool>;
template class AVLNode<char, PrefixTrie<char, SourceFile*>*>;
template class AVLNode<SourceFile*, bool>;

//const int AVLTree<int, int>::emptyValue = 0;
//const char AVLTree<char, char>::emptyValue = '\0';
//thread_local int AVLTree<int, int>::oldValue = 0;
//thread_local char AVLTree<char, char>::oldValue = '\0';
PrefixTrie<char, SourceFile*>* const AVLTree<char, PrefixTrie<char, SourceFile*>*>::emptyValue = nullptr;
thread_local PrefixTrie<char, SourceFile*>* AVLTree<char, PrefixTrie<char, SourceFile*>*>::oldValue = nullptr;
bool const AVLTree<SourceFile*, bool>::emptyValue = false;
thread_local bool AVLTree<SourceFile*, bool>::oldValue = false;

template <class Key, class Value> AVLTree<Key, Value>::AVLTree()
: onlyInDebugWithComma(ObjCounter(onlyWhenTrackingIDs("AVLTREE")))
root(nullptr) {
}
template <class Key, class Value> AVLTree<Key, Value>::~AVLTree() {
	deleteTree(root);
}
//delete the node's entire tree
template <class Key, class Value> void AVLTree<Key, Value>::deleteTree(AVLNode<Key, Value>* node) {
	if (node != nullptr) {
		deleteTree(node->left);
		deleteTree(node->right);
		delete node;
	}
}
//set a value in the tree
template <class Key, class Value> Value AVLTree<Key, Value>::set(Key key, Value value) {
	if (root == nullptr) {
		root = new AVLNode<Key, Value>(key, value);
		return emptyValue;
	} else {
		root = setAndRebalance(root, key, value);
		return oldValue;
	}
}
//set a value in the tree as determined by the node, and rebalance if one side is too tall
template <class Key, class Value> AVLNode<Key, Value>* AVLTree<Key, Value>::setAndRebalance(
	AVLNode<Key, Value>* node, Key key, Value value)
{
	if (node == nullptr) {
		oldValue = emptyValue;
		return new AVLNode<Key, Value>(key, value);
	} else if (key == node->key) {
		oldValue = node->value;
		node->value = value;
		return node;
	}

	//this will be replaced with .. accessors in Copper
	struct AVLNodeAccessor {
		bool isLeft;
		AVLNodeAccessor(bool pIsLeft): isLeft(pIsLeft) {}
		AVLNode<Key, Value>* get(AVLNode<Key, Value>* root) { return isLeft ? root->left : root->right; };
		void set(AVLNode<Key, Value>* root, AVLNode<Key, Value>* leaf) {
			if (isLeft) root->left = leaf; else root->right = leaf;
		};
	};
	//whether the key is on the left or the right, the accessors work properly
	AVLNodeAccessor leftAccessor (key < node->key);
	AVLNodeAccessor rightAccessor (!leftAccessor.isLeft);

	AVLNode<Key, Value>* leftNodeRightChild;
	AVLNode<Key, Value>* leftNode = setAndRebalance(leftAccessor.get(node), key, value);
	char rightNodeHeight = AVLNode<Key, Value>::nodeHeight(rightAccessor.get(node));
	//no rebalancing needed:    A<=X+2
	//                         / \
	//                   X+1>=B   C: X
	if (leftNode->height < rightNodeHeight + 2) {
		leftAccessor.set(node, leftNode);
		node->height = max(node->height, leftNode->height + 1);
		return node;
	//rebalancing needed:    A: X+3                B: X+2
	//                      / \                   / \
	//                X+2: B   C: X    >    X+1: D   A: X+1
	//                    / \                       / \
	//              X+1: D   E: X               X: E   C: X
	} else if (AVLNode<Key, Value>::nodeHeight(leftAccessor.get(leftNode)) >
		AVLNode<Key, Value>::nodeHeight(leftNodeRightChild = rightAccessor.get(leftNode)))
	{
		leftAccessor.set(node, leftNodeRightChild);
		node->height = rightNodeHeight + 1;
		rightAccessor.set(leftNode, node);
		leftNode->height = node->height + 1;
		return leftNode;
	//rebalancing needed:    A: X+3
	//                      / \                         E: X+2
	//                X+2: B   C: X               .--**' '**--.
	//                    / \          >    X+1: B             A: X+1    (F=X || G=X) && F>=X-1 && G>=X-1
	//                X: D   E: X+1             / \           / \
	//                      / \             X: D   F<=X   X>=G   C: X
	//                  X>=F   G<=X
	} else {
		leftAccessor.set(node, rightAccessor.get(leftNodeRightChild));
		node->height = rightNodeHeight + 1;
		rightAccessor.set(leftNode, leftAccessor.get(leftNodeRightChild));
		leftNode->height = leftAccessor.get(leftNode)->height + 1;
		leftAccessor.set(leftNodeRightChild, leftNode);
		rightAccessor.set(leftNodeRightChild, node);
		leftNodeRightChild->height = node->height + 1;
		return leftNodeRightChild;
	}
}
//set all values from the other tree in this tree
template <class Key, class Value> void AVLTree<Key, Value>::setAllFrom(AVLTree<Key, Value>* other) {
	return setAllFrom(other->root);
}
//set all values from the node in this tree
template <class Key, class Value> void AVLTree<Key, Value>::setAllFrom(AVLNode<Key, Value>* node) {
	if (node == nullptr)
		return;

	setAllFrom(node->left);
	setAllFrom(node->right);
	set(node->key, node->value);
}
//get a value from the tree
template <class Key, class Value> Value AVLTree<Key, Value>::get(Key key) {
	for (AVLNode<Key, Value>* node = root; node != nullptr; node = key < node->key ? node->left : node->right) {
		if (key == node->key)
			return node->value;
	}
	return emptyValue;
}
template <class Key, class Value> AVLNode<Key, Value>::AVLNode(Key pKey, Value pValue)
: onlyInDebugWithComma(ObjCounter(onlyWhenTrackingIDs("AVLNODE")))
key(pKey)
, value(pValue)
, height(1)
, left(nullptr)
, right(nullptr) {
}
template <> AVLNode<char, PrefixTrie<char, SourceFile*>*>::~AVLNode() {
	delete value;
}
template <class Key, class Value> AVLNode<Key, Value>::~AVLNode() {
	//don't delete left or right, so that we can delete nodes without deleting their whole trees
	//deleting whole trees will happen via AVLTree::deleteTree when the AVLTree is deleted
}
template <class Key, class Value> char AVLNode<Key, Value>::nodeHeight(AVLNode<Key, Value>* node) {
	return node != nullptr ? node->height : 0;
}
