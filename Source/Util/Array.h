#include "../General/globals.h"

template <class Type> class Array onlyInDebug(: public ObjCounter) {
public:
	int length; //copper: readonly
private:
	Type* inner;
	int innerLength;

public:
	Array();
	virtual ~Array();

	void deleteContents();
private:
	void resize(int scale);
	void shiftBack(int pos, int shift);
public:
	Type get(int pos);
	void set(int pos, Type t);
	void add(Type t, int pos);
	void add(Type t);
	void add(Array<Type>* a, int pos, bool deletable);
	void add(Array<Type>* a, bool deletable);
	void add(Array<Type>* a, int pos);
	void add(Array<Type>* a);
	void remove(int pos, int num);
	void remove(int pos);
	void replace(int pos, int count, Array<Type>* a);
	Type first();
	Type last();
	Type pop();
	void clear();
};
template <class Type> class ArrayIterator onlyInDebug(: public ObjCounter) {
private:
	int index;
	Array<Type>* a;

public:
	ArrayIterator(Array<Type>* pA);
	virtual ~ArrayIterator();

	Type getFirst();
	Type getNext();
	Type getThis();
	Type getPrevious();
	//Type getLast();
	//bool hasNext();
	bool hasThis();
	//bool hasPrevious();
	void replaceThis(Type t);
};
