/*
	Author:		Steven Bremner
	Date:		April 23, 2014

	Description:
		A tool for parsing Kiwi Log files to find matches to regex strings

	Test command line args are set for debug mode as the following:
		{path} -f "..\\disk2.txt" -r "(Computer)=(.*?)\s" -g 1,2"
*/

#include "includes.h"

#include "argopts.h" //custom arg parsing library
#include "banners.h" //used by opthandler function

#include <io.h>
#include <regex>
#include <vector>

//All the possible variables required to be stored from the opts
FILE * iFile = NULL;
FILE * oFile = NULL;
std::string search = "";
std::string regexp = "";
int * captureGroups = NULL;
int groupCount = 0;

//unsigned int captureGroup = 0;

//flag selected (default false)
bool countflag = false, printflag = false, uniqueoutput = false, hidebanner = false, caseinsensitive = false;

void cleanup(void)
{
	if(iFile != NULL){
		fclose(iFile);
	}

	if(oFile != NULL){
		fclose(oFile);
	}
}

int fpeek(FILE * stream){
	int c;

	c = fgetc(stream);
	ungetc(c, stream);

	return c;
}

void opthandler(OPTIONS opts)
{
	//Used for looping through the params below
	PARAM p;

	//Loop 
	while((p = opts.nextarg()) != INVALID_PARAM)
	{
		std::string flag = p.getflag();

		if(flag == "-h" || flag == "-H" || flag == "--help")
		{
			printusage(opts.execPath);
			exit(-1); //if help flag is toggled we won't actually do anything else
		}
		else if(flag == "-f" || flag == "-F" || flag == "--file")
		{
			//Requires data: <file>
			std::string data = p.getdata();
			//assert(data != "");
			if(data == ""){
				throw OptionError("ERROR :: File name required when using --file flag");
			}

			iFile = fopen(data.c_str(), "r");
			//assert(iFile != NULL);
			if(iFile == NULL){
				throw OptionError("ERROR :: Input file could not be opened. Check that the file exists.");
			}
		}
		else if(flag == "-s" || flag == "-S" || flag == "--search")
		{
			//Requires data: <string>
			search = p.getdata();
			//assert(search != "");
			if(search == ""){
				throw OptionError("ERROR :: No search term was specified.");
			}
		}
		else if(flag == "-r" || flag == "-R" || flag == "--regex")
		{
			//Requires data: <regexp>
			regexp = p.getdata();
			//assert(regexp != "");
			if(regexp == ""){
				throw OptionError("ERROR :: No regular expression (regex) was specified.");
			}
		}
		else if(flag == "-g" || flag == "-G" || flag == "--group")
		{
			//Requires data: <number>
			std::string data = p.getdata();
			//assert(data != "");
			if(data == ""){
				throw OptionError("ERROR :: No group number was specified");
			}

			groupCount = (int)std::count(data.begin(), data.end(), ',') + 1;
			captureGroups = (int *)malloc(sizeof(int) * groupCount);

			int count = 0;
			char * toks = strtok(p.data, ",");

			while(toks != NULL && count < groupCount)
			{
				//assign the int to the captureGroups array
				captureGroups[count++] = atoi(toks);
				toks = strtok(NULL, ",");
			}
		}
		else if(flag == "-u" || flag == "-U" || flag == "--unique")
		{
			uniqueoutput = true;
		}
		else if(flag == "-c" || flag == "-C" || flag == "--count")
		{
			//provides a count that matches either search string or regex
			countflag = true;
		}
		else if(flag == "-o" || flag == "-O" || flag == "--output")
		{
			//Requires data: <file>
			std::string data = p.getdata();
			//assert(data != "");
			if(data == ""){
				throw OptionError("ERROR :: File name required when using --output flag.");
			}
			
			oFile = fopen(data.c_str(), "w");
			//assert(oFile != NULL);
			if(oFile == NULL){
				throw OptionError("ERROR :: Output file could not be opened.");
			}
		}
		else if(flag == "-p" || flag == "-P" || flag == "--print")
		{
			//prints the output to the screen
			printflag = true;
		}
		else if(flag == "--ignore-case")
		{
			caseinsensitive = true;
		}
		else if(flag == "--hide-banner")
		{
			hidebanner = true;
		}
	}
	
	//if we have no input, how can we run the parser?
	if(iFile == NULL)
	{		
		int stdinLength = filelength(fileno(stdin));
		
		// Less than 0 = there was an error (piped input has filelength of 0 still)
		if(stdinLength < 0)
		{
			fprintf(stderr, "ERROR :: Input file required for parsing\n");
			printusage(opts.execPath);
			exit(-1);
		}
		
		if(stdinLength == 0)
		{
			// Less than 0 = there was an error setting the file pointer to the start of the stream (no data)
			if(fseek(stdin, 0, 0) < 0){
				fprintf(stderr, "ERROR :: Input file required for parsing\n");
				printusage(opts.execPath);
				exit(-1);
			}
		}

		// We passed the peek check (we can use stdin as our input)
		// Input was piped in (e.g.):
		//		cat file.txt | LogParser.exe --search "find this" --print
		//		type file.txt | LogParser.exe --search "find this" --print
		iFile = stdin;
	}

	#ifdef DEBUG
		printf("Finished handling opts\n");
	#endif

}

