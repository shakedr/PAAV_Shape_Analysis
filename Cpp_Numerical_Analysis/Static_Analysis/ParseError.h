#pragma once
#include <exception>
using namespace std;


class ParseError : public exception
{
	virtual const char* what() const throw()
	{
		return "Parsing Error Occured";
	}
};