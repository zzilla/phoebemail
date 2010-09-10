#ifndef _ZKit_TestSuite_h_
#define _ZKit_TestSuite_h_

#include "ZKit_Config.h"

BEGIN_ZKIT
//用于进行单元测试的工具类. by qguo.
class TestFixture
{
public:
	virtual ~TestFixture(){};
	virtual int Run() = 0;
protected:
	virtual int SetUp() {return 0;};
	virtual int TearDown() {return 0;};
};

class TestSuite
{	
public:	
	static TestSuite* Instance();

	static void UnInstance();

	void Add(TestFixture* test);

	void Run();
private:
	TestSuite();
	~TestSuite();

	static TestSuite* m_instance;

	typedef std::vector<TestFixture*> Vector;
	typedef Vector::iterator Iterator;
	Vector m_tests;
};

#define RUN_TESTCASE_COUNT(TEST_METHOD, count) \
{\
	int _i = 0;\
	clock_t _start = 0;\
	clock_t _cost  = 0;\
	clock_t _total = 0;\
	if (0 == SetUp())\
	{\
		printf("\n\n%s starts for %d times!",  #TEST_METHOD, count);\
		while (_i < count)\
		{\
			printf("\n\n%s[%d] starts!", #TEST_METHOD, _i);\
			_start = clock();\
			TEST_METHOD();\
			_cost = clock() - _start;\
			_total += _cost;\
			printf("\n%s[%d] costs %d ticks!",#TEST_METHOD, _i, _cost);\
			++_i;\
		};\
		printf("\n%s stops, average: %d/%d = %d ticks", #TEST_METHOD, _total, count, _total/count);\
		if (0 != TearDown()) assert(0);\
	}\
	else\
	{\
	    assert(0);\
	}\
};

#define ADD_TESTFIXTURE(TEST_FIXTURE) TestSuite::Instance()->Add(new TEST_FIXTURE());
#define RUN_TESTSUITE TestSuite::Instance()->Run();
#define REL_TESTSUITE TestSuite::UnInstance();

END_ZKIT
#endif // _ZKit_TestSuite_h_