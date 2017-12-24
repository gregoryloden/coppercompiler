#replace a (b)
#replace a (c)

#replace d (d)
int d;

#replace-input echo(x) (x)
echo;
(echo);
echo(,);
echo(a,);
echo(,b);
echo(a,b);

#replace-input concat(a, b) (ab)
concat(3, 4);
concat(a3, 4);
concat(3, b4);
