#pragma once

#include <exception>
using namespace std;


class TypeError : public exception
{
	virtual const char* what() const throw()
	{
		return "An abstract entity was casted to the wrong type, TypeError raised \n";
	}
};