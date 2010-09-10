#include "ZKit_TestSuite.h"
#include <typeinfo.h>

BEGIN_ZKIT

TestSuite* TestSuite::m_instance = NULL;
TestSuite* TestSuite::Instance()
{
	if (m_instance == NULL)
	{
		m_instance = new TestSuite();
	}
	return m_instance;
};

void TestSuite::Add( TestFixture* test )
{
	m_tests.push_back(test);
};

void TestSuite::Run()
{
	TestFixture* t = NULL;
	using namespace std;

	for (Iterator i = m_tests.begin(); i != m_tests.end(); ++i)
	{
		t = *i;
		printf("\n**************Tests Begin******************");
		printf("\nHello, I'm %s", (typeid(*t).name()));
		t->Run();
		printf("\n**************Tests End********************");
	}
};

TestSuite::TestSuite()
{

};

TestSuite::~TestSuite()
{
	for (Iterator i = m_tests.begin(); i != m_tests.end(); ++i)
	{
		TestFixture* t = (*i);
		delete t;
		t = NULL;
	}
	m_tests.clear();
};

void TestSuite::UnInstance()
{
	delete m_instance;
}

END_ZKIT