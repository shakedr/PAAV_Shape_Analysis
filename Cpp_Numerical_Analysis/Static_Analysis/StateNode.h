#pragma once

#include "Variable.h"
#include "Constant.h"
#include "Tools.h"
#include <string>
#include <map>
using namespace std;




/********************************************************\
An abstract state of the program, i.e., L_i in the project guidelines
Formally: A group of pairs (Var*, X) when X depends on the analysis
examples:
	Numerical Analysis: X is a Constant, and the pair is an equation
	Pointer Analysis: X is {Address, Variable}.

	** Insetad of using templates, in pointer analysis we will generalize "Constants"
\********************************************************/


class CFG;

class StateNode
{
protected:
	std::string name;

	int outgoing_edges;
	int incoming_edges;
	CFG* containing_graph_ptr;

public:
	
	// Note: All derived classes must have copy constructor!
	StateNode::StateNode(CFG* ptr) { this->containing_graph_ptr = ptr; };

	/****************************************\
	* Name: join (abstract)
	*
	* Description:
	* Join operation between 'this' and the state given in the argument
	* 'this' is updated as the result of the join operation
	* in:	const StateNode& state_2
	* out:
	* inout: this->fail_node
	* Notes:
	\*****************************************/
	virtual	void		StateNode::join(const StateNode& state_2) = 0;


	/****************************************\
	* Name: is_equivalent (abstract)
	*
	* Description:
	* Check if the given argument state is equivalent to 'this'
	* in:	StateNode& compared
	* out:  bool
	* inout: 
	* Notes:
	\*****************************************/
	virtual bool const	StateNode::is_equivalent(StateNode& compared) = 0;


	/****************************************\
	* Name: copy_abstract_state (abstract)
	*
	* Description:
	* Copy the state of the given argument to 'this;
	* in:	StateNode* state_to_copy
	* out:
	* inout: this
	* Notes:
	\*****************************************/
	virtual void		StateNode::copy_abstract_state(StateNode* state_to_copy) = 0;


	/****************************************\
	* Name: print(abstract)
	*
	* Description:
	* Prints state status
	* in:
	* out:
	* inout:
	* Notes:
	\*****************************************/
	virtual void		StateNode::print() = 0;

	virtual ~StateNode() = 0;



	void StateNode::inc_incoming() { this->incoming_edges++; };
	void StateNode::inc_outgoing() { this->outgoing_edges++; };
	std::string	const	StateNode::get_name()	{ return name; };
	bool		const	StateNode::is_loop() { return (this->incoming_edges > 1); };
	bool		const	StateNode::is_branch() { return (this->outgoing_edges > 1); };
};

