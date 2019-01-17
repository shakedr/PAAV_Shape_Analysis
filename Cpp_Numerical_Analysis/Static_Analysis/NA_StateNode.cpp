/*********************\
source file for the NA_StateNode class

A state in Numerical Analysis
Contains a mapping of variables to constants
every mapping is an equality

\*********************/
#pragma once
#include "CFG.h"
#include "NA_StateNode.h"
#include <iostream>
#include <string>
#include <stdio.h>


// Constructors
NA_StateNode::NA_StateNode(std::string name, CFG* ptr) : StateNode(ptr)
{
	for (std::vector<Variable*>::const_iterator iter = this->containing_graph_ptr->var_list.begin(); iter != this->containing_graph_ptr->var_list.end(); iter++)
	{
		this->assignments.insert(std::pair<Variable*, NA_Constant*>((*iter), new NA_Constant()));
	}
	// this->equations should contain all variable equations
	typedef std::pair<Variable*, Variable*> equality;
	std::vector<Variable*>::iterator iter1, iter2;
	for (iter1 = this->containing_graph_ptr->var_list.begin(); iter1 != this->containing_graph_ptr->var_list.end(); iter1++)
	{
		for (iter2 = this->containing_graph_ptr->var_list.begin(); iter2 != this->containing_graph_ptr->var_list.end(); iter2++)
		{
			this->add_var_equality(*iter1, *iter2);
		}
	}
	this->name = name;
	this->outgoing_edges = 0;
	this->incoming_edges = 0;
}




NA_StateNode::NA_StateNode(NA_StateNode* node_to_copy, std::string name = "") : StateNode(node_to_copy->containing_graph_ptr)
{
	std::map<Variable*, NA_Constant*>::iterator as_iter;
	for (as_iter = node_to_copy->assignments.begin(); as_iter != node_to_copy->assignments.end(); as_iter++)
	{
		this->assignments.insert(std::pair<Variable*, NA_Constant*> (as_iter->first, new NA_Constant(*(as_iter->second))));
	}
	
	this->equalities =   std::vector<VE_equality>(node_to_copy->equalities) ;
	this->name = std::string(name);
	this->incoming_edges = node_to_copy->incoming_edges;
	this->outgoing_edges = node_to_copy->outgoing_edges;
}


// Destructor

NA_StateNode::~NA_StateNode()
{
	this->equalities.clear();
	std::map<Variable*, NA_Constant*>::iterator as_iter;
	for (as_iter = this->assignments.begin(); as_iter != this->assignments.end(); as_iter++)
	{
		delete as_iter->second;
	}
	this->assignments.clear();
}



// Methods

NA_Constant* const NA_StateNode::get_variable_value(Variable* var)
{
	return this->assignments.at(var);
}


void NA_StateNode::set_variable(Variable* var, NA_Constant& cons)
{	
	this->get_variable_value(var)->copy_val(cons);
}


void NA_StateNode::join(const StateNode& state_2)
{
	// Check that state2 is NA_StateNode, otherwise TypeError
	NA_StateNode* state_2_ptr = (NA_StateNode*)&state_2;
	std::map<Variable*, NA_Constant*>::iterator iter;

	std::vector<Variable*>::iterator var_iter;
	for (var_iter = this->containing_graph_ptr->var_list.begin(); var_iter != this->containing_graph_ptr->var_list.end(); var_iter++)
	{
		Variable* current_variable = (*var_iter);
		this->set_variable(current_variable,
			this->get_variable_value(current_variable)->join(state_2_ptr->get_variable_value(current_variable)));
	}

	// Join(EQ1, EQ2) = intersect(EQ1, EQ2) in the VE lattice
	std::vector<VE_equality>::iterator eq_iter1, eq_iter2;
	std::vector<VE_equality> out;
	for (eq_iter1 = state_2_ptr->get_equalities().begin();
		eq_iter1 != state_2_ptr->get_equalities().end();
		eq_iter1++)
	{
		for (eq_iter2 = this->equalities.begin();
			eq_iter2 != this->equalities.end();
			eq_iter2++)
		{
			if ((eq_iter1->first == eq_iter2->first) && (eq_iter1->second == eq_iter2->second))
			{
				out.push_back(VE_equality(eq_iter1->first, eq_iter1->second));
			}
		}
	}
	this->equalities = out; // Set as intersection
	
}


