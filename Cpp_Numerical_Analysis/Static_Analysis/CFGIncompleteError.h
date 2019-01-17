#pragma once


#include <exception>
using namespace std;


class CFGIncompleteError : public exception
{
	virtual const char* what() const throw()
	{
		return "Error: CFG is not complete  when analyzing, or added edge between non-existant nodes";
	}
};
