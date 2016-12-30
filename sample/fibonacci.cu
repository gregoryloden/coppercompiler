Function main = void() (
	Main.print("The first 20 fibonacci numbers:\n");
	Main.print("Loop:\n");
	for (int i = 1; i <= 20; i++) (
		fib(i);
		Main.print(Main.str(returnValue) + ", ");
	)
	Main.print("\nRecursion:\n");
	for (int j = 1; j <= 20; j++) (
		fibRecursive(j);
		Main.print(Main.str(returnValue) + ", ");
	)
	Main.print("\nRecursion with accumulator:\n");
	for (int j = 1; j <= 20; j++) (
		fibRecursiveAcc(j);
		Main.print(Main.str(returnValue) + ", ");
	)
	Main.print("\n");
);
//i haven't actually implemented return values yet
//that or returns are bugged
//or both
//i can't remember
int returnValue;
Function fib = void(int i) (
	int fib1 = 1;
	int fib2 = 0;
	for (; i > 1; i--) (
		int sum = fib1 + fib2;
		fib2 = fib1;
		fib1 = sum;
	)
	returnValue = fib1;
);
Function fibRecursive = void(int i) (
	if (i < 2) (
		returnValue = i;
		//return;
	) else (
		fibRecursive(i - 1);
		int total = returnValue;
		fibRecursive(i - 2);
		returnValue += total;
	)
);
Function fibRecursiveAcc = void(int i) (
	fibRecursiveAccInternal(i, 1, 0);
);
Function fibRecursiveAccInternal = void(int i, int acc, int acc2) (
	if (i <= 1)
		returnValue = acc;
	else
		fibRecursiveAccInternal(i - 1, acc + acc2, acc);
);
