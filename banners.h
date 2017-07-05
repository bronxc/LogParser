/*
	Author:		Steven Bremner
	Date:		April 23, 2014

	Description:
		All of the banners used by the opthandler in main.cpp
*/

#ifndef BANNERS_H
	#define BANNERS_H

#include "includes.h"

inline void printbanner()
{
	printf("%s%-10s%s%-10s%s%-10s%s%s",
		"\n-------------------------\n\n",
		"Author:", "Steven Bremner\n",
		"Date:", "April 23, 2014\n",
		"\nThanks for using the file parsing tool from SteveInternals!\n"
	);
}

inline void printexamples(std::string execPath)
{
	printf("Examples:\n\n");
	
	//Example 1
	printf(" Search a file for a string and write output to file\n");
	printf("\t%s -f infile.txt -s \"credit card\" -o outfile.txt\n\n", execPath.c_str());

	//Example 2
	printf(" Search a file for regex matches and print them to the screen\n");
	printf("\t%s --file infile.txt --regex [a-zA-Z]{4} --print\n\n", execPath.c_str());

	//Example 3
	printf(" Search a file after piping output from another program\n");
	printf("\tstrings infile.txt | %s --search \"info\" --print\n", execPath.c_str());
}

inline void printusage(std::string execPath)
{
	printf("\nUsage for %s:\n\n", execPath.c_str());
	printf(" %-24s\t%s\n\n", "-f,-F,--file <file>", "input file to be parsed (required - can also pipe files)");
	printf(" %-24s\t%s\n\n", "-s,-S,--search <string>", "search for string inside file (case sensitive)");
	printf(" %-24s\t%s\n\n", "-r,-R,--regex <regexp>", "search for regex matches to regexp (see --group)");
	printf(" %-24s\t%s\n\n", "-g,-G,--group <g1,g2,...>", "specifies capture group(s) for regex (see --regex)");
	printf(" %-24s\t%s\n\n", "-u,-U,--unique", "specifies output from regex search should be unique");
	printf(" %-24s\t%s\n\n", "--ignore-case", "makes parsing case insensitive");
	//printf(" %-24s\t%s\n\n", "-c,-C,--count", "counts matches for either search or regex");
	printf(" %-24s\t%s\n\n", "-o,-O,--output <file>", "specifies output file to write results");
	printf(" %-24s\t%s\n\n", "-p,-P,--print", "prints the results to the screen");
	printf(" %-24s\t%s\n\n", "--hide-banner", "hides the SteveInternals banner from the final output");
	printf(" %-24s\t%s\n\n", "-h,-H,--help", "prints the usage or help page");

	printexamples(execPath);

	printbanner();
}

#endif