#pragma once

#include "StateNode.h"
#include "Tools.h"


/******************\
Command for an abstract edge
Given a StateNode, dervies the StateNode after the command
\******************/

class CFG;

class Command
{
protected:
	CFG* containing_graph_ptr;
	std::string command_string;

public:
	
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
	virtual bool Command :: apply(StateNode& pre_state,StateNode& dest) = 0; 

	/****************************************\
	* Name: eval_expression (abstract)
	*
	* Description:
	* Evaluate the expression in the command (assume/assert) in the state
	* in:	StateNode& state	: The state by which to evaluate the expression
	* out:	bool3				:
	* inout:
	* Notes: 
	\*****************************************/
	virtual bool3 Command :: eval_expression(StateNode& state) = 0;

	std::string get_command_string() { return this->command_string; };

	/* Constructor */
	Command::Command(void* containing_graph) { containing_graph_ptr = (CFG*)containing_graph; };

	virtual ~Command() = 0;
};