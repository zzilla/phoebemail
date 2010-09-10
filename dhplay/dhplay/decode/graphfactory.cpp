#include "graphfactory.h"

GraphFactory g_GraphFactory;

GraphFactory::GraphFactory()
{
	InitializeCriticalSection(&m_graphfactoryCritSec);
}

GraphFactory::~GraphFactory()
{
	DeleteCriticalSection(&m_graphfactoryCritSec);

 	while (!m_graphlist.empty())
	{
		DhPlayGraph* tmp_playgraph = m_graphlist.back();
 		delete tmp_playgraph;
 		tmp_playgraph = NULL;
 		m_graphlist.pop_back();
 	}
}

DhPlayGraph* GraphFactory::getObject()
{
	EnterCriticalSection(&m_graphfactoryCritSec);

	DhPlayGraph* tmp_playgraph = NULL;

	if (m_graphlist.empty())
	{
		tmp_playgraph = new DhPlayGraph;
	}
	else
	{
		tmp_playgraph = m_graphlist.back();
		m_graphlist.pop_back();
	}

	LeaveCriticalSection(&m_graphfactoryCritSec);

	return tmp_playgraph;
}

void GraphFactory::returnObject(DhPlayGraph* object)
{
	EnterCriticalSection(&m_graphfactoryCritSec);
	
	m_graphlist.push_back(object);
	
	LeaveCriticalSection(&m_graphfactoryCritSec);
}