bool const NA_StateNode::is_equivalent(StateNode& compared)
{
	bool diff = 0;
	NA_StateNode& cast_compared = (NA_StateNode&)compared;
	std::map<Variable*, NA_Constant*>::iterator this_assignments_iter = this->assignments.begin();
	// While they are still the same AND we did not reach the end
	while ((diff == 0) && (this_assignments_iter != this->assignments.end()))
	{
		NA_Constant* current_var_val = (*this_assignments_iter).second;
		NA_Constant* compared_val = cast_compared.get_variable_value((*this_assignments_iter).first);
		if (current_var_val->is_equal(*compared_val).get_val() != TRUE )
		{
			diff = 1;
		}
		this_assignments_iter++;
	}

	// Check that equalities are the same.
	if (!(this->equalities_contained(cast_compared) && cast_compared.equalities_contained(*this)))
	{
		diff = 1;
	}
	
	return !diff;
}

void NA_StateNode::copy_abstract_state(StateNode* state_to_copy)
{
	// NA_StateNode - only copies NA_StateNode
	NA_StateNode* src_state = (NA_StateNode*)state_to_copy;
	std::map<Variable*, NA_Constant*>::iterator as_iter_dst;
	for (as_iter_dst = this->assignments.begin(); as_iter_dst != this->assignments.end(); as_iter_dst++)
	{
		this->set_variable(as_iter_dst->first, *(src_state->get_variable_value(as_iter_dst->first)));
	}
	this->equalities = src_state->equalities;
}

void NA_StateNode::print()
{
	cout << "State " << this->get_name();
	cout << " [  ";
	for (std::map<Variable*, NA_Constant*>::iterator iter = this->assignments.begin(); iter != this->assignments.end(); iter++)
	{
		cout << "\n :: " << iter->first->get_name() << " = " << iter->second->get_val_string() << "," << "Parity = " << iter->second->get_parity() <<" :: ";
	}
	cout << " ]  \n ";
	cout << "Equalities:";
	for (std::vector<VE_equality>::iterator iter = this->equalities.begin(); iter != this->equalities.end(); iter++)
	{
		cout << "\n :: " << iter->first->get_name() << " == " << iter->second->get_name() << " ::  ";
	}
	cout << " \n";
}

void	NA_StateNode::add_var_equality(Variable* var1, Variable* var2)
{
	if (var1 != var2)
	{
		// Ensure no double listings
		this->rem_var_equality(var1, var2);
		this->rem_var_equality(var2, var1);

		this->equalities.push_back(VE_equality(var1, var2));
		this->equalities.push_back(VE_equality(var2, var1));
	}
}

void	NA_StateNode::rem_var_equality(Variable* var1, Variable* var2)
{
	std::vector<VE_equality>::iterator eq_iter = std::find(this->equalities.begin(), this->equalities.end(), VE_equality(var1, var2));
	if (eq_iter != this->equalities.end())
	{
		this->equalities.erase(eq_iter);
	}
	eq_iter = std::find(this->equalities.begin() ,this->equalities.end() , VE_equality(var1, var2));
	if (eq_iter != this->equalities.end()) 
	{
		this->equalities.erase(eq_iter);
	}
}