void optexec()
{
	#ifdef DEBUG
		printf("DEBUG :: Beginning execution...\n\n");
	#endif

	//We will read line-by-line and adjust our buffer as required
	unsigned int bufsz = 256;
	char * buf = (char *)malloc(sizeof(char) * bufsz);

	int linecount = 1;

	std::vector<std::string> dataset;

	if(caseinsensitive && search != ""){
		transform(search.begin(), search.end(), search.begin(), ::tolower);
	}

	//loop while not End of File (EOF)
	while(fgets(buf,bufsz,iFile) != NULL)
	{
		int len = strlen(buf);
		
		if(len == (bufsz - 1) && buf[bufsz-2] != '\n')
		{
			//Need to resize if we are in here
			#ifdef DEBUG
				printf("DEBUG :: Need to resize from %d to %d\n", bufsz, bufsz*2);
			#endif

			bufsz *= 2; //double our buffer size;
			free(buf);
			buf = (char *)malloc(sizeof(char) * bufsz);
				
			//move our file pointer back and we will re-read the data
			fseek(iFile, -len, SEEK_CUR);
		}
		else
		{
			std::string data = std::string(buf);
			
			//we have our line read here... its in 'buf' or 'data'
			if(regexp != ""){
				try{
					// Make it case insensitive if user requested that
					const std::regex expr = caseinsensitive ? std::regex(regexp, std::regex::icase) : std::regex(regexp);
					//const std::regex expr(regexp);

					std::smatch match;

					while(std::regex_search(data, match, expr))
					{
						std::string outline = "";

						for(int i = 0; i < groupCount; i++)
						{
							#ifdef DEBUG
								printf("Group[%d]: %s\n", captureGroups[i], match.str(captureGroups[i]).c_str());
							#endif

							outline += match.str(captureGroups[i]);
							//check if we are not on the last one to append a comman
							if(i < (groupCount - 1))
							{
								outline += ",";
							}

							//if(printflag) fprintf(stdout, "%s,", match.str(captureGroups[i]).c_str());
							//if(oFile != NULL) fprintf(oFile, "%s,", match.str(captureGroups[i]).c_str());
						}
						
						if(uniqueoutput)
						{
							if(std::find(dataset.begin(), dataset.end(), outline) == dataset.end())
							{
								//we have unique data here
								dataset.push_back(outline);
								
								if(printflag) fprintf(stdout, "%s", outline.c_str());
								if(oFile != NULL) fprintf(oFile, "%s", outline.c_str());
								
								//add the new line wherever we should be printing it
								if(printflag) fputs("\n", stdout);
								if(oFile != NULL) fputs("\n", oFile);
							}
						}
						else
						{
							//don't care if it's unique output or not... just print it
							if(printflag) fprintf(stdout, "%s", outline.c_str());
							if(oFile != NULL) fprintf(oFile, "%s", outline.c_str());

							//add the new line wherever we should be printing it
							if(printflag) fputs("\n", stdout);
							if(oFile != NULL) fputs("\n", oFile);
						}

						data = match.suffix(); //search the remainder of the string
					}
				}
				catch(std::regex_error& e)
				{
					fprintf(stderr, "ERROR :: regexp was not a valid regular expression\n");
					exit(-1);
				}
			}

			if(search != "")
			{
				if(caseinsensitive){
					// For case insensitive searches, we must send all of our data to lower
					// (search term is sent to lower also - see above)
					transform(data.begin(), data.end(), data.begin(), ::tolower);
				}

				std::size_t index = data.find(search, 0);

				if(index != std::string::npos)
				{
					if(printflag) fprintf(stdout, "Line: %d | %s", linecount, buf);
					if(oFile != NULL) fprintf(oFile, "Line: %d | %s", linecount, buf);
				}
			}
			
			linecount++; //we are ready for the next line now!
		}
	}

	free(buf);
}

int main(int argc, char * argv[])
{
	OPTIONS opts;

	atexit(cleanup);

	if(argc <= 1)
	{
		//no arguments provided... print usage
		printusage(std::string(argv[0]));
		return 0;
	}

	getargs(argc, argv, &opts);

	//printopts(opts);
	try{
		opthandler(opts);

		//this should execute the command (args should be populated in the opthandler function)
		optexec();

		if(!hidebanner){
			printbanner();
		}
	} catch(OptionError ex){
		// If we made it here, we had an issue in opthandler (didn't pass a test)
		printf("%s\n", ex.what());
		printusage(opts.execPath);
	}

	return 0;
}