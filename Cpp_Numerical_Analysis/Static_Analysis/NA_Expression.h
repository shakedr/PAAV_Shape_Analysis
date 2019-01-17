#pragma once

#include "Tools.h"
#include "Expression.h"
#include "NA_StateNode.h"
#include "NA_AtomicExpression.h"
#include <vector>
using namespace std;


/****************\
A Numerical Analysis class for expressions
interpreted as a conjunction of NA_AtomicExpression (i.e. x_1 AND x_2... AND x_n)
Parentheses don't matter because conjunction is associative
\****************/



class NA_Expression : public Expression
{
private:
	std::vector<NA_AtomicExpression*> predicates;
	std::vector<NA_Expression*>		  disjunction_expressions;

	// 

	/****************************************\
	* Name: parse_conjunction
	*
	* Description:
	* Given a conjunction string, parses it into the predicate vector
	* in:
	* out:
	* inout: 'this'
	* Notes:
	\*****************************************/
	void NA_Expression::parse_conjunction(std::string expression_string);

	/****************************************\
	* Name: parse_predicates
	*
	* Description:
	* Given a string, parses it into the expressions and predicates vector
	* in:
	* out:
	* inout: 'this'
	* Notes:
	\*****************************************/
	void NA_Expression::parse_predicates(std::string expression_string);


	/****************************************\
	* Name: parse_predicates
	*
	* Description:
	* Given a string containing a single predicate, parses it into the predicate vector
	* in:
	* out:
	* inout: 'this'
	* Notes: Helper method for this->parse_predicates()
	*		 Raises ParseError exception if single_predicate_string is not according to format
	\*****************************************/
	void NA_Expression::parse_single_predicate(std::string single_predicate_string);


public:

	/* Constructor */
	NA_Expression::NA_Expression(std::string expression_string, CFG* graph_ptr, bool disjunction = 0);

	NA_Expression::~NA_Expression();

	void NA_Expression::insert_predicate(NA_AtomicExpression* inserted) { this->predicates.push_back(inserted); };

	/****************************************\
	* Name: eval
	*
	* Description:
	* evaluate the expression in the argument state
	* in:	StateNode* state
	* out:	bool3
	* inout: 
	* Notes: 
	\*****************************************/
	bool3 NA_Expression::eval(StateNode* state);

	std::vector<NA_AtomicExpression*>& get_predicates() { return this->predicates; };

};