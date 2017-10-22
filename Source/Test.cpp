#ifdef DEBUG
	#include "Project.h"

	#define instantiateAVLTreeTestFunctions(keyType, valueType) \
		template void Test::setAndValidate(AVLTree<keyType, valueType>* tree, keyType key, valueType value);\
		template void Test::validateHeights(AVLNode<keyType, valueType>* node);

	instantiateAVLTreeTestFunctions(int, int);
	instantiateAVLTreeTestFunctions(char, char);

	void Test::testAll() {
		testFiles();
		testUtil();
	}
	void Test::testFiles() {
		testFile("Test/Step01_Lex_lex.cu", 0);
		testFile("Test/Step01_Lex_badLex.cu", 10);
		testFile("Test/Step01_Lex_blockCommentEOF.cu", 1);
		testFile("Test/Step01_Lex_intBaseEOF.cu", 1);
		testFile("Test/Step01_Lex_floatEOF.cu", 1);
		testFile("Test/Step01_Lex_stringEOF.cu", 1);
		testFile("Test/Step01_Lex_stringEscapeSequenceEOF.cu", 1);
		testFile("Test/Step01_Lex_stringHexEscapeSequenceEOF.cu", 1);
		testFile("Test/Step01_Lex_characterEOF.cu", 1);
		testFile("Test/Step01_Lex_characterUnterminatedEOF.cu", 1);
		testFile("Test/Step01_Lex_directiveEOF.cu", 1);
		testFile("Test/Step02_ParseDirectives_parseDirectives.cu", 0);
		testFile("Test/Step02_ParseDirectives_badParseDirectives.cu", 10);
		testFile("Test/Step02_ParseDirectives_leftParenthesisEOF.cu", 1);
		testFile("Test/Step02_ParseDirectives_replaceNameEOF.cu", 1);
		testFile("Test/Step02_ParseDirectives_replaceBodyEOF.cu", 1);
		testFile("Test/Step02_ParseDirectives_includeFileEOF.cu", 1);
		testFile("Test/Step02_ParseDirectives_replaceInputParametersParenthesisEOF.cu", 1);
		testFile("Test/Step02_ParseDirectives_replaceInputParametersEOF.cu", 1);
		testFile("Test/Step02_ParseDirectives_replaceInputParametersCommaEOF.cu", 1);
		testFile("Test/Step02_ParseDirectives_replaceInputSecondParameterEOF.cu", 1);
	}
	void Test::testFile(const char* fileName, int errorsExpected) {
		Pliers* p = new Pliers(fileName, false, false);
		assert(p->allFiles->get(0)->contentsLength > 0);
		if (p->errorMessages->length != errorsExpected) {
			forEach(ErrorMessage*, errorMessage, p->errorMessages, ei) {
				errorMessage->printError();
			}
			assert(false);
		}
		delete p;
	}
	void Test::testUtil() {
		AVLTree<int, int>* tree1 = new AVLTree<int, int>();
		setAndValidate(tree1, 1, 1);
		setAndValidate(tree1, 2, 1);
		setAndValidate(tree1, 3, 1);
		setAndValidate(tree1, 4, 1);
		setAndValidate(tree1, 5, 10);
		setAndValidate(tree1, 6, 1);
		setAndValidate(tree1, 7, 1);
		setAndValidate(tree1, 8, 1);
		setAndValidate(tree1, 9, 1);
		setAndValidate(tree1, 10, 15);
		setAndValidate(tree1, 11, 1);
		setAndValidate(tree1, 12, 1);
		setAndValidate(tree1, 13, 1);
		setAndValidate(tree1, 14, 1);
		setAndValidate(tree1, 15, 19);
		setAndValidate(tree1, 9, 19);
		delete tree1;
		AVLTree<char, char>* tree2 = new AVLTree<char, char>();
		setAndValidate(tree2, (char)15, (char)1);
		setAndValidate(tree2, (char)14, (char)1);
		setAndValidate(tree2, (char)13, (char)13);
		setAndValidate(tree2, (char)12, (char)1);
		setAndValidate(tree2, (char)11, (char)51);
		setAndValidate(tree2, (char)10, (char)1);
		setAndValidate(tree2, (char)9, (char)19);
		setAndValidate(tree2, (char)8, (char)1);
		setAndValidate(tree2, (char)7, (char)1);
		setAndValidate(tree2, (char)6, (char)1);
		setAndValidate(tree2, (char)5, (char)12);
		setAndValidate(tree2, (char)4, (char)1);
		setAndValidate(tree2, (char)3, (char)81);
		setAndValidate(tree2, (char)2, (char)1);
		setAndValidate(tree2, (char)1, (char)1);
		delete tree2;
		AVLTree<char, char>* tree3 = new AVLTree<char, char>();
		setAndValidate(tree3, (char)6, (char)1);
		setAndValidate(tree3, (char)7, (char)2);
		setAndValidate(tree3, (char)2, (char)3);
		setAndValidate(tree3, (char)1, (char)4);
		setAndValidate(tree3, (char)4, (char)5);
		setAndValidate(tree3, (char)5, (char)6);
		setAndValidate(tree3, (char)3, (char)7);
		delete tree3;
	}
	template <class Key, class Value> void Test::setAndValidate(AVLTree<Key, Value>* tree, Key key, Value value) {
		tree->set(key, value);
		assert(tree->get(key) == value);
		validateHeights(tree->root);
	}
	template <class Key, class Value> void Test::validateHeights(AVLNode<Key, Value>* node) {
		if (node == nullptr)
			return;
		validateHeights(node->left);
		validateHeights(node->right);
		char leftHeight = AVLNode<Key, Value>::nodeHeight(node->left);
		char rightHeight = AVLNode<Key, Value>::nodeHeight(node->right);
		assert(leftHeight < node->height);
		assert(rightHeight < node->height);
		assert(leftHeight >= node->height - 2);
		assert(rightHeight >= node->height - 2);
		assert(leftHeight == node->height - 1 || rightHeight == node->height - 1);
	}
#endif
