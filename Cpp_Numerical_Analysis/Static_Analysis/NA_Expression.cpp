#pragma once

#include "NA_Expression.h"
#include "CFG.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>

#define AND_OPERATOR '&'
#define OR_OPERATOR '|'
#define EQUALITY 1
#define INEQUALITY 0

/****************\
A Numerical Analysis class for expressions
interpreted as a conjunction of NA_AtomicExpression
if set as disjunction, use a vector of NA_Expressions, each will be a conjunction
Types of atomic expressions:
(even i)
(odd i)
(i == j)		(i != j)
(i == K)		(i != K)
(K_1 == K_2)	(K_1 != K_2)


Non atomic expressions will be of the form
(even i & odd j & i==K)
\****************/




NA_Expression::NA_Expression(std::string expression_string, CFG* graph_ptr, bool disjunction) : Expression (graph_ptr, disjunction)
{
	// Parse expression string into atomic expressions and create one for each of them.
	// Remove brackets
	expression_string = expression_string.substr(1, expression_string.length() - 2);
	this->parse_predicates(expression_string);
}

NA_Expression::~NA_Expression()
{
	std::vector<NA_AtomicExpression*>::iterator at_iter;
	for (at_iter = this->predicates.begin(); at_iter != this->predicates.end(); at_iter++)
	{
		delete *at_iter;
		
	}
	this->predicates.clear();
	std::vector<NA_Expression*>::iterator exp_iter;
	for (exp_iter = this->disjunction_expressions.begin(); exp_iter != this->disjunction_expressions.end(); exp_iter++)
	{
		delete (*exp_iter);
	}
}


bool3 NA_Expression::eval(StateNode* state)
{
	bool3 ans = bool3(FALSE);
	// If disjunction act as if disjunction
	if (this->disjunction)
	{
		std::vector<NA_Expression*>::iterator expr_iter = this->disjunction_expressions.begin();
		while ((expr_iter != this->disjunction_expressions.end()) && (ans.get_val() != TRUE)) // disjunction: iterate until finished or until TRUE
		{
			ans = ans || (*expr_iter)->eval(state);
			expr_iter++;
		}
	}
	else // Conjunction
	{
		ans = bool3(TRUE);
		NA_AtomicExpression* predicate;
		std::vector<NA_AtomicExpression*>::iterator pred_iter = this->predicates.begin();
		while ((pred_iter != this->predicates.end()) && (ans.get_val() == TRUE)) // Conjunction: iterate while TRUE until finished
		{
			int current_type = (*pred_iter)->get_type();
			switch (current_type)
			{
			case NA_EXP_EVEN:
				predicate = (Aexp_Even*) *(pred_iter);
				ans = ans && (predicate->eval(state));
				break;
			case NA_EXP_ODD:
				predicate = (Aexp_Odd*) *(pred_iter);
				ans = ans && (predicate->eval(state));
				break;
			case NA_EXP_VAR_EQ_VAR:
				predicate = (Aexp_VarEq*) *(pred_iter);
				ans = ans && (predicate->eval(state));
				break;
			case NA_EXP_VAR_EQ_CONST:
				predicate = (Aexp_VarEqConst*) *(pred_iter);
				ans = ans && (predicate->eval(state));
				break;
			case NA_EXP_CONST_EQ_CONST:
				predicate = (Aexp_ConstEqConst*) *(pred_iter);
				ans = ans && (predicate->eval(state));
				break;
			case NA_EXP_CONST_NEQ_CONST:
				predicate = (Aexp_ConstNeqConst*) *(pred_iter);
				ans = ans && (predicate->eval(state));
				break;
			case NA_EXP_VAR_NEQ_CONST:
				predicate = (Aexp_VarNeqConst*) *(pred_iter);
				ans = ans && (predicate->eval(state));
				break;
			case NA_EXP_VAR_NEQ_VAR:
				predicate = (Aexp_VarNeq*) *(pred_iter);
				ans = ans && (predicate->eval(state));
				break;
			}
			pred_iter++;
		}
	}
	return ans;
}


// Parse an arbitrary set of predicates
void NA_Expression::parse_predicates(std::string expression_string)
{
	if (this->disjunction == TRUE)
	{
		// find or operator position
		std::size_t or_pos = expression_string.find(OR_OPERATOR);
		// if none found - same as conjunction
		if (or_pos == std::string::npos)
		{
			this->disjunction_expressions.push_back(new NA_Expression(expression_string, this->containing_graph_ptr));
		}
		else
		{
			this->parse_predicates(expression_string.substr(0, or_pos ));
			this->parse_predicates(expression_string.substr(or_pos + 1, expression_string.length() - or_pos));
		}
	}
	else
	{
		parse_conjunction(expression_string);
	}
}


