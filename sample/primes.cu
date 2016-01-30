Function main = void() (
	Main.print("The first 100 prime numbers:\n");
	int numberToTest = 1;
	for (int i = 1; i <= 100;) (
		numberToTest += 1;
		boolean prime = true;
		for (int j = numberToTest / 2; j > 1; j--) (
			if (numberToTest % j == 0) (
				prime = false;
				break;
			)
		)
		if (prime) (
			Main.print(Main.str(numberToTest) + ", ");
			i++;
		)
	)
);
