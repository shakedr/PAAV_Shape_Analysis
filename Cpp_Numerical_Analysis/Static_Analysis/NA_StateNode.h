#pragma once

#include "Variable.h"
#include "NA_Constant.h"
#include "StateNode.h"
#include "Tools.h"
#include <map>
#include <vector>
using namespace std;



/********************************************************\
header file
A state for Numerical Analysis
Contains:
	a mapping of variables to constants - Constant propagation
	a map of pairs of variables			- Variable Equation

Analysis is the cartesion product CPxVE
 
\********************************************************/

class VE_equality
{
public:
	Variable* first;
	Variable* second;

	VE_equality::VE_equality(Variable* f, Variable* s) { this->first = f; this->second = s; };

	bool operator== (const VE_equality& sec) const
	{
		return (this->first == sec.first && this->second == sec.second);
	}
};




class NA_StateNode : public StateNode
{
private:
	map<Variable*, NA_Constant*> assignments;

	// A map containing 2 pairs for each equality for ease of use, so each can be used as a key.
	std::vector<VE_equality> equalities;


public:
	// Constructors
	NA_StateNode::NA_StateNode(std::string name, CFG* ptr);    // Should create an empty node
	NA_StateNode::NA_StateNode(NA_StateNode* node_to_copy, std::string name);

	// Destructors
	NA_StateNode::~NA_StateNode();
	
	// Methods

	/****************************************\
	* Name: get_variable_value
	*
	* Description:
	* Returns the value of the variable in the current state
	* in:	Variable* var - The requested variable
	* out:	NA_Constant* - Pointer to the value
	* inout:
	* Notes:
	\*****************************************/
	NA_Constant* const	NA_StateNode::get_variable_value(Variable* var);

	/****************************************\
	* Name: set_variable
	*
	* Description:
	* Sets constant value 'cons' in variable 'var' in state 'this'
	* in:	Variable* var
	*		NA_Constant& cons
	* out:	
	* inout: 'this'
	* Notes:
	\*****************************************/
	void				NA_StateNode::set_variable(Variable* var, NA_Constant& cons);

	/****************************************\
	* Name: join
	*
	* Description:
	* Sets 'this' to be the join between 'this' and input 'state_2'
	* in:	StateNode& state_2
	* out:
	* inout: 'this'
	* Notes: Used to join into transition states and then copy
	\*****************************************/
	void				NA_StateNode::join(const StateNode& state_2);

	/****************************************\
	* Name: is_equivalent
	*
	* Description:
	* Check if the given argument state is equivalent to 'this'
	* in:	StateNode& compared
	* out:  bool
	* inout:
	* Notes:
	\*****************************************/
	bool const			NA_StateNode::is_equivalent(StateNode& compared);

	/****************************************\
	* Name: copy_abstract_state
	*
	* Description:
	* Copy the state of the given argument to 'this;
	* in:	StateNode* state_to_copy
	* out:
	* inout: this
	* Notes:
	\*****************************************/
	void				NA_StateNode::copy_abstract_state(StateNode* state_to_copy);

	/****************************************\
	* Name: print
	*
	* Description:
	* Prints state status
	* in:
	* out:
	* inout:
	* Notes:
	\*****************************************/
	virtual void		NA_StateNode::print();


	std::vector<VE_equality>& NA_StateNode::get_equalities() { return this->equalities; };

	/****************************************\
	* Name: add_var_equality
	*
	* Description:
	* Adds that var1==var2 in this state
	* in:		Variable* var1
	*			Variable* var2
	* out:
	* inout: 'this'
	* Notes: Adds var1 -> var2 , var2 -> var1
	\*****************************************/
	void				NA_StateNode::add_var_equality(Variable* var1, Variable* var2);


	/****************************************\
	* Name: rem_var_equality
	*
	* Description:
	* Removes that var1==var2 in this state
	* in:		Variable* var1
	*			Variable* var2
	* out:
	* inout: 'this'
	* Notes: If equality does not exist, doesn't remove anything
	\*****************************************/
	void				NA_StateNode::rem_var_equality(Variable* var1, Variable* var2);

	/****************************************\
	* Name: rem_all_var_equalities
	*
	* Description:
	* Removes All equalities involving the argument variable
	* Used to change the state when a variable's value has been updated
	* in:		Variable* var1
	*			Variable* var2
	* out:
	* inout: 'this'
	* Notes: Does not check if equalities exist. They would be remade by "reduce" if relevant
	\*****************************************/
	void				NA_StateNode::rem_all_var_equalities(Variable* var);


	/****************************************\
	* Name: reduce_left
	*
	* Description:

	* Derives values from constant propagation to variable equality
	* Goes over all variables in CP. For every pair that equal the same value, add equality
	* in:
	* out:
	* inout: 'this'
	* Notes: 
	\*****************************************/
	void				NA_StateNode::reduce_left();


	/****************************************\
	* Name: reduce_right
	*
	* Description:
	* Derives values from equalities to constant propagation
	* If two variables are equal in VE and one has a value, set the same value in the other
	* in:
	* out:
	* inout: 'this'
	* Notes:
	\*****************************************/
	void				NA_StateNode::reduce_right();

	/****************************************\
	* Name: reduce
	*
	* Description:
	* Iterate using reduce_right and reduce_left until reaching a fixed point
	* in:
	* out:
	* inout: 'this'
	* Notes:
	\*****************************************/
	void				NA_StateNode::reduce();


	/****************************************\
	* Name: equalities contained
	*
	* Description:
	* Returns 1 if all equalities in "contained" are in this.equalities
	* in: NA_StateNode& contained - state who's equalities are included or not
	* out: bool 
	* inout:
	* Notes:
	\*****************************************/
	bool NA_StateNode::equalities_contained(NA_StateNode& contained);


};