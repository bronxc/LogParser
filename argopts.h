/*
	Author:		Steven Bremner
	Date:		April 23, 2014

	Description:
		Library for parsing command line parameters
*/

#ifndef ARGOPTS_H
	#define ARGOPTS_H

#include "includes.h"
#include <exception>

typedef struct _param{
	char * flag;
	char * data;

	_param() : flag(NULL), data(NULL) { }

	//Get the string version of the flag/data
	inline std::string getflag() { return (flag == NULL) ? "" : std::string(flag); }
	inline std::string getdata() { return (data == NULL) ? "" : std::string(data); }

	inline void setflag(char * _f)
	{
		strdup(&(this->flag), _f);
		
		#ifdef DEBUG
			printf("this->flag = %s\n", this->flag);
		#endif
	}
	inline  void setdata(char * _d)
	{
		strdup(&(this->data), _d);
		
		#ifdef DEBUG
			printf("this->data = %s\n", this->data);
		#endif
	}

	private:
		inline static char * strdup(char ** dst, char * src)
		{
			//non-error check shorthand version looks as follows:
			//return strcpy((*dst)=(char*)malloc(strlen(src)+1), src);

			//below is the version with included error checking for failed malloc
			*dst = (char*)malloc(strlen(src)+1);

			if(*dst  == NULL)
			{
				return NULL;
			}

			return strcpy(*dst, src);
		}

} PARAM;

#define INVALID_PARAM PARAM()

static inline bool operator==(struct _param& lhs, struct _param& rhs)
{
	//if flag and data are the same, they are the same
	return (lhs.getflag() == rhs.getflag() && lhs.getdata() == rhs.getdata());
}

static inline bool operator!=(struct _param& lhs, struct _param& rhs)
{
	return !(lhs == rhs);
}

typedef struct _options{
	//path to our executable program (capture in argv[0])
	std::string execPath;
	
	//number of opts (params)
	unsigned int count;
	
	//list of [flag,data] pairs
	PARAM * params;

	_options() : execPath(""), count(0), params(NULL), currentParam(0) { }

	PARAM nextarg();
	PARAM getargAt(unsigned int index);

	private:
		unsigned int currentParam;

} OPTIONS;

//Primary function call (the parsing should handle itself from here)
void getargs(int argc, char * argv[], OPTIONS * opts);

//prints the OPTIONS struct
const void printopts(OPTIONS opts);

//Peeks as peekindex to check if it contains a flag
bool peekopt(int argc, char * argv[], int peekindex);

//Counts number of input flags (options)
int getoptcount(int argc, char * argv[]);

class OptionError : public std::runtime_error{
private:

public:
	OptionError()
		: std::runtime_error("ERROR :: An error occured while handling input arguments")
	{
	}

	OptionError(char * message)
		: std::runtime_error(message)
	{
	}

	virtual const char* what() const throw()
	{
		return std::runtime_error::what();
	}
};

#endif