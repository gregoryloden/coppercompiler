  /*These comments * are included*/
	   //to maximize code coverage
int a_1 = 0xaF / 5_ * -5;
bool b = true;
bool c = false;
int d = 01 + 0o7 - 0b10101;
float e = 1.0 * 1.0^1 / 100.0^-2 + 0xF.0^F;
String f = "a\n\r\t\0\b\x20\\\"\'";
byte g = ' ';
Function main = void() (
	bool(int i, int j) (
		i *= -i++ * ~j--;
		i /= i~~ / j~-;
		i %= i % j;
		i += i + j;
		i -= i - j;
		i <<= i << j;
		i >>= i >> j;
		i >>>= i >>> j;
		i &= i & j;
		i |= i | j;
		i ^= i ^ j;
		bool k = (i > j || i < j) == (i >= j && i <= j);
		String..length;
		Main.str(i);
		return !k~! != false ? i : j;
	)(1, 2);
);
#replace zzzzzz ()
