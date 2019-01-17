#pragma once

#include <exception>
using namespace std;


class AssertError : public exception
{
	virtual const char* what() const throw()
	{
		return "The required claim could not be asserted";
	}
};