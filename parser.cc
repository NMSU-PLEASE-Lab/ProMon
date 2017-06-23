#include "parser.h"

using namespace std;

/*
 * Returns all monitoring elements read from input XML file
 */
vector<instRecord> Parser::getRecord()
{
    return arrangedrecs;
}

/*
 * Loads the input XML file and processes monitoring elements.
 * input XML contains records of single and dual events
 */
void Parser::parse(const char *pFilename)
{
    TiXmlDocument doc(pFilename);
    if (!(doc.LoadFile())) {
        ProMon_logger(PROMON_ERROR, "ProMon Parser: XML file %s could not be loaded\n",
                      pFilename);
        ProMon_logger(PROMON_ERROR, "ProMon Parser: Working directory is (%s)\n", getcwd(0, 0));
        return;
    }

    TiXmlHandle hDoc(&doc);
    TiXmlElement *pElem;
    TiXmlHandle hRoot(0);

    pElem = hDoc.FirstChildElement().Element();
    if (!pElem) {
        cerr << "ProMon Parser: No valid root!!\n";
        return;
    }

    // save this for later
    hRoot = TiXmlHandle(pElem);

    /*
     * Handle the process limiter (ProcLimiter).
     * ProcLimiter defines which node to send monitoring data.
     * Only nodes that their rank is factor of procLimiter generates data.
     * (rank%procLimiter == 0 then send data)
     * The default is 1
     */
    pElem = hRoot.FirstChild("ProcLimiter").Element();
    if (!pElem) {
        setProcLimiter(1);
    } else {
        const char *procLim = pElem->GetText();
        setProcLimiter(atoi(procLim));
    }

    /*
     * Handle the MonElement records
     */
    pElem = hRoot.FirstChild("MonElement").Element();
    for (; pElem; pElem = pElem->NextSiblingElement()) {
        const char *pKey = pElem->Value();

        /*
         * Dual events contain two records
         * We will discard any record2 for Singulars and Data_Access events.
         */
        instRecord record1;
        instRecord record2;

        if (strcmp(pKey, "MonElement") == 0)
            monElement(pElem, &record1, &record2);

        if (record1.function != "") {
            ProMon_logger(PROMON_DEBUG, "ProMon Parser: %s;%s;%s\n", record1.function.c_str(),
                          record1.position.c_str(), record1.name.c_str());
            records.push_back(record1);
        }
        if (record2.function != "") {
            ProMon_logger(PROMON_DEBUG, "ProMon Parser: %s;%s;%s\n", record2.function.c_str(),
                          record2.position.c_str(), record2.name.c_str());
            records.push_back(record2);
        }
    }

    ProMon_logger(PROMON_DEBUG, "ProMon Parser: total records: %d\n", records.size());

    /*Arrange all monitoring element*/
    arrangeRecords(&records);

}

/*
 * This function handles the one monitoring element.
 * every record has different elements that will be handled by this function.
 */
