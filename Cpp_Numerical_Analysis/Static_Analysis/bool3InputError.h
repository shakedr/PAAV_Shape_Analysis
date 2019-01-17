#pragma once


#include <exception>
using namespace std;


class bool3InputError : public exception
{
	virtual const char* what() const throw()
	{
		return "A bool 3 was given an illegal value (allowed values are 1 (TRUE), 0 (UNK), -1 (FALSE)";
	}
};