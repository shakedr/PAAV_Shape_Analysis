#pragma once

#include "NA_Constant.h"
#include <math.h>



NA_Constant::NA_Constant(int set_val, int tb)
{
	if (tb == TOP)
	{
		this->top_bottom = TOP;
		this->val = set_val;
	}
	else if (tb == BOTTOM)
	{
		this->top_bottom = BOTTOM;
		this->val = 0;
	}
	else
	{
		this->top_bottom = 0;
		this->val = set_val;
	}
	this->set_parity(set_val);
};

void	NA_Constant::set_val(int new_val)
{
	this->set_tb(0);
	this->val = new_val;
	this->set_parity(new_val);
}


void NA_Constant::randomize()
{
	this->set_val(rand() % 30);
}

NA_Constant	NA_Constant::operator+ (int argument)
{
	return NA_Constant(this->get_val() + argument, this->top_bottom);
}

NA_Constant	NA_Constant::operator- (int argument)
{
	return NA_Constant(this->get_val() - argument, this->top_bottom);
}

bool3	NA_Constant::is_equal(NA_Constant& other)
{
	// Both TOP
	if ((this->is_top() && other.is_top()))
	{
		return bool3(TRUE);
	}
	// Both BOTTOM
	else if (this->is_bottom() && other.is_bottom())
	{
		return bool3(TRUE);
	}
	// Both not top and not bottom and value is the same
	else if  ((!this->is_top()) && (!this->is_bottom()) && (!other.is_top()) && (!other.is_bottom()) && (this->get_val() == other.get_val()))
	{
		return bool3(TRUE);
	}
	// One is TOP, the other is not - still false
	else if ((this->is_top() && !other.is_top()) || (other.is_top() && !this->is_top()))
	{
		return bool3(UNK);
	}
	else
	{
		return bool3(FALSE);
	}
}

NA_Constant NA_Constant::join(NA_Constant* arg)
{
	// join (top, x) = top
	if (arg->is_top() || this->is_top())
	{
		this->set_tb(TOP);
		this->parity = PARITY_UNK;
		return *this;
	}

	// join (x , bottom) = x
	else if (arg->is_bottom())
	{
		return *this;
	}

	// join (bottom, x) = x
	else if (this->is_bottom())
	{
		return *arg;
	}

	// join (x != TOP, BOTTOM, y != TOP, BOTTOM) = x if x == y, TOP otherwise
	else
	{
		if (this->get_val() == arg->get_val())
		{
			return *this;
		}
		else
		{
			this->set_tb(TOP);
			this->parity = PARITY_UNK;
			return *this;
		}
	}

}

std::string NA_Constant::get_val_string()
{
	if (this->is_bottom())
	{
		return std::string("Bottom");
	}
	else if (this->is_top())
	{
		return std::string("Top");
	}
	else
	{
		return std::to_string(this->get_val());
	}
}


void NA_Constant::copy_val(NA_Constant& other)
{
	this->set_val(other.get_val());
	this->set_tb(other.get_tb());
	this->parity = other.get_parity();
}