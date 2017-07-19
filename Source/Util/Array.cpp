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
//add an item to the array at the given spot
template <class Type> void Array<Type>::add(Type t, int pos) {
	if (length >= innerLength)
		resize(2);
	for (int i = length; i > pos; i--)
		inner[i] = inner[i - 1];
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
	Type* otherInner = a->inner;
	if (shift + length > innerLength) {
		int scale = 2;
		while (shift + length > innerLength * scale)
			scale *= 2;
		resize(scale);
	}
	for (int i = length - 1; i >= pos; i--)
		inner[i + shift] = inner[i];
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
//return the first item
template <class Type> Type Array<Type>::first() {
	return inner[0];
}
//pop off the last item
template <class Type> Type Array<Type>::pop() {
	length -= 1;
	return inner[length];
}
template <class Type> ArrayIterator<Type>::ArrayIterator(Array<Type>* a)
onlyInDebug(: ObjCounter(onlyWhenTrackingIDs("ARYITR"))) {
	if (a != NULL) {
		inner = a->inner;
		length = a->length;
	} else
		length = 0;
}
template <class Type> ArrayIterator<Type>::~ArrayIterator() {
	//do not delete inner because it's owned by an array
}
//get the first element of the array
template <class Type> Type ArrayIterator<Type>::getFirst() {
	index = 0;
	if (length < 1)
		return NULL;
	return inner[0];
}
//get the next element of the array
template <class Type> Type ArrayIterator<Type>::getNext() {
	if ((index += 1) >= length)
		return NULL;
	return inner[index];
}
//check if this iterator has the element it just returned via getNext()
template <class Type> bool ArrayIterator<Type>::hasThis() {
	return index < length;
}
