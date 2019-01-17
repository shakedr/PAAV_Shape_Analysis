#pragma once






#include <sstream>
#include <fstream>
#include <string>
#include <time.h>
// #include <vld.h>
#include "CFG.h"
#include "NA_StateNode.h"
#include "NA_Command.h"
#include "Tools.h"


#define SPACE " "
#define MAX_SEARCH_SIZE 5


/*****************************\
* stages of an analysis:
* get a program (a list of lines)
* parse into a CFG (set states L_i on either side of the line and derive their CFG)
* Assert commands point to a special "fail" state
* run over CFG:
* Start at starting node
* for each edge updated: JOIN (command(source state), destination)
* When reaching assert: if assertion is TRUE, keep running. if FALSE or UNK, add fail node to analysis queue.
* If fail node is analyzed - throw Assert Error for bad assertion.
\*****************************/


/*******\
* Parser assumptions:
* text input is of the form
* first line:
* list of variables separated by whitespace
* following lines:
* $PRESTATE L_i$ $command$ $POSTSTATE L_j$ \n
\*******/


// main and parser for Numerical Analysis

void main()
{
	try
	{

		// parse lines one by one
		srand(time(NULL));
		std::string line;
		std::ifstream infile;
		infile.open("C:\\Users\\Shaked\\Documents\\visual\ studio\ 2017\\Projects\\Static_Analysis\\Factorial.txt");  

		// Start by parsing list of variables
		std::getline(infile, line);
		CFG cfg = CFG(line);

		// Parse line by line
		// For every pre/post state, add a statenode to the CFG and connect with an edge of the command
		// If the node exists, only add edge
		while (std::getline(infile, line))
		{
			size_t first_space = line.find(SPACE);
			size_t second_space = line.rfind(SPACE);
			std::string pre_state_name = line.substr(0, first_space);
			std::string command_string = line.substr(first_space + 1, (second_space - first_space) - 1);
			std::string post_state_name = line.substr(second_space + 1, line.length() - second_space);
			if (cfg.get_state(pre_state_name) == NULL)
			{
				cfg.add_node(new NA_StateNode(pre_state_name, &cfg));
				cfg.get_state(pre_state_name)->inc_outgoing();
			}
			else
			{
				// Increment outgoing edges
				cfg.get_state(pre_state_name)->inc_outgoing();
			}
			if (cfg.get_state(post_state_name) == NULL)
			{
				cfg.add_node(new NA_StateNode(post_state_name,&cfg));
				cfg.get_state(post_state_name)->inc_incoming();
			}
			else
			{
				// Increment incoming edges
				cfg.get_state(post_state_name)->inc_incoming();
			}
			if (command_string.find("assert") != std::string::npos)
			{
				if (cfg.is_failnode_set() == false)
				{
					cfg.set_fail_node(new NA_StateNode(std::string(FAIL_NODE_NAME), &cfg));
				}
				cfg.add_edge(new Edge(cfg.get_state(pre_state_name),
					cfg.get_state(FAIL_NODE_NAME),
					new NA_Command(command_string, &cfg)));
			}
			cfg.add_edge(new Edge(cfg.get_state(pre_state_name),
				cfg.get_state(post_state_name),
				new NA_Command(command_string, &cfg)));
		};
		infile.close();
		// iterate until reaching fixed point
		cfg.analyze();
	}
	catch (ParseError error)
	{
		printf("Error occured: ParseError");
	}
	catch (AssertError error)
	{
		printf("\n Error occured: AssertError : Failed to prove an assertion in the code");
	}
	catch (bool3InputError error)
	{
		printf("Error occured: bool3InputError");
	}
	catch (CFGIncompleteError error)
	{
		printf("Error occured: CFGIncompleteError");
	}
	catch (ParseCommandError error)
	{
		printf("Error occured: ParseCommandError");
	}
	catch (TypeError error)
	{
		printf("Error occured: TypeError");
	}
	catch (JoinError error)
	{
		printf("Error occured: Tried to join two unjoinable states");
	}
	catch (...)
	{
		printf("Error occured: unknown type of exception raised");
	}

	printf("Finished?!");
	// Add deletes to everything

}


