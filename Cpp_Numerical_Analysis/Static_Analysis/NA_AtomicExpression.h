#pragma once

#include "Variable.h"
#include "NA_Constant.h"
#include "NA_StateNode.h"
#include "Tools.h"


/***********************\
abstract class
Evaluatable atomic predicate
given StateNode, it is true/false/MAYBE

(even i)
(odd i)
(i == j)		(i != j)
(i == K)		(i != K)
(K_1 == K_2)	(K_1 != K_2)

\************************/

class NA_AtomicExpression
{
protected:
	int exp_type_enum;

public:

	/****************************************\
	* Name: eval (abstract)
	*
	* Description:
	* Returns a bool3 with the evaluation of the predicate in the argument state
	* in:	StateNode* state
	* out:  bool3
	* inout:
	* Notes:
	\*****************************************/
	virtual bool3		NA_AtomicExpression::eval(StateNode* state) = 0;

	/****************************************\
	* Name: get_type (abstract)
	*
	* Description:
	* Returns an enumeration value of the type of predicate this is
	* in:	
	* out:  int - an enumeration according to:
	NA_EXP_EVEN 0
	NA_EXP_ODD 1
	NA_EXP_VAR_EQ_VAR 2
	NA_EXP_VAR_EQ_CONST 3
	NA_EXP_CONST_EQ_CONST 4
	\*****************************************/
	int					NA_AtomicExpression::get_type() const { return this->exp_type_enum; };

	virtual NA_AtomicExpression::~NA_AtomicExpression() = 0;

};
/*********************\
|     Even / Odd      |
\*********************/
class Aexp_Even : public NA_AtomicExpression
{
private:
	Variable* var;

public:
	Aexp_Even::Aexp_Even(Variable* var) { this->var = var; this->exp_type_enum = NA_EXP_EVEN; };
	bool3	Aexp_Even::eval(StateNode* state);
};

class Aexp_Odd : public NA_AtomicExpression
{
private:
	Variable* var;

public:
	Aexp_Odd::Aexp_Odd(Variable* var) { this->var = var; this->exp_type_enum = NA_EXP_ODD; };
	bool3	Aexp_Odd::eval(StateNode* state);
};


/*********************\
|     Equalities      |
\*********************/

class Aexp_VarEq : public NA_AtomicExpression
{
private:
	Variable* var1;
	Variable* var2;
	
public:
	Aexp_VarEq::Aexp_VarEq(Variable* first_var, Variable* second_var) { this->var1 = first_var; this->var2 = second_var;
																		this->exp_type_enum = NA_EXP_VAR_EQ_VAR; };
	bool3	Aexp_VarEq::eval(StateNode* state);
	Variable* Aexp_VarEq::get_var1() { return this->var1; };
	Variable* Aexp_VarEq::get_var2() { return this->var2; };
};

class Aexp_VarEqConst : public NA_AtomicExpression
{
private:
	Variable* var;
	NA_Constant cons;

public:
	Aexp_VarEqConst::Aexp_VarEqConst(Variable* var, NA_Constant cons) { this->var = var; this->cons = cons;
																		this->exp_type_enum = NA_EXP_VAR_EQ_CONST;	};
	Variable* get_var() { return this->var; };
	NA_Constant& get_constant() { return this->cons; };
	bool3	Aexp_VarEqConst::eval(StateNode* state);
};

class Aexp_ConstEqConst : public NA_AtomicExpression
{
private:
	NA_Constant cons1;
	NA_Constant cons2;
	
public:
	Aexp_ConstEqConst::Aexp_ConstEqConst(NA_Constant const1, NA_Constant const2) {	this->cons1 = const1; this->cons2 = const2;
																					this->exp_type_enum = NA_EXP_CONST_EQ_CONST;	};
	bool3	Aexp_ConstEqConst::eval(StateNode* state);
};

/*********************\
|     Inequalities    |
\*********************/


class Aexp_VarNeq : public NA_AtomicExpression
{
private:
	Variable* var1;
	Variable* var2;

public:
	Aexp_VarNeq::Aexp_VarNeq(Variable* first_var, Variable* second_var) {
		this->var1 = first_var; this->var2 = second_var;
		this->exp_type_enum = NA_EXP_VAR_NEQ_VAR;
	};
	bool3	Aexp_VarNeq::eval(StateNode* state);
	Variable* Aexp_VarNeq::get_var1() { return this->var1; };
	Variable* Aexp_VarNeq::get_var2() { return this->var2; };
};

class Aexp_VarNeqConst : public NA_AtomicExpression
{
private:
	Variable* var;
	NA_Constant cons;

public:
	Aexp_VarNeqConst::Aexp_VarNeqConst(Variable* var, NA_Constant cons) {
		this->var = var; this->cons = cons;
		this->exp_type_enum = NA_EXP_VAR_NEQ_CONST;
	};
	bool3	Aexp_VarNeqConst::eval(StateNode* state);
};

class Aexp_ConstNeqConst : public NA_AtomicExpression
{
private:
	NA_Constant cons1;
	NA_Constant cons2;

public:
	Aexp_ConstNeqConst::Aexp_ConstNeqConst(NA_Constant const1, NA_Constant const2) {
		this->cons1 = const1; this->cons2 = const2;
		this->exp_type_enum = NA_EXP_CONST_NEQ_CONST;
	}
	bool3	Aexp_ConstNeqConst::eval(StateNode* state);
};



/*********************\
|		  Sum	      |
\*********************/



typedef std::vector<Variable*> varlist;



/* Returns (\Sigma(x in lhs) == \Sigma (y in rhs) */

class Aexp_sum : public NA_AtomicExpression
{
private:
	varlist lhs;
	varlist rhs;

public:
	Aexp_sum::Aexp_sum(varlist& v1, varlist& v2)
	{
		varlist::iterator var_iter;
		this->lhs = v1;
		this->rhs = v2;
	}

	Aexp_sum::~Aexp_sum()
	{
		lhs.clear();
		rhs.clear();
	}

	bool3 Aexp_sum::eval(StateNode* state);
};


