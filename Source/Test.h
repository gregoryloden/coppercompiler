#ifdef DEBUG
	template <class Key, class Value> class AVLTree;
	template <class Key, class Value> class AVLNode;

	class Test {
	public:
		static int filesTested;

		static void testAll();
	private:
		static void testFiles();
		static void testFile(const char* fileName, int errorsExpected);
		static void testUtil();
		template <class Key, class Value> static void setAndValidate(AVLTree<Key, Value>* tree, Key key, Value value);
		template <class Key, class Value> static void validateHeights(AVLNode<Key, Value>* node);
	};
#endif