void Parser::monElement(TiXmlElement *pElem, instRecord *record1,
                        instRecord *record2)
{
    /* Let's make sure we follow the PML xml schema */
    bool hasName = false, hasType = false, hasPoint = false,
        hasBeginPoint = false, hasEndPoint = false, hasVariable = false;

    TiXmlHandle hRoot = TiXmlHandle(pElem);

    pElem = hRoot.FirstChild().Element();
    for (; pElem; pElem = pElem->NextSiblingElement()) {
        const char *pKey = pElem->Value();
        const char *pText = pElem->GetText();
        if (strcmp(pKey, "SamplingRate") == 0) {
            record1->samplingRate = atoi(pText);
            record2->samplingRate = atoi(pText);
        } else if (strcmp(pKey, "Priority") == 0) {
            record1->priority = atoi(pText);
            record2->priority = atoi(pText);
        } else if (strcmp(pKey, "Category") == 0) {
            record1->category = pText;
            record2->category = pText;
        } else if (strcmp(pKey, "Type") == 0) {
            hasType = true;
            record1->type = pText;
            record2->type = pText;
        } else if (strcmp(pKey, "Name") == 0) {
            hasName = true;
            if (!hasType) {
                ProMon_logger(PROMON_ERROR, "ProMon Parser: Type has to be defined prior to Name!!");
                return;
            }
            if (record1->type == "DUAL") {
                record1->name = DUAL_BEGIN + string(pText);
                record2->name = DUAL_END + string(pText);
            } else if (record1->type == "SINGULAR") {
                record1->name = SINGULAR + string(pText);
            } else if (record1->type == "HEARTBEAT") {
                record1->name = HEARTBEAT + string(pText);
            } else if (record1->type == "DATA_ACCESS") {
                record1->name = DATA_ACCESS + string(pText);
            } else {
                record1->name = pText;
                record2->name = pText;
            }
        } else if (strcmp(pKey, "Programmable") == 0) {
            if (strcmp(pText, "true") == 0) {
                record1->programmable = true;
                record2->programmable = true;
            }
        } else if (strcmp(pKey, "Point") == 0) {
            hasPoint = true;
            point(pElem, record1);
        } else if (strcmp(pKey, "BeginPoint") == 0) {
            hasBeginPoint = true;
            point(pElem, record1);
        } else if (strcmp(pKey, "EndPoint") == 0) {
            hasEndPoint = true;
            point(pElem, record2);
        } else if (strcmp(pKey, "Variable") == 0) {
            hasVariable = true;
            variable(pElem, record1);
        }
    }

    if (!hasType || !hasName) {
        ProMon_logger(PROMON_ERROR, "ProMon Parser: Type or Name has not been defined!!");
        return;
    }

    if (hasPoint && (hasBeginPoint || hasEndPoint)) {
        ProMon_logger(PROMON_ERROR, "ProMon Parser: Monitoring elements can have either Point or (Begin/End)Point!!");
        return;
    }

    if (hasVariable && (hasBeginPoint || hasEndPoint)) {
        ProMon_logger(PROMON_ERROR,
                      "ProMon Parser: Monitoring elements that reads variables only can have Point to define their position!!");
        return;
    }

    /*
     * Make sure both are set, then do the changes to the name
     * Practically, always recrod1 is set.
     */
    if (record1->programmable && record2->programmable) {
        record1->name = record1->name + PROGRAMMABLE;
        record2->name = record1->name + PROGRAMMABLE;
    }



    /* All needed values are available.
     * The event name that is passed to Injector will be in either these formats:
     * SamplingRate:Priority:Category:name or
     * SamplingRate:Priority:Category:DATA_ACCESS:Type:Name:varType (varType is added in the variable function)
     */
    record1->name = static_cast<ostringstream *> (&(ostringstream() << record1->samplingRate))->str() + ":"
        + static_cast<ostringstream *> (&(ostringstream() << record1->priority))->str() + ":"
        + record1->category + ":" + record1->name;
    record2->name = static_cast<ostringstream *> (&(ostringstream() << record2->samplingRate))->str() + ":"
        + static_cast<ostringstream *> (&(ostringstream() << record2->priority))->str() + ":"
        + record2->category + ":" + record2->name;
}

/*
 * This function handles Variable tags
 * Variable type will be appended to Name
 */
void Parser::variable(TiXmlElement *pElem, instRecord *record)
{
    bool hasType = false, hasName = false;
    TiXmlHandle hRoot = TiXmlHandle(pElem);
    pElem = hRoot.FirstChild().Element();
    for (; pElem; pElem = pElem->NextSiblingElement()) {
        const char *pText = pElem->GetText();
        const char *pKey = pElem->Value();
        if (strcmp(pKey, "Type") == 0) {
            hasType = true;
            /* Make sure you the type is a valid one */
            checkDataType(pText);
            record->variableType = pText;
        } else if (strcmp(pKey, "Name") == 0) {
            hasName = true;
            record->variableName = pText;
            record->name = record->name + ":" +  record->variableType;
        }

    }

    if (!hasType || !hasName) {
        ProMon_logger(PROMON_ERROR, "ProMon Parser: In the Variable tag, Name or Type is not defined!!");
        return;
    }

}

/*
 * This function handles Point tags
 */
