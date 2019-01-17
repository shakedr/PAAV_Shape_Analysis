#pragma once

/* Include libraries */
#include <vector>
#include <string>
#include <map>
#include "Variable.h"





/* Include exception types */
#include "AssertError.h"
#include "bool3InputError.h"
#include "ParseError.h"
#include "TypeError.h"
#include "CFGIncompleteError.h"
#include "ParseCommandError.h"
#include "JoinError.h"


/* bool3 macros*/
#define TRUE 1
#define FALSE -1
#define UNK 0

/*Constant macros*/
#define TOP		1
#define BOTTOM -1

/*NA_Expression type enum macros*/
#define NA_EXP_EVEN 0
#define NA_EXP_ODD 1
#define NA_EXP_VAR_EQ_VAR 2
#define NA_EXP_VAR_EQ_CONST 3
#define NA_EXP_CONST_EQ_CONST 4
#define NA_EXP_VAR_NEQ_VAR 5
#define NA_EXP_VAR_NEQ_CONST 6
#define NA_EXP_CONST_NEQ_CONST 7


#define FAIL_NODE_NAME "fail"

/* Namespace */
using namespace std;


class bool3
{	
private:
	int val; 

public:
	int get_val() const { return this->val; };
	bool3::bool3 (int to_set)	
	{
		if ((to_set != TRUE) & (to_set != UNK) & (to_set != FALSE)) 
		{
			throw bool3InputError(); 
		}
		else {this->val = to_set;  	}
	};

	bool3 bool3::operator|| (bool operand) {
		if (this->get_val() == FALSE) 
		{ 
			if (operand == 1) { return bool3(TRUE); }
			else { return bool3(FALSE); }
		}
		else if (operand == 0) { return *this; }
		else if ((this->get_val() == TRUE) || (operand == TRUE)) { return bool3(TRUE); }
		else { return bool3(UNK); };  // this == UNK
	};


	bool3 bool3::operator|| (bool3 operand) {
		if (this->get_val() == FALSE) { return operand; }
		else if (operand.get_val() == FALSE) { return *this; }
		else if ((this->get_val() == TRUE) || (operand.get_val() == TRUE)) { return bool3(TRUE); }
		else { return bool3(UNK); };  // this == UNK
	};


	bool3 bool3::operator&& (bool operand) {
		if (this->get_val() == FALSE) { return bool3(FALSE); }
		else if (operand == 0) { return bool3(FALSE); }
		else if (this->get_val() == TRUE) 
		{
			if (operand == 1) { return bool3(TRUE); }
			else { return bool3(FALSE); }
		}
		else { return bool3(UNK); };  // this == UNK
	};

	bool3 bool3::operator&& (bool3 operand) {
		if (this->get_val() == FALSE) { return bool3(FALSE); }
		else if (operand.get_val() == FALSE) { return bool3(FALSE); }
		else if (this->get_val() == TRUE) { return operand; }
		else { return bool3(UNK); };  // this == UNK
	};
};

