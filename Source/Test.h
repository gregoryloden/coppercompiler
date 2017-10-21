#ifdef DEBUG
	class Test {
	public:
		static void testFiles();
	private:
		static void testFile(const char* fileName, int errorsExpected);
	};
#endif
