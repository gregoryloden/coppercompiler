#include "Project.h"

#define deleteSelfButNotContents(type) template <> void Array<type>::deleteSelfAndContents() { delete this; }

template class Array<char>;
template class Array<int>;
template class Array<string>;
template class Array<Token*>;
template class Array<LexToken*>;
template class Array<Identifier*>;
template class Array<CDirective*>;
template class Array<CDirectiveReplace*>;
template class Array<SourceFile*>;
template class Array<Array<Token*>*>;
template class Array<AVLNode<SourceFile*, bool>*>;
template class Array<AVLNode<char, PrefixTrie<char, SourceFile*>*>*>;
template class Array<AVLNode<char, PrefixTrie<char, CDirectiveReplace*>*>*>;
template class ArrayIterator<char>;
template class ArrayIterator<int>;
template class ArrayIterator<string>;
template class ArrayIterator<Token*>;
template class ArrayIterator<LexToken*>;
template class ArrayIterator<Identifier*>;
template class ArrayIterator<CDirective*>;
template class ArrayIterator<CDirectiveReplace*>;
template class ArrayIterator<SourceFile*>;
template class ArrayIterator<Array<Token*>*>;
template class ArrayIterator<AVLNode<SourceFile*, bool>*>;
template class ArrayIterator<AVLNode<char, PrefixTrie<char, SourceFile*>*>*>;
template class ArrayIterator<AVLNode<char, PrefixTrie<char, CDirectiveReplace*>*>*>;

template <class Type> Array<Type>::Array()
: onlyInDebug(ObjCounter(onlyWhenTrackingIDs("ARRAY")) COMMA)
inner(new Type[1])
, innerLength(1)
, length(0) {
}
template <class Type> Array<Type>::~Array() {
	delete[] inner;
}
deleteSelfButNotContents(char)
deleteSelfButNotContents(int)
deleteSelfButNotContents(string)
template <class Type> void Array<Type>::deleteSelfAndContents() {
	forEach(Type, t, this, ti)
		delete t;
	delete this;
}
//resize the array by the given scale
template <class Type> void Array<Type>::resize(int scale) {
	Type* newInner = new Type[innerLength * scale];
	for (int i = length - 1; i >= 0; i--)
		newInner[i] = inner[i];
	delete[] inner;
	inner = newInner;
	innerLength *= scale;
}
//shift the elements from [pos] to the end of the array, towards the end of the array, shifting them [shift] slots
//used when inserting/overwriting the old spaces
template <class Type> void Array<Type>::shiftBack(int pos, int shift) {
	if (length + shift > innerLength) {
		int scale = 2;
		while (length + shift > innerLength * scale)
			scale *= 2;
		resize(scale);
	}
	for (int i = length - 1; i >= pos; i--)
		inner[i + shift] = inner[i];
}
//get an item from the array
template <class Type> Type Array<Type>::get(int pos) {
	return inner[pos];
}
//set an item in the array at a specified index
template <class Type> void Array<Type>::set(int pos, Type t) {
	inner[pos] = t;
}
//add an item to the array at the given spot
template <class Type> void Array<Type>::add(Type t, int pos) {
	shiftBack(pos, 1);
	inner[pos] = t;
	length++;
}
//add an item to the array
template <class Type> void Array<Type>::add(Type t) {
	if (length >= innerLength)
		resize(2);
	inner[length] = t;
	length++;
}
//add another array's items to this array at the given spot, deleting the array if indicated
template <class Type> void Array<Type>::add(Array<Type>* a, int pos, bool deletable) {
	int shift = a->length;
	shiftBack(pos, shift);
	Type* otherInner = a->inner;
	for (int i = shift - 1; i >= 0; i--)
		inner[i + pos] = otherInner[i];
	length += shift;
	if (deletable)
		delete a;
}
//add another array's items to the end of this array, deleting the array if indicated
template <class Type> void Array<Type>::add(Array<Type>* a, bool deletable) {
	add(a, length, deletable);
}
//add another array's items to this array at the given spot
template <class Type> void Array<Type>::add(Array<Type>* a, int pos) {
	add(a, pos, true);
}
//add another array's items to the end of this array
template <class Type> void Array<Type>::add(Array<Type>* a) {
	add(a, length, true);
}
//remove the given number of items at the given spot
template <class Type> void Array<Type>::remove(int pos, int num) {
	for (int i = pos + num; i < length; i++)
		inner[i - num] = inner[i];
	length -= num;
}
//remove the item at the given spot
template <class Type> void Array<Type>::remove(int pos) {
	remove(pos, 1);
}
//replace the elements in the given range with the contents of the other array
template <class Type> void Array<Type>::replace(int pos, int count, Array<Type>* a) {
	if (count > a->length)
		remove(pos + a->length, count - a->length);
	else if (count < a->length)
		shiftBack(pos + count, a->length - count);
	Type* otherInner = a->inner;
	for (int i = a->length - 1; i >= 0; i--)
		inner[i + pos] = otherInner[i];
}
//return the first item
template <class Type> Type Array<Type>::first() {
	return inner[0];
}
//pop off the last item
template <class Type> Type Array<Type>::pop() {
	length -= 1;
	return inner[length];
}
//"empty" the array
template <class Type> void Array<Type>::clear() {
	length = 0;
}
template <class Type> ArrayIterator<Type>::ArrayIterator(Array<Type>* pA)
: onlyInDebug(ObjCounter(onlyWhenTrackingIDs("ARYITR")) COMMA)
a(pA)
, index(0) {
}
template <class Type> ArrayIterator<Type>::~ArrayIterator() {
	//do not delete the array, something else owns it
}
//get the first element of the array
template <class Type> Type ArrayIterator<Type>::getFirst() {
	return a->get(index = 0);
}
//get the next element of the array
template <class Type> Type ArrayIterator<Type>::getNext() {
	return a->get(index += 1);
}
//check if this iterator has the element it just returned via getNext()
template <class Type> bool ArrayIterator<Type>::hasThis() {
	return index < a->length;
}
//set an item in the array at the current index
template <class Type> void ArrayIterator<Type>::replaceThis(Type t) {
	a->set(index, t);
}
