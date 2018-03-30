#include "Test/Step07_Semant_circularInclude_2.cu"
int a = b;
int c = 4;
Function main = void() (
	Main.print(Main.str(c));
);
