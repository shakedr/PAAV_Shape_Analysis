#pragma once
#include "CFG.h"
#include "NA_Command.h"

#define VARIABLE_ASSIGNMENT	0
#define CONSTANT_ASSIGNMENT	1
#define RANDOM_ASSIGNMENT	2
#define ASSIGN_INC			3
#define ASSIGN_DEC			4
#define ASSUME				5
#define ASSERT				6


/***************************\
Class for all commands of Numerical Analysis
skip
i := j			(variable assignment)	0
i := K			(constant assignment)	1
i := ?			(random assignment)		2
i := j + 1		(assign+increment)		3
i := j - 1		(assign+decrement)		4
assume E		(assume)				5
assert ORC		(assert)				6


\****************************/


// Constructors

// Parses command string into the command type and the command itself.
NA_Command::NA_Command(std::string command_string, CFG* graph_ptr) : Command(graph_ptr) 
{
	// Note - this.expr is automatically instantiated as empty. only need to edit it if necessary
	this->command_string = command_string;
	std::string assign_op(":=");
	std::size_t assignee_end = command_string.find(assign_op);
	char* end; // For use in strtol
	this->assigned_const = NA_Constant(0,0);

	// If assume or assert command
	if (assignee_end == std::string::npos)
	{
		if (command_string.find(std::string("assume")) != std::string::npos)
		{
			this->type = ASSUME;
			// Slice off the "assume" part of the command_string
			std::string expression_string = command_string.substr(6, command_string.length() - 6);
			this->expr = new NA_Expression(expression_string, this->containing_graph_ptr);
		}
		else if (command_string.find(std::string("assert")) != std::string::npos)
		{
			this->type = ASSERT;
			std::string expression_string = command_string.substr(6, command_string.length() - 6);
			// Need to insert this->expr parsing into a disjunction of expressions.
			this->expr = new NA_Expression(expression_string, this->containing_graph_ptr, TRUE);
		}
		else
		{
			throw ParseCommandError();
		}
	}
	else
	{
		// Not assume/assert:
		// From assignee end + 2 (after operator) to end
		std::string assigned_string = command_string.substr(assignee_end + 2, command_string.size());
		this->assignee = this->containing_graph_ptr->get_var(command_string.substr(0, assignee_end));
		if (this->assignee == NULL)
		{
			throw ParseCommandError();
		}
		const char* assigned_cstr = assigned_string.c_str();
		long assigned_int = strtol(assigned_cstr, &end, 0);

		// Case assign  inc
		if (assigned_string.find("+") != std::string::npos)
		{
			// Find correct variable before "+" , and correct assigned var before +1
			this->type = ASSIGN_INC;
			this->assigned_var = this->containing_graph_ptr->get_var(command_string.substr(assignee_end + 2,
				command_string.length() - assignee_end - 4));

		}

		// Case assign dec
		else if (assigned_string.find("-") != std::string::npos)
		{
			this->type = ASSIGN_DEC;
			this->assigned_var = this->containing_graph_ptr->get_var(command_string.substr(assignee_end + 2,
				command_string.length() - assignee_end - 4));
		}

		// Case variable assignment:
		else if (this->containing_graph_ptr->get_var(assigned_string) != NULL)
		{
			this->type = VARIABLE_ASSIGNMENT;
			this->assigned_var = this->containing_graph_ptr->get_var(assigned_string);
		}

		// Case random assignment
		else if (assigned_string.compare(std::string("?")) == 0)
		{
			this->type = RANDOM_ASSIGNMENT;
		}

		// Case constant assignment
		// If end == assigned_name_cstr it did not read a constant from the cstring
		else if (end != assigned_cstr)
		{
			this->type = CONSTANT_ASSIGNMENT;
			this->assigned_const.set_val(assigned_int);
		}
		// Otherwise something is crap
		else
		{
			throw ParseCommandError();
		}
	}
}

NA_Command::~NA_Command()
{
	if (this->type == ASSUME | this->type == ASSERT)
	{
		delete this->expr;
	}
}



// Methods

