#pragma once

#include <exception>
using namespace std;


class JoinError : public exception
{
	virtual const char* what() const throw()
	{
		return "Tried to join or reduce and detected an impossible state or a contradiction";
	}
};
