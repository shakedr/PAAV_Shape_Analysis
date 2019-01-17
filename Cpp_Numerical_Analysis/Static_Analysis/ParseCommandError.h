#pragma once


#include <exception>
using namespace std;


class ParseCommandError : public exception
{
	virtual const char* what() const throw()
	{
		return "An error had occured while parsing a command";
	}
};