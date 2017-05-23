#include <fstream>
#include <string>
#include <cstring>
#include <iostream>
#include <dirent.h>
#include <stdlib.h>
#include "predefs.h"
using namespace std;

/*
 * printOutput is given the job id and the rank of the application and creates the summary of the application run
 * by that particular node.
 */
void printOutput(const char* id, const char* rank)
{
   cout<<"===================================================="<<endl;
   string line;
   const char* buf;

   /*
    * Home for ProMon
    * Makefile mandates the user to define it
    */
   char* envVar = getenv("PROMONHOME");
   if (envVar == NULL)
   {
      fprintf(stderr, "ProMon Injector: PROMONHOME environment variable is not defined!!\n");
      return;
   }

   string HOME = envVar;
   string directory = HOME + "/" + LOG_DIRECTORY;
   directory = directory + id + "/" + rank + "/"+ LOG_SUMMARY;
   buf = directory.c_str();

   ifstream fileInput(buf);

   if (fileInput.is_open())
   {
      while (fileInput.good())
      {
         getline(fileInput, line);
         if (line.empty())
            break;

         cout << '\t' << line.c_str() << endl;
      }
      fileInput.close();
   } else
      cout << '\t' << "No summary file was found!" << endl
            << "\tPossible reason: The running application did not end." << endl;
   cout<<"===================================================="<<endl;
}

/*
 * findAllRanks will go through the directory of the given job id and calls "printOutput" function for each node.
 * Each node has its own directory under the job id.
 */
void findAllRanks(const char* id)
{
   const char* buf;

   /*
    * Home for ProMon
    * Makefile mandates the user to define it
    */
   char* envVar = getenv("PROMONHOME");
   if (envVar == NULL)
   {
      fprintf(stderr, "ProMon Injector: PROMONHOME environment variable is not defined!!\n");
      return;
   }

   string HOME = envVar;
   string directory = HOME + "/" + LOG_DIRECTORY;
   directory = directory + id + "/";
   buf = directory.c_str();

   unsigned char isFolder =0x4;
   DIR *dir = opendir(buf);
   struct dirent *dirEntry;

   while( (dirEntry=readdir(dir)) != NULL)
   {
      if ( dirEntry->d_type == isFolder)
      {
           if(strcmp(dirEntry->d_name, ".") != 0 && strcmp(dirEntry->d_name, "..") != 0)
              printOutput(id, dirEntry->d_name);
      }
   }
   closedir(dir);
}

int main(int argc, char* argv[])
{
   if(argc < 2)
   {
      cout<<"ERROR usage: promo_report [Job ID]\n";
      return -1;
   }
   cout << endl << '\t' << "<<** Summary of JOB ID " << argv[1] << " **>>"
         << endl << endl;
   findAllRanks(argv[1]);
   cout << endl << endl;
   return 0;
}

