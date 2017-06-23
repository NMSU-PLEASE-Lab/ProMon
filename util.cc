#include <stdio.h>
#include "predefs.h"
#include "util.h"


using namespace std;

/*
 * It is better to avoid strtok. 
 * strtok is hard to maintain specially if it is called inside functions
 */
vector<string> split(string str, string delim)
{
      size_t start = 0;
      size_t end;
      vector<string> v;

      while( (end = str.find(delim, start)) != string::npos )
      {
            v.push_back(str.substr(start, end-start));
            start = end + delim.length();
      }

      /* handle partical events buffered */
      if(str.substr(start).size() !=0 )
         v.push_back(str.substr(start));

      return v;
}

/*
 * The function loads the environment variables defined in ProMon.cfg.
 * With the progress in the project, we may need to add more data there.
 */
int readConfigurationFile()
{
   /*
    * Home for ProMon
    * Makefile mandates the user to define it
    */
   char* envVar = getenv("PROMONHOME");
   if (envVar == NULL)
   {
      ProMon_logger(PROMON_ERROR, "ProMon Util: PROMONHOME environment variable is not defined!!");
      return -1;
   }

   string HOME = envVar;
   string line;
   ifstream promonFile((HOME + "/" + "ProMon.cfg").c_str());

   if (promonFile.fail())
   {
      ProMon_logger(PROMON_ERROR, "ProMon Util: Could not read ProMon.cfg file!!");
      return -1;
   }

   /*
    * Read all configuration data and set them as environment variable
    */
   while (!promonFile.eof())
   {
      getline(promonFile, line);
      char tmp[BUF_SIZE];
      sprintf(tmp, "%s", line.c_str());
      char *envVariable = strtok(tmp, "=");
      char *envValue = strtok(NULL, "=");
      /*
       * on some systems, even though it gets to end of file but still reads the null
       * then announces it is end of file. This check will prevent reading null.
       */
      if(envValue == NULL || envVariable == NULL)
         continue;
      setenv(envVariable, envValue, 1);

      ProMon_logger(PROMON_DEBUG, "ProMon Analyzer: The variable: %s = %s",
            envVariable, envValue);
   }
   promonFile.close();

   return 0;
}

//====================================================
#ifdef MT_WITH_BOOST 
void LOG_init(const char* fileName)
{
   /*
   * This section prepares the Boost Log for printing in file
   */
   #ifdef MT_WITH_LOG
   boost::shared_ptr< sinks::text_file_backend > backend =
       boost::make_shared< sinks::text_file_backend >(
           keywords::file_name = string(fileName)+"_%3N.log",
           keywords::rotation_size = 5 * 1024 * 1024,
           keywords::time_based_rotation = sinks::file::rotation_at_time_point(12, 0, 0) 
       );   

   typedef sinks::synchronous_sink< sinks::text_file_backend > sink_t;
   boost::shared_ptr< sink_t > sink1(new sink_t(backend));

   sink1->set_formatter
   (
         expr::stream <<"["<< expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S")<<"]"
                      <<"["<< expr::attr< unsigned int >("LineID")<<"]"
                      <<"["<<logging::trivial::severity<<"] "
                      <<expr::smessage
   );
   
   /*
    * Set the Boost Log with sink1 for file log
    */
   logging::core::get()->add_sink(sink1);
   #endif
 
   /*
    * This section prepares the Boost Log for printing consol.
    */
   typedef sinks::synchronous_sink< sinks::text_ostream_backend > text_sink;
   boost::shared_ptr< text_sink > sink2 = boost::make_shared< text_sink >();

   // We have to provide an empty deleter to avoid destroying the global stream object
   boost::shared_ptr< std::ostream > stream(&std::clog, logging::empty_deleter());
   sink2->locked_backend()->add_stream(stream);

   sink2->set_formatter
   (
         expr::stream <<"["<< expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d %H:%M:%S")<<"]"
                      <<"["<< expr::attr< unsigned int >("LineID")<<"]"
                      <<"["<<logging::trivial::severity<<"] "
                      <<expr::smessage
   );

   /*
    * Set the Boost Log with both sink2 for file console
    */
   logging::core::get()->add_sink(sink2);
   logging::add_common_attributes();
}

/*
 * Global variable needed for printing log data
 */
src::severity_logger< severity_level > lg;

