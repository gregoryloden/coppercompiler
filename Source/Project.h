//general
#include "globals.h"
#include "helpers.h"

//util
#include "Util/Array.h"
#include "Util/BigInt.h"

//structure
#include "pliers.h"
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
