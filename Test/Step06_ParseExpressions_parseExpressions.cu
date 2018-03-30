#include "Test/Step06_ParseExpressions_parseExpressions.cu"
;
String a;
int b = f();
int c, int d, e = -b + 4 + (b++) * (int)(raw byte)4;
Function f = int() (
	return i(4);
);
Function main = void() (
	int g = (int h = 1);
	#replace nothing ()
	if (g == g)
		g = true ? 4 : 4;
	if (g > g) (
		g = false ? 4 : true ? 4 : 4;
	) else (
		h = true ? true ? 4 : 4 : 4;
	)
	if (g != 4)
		if (h == 4)
			h = 5;
		else
			g = 10;
	if (h == 4)
		(f());
	else if (h == 5)
		for (int n = 0; n < 10; n++) (
			while (n < 10)
				n++;
			break;
			while (n < 10) (
				n++;
			)
		)
	else
		return;
	for (int o = 0; o < 10;) (
		do
			o++;
		while (o < 10)
		do (
			o++;
			continue 2;
		) while (o < 10);
	)
	int p = 0;
	for (; p < 10; p++)
		;
	for (; p < 10;)
		p++;
);
Function i = int(int j) (
	return k(j, j);
);
Function k = int(int l, int m) (
	return l + m;
);
Function r = void() (
	if (true) (
	)
	if (false)
		;
	if (true) (
		if (true) ()
	) else if (false) (
		Function o = void () ();
	) else (
		int p = ((Function<Function<int()>()>) q) () ();
	)
);
Function q = Function<int()>() (
	return f;
);
