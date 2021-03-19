#include "stdafx.h"
#include "ConsoleHelper.h"

#include <stdarg.h>
#include <iostream>

/*static*/ void ConsoleHelper::WriteLine(const char * format, ...)
{
	va_list args;
	va_start(args, format);

	char toWrite[200];
	::vsnprintf(toWrite, 200, format, args);

	std::cout << toWrite;
	std::cout << "\r\n";
}

/*static*/ string ConsoleHelper::ReadLine(const char * format, ...)
{
	va_list args;
	va_start(args, format);

	char toWrite[200];
	::vsnprintf(toWrite, 200, format, args);

	cout << toWrite;
	cout << "\r\n";

	string input = "";
	getline(cin, input);
	return input;
}



