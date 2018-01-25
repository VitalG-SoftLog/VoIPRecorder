#include "stdafx.h"
#include <fstream>
#include <iostream>
#include "Debug.h"
#include <windows.h>
#include "Locker.h"

using namespace std;

#ifdef _DEBUG_LOG

static bool							consoleOut = true;
static string						logFileName("debug_out.txt");
static bool							appStart = true;
static critical_section				debugLocker;

const string& debug::log_file_name() { return logFileName; }

static void check_app_start() 
{
	if(!appStart) return;
	appStart = false;
	ofstream f1( logFileName.c_str() );
}

static void out_string(const string& out) 
{
	ofstream ff( logFileName.c_str(),  ios::out | ios::app);
	ff<<out<<endl;
	ff.close();
	if(consoleOut) cout<<out<<endl;
}

void debug::log(const std::string& module_name, const std::string& mes) 
{
	scope_locker scope_locker(debugLocker);
	check_app_start();
	string s = string(" ////// ") + module_name + string(": * ") + mes + string(" * ");
	out_string(s);
}

void debug::warning(const std::string& module_name, const std::string& mes) 
{
	scope_locker scope_locker(debugLocker);
	check_app_start();
	string s = string(" --WR-- ") + module_name + string(": * ") + mes + string(" * ");
	out_string(s);
}

static void print_error(const string& module_name, const string& mes, const string& del) 
{
	check_app_start();
	string str;
	str = del + module_name + string(": * ") + mes + string(" * ");
	out_string(str);
}

void debug::error(const std::string& module_name, const std::string& mes) 
{
	scope_locker scope_locker(debugLocker);
	print_error(module_name, mes, " --!!-- ");
}

void debug::api_error(const std::string& module_name, const std::string& mes) 
{
	scope_locker scope_locker(debugLocker);
	print_error(module_name, mes, " --API- ");
}

bool debug::is_console_out() { return consoleOut; }
void debug::console_out(bool b) { consoleOut = b; }

#endif//_DEBUG_LOG
