#pragma once
#include "Edge.h"



Edge::Edge(StateNode* src, StateNode* dst, Command* cmd)
{
	this->source = src;
	this->dest = dst;
	this->command = cmd;
};

Edge::~Edge()
{
	delete this->command;
	// States are cleared by CFG.
}