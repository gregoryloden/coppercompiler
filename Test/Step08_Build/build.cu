int aGlobal;
int bGlobal = 4;
int cGlobal = bGlobal;
int dGlobal = (4 == 4 ? (aGlobal = 5) : (aGlobal = 4)) + aGlobal;
int eGlobal = (4 == 4 ? (aGlobal = 5) : 4) + aGlobal;
int fGlobal = (4 == 4 ? 5 : (aGlobal = 4)) + aGlobal;
Function main = void() (
	int a = 1;
	int b;
	if (a == 1) (
		b = 2;
	) else (
		b = 3;
	)
	int c = b;
	int d;
	a = (4 == 4 ? (d = 5) : (d = 6)) + d;
	a = (4 == 4 ? (d = 5) : 6) + d;
	a = (4 == 4 ? 5 : (d = 6)) + d;
	int e;
	a = (4 == 4 ? (e = 5) : 6) + 7;
	int f;
	a = (4 == 4 ? 5 : (f = 6)) + 7;
);
