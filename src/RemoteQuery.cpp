/*
 * File:   RemoteQuery.cpp
 * Author: mbouchard
 *
 * Created on November 4, 2011, 10:25 AM
 */

#include "RemoteQuery.h"

long RemoteQuery::GZipAndSend(string in, bool dorecv)
{
	z_stream strm;
	int len = in.length();

    strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.next_in = (unsigned char*)in.c_str();
	strm.avail_in = len;
	deflateInit2(&strm, /*Z_DEFAULT_COMPRESSION*/Z_BEST_COMPRESSION, Z_DEFLATED, (15+16), 8, Z_DEFAULT_STRATEGY);

	long unsigned int complen = deflateBound(&strm, len);
	compstr = new unsigned char[complen+4];
	strm.avail_out = complen;
	strm.next_out = compstr+4;
	lSize = deflate(&strm, Z_FINISH);
	deflateEnd(&strm);

	//printf("%ld (%ld) %ld %ld\n", long(in.length()), complen-int(strm.avail_out), complen, lSize);
	tr = (int*)compstr;
	*tr = int(complen-strm.avail_out);

	n = send(sockfd,compstr, complen-strm.avail_out+4,0);
	if(dorecv)
		n = recv(sockfd,&res,1,0);

	delete[] compstr;

    return 0;
}

long RemoteQuery::DoQuery(string db, string query)
{
    res = '0';

    if(res != '1')

    portno = 53219;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
    	printf("ERROR opening socket");
        return 1;
    }

    memset(&serv_addr,0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;

    serv_addr.sin_addr.s_addr = inet_addr("132.203.210.76");
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr)) < 0)
    {
    	printf("ERROR connecting");
    	return 1;
    }

    GZipAndSend("<connect>Integrated Security=SSPI;Connection Reset=True;persist security info=False;Server=forac-db1"
    	        "\\FORACDEV2012;Database="+db);
	//	"\\FORACDEV2008;Database="+db);
    if(res != '1')
    	return 1;
    GZipAndSend("<beginTransaction>");
    if(res != '1')
    	return 1;
    GZipAndSend(string("<query>")+query);
    if(res != '1')
    	return 1;
    GZipAndSend("<commit>");
    if(res != '1')
    	return 1;
    GZipAndSend("<disconnect>");
    if(res != '1')
        return 1;
    GZipAndSend("<close>");
    if(res != '1')
        return 1;

    close(sockfd);

 	return 0;
}

long RemoteQuery::BeginMultQueries(string db)
{
    res = '0';

    if(res != '1')

    portno = 53219;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
    	printf("ERROR opening socket");
        return 1;
    }

    memset(&serv_addr,0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;

    serv_addr.sin_addr.s_addr = inet_addr("132.203.210.76");
    serv_addr.sin_port = htons(portno);

    if (connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr)) < 0)
    {
    	printf("ERROR connecting");
    	return 1;
    }

    GZipAndSend("<connect>Integrated Security=SSPI;Connection Reset=True;persist security info=False;Server=forac-db1"
                "\\FORACDEV2012;Database="+db);
//    		"\\FORACDEV2008;Database="+db);

    if(res != '1')
    	return 1;

	return 0;
}
long RemoteQuery::AddQuery(string query)
{
	GZipAndSend("<beginTransaction>");
	if(res != '1')
		return 1;
	GZipAndSend(string("<query>")+query);
	if(res != '1')
		return 1;
	GZipAndSend("<commit>");
	if(res != '1')
		return 1;

	return 0;
}
long RemoteQuery::EndMultQueries()
{
	GZipAndSend("<disconnect>");
	if(res != '1')
		return 1;
	GZipAndSend("<close>");
	if(res != '1')
		return 1;

	close(sockfd);

	return 0;
}
