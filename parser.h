#ifndef TAGHANDLER_H
#define TAGHANDLER_H

#include <string>
#include <vector>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include "tinyxml.h"
#include <vector>
#include <string>
#include "predefs.h"
#include "parser.h"
#include "util.h"
#include "tinyxml.h"

using namespace std;

struct instRecord
{
   string function;
   string position;
   bool eachIteration;
   int    basicBlockNo;
   string    loopNo;
   string name;
   string type;
   string variableName;
   string variableType;
   int samplingRate;
   int priority;
   string category;
   bool programmable;
   instRecord()
   {
      basicBlockNo=-1; loopNo="";type="";
      samplingRate=1; priority=1; category="";
      variableType ="";
      eachIteration = false;
      programmable = false;
   }
};


/* Important NOTE:
 * Monitoring events are in three types. SINGULAR, DUAL, and DATA_ACCESS.
 * The name for SINGULAR and DATA_ACCESS are directly taken from the xml provided by users. => no change in name
 * in the case of DUAL, there are two events. One event starts with Begin_ and the second event starts with End_
 * This is done by parser and user does not need to worry about it.
 */

class Parser{

   public:
   Parser(){procLimiter = 1;}
   void parse(const char*);
   vector<instRecord> getRecord();

   private:
   void defineProcLimiterEnvironmentVariable();
   void point(TiXmlElement* pElem, instRecord* record);
   void variable(TiXmlElement* pElem, instRecord* record);
   void monElement(TiXmlElement*, instRecord*, instRecord*);
   void arrangeRecords(vector<instRecord>*);
   void addAppArgsToRunMonElement(const char* appArgs);
   void checkDataType(const char *pText);
   void setProcLimiter(int procLimiter);
   int getProcLimiter();

   /* The processor limitter. Defined by user in the xml file */
   int procLimiter;

   /*
    * records contains all monitoring data from XML input file. The monitoring elements are read by Parser one by one.
    */
   vector<instRecord> records;

   /*
    * arragedrecs contains monitoring elements that is sorted.
    * The sort criteria is based on all Begin_ should come first then END_ come at the end.
    * The arrangement is a necessity for Dyninst to instrument properly. By this, the order of events generated are perserved.
    * Above that, the arrangement of monitoring elements makes the instrumentation
    * more consistent from one tool to another.
    */
   vector<instRecord> arrangedrecs;

};
#endif /*TAGHANDLER_H*/
