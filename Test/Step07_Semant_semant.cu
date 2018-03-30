int a = 4000 + (int b = (Function echo = echoInt)(int c = 4) + (int)4 + echo(getInt()));
int d;
float f = 1.5;
Function main3 = main2;
Function main2 = main;
Function main = void() (
	Main.print(Main.str(c));
	byte g, short h, String i;
	g = (byte)a;
	h = (short)g;
	float j = f * h;
	i = (String)getString();
	bool k = (int l = 4) == 4 && (int m = l) == 5 && (int n = m) == 6;
	bool m = k && l == 4 ? (int n = 4) == 4 && n == 4 : (int n = 4) == 4 || n == 4;

	if ((int n = 4) == 4 && (int o = 4) == 4)
		n = o;
	if ((int o = n) == 4 && (int p = 4) == 4) (
		o = p;
	) else (
		int p = 3;
		Main.print(Main.str(p));
		return;
	)
	p = o;

	if ((int q = 4) == 5 || (int r = 4) == 5)
		q = q;
	else
		q = r;
	if ((int r = q) == 5 || (int s = 4) == 5)
		return;
	else
		r = s;
	s = r;

	for (int t = 0; t < 10; t++)
		;
	for (int t = 0; t < 10; t++)
		;
	for (; s < 10; s++)
		;
	for (; s < 10;)
		s++;
	s = (raw int)getString();
	//something.something.something();
	n = true ? true ? true ? 1 : 0 : 0 : 0;
	o = true ? false ? 0 : false ? 0 : 2 : 0;
	p = false ? false ? 0 : 0 : true ? 3 : 0;
	q = false ? 0 : false ? 0 : false ? 0 : 5;
	Main.print(Main.str(n));
	Main.print(Main.str(o));
	Main.print(Main.str(p));
	Main.print(Main.str(q));
);
Function getInt = int() (
	return 3;
);
Function echoInt = int(int e) (
	return e;
);
Function getString = String() (
	if (true)
		return "a string";
	else
		return true ? "true" : "false";
);
Function genericFunctionReturningFunction = Function() ( return getInt; );
//Function intFunction2, stringFunction2 = Group(getInt, getString);
//int fromGroup = Group(3);
