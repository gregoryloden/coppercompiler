template <class Key, class Value> class PrefixTrie;

class CType onlyInDebug(: public ObjCounter) {
public:
	CType(onlyWhenTrackingIDs(char* pObjType));
	virtual ~CType();

	static PrefixTrie<char, CType*>* globalTypes;
};
class CVoid: public CType {
public:
	CVoid();
	virtual ~CVoid();
};