void	NA_StateNode::rem_all_var_equalities(Variable* var)
{
	std::vector<VE_equality>::iterator eq_iter = this->equalities.begin();
	for ( ; eq_iter != this->equalities.end(); )
	{
		if ((eq_iter->first == var) || (eq_iter->second == var))
		{
			this->equalities.erase(eq_iter);
			eq_iter = this->equalities.begin();
		}
		else
		{
			eq_iter++;
		}
	}
}


void	NA_StateNode::reduce_left()
{
	// If 'a' = K and 'b' = K , add ['a' = 'b'] to equalities

	std::map<Variable*, NA_Constant*>::iterator assignment_iter1, assignment_iter2;
	for (	assignment_iter1 = this->assignments.begin();
			assignment_iter1 != this->assignments.end() ;
			assignment_iter1++)
	{
		if (assignment_iter1->second->is_bottom() || assignment_iter1->second->is_top())
		{
			continue;
		}
		for (assignment_iter2 = std::next(assignment_iter1);
			assignment_iter2 != this->assignments.end();
			assignment_iter2++)
		{
			// If one of them is top or bottom, nothing to reduce here.
			if (assignment_iter2->second->is_bottom() || assignment_iter2->second->is_top())
			{
				continue;
			}
			if (assignment_iter1->second->get_val() == assignment_iter2->second->get_val())
			{
				// If value is equal, add the equality
				this->add_var_equality(assignment_iter1->first, assignment_iter2->first);
			}
		}
	}
}


void	NA_StateNode::reduce_right()
{
	// If 'a' = K and 'a' = 'b' then add 'b' = K to assignments
	std::map<Variable*, NA_Constant*>::iterator assignment_iter1;
	for (assignment_iter1 = this->assignments.begin();
		assignment_iter1 != this->assignments.end();
		assignment_iter1++)
	{
		// If 'a' = TOP/BOTTOM nothing to do here, continue to next variable in CP
		if (assignment_iter1->second->is_bottom() || assignment_iter1->second->is_top())
		{
			continue;
		}
		std::vector<VE_equality>::iterator equality_iter;

		for (equality_iter = this->equalities.begin();
			equality_iter != this->equalities.end();
			equality_iter++)
		{
			if (equality_iter->first == assignment_iter1->first)
			{
				// Found a variable with an equation
				// If the other variable is assigned a different value and isnt top, ERROR
				if (this->assignments.find(equality_iter->second)->second->get_val() != assignment_iter1->second->get_val()
					&& (!this->assignments.find(equality_iter->second)->second->is_top()))
				{
					throw JoinError();
				}
				// if found that for same 'a', 'a' = 'b' \in equalities and 'a' = K \in assignments ==> add 'b' = K to assignments
				else
				{
					this->set_variable(equality_iter->second, *(assignment_iter1->second));
				}
			}
		}
	}
}

void	NA_StateNode::reduce()
{
	bool fixed_point = 0;
	// Create local copy to recognize a fixed point
	NA_StateNode this_copy = NA_StateNode(this, std::string("Reduction Copy"));
	while (!fixed_point)
	{
		this->reduce_left();
		this->reduce_right();
		// If equivalent, reached fixed point.
		if (this->is_equivalent(this_copy))
		{
			fixed_point = 1;
		}
		else
		{
			// If not fixed point, update the copy
			this_copy.copy_abstract_state(this);
		}
	}
}


bool NA_StateNode::equalities_contained(NA_StateNode& contained)
{

	typedef VE_equality pair;

	std::vector<VE_equality>::iterator this_iter, con_iter;
	bool ans = 1;
	for (con_iter = contained.get_equalities().begin();
		con_iter != contained.get_equalities().end();
		con_iter++)
	{
		bool found = 0;
		for (this_iter = this->equalities.begin();
			this_iter != this->equalities.end();
			this_iter++)
		{
			if ((this_iter->first == con_iter->first) && (this_iter->second == con_iter->second))
			{
				found = 1;
				break;
			}
		}
		if (found == 0)
		{
			ans = 0;
			break;
		}
	}
	return ans;
}
