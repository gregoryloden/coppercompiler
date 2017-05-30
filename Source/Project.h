//general
#include "globals.h"
#include "helpers.h"

//structure
#include "pliers.h"
#include "AssemblyInstruction.h"
#include "Representation.h"
#include "Tokens.h"

//steps
#include "Step01_Lex.h"
#include "Step02_ParseDirectives.h"
#include "Step03_Include.h"
#include "Step04_Replace.h"
#include "Step05_ParseExpressions.h"
#include "Step06_Semant.h"
