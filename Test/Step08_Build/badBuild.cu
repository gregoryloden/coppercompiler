int aGlobal = aGlobal;
int bGlobal = initialized + aGlobal; int initialized = 4;
int cGlobal = (4 == 4 ? (aGlobal = 5) : 4) + aGlobal;
int dGlobal = (4 == 4 ? 5 : (aGlobal = 4)) + aGlobal;
