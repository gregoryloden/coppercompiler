int uninitialized;
Function main3 = main2;
Function main2 = main;
Function main = void() (
	int a = 1;
	int b;
	if (a == 1) (
		b = 2;
	) else (
		b = 3;
	)
	int c = b;
);
Function intFunction = int() (
	return 3;
);
Function stringFunction = String() (
	return "";
);
//Function intFunction2, stringFunction2 = Group(intFunction, stringFunction);
//int fromGroup = Group(3);
