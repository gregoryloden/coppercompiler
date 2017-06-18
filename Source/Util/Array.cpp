#include "Project.h"

template class Array<string>;
template class Array<LexToken*>;
template class Array<Token*>;
template class Array<Identifier*>;
template class Array<CDirective*>;
template class Array<SourceFile*>;
template class ArrayIterator<string>;
template class ArrayIterator<LexToken*>;
template class ArrayIterator<Token*>;
template class ArrayIterator<Identifier*>;
template class ArrayIterator<CDirective*>;
template class ArrayIterator<SourceFile*>;

template <class type> Array<type>::Array()
: onlyInDebugWithComma(ObjCounter("ARRY"))
inner(new type[1])
, innerLength(1)
, length(0) {
}
template <class type> Array<type>::~Array() {
	delete[] inner;
}
//resize the array by the given scale
template <class type> void Array<type>::resize(int scale) {
	type* newInner = new type[innerLength * scale];
	for (int i = 0; i < length; i++)
		newInner[i] = inner[i];
	delete[] inner;
	inner = newInner;
	innerLength *= scale;
}
//add an item to the array at the given spot
template <class type> void Array<type>::add(type t, int pos) {
	if (length >= innerLength)
		resize(2);
	for (int i = length; i > pos; i--)
		inner[i] = inner[i - 1];
	inner[pos] = t;
	length++;
}
//add an item to the array
template <class type> void Array<type>::add(type t) {
	if (length >= innerLength)
		resize(2);
	inner[length] = t;
	length++;
}
//add another array's items to this array at the given spot, deleting the array if indicated
template <class type> void Array<type>::add(Array<type>* a, int pos, bool deletable) {
	int shift = a->length;
	type* otherInner = a->inner;
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
template <class type> void Array<type>::add(Array<type>* a, bool deletable) {
	add(a, length, deletable);
}
//add another array's items to this array at the given spot
template <class type> void Array<type>::add(Array<type>* a, int pos) {
	add(a, pos, true);
}
//add another array's items to the end of this array
template <class type> void Array<type>::add(Array<type>* a) {
	add(a, length, true);
}
//remove the given number of items at the given spot
template <class type> void Array<type>::remove(int pos, int num) {
	for (int i = pos + num; i < length; i++)
		inner[i - num] = inner[i];
	length -= num;
}
//remove the item at the given spot
template <class type> void Array<type>::remove(int pos) {
	remove(pos, 1);
}
//return the inner array
template <class type> type* Array<type>::getInner() {
	return inner;
}
//return the length
template <class type> int Array<type>::getLength() {
	return length;
}
//return the first item
template <class type> type Array<type>::first() {
	return inner[0];
}
//pop off the last item
template <class type> type Array<type>::pop() {
	length -= 1;
	return inner[length];
}
template <class type> ArrayIterator<type>::ArrayIterator(Array<type>* a)
onlyInDebug(: ObjCounter("ARYI")) {
	if (a != NULL) {
		inner = a->getInner();
		length = a->getLength();
	} else
		length = 0;
}
template <class type> ArrayIterator<type>::~ArrayIterator() {
	//do not delete inner because it's owned by an array
}
//get the first element of the array
template <class type> type ArrayIterator<type>::getFirst() {
	index = 0;
	if (length < 1)
		return NULL;
	return inner[0];
}
//get the next element of the array
template <class type> type ArrayIterator<type>::getNext() {
	if ((index += 1) >= length)
		return NULL;
	return inner[index];
}
//check if this iterator has the element it just returned via getNext()
template <class type> bool ArrayIterator<type>::hasThis() {
	return index < length;
}
