#ifndef UTIL_H
#define UTIL_H

#include <vector>
#include <string>
#include <stdlib.h>
#include <stdarg.h>
#include <fstream>
#include <cstring>

using namespace std;

//====================================================
#ifdef MT_WITH_BOOST 

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/sync_frontend.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/log/common.hpp>
#include <boost/log/utility/empty_deleter.hpp>
#include <boost/log/expressions/formatters/stream.hpp>
#include <boost/log/expressions/formatters/format.hpp>
#include <boost/log/expressions/formatters/date_time.hpp>
#include <boost/log/support/date_time.hpp>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;

using namespace logging::trivial;

/*
 * Logging interface.
 * This is boost log.
 */
void LOG_init();
void LOG_debug(const char *msg);
void LOG_info(const char *msg);
void LOG_error(const char *msg);
void LOG_fatal(const char *msg);

#endif // for MT_WITH_BOOST
//====================================================
vector<string> split(string str, string delim);
int readConfigurationFile();

void ProMon_logger_init(const char* fileName);
void ProMon_logger(int type, const char *fmt,...);
void ProMon_logger_close();


#endif /*UTIL_H*/
