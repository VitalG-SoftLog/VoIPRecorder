// Header: StringUtils.h
//////////////////////////////////////////////////////////////////////////
#pragma once

#include <string>
#include <list>
#include <vector>

//////////////////////////////////////////////////////////////////////////
// Namespace: StringUtils
// Strings utils functions
//////////////////////////////////////////////////////////////////////////
namespace StringUtils
{
	// Function: Explode
	// Breaks apart a string into substrings separated by a character string
	// does not use a strtok() style list of separator characters returns a 
	// list of strings
	// 
	// Params:
	// inString - [in] const reference to source string.
	// separator - [in] const reference to string which contain tokens 
	// separator.
	//
	// Returns:
	// List of tokens strings.
	std::list<std::string> Explode(const std::string &inString, const std::string &separator);


	// Function: VExplode
	// Breaks apart a string into substrings separated by a character string
	// does not use a strtok() style list of separator characters returns a 
	// list of strings
	// 
	// Params:
	// inString - [in] const reference to source string.
	// separator - [in] const reference to string which contain tokens 
	// separator.
	//
	// Returns:
	// Vector of tokens strings.
	std::vector<std::string> VExplode(const std::string &inString, const std::string &separator);


	// Function: Trim
	// Trim trailing and leading whitespace characters.
	//
	// Params:
	// inString - [in] const reference to input string.
	// 
	// Returns:
	// Return input string without trailing and leading whitespace characters.
	std::string Trim(const std::string &inString);

	// Function: ToLowerCase
	// Convert input string to lowercase.
	//
	// Params:
	// inString - [in] const reference to input string.
	// 
	// Returns:
	// Return lowercased string.
	std::string ToLowerCase(const std::string &inString);

	// Function: ToUpperCase
	// Convert input string to uppercase.
	//
	// Params:
	// inString - [in] const reference to input string.
	// 
	// Returns:
	// Return uppercased string.
	std::string ToUpperCase(const std::string &inString);

	// Function: StringToInt
	// Convert string value to integer.
	//
	// Params:
	// inString - [in] const reference to input string.
	// 
	// Returns:
	// Return integer.
	int StringToInt(const std::string &inString);

	// Function: IntToString
	// Convert integer value to string.
	//
	// Params:
	// n - [in] integer value.
	// 
	// Returns:
	// Return string with numeric value.
	std::string IntToString(int n);

	// Function: IntToHex
	// Convert integer value to HEX string.
	//
	// Params:
	// n - [in] integer value.
	// len - [in] if not 0 set the output padding.
	// 
	// Returns:
	// Return string with HEX value.
	std::string IntToHex(int n, int len=0);

	// Function: PtrToHex
	// Convert pointer value to string.
	//
	// Params:
	// p - [in] pointer.
	// 
	// Returns:
	// Return string with pointer value.
	std::string PtrToHex(void *p);

	// Function: IsAllDigits
	// Return true if all symbols in string is digits.
	//
	// Params:
	// sIn - [in] const reference to input string.
	// 
	// Returns:
	// returns true if all chars is digits.
	bool IsAllDigits(const std::string &sIn);
};
