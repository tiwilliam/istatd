/*
 *  Copyright 2009 William Tisäter. All rights reserved.
 * 
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *	1.  Redistributions of source code must retain the above copyright
 *		notice, this list of conditions and the following disclaimer.
 *
 *	2.  Redistributions in binary form must reproduce the above copyright
 *		notice, this list of conditions and the following disclaimer in the
 *		documentation and/or other materials provided with the distribution.
 *
 *	3.  The name of the copyright holder may not be used to endorse or promote
 *		products derived from this software without specific prior written
 *		permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY
 *  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL WILLIAM TISÄTER BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 *  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <vector>
#include <iostream>

#include "argument.h"

using namespace std;

ArgumentSet::ArgumentSet(int argc, char ** argv)
{
	int c;
	Argument temp;
	string argument, value;
	
	for (c = 1; c < argc; c++)
	{
		argument = string(argv[c]);
		
		if (argument.substr(0, 1) == "-")
		{
			// Handle long options
			if (argument.substr(1, 1) == "-")
			{
				// Is this a toggle flag or a flag with value?
				if (argument.find_first_of("=") < string::npos)
				{
					temp.value = argument.substr(argument.find_first_of("=") + 1);
					temp.argument = argument.substr(2, argument.find_first_of("=") - 2);
				}
				else
				{
					temp.value = "1";
					temp.argument = argument.substr(2);
				}
			}
			else
			{
				if ((c + 1) < argc)
					value = string(argv[c + 1]);
				else
					value = "1";
				
				if (value.substr(0, 1) == "-")
					value = "1";
				
				argument = argument.substr(1);
				
				temp.value = value;
				temp.argument = argument;
			}
			
			this->args.push_back(temp);
		}
	}
}

bool ArgumentSet::isset(const string & arg)
{
	for (vector<Argument>::iterator i = args.begin(); i != args.end(); i++)
	{
		if (i->argument == arg)
			if (i->value != "")
				return 1;
	}
	
	return 0;
}

string ArgumentSet::get(const string & arg, const std::string & val)
{
	for (vector<Argument>::iterator i = args.begin(); i != args.end(); i++)
	{
		if (i->argument == arg)
			return i->value;
	}
	
	return val;
}
