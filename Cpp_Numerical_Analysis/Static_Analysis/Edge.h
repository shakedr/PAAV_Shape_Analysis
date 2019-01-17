#pragma once

#include "StateNode.h"
#include "Command.h"




/**********************\
Class representing an edge in the CFG
Operator in abstract domain
Contains the operation and points to the pair of states
\**********************/


class Edge 
{
private:
	StateNode* source;
	Command* command;   // pointer because cannot instantiate a derived
	StateNode* dest;
public:
	
	/* Constructor */
	Edge::Edge(StateNode* src, StateNode* dst, Command* cmd);
	Edge::~Edge();

	StateNode*	Edge::get_source()	const { return source; };
	StateNode*	Edge::get_dest()		const { return dest; };
	Command*	Edge::get_command() const { return command; };

};