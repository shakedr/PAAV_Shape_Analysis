#pragma once

#include "Command.h"
#include "NA_Expression.h"
#include "NA_Constant.h"
#include "NA_StateNode.h"
#include "Tools.h"



/***************************\
*   Class for all commands of Numerical Analysis
*   skip
*   i := j			(variable assignment)
*   i :- K			(constant assignment)
*   i := ?			(random assignment)
*   i := j + 1		(assign+increment)
*   i := j - 1		(assign+decrement)
*   assume E		(assume)
*   assert ORC		(assert)
\****************************/

class NA_Command : public Command
{
private:
	int type;
	NA_Expression*	expr;				// Used in assume or assert commands, otherwise empty
	Variable*		assignee;			// Used in assignment commands 
	NA_Constant		assigned_const;		// Used in assignment commands
	Variable*		assigned_var;		// Used in var assignment commands and assign dec/inc


public:
	// Constructors
	NA_Command::NA_Command(std::string command_string, CFG* graph_ptr) ;   // Parses command string into the command type and the command itself.
	
	// Destructor
	NA_Command::~NA_Command();

	// Methods

	int			NA_Command::get_type()	{ return this->type; };

	/****************************************\
	* Name: eval_expression
	*
	* Description:
	* Evaluate the command's condition in the argument 'state'
	* in:	StateNode* state
	* out:  bool3
	* inout:
	* Notes:
	\*****************************************/
	bool3		NA_Command::eval_expression(StateNode& state);

	/****************************************\
	* Name: apply
	*
	* Description:
	* Apply the command to the pre_state by join(command(pre_state), dest)
	* in:	StateNode& pre_state
	* out:  bool : 1 if the result differs from "dest", 0 if result is equivalent to original "dest"
	* inout: StateNode& dest : reference to destination node updated
	* Notes: Does not clear the new state created!
	*		 Applies the command using the type-appropriate method
	\*****************************************/
	bool	NA_Command::apply(StateNode& pre_state, StateNode& dest);

	// Numerical Analysis type-appropriate methods

	void NA_Command::var_assignment		(NA_StateNode& state);
	void NA_Command::constant_assignment(NA_StateNode& state);
	void NA_Command::random_assignment	(NA_StateNode& state);
	void NA_Command::assign_increment	(NA_StateNode& state);
	void NA_Command::assign_decrement	(NA_StateNode& state);
	void NA_Command::assume				(NA_StateNode& state);
	void NA_Command::assert				(NA_StateNode& state);
};