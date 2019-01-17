
#pragma once

#include "CFG.h"
#include <algorithm>
#include <sstream>
#include <fstream>
#include <iostream>
#include <string>
#include <stdio.h>
using namespace std;




CFG::CFG(std::string var_line)
{
	std::istringstream line_stream(var_line);
	for (std::string var_name; line_stream >> var_name;)
	{
		this->add_var(var_name); 
	}
}

CFG::~CFG()
{
	std::vector<StateNode*>::iterator state_iter;
	for (state_iter = this->states.begin(); state_iter != this->states.end(); state_iter++)
	{
		StateNode* to_delete = (*state_iter);
		delete to_delete;
	}
	std::vector<Edge*>::iterator edge_iter;
	for (edge_iter = this->edges.begin(); edge_iter != this->edges.end(); edge_iter++)
	{
		delete (*edge_iter);
	}
	std::vector<Variable*>::iterator var_iter;
	for (var_iter = this->var_list.begin(); var_iter != this->var_list.end(); var_iter++)
	{
		delete (*var_iter);
	}
}


// Returns NULL if not found.
StateNode*	CFG::get_state(std::string name)
{
	if (this->states.empty())
	{
		return NULL;
	}
	std::vector<StateNode*>::iterator state_iter =  this->states.begin();
	std::vector<StateNode*>::iterator end =			this->states.end();
	while (state_iter != end)
	{
		if ((*state_iter)->get_name() == name)
		{
			return (*state_iter);
		}
		state_iter++;
	}
	return NULL;
}

 // Returns NULL if not found.
Variable*		CFG::get_var(std::string var_name)
{
	std::vector<Variable*>::iterator var_iter;
	Variable* requested_var = NULL;
	for (var_iter = this->var_list.begin(); var_iter != this->var_list.end(); var_iter++)
	{
		if ((*var_iter)->get_name() == var_name)
		{
			requested_var = (*var_iter);
		}
	}
	return requested_var;
}


void		CFG::add_node(StateNode* node)
{
	if (this->states.empty())
	{
		this->states.push_back(node);
		this->start_node = node;
	}
	else
	{
		this->states.push_back(node);
	}
}

void		CFG::add_edge(Edge* edge)
{
	
	// If edge->src or edge->dst do not exist in graph - exception
	if ((this->get_state(edge->get_source()->get_name()) == NULL) | (this->get_state(edge->get_dest()->get_name()) == NULL))
	{
		throw CFGIncompleteError();
	}

	this->edges.push_back(edge);
};


void		CFG::analyze()
{
	if (!this->is_failnode_set())
	{
		throw CFGIncompleteError();
	}

	// Create an analysis stack
	// While stack is not empty:
	//		Pop statenode
	//		For every outgoing edge:
	//			if assume node: update for every edge which is not a FALSE condition
	//			run command
	//			joined = join with dest node
	//			if joined != dest node:
	//				dest node = joined
	//				push new dest_node
	//			else
	//				continue
	//													
	std::queue<StateNode*>*			analysis_stack = new std::queue<StateNode*>;
	StateNode* 						current_state;
	std::vector<Edge*>::iterator	current_edge_iter;
	std::sort(this->edges.begin(), this->edges.end(), edge_compare_by_src_name);
	analysis_stack->push (this->start_node);
	int num_of_outgoing_edges = 0;
	while (!analysis_stack->empty())
	{
		// this->print();
		current_state = analysis_stack->front();
		analysis_stack->pop();

		// If the current state is the fail state - bad ASSERT happened
		if (current_state->get_name() == FAIL_NODE_NAME)
		{
			throw AssertError();
		}

		// Find first outgoing edge, update, push if necessary :

		current_edge_iter = std::lower_bound(	this->edges.begin(), this->edges.end(),
												current_state->get_name(),
												compare_edge_src_name_to_str);
		// Check if found any edge. If none found - continue
		// Case: 0 outgoing edges
		if ((*current_edge_iter)->get_source() != current_state)
		{
			// Didn't find the current_state in the edges i.e. it has no outgoing edge
			continue;
		}
		// Case: 1 outgoing edge
		std::vector<Edge*>::iterator next_edge_iter = current_edge_iter + 1;
		if (next_edge_iter == this->edges.end()) // Last edge is the one we got, don't check ahead
		{
			num_of_outgoing_edges = 1;
		}
		else  // See if next edge is also from this node
		{
			if ((*next_edge_iter)->get_source() == current_state)  // It is also an outgoing edge from current state
			{
				num_of_outgoing_edges = 2;
			}
			else
			{
				num_of_outgoing_edges = 1;
			}
		}

		switch (num_of_outgoing_edges)
		{

			// Case: 1 outgoing edge
			case 1:
				update_edge_destination(current_edge_iter, analysis_stack);
				break;
			// Case: 2 outgoing edges, i.e., it is an "assume" or "assert" node. update both. Will update only if condition holds...
			case 2:
					update_edge_destination(current_edge_iter, analysis_stack);
					update_edge_destination(next_edge_iter, analysis_stack);
				break;
		}
	} // End of while loop - stack is now empty, reached a fixed point
	delete analysis_stack;
}

void update_edge_destination(std::vector<Edge*>::iterator current_edge_iter, std::queue<StateNode*>* analysis_stack)
{
	// Create transition state -> apply the command to the edge source node
	cout << "\n Updating edge (" << (*current_edge_iter)->get_source()->get_name() << " , " << (*current_edge_iter)->get_dest()->get_name() << " ) \n";
	StateNode* dest_ptr = (*current_edge_iter)->get_dest();
	StateNode* source_ptr = (*current_edge_iter)->get_source();
	cout << "\n Source Edge: ";
	source_ptr->print();
	cout << "\n Command is:" << (*current_edge_iter)->get_command()->get_command_string();
	cout << "\n Old dest state:";
	dest_ptr->print();
	bool destination_not_pushed = (*current_edge_iter)->get_command()->apply(*(source_ptr), *(dest_ptr)); // apply command: dest = join(command(source), dest)
	cout << "\n New dest state:";
	dest_ptr->print();
	cout << "\n ---------------------------------";
	// Add to stack if changed and analysis should continue
	if (!destination_not_pushed)
	{
		analysis_stack->push((*current_edge_iter)->get_dest());
	}
}



void	CFG::set_fail_node(StateNode* node)
{
	this->add_node(node);
	this->fail_node = node;
	this->fail_node_set = TRUE;
}

bool	CFG::is_failnode_set()
{
	return this->fail_node_set;
}

void	CFG::print()
{
	for (std::vector<StateNode*>::iterator iter = this->states.begin(); iter != this->states.end(); iter++)
	{
		(*iter)->print();
	}
}


bool edge_compare_by_src_name(Edge* first, Edge* second)
{
	return (first->get_source()->get_name().compare(second->get_source()->get_name()) > 0);
}

bool compare_edge_src_name_to_str(Edge* first, const std::string str)
{
	return (first->get_source()->get_name().compare(str) > 0);
}