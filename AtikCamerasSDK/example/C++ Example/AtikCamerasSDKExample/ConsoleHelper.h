#pragma once

#include <string>

using namespace std;

class ConsoleHelper
{
public:
	static void   WriteLine(const char * format, ...);
	static string ReadLine( const char * format, ...);
};

