#ifndef __DEBUGGH_H_H_
#define __DEBUGGH_H_H_

#include <sstream>
#include <string>

#define _MODULE_NAME(xx) static const std::string __module_name(#xx);

#ifndef _ERROR_NUMBER
#  define _ERROR_NUMBER() GetLastError()
#endif//_ERROR_NUMBER()

#ifdef _DEBUG
#  define NDEBUG
#  define _DEBUG_LOG
#endif//_DEBUG

#ifdef _DEBUG_LOG

namespace debug 
{
	bool is_console_out();
	void console_out(bool);
	const std::string& log_file_name();

	void log(const std::string& module_name, const std::string& mes);
	void warning(const std::string& module_name, const std::string& mes);
	void error(const std::string& module_name, const std::string& mes);
	void api_error(const std::string& module_name, const std::string& mes);
};

#define _LOG(xx) { std::ostringstream ___str; ___str<<xx; \
								debug::log(__module_name, ___str.str()); }

#define _WARNING(xx) { std::ostringstream ___str; ___str<<xx; \
							debug::warning(__module_name, ___str.str()); }

#define _ERROR(xx) { std::ostringstream ___str; ___str<<xx; \
								debug::error(__module_name, ___str.str()); }

#define _API_ERROR(xx) { std::ostringstream ___str; ___str<<xx; \
							debug::APIError(__module_name, ___str.str()); }
#else//_DEBUG_LOG

#define _LOG(xx) {if(false) { std::ostringstream ___str; ___str<<xx; }}

#define _WARNING(xx) {if(false) { std::ostringstream ___str; ___str<<xx; }}

#define _ERROR(xx) {if(false) { std::ostringstream ___str; ___str<<xx; }}

#define _API_ERROR(xx) {if(false) { std::ostringstream ___str; ___str<<xx; }}

#endif//_DEBUG_LOG

#endif//__DEBUGGH_H_H_
