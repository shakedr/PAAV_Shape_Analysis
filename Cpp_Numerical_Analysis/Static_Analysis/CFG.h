#pragma once

#include <stdio.h>
#include <vector>
#include <queue>
#include <stack>
#include "Edge.h"
#include "StateNode.h"
using namespace std;


/********************************************************\
Control Flow Graph class
a graph which is an abstraction of the parsed program
G = (V = StateNodes, E = Edges/operators)
Implemented as a vector of states and a vector of edges
\********************************************************/





class CFG
{
private:
	StateNode* start_node;
	StateNode* fail_node;
	vector<StateNode*> states;
	vector<Edge*> edges;

	bool fail_node_set;

public:

	vector<Variable*> var_list;	
	// Constructors:
	CFG::CFG(std::string var_line);

	// Destructor:
	CFG::~CFG();

	// Methods:
	/****************************************\
	* Name: set_fail_node
	*
	* Description:
	* Sets the CFGs fail node to which it jumps from an assert if the assert failed at any point
	* in:	StateNode* fail
	* out:	
	* inout: this->fail_node	
	* Notes:
	\*****************************************/
	void		CFG::set_fail_node(StateNode* fail);

	/****************************************\
	* Name: get_var
	*
	* Description:
	* Returns a pointer to the variable with the corresponding name in the var_name argument
	* in:		std::string var_name
	* out:		Variable*
	* inout:
	* Notes: Returns NULL if no such variable exists in the CFG
	\*****************************************/
	Variable*	CFG::get_var(std::string var_name);

	/****************************************\
	* Name: is_failnode_set
	*
	* Description:
	* Safety check as to whether the failnode was set
	* in:	
	* out:		bool ans
	* inout:
	* Notes:
	\*****************************************/
	bool		CFG::is_failnode_set();

	/****************************************\
	* Name: get_state
	*
	* Description:
	* Returns a pointer to the state with the corresponding name in the var_name argument
	* in:		std::string name
	* out: 
	* inout:
	* Notes: Returns NULL if no such state exists
	\*****************************************/
	StateNode*	CFG::get_state(std::string name);   // Returns NULL if not found.

	/****************************************\
	* Name: add_node
	*
	* Description:
	* Add another node to the CFG
	* in:		StateNode* node - A pointer to the node to be added
	* out: 
	* inout: this
	* Notes:
	\*****************************************/
	void		CFG::add_node(StateNode* node);

	/****************************************\
	* Name: add_edge
	*
	* Description:
	* Add an edge to the CFG
	* in:		Edge* node - A pointer to the edge to be added
	* out:
	* inout: this
	* Notes: Raises an exception if one of the states on the edge does not exist
	\*****************************************/
	void		CFG::add_edge(Edge* edge);

	/****************************************\
	* Name: add_var
	*
	* Description:
	* Add a variable to the list of program variables for this CFG
	* in:		std::string name - The name of that variable
	* out:
	* inout: this
	* Notes: Creates a new variable item with the given and adds it to the CFG variable list
	\*****************************************/
	void		CFG::add_var(std::string name) {this->var_list.push_back(new Variable(name));}

	/****************************************\
	* Name: add_var
	*
	* Description:
	* Analyzes the control flow graph until reaching a fixed point
	* in:		
	* out:
	* inout: this
	* Notes: Edits all the states until there are no more changes to implement
	*		 Stops when a fixed point is reached
	*        
	\*****************************************/
	void		CFG::analyze();   


	/****************************************\
	* Name: print
	*
	* Description:
	* Prints all the states in the graph
	* in:		
	* out:
	* inout: 
	* Notes: 
	\*****************************************/
	void		CFG::print();
};


/*  Helper Functions */

/****************************************\
* Name: edge_compare_by_src_name
*
* Description:
* Compare two edges by the name of their source state. used for std::lower_bound to find a states outgoing edges
* in:
* out:
* inout: 
* Notes: 
\*****************************************/
bool edge_compare_by_src_name(Edge* first, Edge* second);

/****************************************\
* Name: compare_edge_src_name_to_str
*
* Description:
* Compare two edges by the name of their source state. used for std::lower_bound to find a states outgoing edges
* in:
* out:
* inout:
* Notes:
\*****************************************/
bool compare_edge_src_name_to_str(Edge* first, std::string const str);

/****************************************\
* Name: update_edge_destination
*
* Description:
* function that updates the destination according to a transition state using JOIN
* in: std::vector<Edge*>::iterator current_edge_iter : The edge currently being updated
* out:
* inout: std::stack<StateNode*>* analysis_stack : The current analysis stack, edits if additional calls are due
* Notes:
\*****************************************/
void update_edge_destination(std::vector<Edge*>::iterator current_edge_iter, std::queue<StateNode*>* analysis_stack);


