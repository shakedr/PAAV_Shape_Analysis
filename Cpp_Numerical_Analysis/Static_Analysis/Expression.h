#pragma once

#include "StateNode.h"
#include "Tools.h"


/******************\
 Logical Expression abstract class
 Conjunction
\******************/

class CFG;

class Expression
{
protected:
	CFG* containing_graph_ptr;
	bool disjunction; // Set to TRUE if it is a disjunction. Otherwise expression is conjunction

public:
	bool Expression::is_disjunction() { return this->disjunction; };
	Expression::Expression(void* containing_graph, bool disj = 0) { containing_graph_ptr = (CFG*)containing_graph; disjunction = disj; };
	virtual ~Expression() = 0;
};