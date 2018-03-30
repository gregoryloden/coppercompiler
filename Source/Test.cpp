#ifdef DEBUG
	#include "Project.h"

	#define instantiateAVLTreeTestFunctions(keyType, valueType) \
		template void Test::setAndValidateTree(AVLTree<keyType, valueType>* tree, keyType key, valueType value);\
		template void Test::validateHeights(AVLNode<keyType, valueType>* node);
	#define instantiatePrefixTrieTestFunctions(keyElementType, valueType) \
		template void Test::setAndValidateTrie(\
			PrefixTrie<keyElementType, valueType>* trie, const keyElementType* key, int keyLength, valueType value);

	instantiateAVLTreeTestFunctions(int, int);
	instantiateAVLTreeTestFunctions(char, char);
	instantiatePrefixTrieTestFunctions(char, char);

	int Test::filesTested = 0;
	void Test::testAll() {
		printf("\nBegin testing\n");
		testUtil();
		testFiles();
		printf("Testing complete\n\n");
	}
	void Test::testFiles() {
		printf("  Testing files\n");
		filesTested = 0;
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
		testFile("Test/Step04_Replace_replace.cu", 0);
		testFile("Test/Step04_Replace_badReplace.cu", 11);
		testFile("Test/Step06_ParseExpressions_parseExpressions.cu", 0);
		testFile("Test/Step06_ParseExpressions_badParseExpressions.cu", 65);
		testFile("Test/Step07_Semant_semant.cu", 0);
		testFile("Test/Step07_Semant_circularInclude.cu", 0);
		testFile("Test/Step07_Semant_badSemant.cu", 73);
		testFile("Test/Step07_Semant_includeClash.cu", 1);
		printf("  Finished testing files: %d total files tested\n", filesTested);
	}
	void Test::testFile(const char* fileName, int errorsExpected) {
		printf("    Testing %s...\n", fileName);
		Pliers* p = new Pliers(fileName, false, false);
		assert(p->allFiles->length >= 1);
		assert(p->allFiles->first()->contentsLength > 0);
		bool wrongLines = false;
		for (int errorMessageI = 0; errorMessageI < p->errorMessages->length; errorMessageI++) {
			if (p->errorMessages->get(errorMessageI)->getRow() != errorMessageI) {
				wrongLines = true;
				break;
			}
		}
		if (p->errorMessages->length != errorsExpected || wrongLines) {
			forEach(ErrorMessage*, errorMessage, p->errorMessages, ei) {
				errorMessage->printError();
			}
			assert(p->errorMessages->length == errorsExpected);
			assert(!wrongLines);
		}
		delete p;
		filesTested++;
		//TODO: at the end of the file is one empty line followed by the expected output with something like this:
		//		/* expected output:
		//		[line of expected output]
		//		[line of expected output]
		//		[line of expected output]
		//		*/
		//the two lines with comment marks should be there character for character
		//the lines of expected output should match the output or erros character for character
		//if the file has errors, at the end it has a comment like
		//		//char XX: [error]
		//which matches the end of the first line of the error string
	}
	void Test::testUtil() {
		printf("  Testing util\n");
		AVLTree<int, int>* tree1 = new AVLTree<int, int>();
		setAndValidateTree(tree1, 1, 1);
		setAndValidateTree(tree1, 2, 1);
		setAndValidateTree(tree1, 3, 1);
		setAndValidateTree(tree1, 4, 1);
		setAndValidateTree(tree1, 5, 10);
		setAndValidateTree(tree1, 6, 1);
		setAndValidateTree(tree1, 7, 1);
		setAndValidateTree(tree1, 8, 1);
		setAndValidateTree(tree1, 9, 1);
		setAndValidateTree(tree1, 10, 15);
		setAndValidateTree(tree1, 11, 1);
		setAndValidateTree(tree1, 12, 1);
		setAndValidateTree(tree1, 13, 1);
		setAndValidateTree(tree1, 14, 1);
		setAndValidateTree(tree1, 15, 19);
		setAndValidateTree(tree1, 9, 19);
		delete tree1;
		AVLTree<char, char>* tree2 = new AVLTree<char, char>();
		setAndValidateTree(tree2, (char)15, (char)1);
		setAndValidateTree(tree2, (char)14, (char)1);
		setAndValidateTree(tree2, (char)13, (char)13);
		setAndValidateTree(tree2, (char)12, (char)1);
		setAndValidateTree(tree2, (char)11, (char)51);
		setAndValidateTree(tree2, (char)10, (char)1);
		setAndValidateTree(tree2, (char)9, (char)19);
		setAndValidateTree(tree2, (char)8, (char)1);
		setAndValidateTree(tree2, (char)7, (char)1);
		setAndValidateTree(tree2, (char)6, (char)1);
		setAndValidateTree(tree2, (char)5, (char)12);
		setAndValidateTree(tree2, (char)4, (char)1);
		setAndValidateTree(tree2, (char)3, (char)81);
		setAndValidateTree(tree2, (char)2, (char)1);
		setAndValidateTree(tree2, (char)1, (char)1);
		delete tree2;
		AVLTree<char, char>* tree3 = new AVLTree<char, char>();
		setAndValidateTree(tree3, (char)6, (char)1);
		setAndValidateTree(tree3, (char)7, (char)2);
		setAndValidateTree(tree3, (char)2, (char)3);
		setAndValidateTree(tree3, (char)1, (char)4);
		setAndValidateTree(tree3, (char)4, (char)5);
		setAndValidateTree(tree3, (char)5, (char)6);
		setAndValidateTree(tree3, (char)3, (char)7);
		delete tree3;
		PrefixTrie<char, char>* trie1 = new PrefixTrie<char, char>();
		setAndValidateTrie(trie1, "value one", 9, (char)1);
		setAndValidateTrie(trie1, "value two", 9, (char)2);
		setAndValidateTrie(trie1, "value three", 11, (char)3);
		setAndValidateTrie(trie1, "value four", 10, (char)4);
		setAndValidateTrie(trie1, "value five", 11, (char)5);
		delete trie1;
		printf("  Finished testing util\n");
	}
	template <class Key, class Value> void Test::setAndValidateTree(AVLTree<Key, Value>* tree, Key key, Value value) {
		Value oldValue = tree->get(key);
		Value oldValue2 = tree->set(key, value);
		assert(oldValue == oldValue2);
		assert(tree->get(key) == value);
		validateHeights(tree->root);
	}
	template <class KeyElement, class Value> void Test::setAndValidateTrie(
		PrefixTrie<KeyElement, Value>* trie, const KeyElement* key, int keyLength, Value value)
	{
		Value oldValue = trie->get(key, keyLength);
		Value oldValue2 = trie->set(key, keyLength, value);
		assert(oldValue == oldValue2);
		assert(trie->get(key, keyLength) == value);
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
