#ifndef _GRAPHFACTORY
#define _GRAPHFACTORY

#include "playgraph.h"
#include <vector>

class GraphFactory
{
public:
	GraphFactory();
	~GraphFactory();

	DhPlayGraph* getObject();
	void returnObject(DhPlayGraph* object);
	
private:

	std::vector<DhPlayGraph*> m_graphlist;
	CRITICAL_SECTION  m_graphfactoryCritSec ;
};

extern GraphFactory g_GraphFactory;

#endif
