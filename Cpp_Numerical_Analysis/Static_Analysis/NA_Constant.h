#pragma once
#include "Constant.h"
#include "Tools.h"

#define PARITY_UNK -1 


/************************************************\
Constant class 
	The NA_Constant contains a list of values val and a parity bit (1 - odd, 0 - even)
	Contains int top_bottom:
						1 : top
						0 : none
						-1: bottom
\************************************************/

class NA_Constant : public Constant
{
private:
	int		val;
	int		top_bottom;
	int		parity;   // 0 - odd, 1 - even, -1 - Unknown

public:
	// Constructors
	NA_Constant::NA_Constant(int set_val, int tb);
	NA_Constant::NA_Constant() { this->val = 0; this->parity = PARITY_UNK; this->top_bottom = BOTTOM; };
	NA_Constant::NA_Constant(NA_Constant& other) { this->val = other.get_val(); this->parity = other.get_parity(); this->top_bottom = other.get_tb(); }


	void	NA_Constant::set_tb(int tb) {
		this->top_bottom = tb; if (tb != 0) { this->set_parity(PARITY_UNK); }
	};
	void	NA_Constant::set_parity(int val) { this->parity = (val+1) % 2; };
	
	// Methods
	void	NA_Constant::set_val(int new_val);

	int		NA_Constant::get_val()				{ return val; };
	int 	NA_Constant::get_parity()			{ return this->parity; };
	bool	NA_Constant::is_odd() {
		if (this->parity != PARITY_UNK) {
			return !this->parity;
		}
		else
		{
			return 0;
		}
};
	bool	NA_Constant::is_top()				{ return (this->top_bottom == TOP); };
	bool	NA_Constant::is_bottom()			{ return (this->top_bottom == BOTTOM); };
	int		NA_Constant::get_tb()				{ return this->top_bottom; };
	bool3	NA_Constant::is_equal(NA_Constant& other);

	std::string NA_Constant::get_val_string();

	/****************************************\
	* Name: randomize
	*
	* Description:
	* Change the constant's value to a random value
	* in:	
	* out:  
	* inout: 'this'
	* Notes: 
	\*****************************************/
	void	NA_Constant::randomize();

	/****************************************\
	* Name: join
	*
	* Description:
	* Returns the result of a join between 'this' and given argument
	* in:	NA_Constant* arg
	* out:	NA_Constant& - join (this, arg)
	* inout: 
	* Notes: If set to TOP/BOTTOM, maintains original 'this' value in other parameters
	\*****************************************/
	NA_Constant NA_Constant::join(NA_Constant* arg);

	void NA_Constant::copy_val(NA_Constant& other);

	NA_Constant	NA_Constant::operator+ (int argument);

	NA_Constant	NA_Constant::operator- (int argument);
};