bool NA_Command::apply(StateNode& pre_state,StateNode& dest)
{
	// WARNING: Add check pre_state is truly NA
	bool ans;
	NA_StateNode* NA_pre_state = (NA_StateNode*) (&pre_state);
	NA_StateNode* NA_dest = (NA_StateNode*)(&dest);
	NA_StateNode transition_state =  NA_StateNode(NA_pre_state, std::string("transition state"));
	switch (this->type)
	{
	case VARIABLE_ASSIGNMENT:
		this->var_assignment(transition_state);
		break;
	case CONSTANT_ASSIGNMENT:
		this->constant_assignment(transition_state);
		break;
	case RANDOM_ASSIGNMENT:
		this->random_assignment(transition_state);
		break;
	case ASSIGN_INC:
		this->assign_increment(transition_state);
		break;
	case ASSIGN_DEC:
		this->assign_decrement(transition_state);
		break;
	case ASSUME:
		if (this->expr->eval(&transition_state).get_val() != FALSE)
		{
			// Some assume commands can have an effect on the post state: "i == 1" means i must be 1 afterwards
			//															  "odd/even i" affect i's parity
			transition_state.join(dest);
			// If the expression is not a disjunction and contains an equality, we must set
			if (!this->expr->is_disjunction())
			{
				std::vector<NA_AtomicExpression*> predicates = this->expr->get_predicates();
				for (std::vector<NA_AtomicExpression*>::iterator pred_iter = predicates.begin();
					pred_iter != predicates.end();
					pred_iter++)
				{
					if ((*pred_iter)->get_type() == NA_EXP_VAR_EQ_CONST)
					{
						// Can cast - type is surely VAR_EQ_CONST
						Aexp_VarEqConst* predicate = (Aexp_VarEqConst*)(*pred_iter);
						transition_state.set_variable(predicate->get_var(), predicate->get_constant());
					}
					else if ((*pred_iter)->get_type() == NA_EXP_VAR_EQ_VAR)
					{
						// Can cast - type is Variable Equation
						// Then these two must be equal
						Aexp_VarEq* predicate = (Aexp_VarEq*)(*pred_iter);
						transition_state.add_var_equality(predicate->get_var1(), predicate->get_var2());
					}
					else if ((*pred_iter)->get_type() == NA_EXP_VAR_NEQ_VAR)
					{
						Aexp_VarNeq* predicate = (Aexp_VarNeq*)(*pred_iter);
						transition_state.rem_var_equality(predicate->get_var1(), predicate->get_var2());
					}
				}
			}
			transition_state.reduce();
			dest.copy_abstract_state(&transition_state);
			return 0;
		}
		else
		{
			return 1;
		}
		break;
	case ASSERT:
		if (dest.get_name() == FAIL_NODE_NAME)  // If directed to fail, should push (return 0) if fail
		{
			if (this->expr->eval(&transition_state).get_val() != TRUE)
			{
				printf("\n Assert failed! \n");
				return 0;
			}
			else
			{
				printf("\n Assert successful! \n");
				return 1;
			}
		}
		else
		{
			transition_state.join(dest);
			dest.copy_abstract_state(&transition_state);
			return 0; // If this assert doesn't lead to FAIL, push it ahead.
		}
		break;
	}
	transition_state.reduce();
	transition_state.join(*NA_dest);
	transition_state.reduce();
	if (transition_state.is_equivalent(*NA_dest))
	{
		ans = 1;
	}
	else
	{
		NA_dest->copy_abstract_state(&transition_state);
		ans = 0;
	}
	return ans;
}




void NA_Command::var_assignment(NA_StateNode& state)
{
	state.set_variable(this->assignee, *(state.get_variable_value(assigned_var)));
	// Remove all equalities since the value has been changed
	state.rem_all_var_equalities(this->assignee);
	// Add an equality between the two variables
	state.add_var_equality(this->assignee, this->assigned_var);
}
void NA_Command::constant_assignment(NA_StateNode& state)
{
	state.set_variable(this->assignee, this->assigned_const);
	// Remove all equalities since the value has been changed
	state.rem_all_var_equalities(this->assignee);
}
void NA_Command::random_assignment(NA_StateNode& state)
{
	this->assigned_const.randomize();
	this->constant_assignment(state);

}
void NA_Command::assign_increment(NA_StateNode& state)
{
	state.set_variable(this->assignee, NA_Constant(state.get_variable_value(assigned_var)->get_val() + 1, state.get_variable_value(assigned_var)->get_tb()));
	// Remove all equalities since the value has been changed
	state.rem_all_var_equalities(this->assignee);
}
void NA_Command::assign_decrement(NA_StateNode& state)
{
	state.set_variable(this->assignee, NA_Constant(state.get_variable_value(assigned_var)->get_val() - 1, state.get_variable_value(assigned_var)->get_tb()));
	// Remove all equalities since the value has been changed
	state.rem_all_var_equalities(this->assignee);
}
void NA_Command::assume(NA_StateNode& state)
{
	// Intentionally empty - since the assume does not affect the state, only branches (implemented in cfg.cpp)
}
void NA_Command::assert(NA_StateNode& state)
{

}


bool3 NA_Command::eval_expression(StateNode& state)
{
	return this->expr->eval(&state);
}