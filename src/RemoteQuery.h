/* 
 * File:   RemoteQuery.h
 * Author: Mathieu Bouchard
 *
 * Created on November 4, 2011, 10:25 AM
 */

#ifndef _REMOTEQUERY_H
#define	_REMOTEQUERY_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <string>
#include "zlib.h"

using namespace std;

class RemoteQuery
{
public:
	RemoteQuery() {}
    ~RemoteQuery() {}
    
    long DoQuery(string db, string q);
    long BeginMultQueries(string db);
    long AddQuery(string q);
    long EndMultQueries();
    long GZipAndSend(string in, bool dorecv=true);

private:
    int sockfd;
    int portno;
    int n;
    sockaddr_in serv_addr;

    char res;
    FILE* f;
    long unsigned int lSize;
    unsigned char* compstr;
    int fd[2];
    int* tr;
 };

#endif	/* _REMOTEQUERY_H */

