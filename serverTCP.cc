#include "serverTCP.h"
using namespace std;

serverTCP::serverTCP(int p):server(p) {
    
}

serverTCP::~serverTCP() {
    
}

/*
 * This function creates a TCP server socket to receive all instrumentation records.
 */
int serverTCP::start()
{
   struct sockaddr_in si_me, si_other;
   int s, newsockfd, i, slen = sizeof(si_other);

   if ((s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1)
      ProMon_logger(PROMON_ERROR, "ProMon SERVERTCP: Could not get TCP socket!");


   memset((char *) &si_me, 0, sizeof(si_me));
   si_me.sin_family = AF_INET;
   si_me.sin_port = htons(port);
   si_me.sin_addr.s_addr = htonl(INADDR_ANY );
    if (bind(s, (struct sockaddr *) &si_me, sizeof(si_me)) == -1)
      ProMon_logger(PROMON_ERROR, "ProMon SERVERTCP: Could not bind!");

   /*
    * The second parameter is the backlog that defines the maximum length for the
    * queue of pending connections for the socket
    */
   listen(s, TCP_QUEUE_SIZE);

   i = 0;
   char buffer[BUF_SIZE+2]; // JEC: +2 fixes the TCP error -- but YOU explain why!

   /*
    * we need to make sure all elements are zero.
    * Some compilers version does not initialize it with zero.
    */
   memset(buffer, 0, sizeof buffer);
   for (;;)
   {
      /*
       * accept connection from client.
       */
      newsockfd = accept(s, (struct sockaddr *) &si_other, (socklen_t*) &slen);
      if (newsockfd < -1)
         ProMon_logger(PROMON_ERROR, "ProMon SERVERTCP: accept error!!! %d", errno);

      int childpid = fork();
      if (childpid < 0)
         ProMon_logger(PROMON_ERROR, "ProMon SERVERTCP: fork error!!! %d", errno);
      
      if (childpid == 0)
      {
         /*
          * The exit is called when the process ends
          */
         string strRecords("");
         vector<string> splitRecs;
         while(1)
         {
            /*
             * Each node is handled by a child process.
             */
            //printf("\nchild %d\n", childpid);
            int ret = recv(newsockfd, buffer, BUF_SIZE, MSG_WAITALL);
            if (ret == -1)
            {   
               ProMon_logger(PROMON_ERROR, "ProMon SERVERTCP: Broken connection!!! %d", errno);
               //handlingEndOfApplicaitonRun
               close(newsockfd);
               _Exit(3); //child exits
               exit(0);
            }
           
            if (ret == 0)
            {   
               ProMon_logger(PROMON_ERROR, "ProMon SERVERTCP: Closed connection on descriptor %d", newsockfd);
               /* Clean up resources in Analyzer */
               string temp = rank_jobids[newsockfd];
               char * buf = new char [temp.length()+1];
               strcpy (buf, temp.c_str());
               char *rank = strtok(buf, ":");
               char *job_id = strtok(NULL, ":");
               ProMon_logger(PROMON_DEBUG,"ProMon SERVERTCP: Cleaning resources for %s:%s!", job_id, rank);
               Analyzer::sumup_clean_static(rank, job_id);
               rank_jobids.erase(newsockfd);

               //handlingEndOfApplicaitonRun
               close(newsockfd);
               _Exit(3); //child exits
               exit(0);
            }

            /*
             * Make sure you handle all records. Including those that
             * partially in the buffer
             */
            strRecords += buffer;

            /* Keep the rank and job id to clean up in case the monitored application crashes. */
            if(rank_jobids.find(newsockfd) == rank_jobids.end())
            {
               char *rank, *job_id;
               rank = strtok(buffer, ";");
               strtok(NULL, ";"); //username, no need here.
               strtok(NULL, ";"); //jobMS, no need here.
               job_id = strtok(NULL, ";");
               rank_jobids[newsockfd] = string(rank)+":"+string(job_id);
            }

            /*
             * Records should not have any new line character 
             */
            string::size_type pos = 0; 
            while ( ( pos = strRecords.find ("\n",pos) ) != string::npos )
            {
               strRecords.erase ( pos, 2 );
            }
           
            /* split is defined on top */ 
            splitRecs = split(strRecords, " ");
            for(vector<string>::iterator it = splitRecs.begin(); it != splitRecs.end(); it++)
            {
               string temp = *it;
               /*
                * If | was not found then this is partial record. The rest are in the buffer. We clear strRecord by only having the partial record.
                * Assumption: All partial record will not end with | and they are always the last record in the buffer
                */
               if(temp.find("|") == string::npos)
                  strRecords = temp;
               else
               {
                  temp[temp.size()-1]='\0';   
                  strRecords.clear();

                  ProMon_logger(PROMON_DEBUG, "ProMon SERVERTCP: Received packet from %s:%d",
                        inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
                  ProMon_logger(PROMON_DEBUG, "ProMon SERVERTCP: %s", temp.c_str());

                  char *record = new char[temp.size()]; //we have set the last character to null (above)
                  strcpy(record, temp.c_str());
                  Analyzer::handleMSG_static(record, sizeof buffer);
                  delete[] record;
               }
            }

            /*
             * make sure the buffer is cleared.
             */
            memset(buffer, 0, sizeof buffer);
    
            i++;
         }
      }
      else
      {
         /*
          * close the copy of child with parent
          */
         close(newsockfd);
      }

   }
   close(s);
   return 0;
}

