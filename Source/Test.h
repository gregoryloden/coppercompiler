#ifdef DEBUG
	template <class Key, class Value> class AVLTree;
	template <class Key, class Value> class AVLNode;
	template <class KeyElement, class Value> class PrefixTrie;

	class Test {
	public:
		static int filesTested;

		static void testAll();
	private:
		static void testFiles();
		static void testFile(const char* fileName, int errorsExpected);
		static void testUtil();
		template <class Key, class Value> static void setAndValidateTree(AVLTree<Key, Value>* tree, Key key, Value value);
		template <class KeyElement, class Value> static void setAndValidateTrie(
			PrefixTrie<KeyElement, Value>* trie, const KeyElement* key, int keyLength, Value value);
		template <class Key, class Value> static void validateHeights(AVLNode<Key, Value>* node);
	};
#endif