void NA_Expression::parse_conjunction(std::string expression_string)
{
	// find '&'
	std::size_t and_pos = expression_string.find(AND_OPERATOR);
	// if and_pos not found - no '&' ,  a single predicate
	if (and_pos == std::string::npos)
	{
		this->parse_single_predicate(expression_string); // Remove brackets
	}
	
	// Else, parse_predicates on both sides
	else
	{
		this->parse_conjunction(expression_string.substr(0, and_pos ));
		this->parse_conjunction(expression_string.substr(and_pos + 1, expression_string.length() - and_pos));
	}
}


void NA_Expression::parse_single_predicate(std::string single_predicate_string)
{
	
	
	// Find type of predicate:
	std::size_t operator_position = single_predicate_string.find("==");
	std::size_t neq_pos = single_predicate_string.find("!=");

	// Case "sum" predicate
	if ((single_predicate_string.find("sum") != std::string::npos))
	{
		varlist lhs , rhs;
		std::istringstream var_stream(single_predicate_string);
		std::string var_name;
		for (; var_stream >> var_name;)
		{
			if (var_name == std::string("sum"))
			{
				continue;
			}
			if (var_name == std::string("=="))
			{
				break;
			}
			else
			{
				lhs.push_back(this->containing_graph_ptr->get_var(var_name));
			}
		}
		var_stream >> var_name; // Get rid of second "sum"

		for (; var_stream >> var_name; )
		{
			rhs.push_back(this->containing_graph_ptr->get_var(var_name));
		}
		Aexp_sum* sum_pred = new Aexp_sum(lhs, rhs);
		this->insert_predicate(sum_pred);
		lhs.clear();
		rhs.clear();
	}
	// Case Eq or Neq
	else if ((operator_position != std::string::npos) | (neq_pos != std::string::npos))
	{
		bool eq_or_neq = EQUALITY;
		if (operator_position == std::string::npos)
		{
			operator_position = neq_pos;
			eq_or_neq = INEQUALITY;
		}
		std::string lval = single_predicate_string.substr(0, operator_position);
		std::string rval = single_predicate_string.substr(operator_position + 2);
		const char* lval_cstr = lval.c_str();
		const char* rval_cstr = rval.c_str();
		char* lend;
		char* rend;
		long lval_int = strtol(lval_cstr, &lend, 0);
		long rval_int = strtol(rval_cstr, &rend, 0);
		// If left size is a var
		if (this->containing_graph_ptr->get_var(lval) != NULL)
		{
			if (this->containing_graph_ptr->get_var(rval) != NULL)
			{
				if (eq_or_neq)
				{
					this->insert_predicate(new Aexp_VarEq(this->containing_graph_ptr->get_var(lval), this->containing_graph_ptr->get_var(rval)));
				}
				else
				{
					this->insert_predicate(new Aexp_VarNeq(this->containing_graph_ptr->get_var(lval), this->containing_graph_ptr->get_var(rval)));
				}
			}
			else if (rend != rval_cstr) // Read an integer here
			{
				if (eq_or_neq)
				{
					this->insert_predicate(new Aexp_VarEqConst(this->containing_graph_ptr->get_var(lval), NA_Constant(rval_int, 0)));
				}
				else
				{
					this->insert_predicate(new Aexp_VarNeqConst(this->containing_graph_ptr->get_var(lval), NA_Constant(rval_int, 0)));
				}
			}
			else
			{
				throw ParseError();
			}
		}
		
		// Else if left size is a constant
		else if (lend != lval_cstr)  // Read an integer on left side
		{
			if (this->containing_graph_ptr->get_var(rval) != NULL)
			{
				if (EQUALITY)
				{
					this->insert_predicate(new Aexp_VarEqConst(this->containing_graph_ptr->get_var(rval), NA_Constant(lval_int, 0)));
				}
				else
				{
					this->insert_predicate(new Aexp_VarNeqConst(this->containing_graph_ptr->get_var(rval), NA_Constant(lval_int, 0)));
				}
			}
			else if (rend != rval_cstr) // Both are constants
			{
				if (EQUALITY)
				{
					this->insert_predicate(new Aexp_ConstEqConst(NA_Constant(lval_int, 0), NA_Constant(rval_int, 0)));
				}
				else
				{
					this->insert_predicate(new Aexp_ConstNeqConst(NA_Constant(lval_int, 0), NA_Constant(rval_int, 0)));
				}
			}
			else
			{
				throw ParseError();
			}
		}
		else
		{
			throw ParseError();
		}
	}

	// Case even
	else if ((single_predicate_string.find("even") != std::string::npos))
	{
		this->insert_predicate(new Aexp_Even(
			this->containing_graph_ptr->get_var(single_predicate_string.substr(single_predicate_string.find("even") + 5))));
	}

	// Case odd
	else if ((single_predicate_string.find("odd") != std::string::npos))
	{
		this->insert_predicate(new Aexp_Odd(
			this->containing_graph_ptr->get_var(single_predicate_string.substr(single_predicate_string.find("odd") + 4))));
	}
	// Else throw exception
	else
	{
		throw ParseError();
	}
}


