#pragma once


#include "NA_AtomicExpression.h"


NA_AtomicExpression::~NA_AtomicExpression()
{

}


/*********************\
|     Even/Odd	      |
\*********************/


bool3 Aexp_Even::eval(StateNode* state)
{
	// Can cast to NA_StateNode - its NA_AtomicExpression types
	NA_StateNode* NA_state	= (NA_StateNode*) state;
	NA_Constant* var_val = NA_state->get_variable_value(this->var);
	if (var_val->get_parity() == 1)
	{
		return bool3(TRUE);
	}
	else if (var_val->get_parity() == 0)
	{
		return bool3(FALSE);
	}
	else
	{
		return bool3(UNK);
	}
}

bool3 Aexp_Odd::eval(StateNode* state)
{
	// Can cast to NA_StateNode - its NA_AtomicExpression types
	NA_StateNode* NA_state = (NA_StateNode*)state;
	NA_Constant* var_val = NA_state->get_variable_value(this->var);
	if (var_val->is_odd() == 1)
	{
		return bool3(TRUE);
	}
	else if (var_val->get_parity() == 0)
	{
		return bool3(TRUE);
	}
	else
	{
		return bool3(UNK);
	}
}



/*********************\
|     Equations     |
\*********************/


bool3 Aexp_VarEq::eval(StateNode* state)
{
	// Can cast to NA_StateNode - its NA_AtomicExpression types
	NA_StateNode* NA_state = (NA_StateNode*)state;
	if (NA_state->get_variable_value(this->var1)->is_top() ||
		NA_state->get_variable_value(this->var2)->is_top() ||
		NA_state->get_variable_value(this->var1)->is_bottom() ||
		NA_state->get_variable_value(this->var2)->is_bottom())
	{
		return bool3(UNK);
	}
	if (NA_state->get_variable_value(this->var1)->get_val() == NA_state->get_variable_value(this->var2)->get_val())
	{
		return bool3(TRUE);
	}
	else
	{
		return bool3(FALSE);
	}
}

bool3 Aexp_VarEqConst::eval(StateNode* state)
{
	// Can cast to NA_StateNode - its NA_AtomicExpression types
	NA_StateNode* NA_state = (NA_StateNode*)state;
	if (NA_state->get_variable_value(this->var)->is_top() ||
		NA_state->get_variable_value(this->var)->is_bottom())
	{
		return bool3(UNK);
	}
	if (NA_state->get_variable_value(this->var)->get_val() == this->cons.get_val())
	{
		return bool3(TRUE);
	}
	else
	{
		return bool3(FALSE);
	}
}

bool3 Aexp_ConstEqConst::eval(StateNode* state)
{
	if (this->cons2.get_val() == this->cons2.get_val())
	{
		return bool3(TRUE);
	}
	else
	{
		return bool3(FALSE);
	}
}



/*********************\
|     Inequations     |
\*********************/

bool3 Aexp_VarNeq::eval(StateNode* state)
{
	// Can cast to NA_StateNode - its NA_AtomicExpression types
	NA_StateNode* NA_state = (NA_StateNode*)state;
	if (NA_state->get_variable_value(this->var1)->is_top() ||
		NA_state->get_variable_value(this->var2)->is_top() ||
		NA_state->get_variable_value(this->var1)->is_bottom() ||
		NA_state->get_variable_value(this->var2)->is_bottom())
	{
		return bool3(UNK);
	}
	if (NA_state->get_variable_value(this->var1)->get_val() != NA_state->get_variable_value(this->var2)->get_val())
	{
		return bool3(TRUE);
	}
	else
	{
		return bool3(FALSE);
	}
}

bool3 Aexp_VarNeqConst::eval(StateNode* state)
{
	// Can cast to NA_StateNode - its NA_AtomicExpression types
	NA_StateNode* NA_state = (NA_StateNode*)state;
	if (NA_state->get_variable_value(this->var)->is_top() ||
		NA_state->get_variable_value(this->var)->is_bottom())
	{
		return bool3(UNK);
	}
	if (NA_state->get_variable_value(this->var)->get_val() != this->cons.get_val())
	{
		return bool3(TRUE);
	}
	else
	{
		return bool3(FALSE);
	}
}

bool3 Aexp_ConstNeqConst::eval(StateNode* state)
{
	if (this->cons2.get_val() != this->cons2.get_val())
	{
		return bool3(TRUE);
	}
	else
	{
		return bool3(FALSE);
	}
}


/*********************\
|		  Sum	      |
\*********************/




bool3 Aexp_sum::eval(StateNode* state)
{
	// Only for NA, so casting is allowed
	NA_StateNode* cast_state = (NA_StateNode*)state;
	varlist::iterator var_iter;
	int rhs_sum = 0;
	for (var_iter = rhs.begin(); var_iter != rhs.end() ; var_iter++)
	{
		NA_Constant* var_val = cast_state->get_variable_value((*var_iter));
		if (var_val->is_top() || var_val->is_bottom())
		{
			return bool3(UNK);
		}
		else
		{
			rhs_sum += var_val->get_val();
		}
	}
	int lhs_sum = 0;
	for (var_iter = lhs.begin(); var_iter != lhs.end();
		var_iter++)
	{
		NA_Constant* var_val = cast_state->get_variable_value((*var_iter));
		if (var_val->is_top() || var_val->is_bottom())
		{
			return bool3(UNK);
		}
		else
		{
			lhs_sum += var_val->get_val();
		}
	}
	if (rhs_sum == lhs_sum)
	{
		return bool3(TRUE);
	}
	else
	{
		return bool3(FALSE);
	}
}