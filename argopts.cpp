/*
	Author:		Steven Bremner
	Date:		April 23, 2014

	Description:
		Library for parsing command line parameters
*/

#include "argopts.h"

PARAM _options::nextarg()
{
	//returns an empty PARAM with NULL flag/data if there is bad input
	return (this->currentParam < count) ? this->params[this->currentParam++] : INVALID_PARAM;
}

PARAM _options::getargAt(unsigned int index)
{
	//returns an empty PARAM with NULL flag/data if there is bad input
	return (index < count) ? this->params[index] : INVALID_PARAM;
}

const void printopts(OPTIONS opts)
{
	printf("Executable Path: %s\n", opts.execPath.c_str());
	printf("Detected args: %d\n", opts.count);
	
	if(opts.count == 0 || opts.params == NULL){
		//no input options or params is still NULL (let's return)
		return;
	}

	for(unsigned int i = 0; i < opts.count; i++)
	{
		printf("| [%d] :: %2s\t%s\n", i+1, opts.params[i].flag, opts.params[i].data);
	}
}

bool peekopt(int argc, char * argv[], int peekindex)
{
	if(peekindex >= argc)
	{
		return false;
	}

	return (argv[peekindex][0] == '-') ? true : false;
}

int getoptcount(int argc, char * argv[])
{
	//Counter for the number of input for each of the following:
	//		1. flag,data pair
	//		2. flag (no data)
	//		3. data (no flag)
	int counter = 0;
	
	//toggle to determine if the previous arg was a flag or not
	bool flag_detected = false;

	//skip first input since it is just the path of the executable
	for(int i = 1; i < argc; i++)
	{
		//starts with '-' means its an option/flag
		if(argv[i][0] == '-')
		{
			flag_detected = true; //we just found a flag!
			counter++;
		}
		else
		{
			//Doesn't start with '-' so it can be 1 of 2 things:
			//	1. Data pair for a flag before it
			//	2. Raw input data with no flag
			// NOTE:
			// If it is case 2, we need to increase our counter

			if(!flag_detected)
			{
				//Uh oh, last param was NOT a flag and neither is this
				//We must have Case #2: Raw input data with no flag
				counter++;
			}

			flag_detected = false; //reset our flag
		}
	}

	return counter;
}

void getargs(int argc, char * argv[], OPTIONS * opts)
{
	//First argument should always be the path of the executable
	opts->execPath = std::string(argv[0]);
	opts->count = getoptcount(argc, argv);

	//Allocate space for all of our arguments if we have any
	if(opts->count > 0){
		opts->params = (PARAM *)malloc(sizeof(PARAM) * opts->count);
	}
	
	#ifdef DEBUG
		printf("DEBUG :: sizeof(PARAM) = %u\n", sizeof(PARAM));
	#endif

	unsigned int counter = 0;

	//Make sure we don't go over the buffer by checking that
	//the counter stays less than the expected number of opts
	//	NOTE: We assume that the parser finds a match to one of the following
	//	on each iteration of the loop (increment the counter each pass)...
	//		1. flag,data pair
	//		2. flag (no data)
	//		3. data (no flag)
	for(unsigned int i = 1; i < static_cast<unsigned>(argc) && counter < opts->count; i++, counter++)
	{
		//this is a flag - let's add to params and check for data
		if(argv[i][0] == '-')
		{
			opts->params[counter].setflag(argv[i]);
			
			//If the next input is NOT a opt, we will capture it as
			//the data pair for the current opt
			if(!peekopt(argc, argv, i+1) && (i+1 < static_cast<unsigned>(argc)))
			{
				//CASE #1: flag,data pair
				opts->params[counter].setdata(argv[i+1]);
				i++; //IMPORTANT: we paired up the flag with the data... skip the next arg
			}
			else
			{
				//CASE #2: flag (no data)
				//there is no data pair for this opt, let's assign an empty string
				opts->params[counter].setdata("");
			}

			#ifdef DEBUG
				printf("DEBUG :: (flag,data) = (%s,%s)\n", opts->params[counter].flag, opts->params[counter].data);
			#endif
		}
		else
		{
			//CASE #3: data (no flag)
			opts->params[counter].setflag(""); //no flag... let's assign an empty string
			opts->params[counter].setdata(argv[i]);

			#ifdef DEBUG
				printf("DEBUG :: Detected input data with no flag set\n");
			#endif
		}
	}
}