void Parser::point(TiXmlElement *pElem, instRecord *record)
{
    bool hasFunction = false, hasPosition = false, hasLoop = false, hasBB = false;
    TiXmlHandle hRoot = TiXmlHandle(pElem);
    pElem = hRoot.FirstChild().Element();
    for (; pElem; pElem = pElem->NextSiblingElement()) {
        const char *pText = pElem->GetText();
        const char *pKey = pElem->Value();
        if (strcmp(pKey, "Function") == 0) {
            hasFunction = true;
            record->function = pText;
        } else if (strcmp(pKey, "Position") == 0) {
            hasPosition = true;
            record->position = pText;
            if (strcmp(pText, "BEGIN") == 0 && strcmp(pText, "END") == 0) {
                ProMon_logger(PROMON_ERROR,
                              "ProMon Parser: In the Point section, Position is not correct!!");
                return;
            }
        }
        /*
         * Check whether to instrument at begin/end of each iteration
         * of the defined loop
         */
        else if (strcmp(pKey, "EachIteration") == 0) {
            if (strcmp(pText, "TRUE") == 0)
                record->eachIteration = true;
            else if (strcmp(pText, "FALSE") == 0)
                record->eachIteration = false;
            else
                ProMon_logger(PROMON_ERROR, "ProMon Parser: Value of EachIteration Tag is not correct!!");
            return;
        }
        else if (strcmp(pKey, "BasicBlock") == 0) {
            hasBB = true;
            record->basicBlockNo = atoi(pText);
        } else if (strcmp(pKey, "Loop") == 0) {
            hasLoop = true;
            /*loop format is now changed to string for eg: 1,1.1,2,2.1,2.2.2 etc*/
            record->loopNo = pText;    // atoi(pText);
        }
    }

    if (hasLoop) record->name = record->name + ";Loop";
    else if (hasBB) record->name = record->name + ";BasicBlock";
    else record->name = record->name + ";Function";
    ProMon_logger(PROMON_DEBUG, "ProMon Parser: RECORD NAME FOR POINT:%s",record->name.c_str());
    if (!hasFunction || !hasPosition) {
        ProMon_logger(PROMON_ERROR, "ProMon Parser: In the Point tag, Function or Position is not defined!!");
        return;
    }

}

/*
 * This function will handle data access
 */
void Parser::checkDataType(const char *pText)
{
    if (strcmp(pText, "short") == 0)
        return;
    else if (strcmp(pText, "int") == 0)
        return;
    else if (strcmp(pText, "float") == 0)
        return;
    else if (strcmp(pText, "long") == 0)
        return;
    else if (strcmp(pText, "double") == 0)
        return;
    else if (strcmp(pText, "bool") == 0)
        return;
    else {
        cerr << "ProMon Parser: " << pText << " Wrong data type !!!" << endl;
        exit(-1);
    }
}

/*
 * This function makes sure that for DUAL events, all Begin_ to be processed first and then all End_.
 * We want to make sure that each being pairs with the correct end.
 * This due to how Dyninst performs instrumentation.
 * We may use different function for other tools like pin
 */
void Parser::arrangeRecords(vector<instRecord> *records)
{
    for (vector<instRecord>::reverse_iterator it = records->rbegin();
         it != records->rend(); ++it) {
        if (it->position == "BEGIN" && it->type == "DUAL") {
            arrangedrecs.push_back(*it);
        } else if (it->position == "BEGIN")
            arrangedrecs.push_back(*it);
    }

    for (vector<instRecord>::iterator it = records->begin();
         it != records->end(); ++it) {
        ProMon_logger(PROMON_DEBUG, "ProMon Parser: %s;%s;%s;\n", it->function.c_str(),
                      it->position.c_str(), it->name.c_str());

        if (it->position == "END" && it->type == "DUAL") {
            arrangedrecs.push_back(*it);
        } else if (it->position == "END")
            arrangedrecs.push_back(*it);
    }

#ifdef DEBUG
    cout << "ProMon Parser: " << "After arranging" << endl;
    for (vector<instRecord>::iterator it = arrangedrecs.begin();
        it != arrangedrecs.end(); ++it)
        cout << "ProMon Parser: " << it->function << ";" << it->position
        << ";" << it->name << endl;
#endif

}

/*
 * Add the parameter of the running application to analyzer.
 * the parameter is not fixed and it varies from one application to another.
 * It provide us a good indicator to analyze the application like if it is restart or new run.
 */
void Parser::addAppArgsToRunMonElement(const char *appArgs)
{
    for (vector<instRecord>::iterator it = arrangedrecs.begin();
         it != arrangedrecs.end(); ++it) {
        size_t found = it->name.find(RUN);
        if (found != string::npos) {
            it->name += ":";
            it->name += appArgs;
        }
    }
}

/* The processor limitter. Defined by user in the xml file */
void Parser::setProcLimiter(int procLim)
{
    if (procLim <= 0) {
        cerr << "procLimiter has to be bigger than 0!!!" << endl;
        exit(-1);
    }

    procLimiter = procLim;
}

int Parser::getProcLimiter()
{
    return procLimiter;
}

void Parser::defineProcLimiterEnvironmentVariable()
{
    /*
     * Parser provide the procLimiter
     */
    int procLimiter = getProcLimiter();
    char envValue[10];
    sprintf(envValue, "%d", procLimiter);
    setenv("PROMON_PROC_LIMITER", envValue, 1);
    ProMon_logger(PROMON_DEBUG, "ProMon Parser: PROMON_PROC_LIMITER %d \n", procLimiter);
}

