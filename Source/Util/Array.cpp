#include "Project.h"

#define instantiateArrayTypes(type) template class Array<type>; template class ArrayIterator<type>;
#define instantiatePrefixTrieAVLNodeArrayTypes(type1, type2) \
	instantiateArrayTypes(AVLNode<type1 COMMA PrefixTrie<type1 COMMA type2>*>*);
#define instantiateNonPointerArrayTypes(type) \
	instantiateArrayTypes(type);\
	template <> void Array<type>::deleteContents() {}

instantiateArrayTypes(AbstractCodeBlock*);
instantiateArrayTypes(AssemblyInstruction*);
instantiateArrayTypes(AssemblyStorage*);
instantiateArrayTypes(CALL*);
instantiateArrayTypes(CDataType*);
instantiateArrayTypes(CDirective*);
instantiateArrayTypes(CDirectiveReplace*);
instantiateArrayTypes(CVariableData*);
instantiateArrayTypes(CVariableDefinition*);
instantiateArrayTypes(ErrorMessage*);
instantiateArrayTypes(FunctionStaticStorage*);
instantiateArrayTypes(FunctionDefinition*);
instantiateArrayTypes(Identifier*);
instantiateArrayTypes(LexToken*);
instantiateArrayTypes(Operator*);
instantiateArrayTypes(ParameterStorage*);
instantiateArrayTypes(Path*);
instantiateArrayTypes(SourceFile*);
instantiateArrayTypes(Statement*);
instantiateArrayTypes(StringStaticStorage*);
instantiateArrayTypes(StringLiteral*);
instantiateArrayTypes(TempStorage*);
instantiateArrayTypes(Token*);
instantiateArrayTypes(VariableDeclarationList*);
instantiateArrayTypes(ValueStaticStorage*);
instantiateArrayTypes(Array<AssemblyStorage*>*);
instantiateArrayTypes(AVLNode<AssemblyStorage* COMMA Array<AssemblyStorage*>*>*);
instantiateArrayTypes(AVLNode<BitSize COMMA int>*);
instantiateArrayTypes(AVLNode<char COMMA char>*);
instantiateArrayTypes(AVLNode<CVariableDefinition* COMMA AssemblyStorage*>*);
instantiateArrayTypes(AVLNode<FunctionDefinition* COMMA FunctionStaticStorage*>*);
instantiateArrayTypes(AVLNode<int COMMA int>*);
instantiateArrayTypes(AVLNode<SourceFile* COMMA bool>*);
instantiateNonPointerArrayTypes(BitSize);
instantiateNonPointerArrayTypes(char);
instantiateNonPointerArrayTypes(int);
instantiateNonPointerArrayTypes(SpecificRegister);
instantiateNonPointerArrayTypes(string);
instantiatePrefixTrieAVLNodeArrayTypes(char, char);
instantiatePrefixTrieAVLNodeArrayTypes(char, CDataType*);
instantiatePrefixTrieAVLNodeArrayTypes(char, CDirectiveReplace*);
instantiatePrefixTrieAVLNodeArrayTypes(char, CVariableData*);
instantiatePrefixTrieAVLNodeArrayTypes(char, CVariableDefinition*);
instantiatePrefixTrieAVLNodeArrayTypes(char, SourceFile*);

template <class Type> Array<Type>::Array()
: onlyInDebug(ObjCounter(onlyWhenTrackingIDs("ARRAY")) COMMA)
inner(new Type[1])
, innerLength(1)
, length(0) {
}
template <class Type> Array<Type>::~Array() {
	delete[] inner;
}
template <class Type> void Array<Type>::deleteContents() {
	forEach(Type, t, this, ti)
		delete t;
}
template <class Type> Array<Type>* Array<Type>::newArrayWith(Type val) {
	Array<Type>* result = new Array<Type>();
	result->add(val);
	return result;
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
	length += shift;
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
template <class Type> void Array<Type>::insert(Type t, int pos) {
	shiftBack(pos, 1);
	inner[pos] = t;
}
//add an item to the array
template <class Type> void Array<Type>::add(Type t) {
	if (length >= innerLength)
		resize(2);
	inner[length] = t;
	length++;
}
//add an item to the array if it's not already in the array
template <class Type> void Array<Type>::addNonDuplicate(Type t) {
	for (int i = length - 1; i >= 0; i--) {
		if (inner[i] == t)
			return;
	}
	add(t);
}
//add another array's items to this array at the given spot
template <class Type> void Array<Type>::insert(Array<Type>* a, int pos) {
	int shift = a->length;
	shiftBack(pos, shift);
	Type* otherInner = a->inner;
	for (int i = shift - 1; i >= 0; i--)
		inner[i + pos] = otherInner[i];
}
//add another array's items to the end of this array
template <class Type> void Array<Type>::add(Array<Type>* a) {
	insert(a, length);
}
//add another array's items (that are not already in this array) to the end of this array
template <class Type> void Array<Type>::addNonDuplicates(Array<Type>* a) {
	Type* otherInner = a->inner;
	int otherLength = a->length;
	for (int i = 0; i < otherLength; i++)
		addNonDuplicate(otherInner[i]);
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
//remove the first item equal to the given item
template <class Type> void Array<Type>::removeItem(Type t) {
	for (int i = 0; i < length; i++) {
		if (inner[i] == t) {
			remove(i);
			return;
		}
	}
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
//return the last item
template <class Type> Type Array<Type>::last() {
	return inner[length - 1];
}
//pop off the last item
template <class Type> Type Array<Type>::pop() {
	return inner[length -= 1];
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
//advance to the next element of the array and return it
template <class Type> Type ArrayIterator<Type>::getNext() {
	return a->get(index += 1);
}
//get the element at the current index of the array
template <class Type> Type ArrayIterator<Type>::getThis() {
	return a->get(index);
}
//go back to the previous element of the array and return it
template <class Type> Type ArrayIterator<Type>::getPrevious() {
	return a->get(index -= 1);
}
//check if this iterator has the element it just returned via getNext()
template <class Type> bool ArrayIterator<Type>::hasThis() {
	return (unsigned int)index < (unsigned int)(a->length);
}
//set an item in the array at the current index
template <class Type> void ArrayIterator<Type>::replaceThis(Type t) {
	a->set(index, t);
}
