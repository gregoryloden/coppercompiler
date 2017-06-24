#include "pliers.h"

//general
#include "globals.h"
#include "helpers.h"

//util
#include "Util/Array.h"
#include "Util/AVLTree.h"
#include "Util/BigInt.h"
#include "Util/Trie.h"

//structure
#include "AssemblyInstruction.h"
#include "Representation.h"
#include "Tokens.h"

//steps
#include "Steps/Step01_Lex.h"
#include "Steps/Step02_ParseDirectives.h"
#include "Steps/Step03_Include.h"
#include "Steps/Step04_Replace.h"
#include "Steps/Step05_ParseExpressions.h"
#include "Steps/Step06_Semant.h"
