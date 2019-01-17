
#pragma once
#include "Tools.h"

/*******\
Class for program variables
\*******/


class Variable
{
private:
	std::string name;

public:
	// Constructors
	Variable::Variable(std::string set_name) { name = set_name; };



	// Methods
	std::string Variable::get_name() {return this->name; };
};