#include "Test/Step04_Replace_replace.cu"
#replace int16 (
	short
)
#replace semicolon (;)
int16 a = 4 semicolon
#replace declareBInt (int b = (a);)
declareBInt;

#replace-input makeString(x) ("x")
String c = makeString(this is   a     /* comment and a */  string);
String d = makeString("other string");
String e = makeString(4);

#replace-input echo(x) (
	x
)
echo(String) f = "f";
#replace-input namedString(x) (String x = "x")
namedString(g);

#replace-input prependStri(x) (Strix)
prependStri(ng) h = "h";
#replace-input insertStNg(x) (Stxng)
insertStNg(ri) i = "i";
#replace-input appendRing(x) (xring)
appendRing(St) j = "j";

#replace-input concat(x, y) (
	xy
)
concat(Str, ing) k = "k";
#replace-input prependSt(x, y) (Stxy)
prependSt(ri, ng) l = "l";
#replace-input insertRi(x, y) (xriy)
insertRi(St, ng) m = "m";
#replace-input appendNg(x, y) (xyng)
appendNg(St, ri) n = "n";
#replace-input insertSRi(x, y) (Sxriy)
insertSRi(t, ng) o = "o";
#replace-input insertSG(x, y) (Sxyg)
insertSG(tr, in) p = "p";
#replace-input insertRiG(x, y) (xriyg)
insertRiG(St, n) q = "q";
#replace-input insertSRiG(x, y) (Sxriyg)
insertSRiG(t, n) r = "r";

echo() echo(String s) = "s";
echo(String t = "t");

Function main = void() (
	#replace-input aba(a, ba, b, ab) (aba u = "bab")
	aba(Str, ing, int, k);
);