/*
 * To print debug information.
 * Boost log is configured to have to print it in the consol and file.
 * It is configurable to disable the consol (or minimize it)
 */
void LOG_debug(const char *msg)
{
   BOOST_LOG_SEV(lg, debug) << msg;
}

/*
 * To print information.
 * Boost log is configured to have to print it in the consol and file.
 * It is configurable to disable the consol (or minimize it)
 */
void LOG_info(const char *msg)
{
   BOOST_LOG_SEV(lg, info) << msg;
}

/*
 * To print information about error occurred.
 * Boost log is configured to have to print it in the consol and file.
 * It is configurable to disable the consol (or minimize it)
 */
void LOG_error(const char *msg)
{
   BOOST_LOG_SEV(lg, error) << msg;
}

/*
 * To print information about fatal events.
 * Boost log is configured to have to print it in the consol and file.
 * It is configurable to disable the consol (or minimize it)
 */
void LOG_fatal(const char *msg)
{
   BOOST_LOG_SEV(lg, fatal) << msg;
}
/* Other options we have: 

   BOOST_LOG_TRIVIAL(trace) << "A debug severity message";
   BOOST_LOG_TRIVIAL(info) << "An informational severity message";
   BOOST_LOG_TRIVIAL(warning) << "A warning severity message";
   BOOST_LOG_TRIVIAL(error) << "An error severity message";
   BOOST_LOG_TRIVIAL(fatal) << "A fatal severity message";
*/

#endif  // MT_WITH_BOOST
//====================================================

/*
 * Log all error in file.
 */
FILE *logFile = NULL;

/*
 * Initialize the logger. 
 * Depends on the selection from the makefile, either built-in logger initializes or 
 * Boost log initialize.
 */
void ProMon_logger_init(const char* fileName)
{
   if (fileName == NULL)
   {
      fprintf(stderr, "File name is not provided!!\n");
      exit(0);
   }

   /*
    * Here one of the selected log system will initialize
    */
   #ifdef MT_WITH_BOOST

      LOG_init(fileName);

   #else
      #ifdef MT_WITH_LOG
      /*
      * Home for ProMon
      * Makefile mandates the user to define it
      */
      char* envVar = getenv("PROMONHOME");
      if (envVar == NULL)
      {
         fprintf(stderr, "PROMONHOME environment variable is not defined!!\n");
         return;
      }
      string HOME = envVar;
      string directory = HOME + "/" + string(fileName) + ".log";
      logFile = fopen(directory.c_str(), "a");
      #endif

   #endif
}

void ProMon_logger_close()
{
   #ifdef MT_WITH_LOG
   if(logFile != NULL)
   {
      fflush(logFile);
      fclose(logFile);
   }
   #endif
}

void ProMon_logger(int type, const char *fmt,...)
{
   va_list ap;

   #ifdef MT_WITH_BOOST
      char buf[BUF_SIZE];
      va_start(ap, fmt);
      vsprintf(buf, fmt, ap);
      va_end(ap);

      if(type == PROMON_ERROR)
      {
         LOG_error(buf);
      }else if(type == PROMON_INFO)
      {
         LOG_info(buf);  
      }else if(type == PROMON_DEBUG)
      {
         #ifdef DEBUG
         LOG_debug(buf);         
         #endif
      }
 
   #else    
      if(type == PROMON_ERROR)
      {
         va_start(ap, fmt);
         vfprintf(stderr, fmt, ap);
         va_end(ap);
         fprintf(stderr, "\n");
  
      }else if(type == PROMON_INFO)
      {
         va_start(ap, fmt);
         vprintf(fmt, ap);
         va_end(ap);
         // Omar
         //fprintf(stderr, "%s\n", fmt);
         // Omar
         printf("\n");
  
      }else if(type == PROMON_DEBUG)
      {
         #ifdef DEBUG
         va_start(ap, fmt);
         vprintf(fmt, ap);
         // Omar
         //fprintf(stderr,fmt, ap);
         // Omar
         va_end(ap);
         
         printf("\n");
         #endif
      }
  
      #ifdef MT_WITH_LOG
      va_start(ap, fmt);
      vfprintf(logFile, fmt, ap);
      va_end(ap);
      fprintf(logFile, "\n");
      // Omar
      //fprintf(stderr, logFile, "\n");
      // Omar
      fflush(logFile);
      #endif

   #endif
}
