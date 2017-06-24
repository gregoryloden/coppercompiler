#include "Project.h"

template <class Key, class Value> Trie<Key, Value>::Trie()
: onlyInDebugWithComma(ObjCounter(onlyWhenTrackingIDs("TRIE")))
hasValue(false)
, value(AVLTree<Key, Value>::emptyValue) {
}
template <class Key, class Value> Trie<Key, Value>::~Trie() {
}
