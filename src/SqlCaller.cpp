/*
 * File:   SqlCaller.cpp
 * Author: mbouchard
 *
 * Created on November 13, 2008, 10:25 AM
 */

#include "SqlCaller.h"
#include <stdlib.h>
#include <stdio.h>
#include <limits>

long getlong(string val, bool pos)
{
 	long ret = numeric_limits<long>::infinity();
 	if(!pos)
 		ret = -numeric_limits<long>::infinity();
 	if(!val.empty())
 		ret = atof(val.c_str());
 	return ret;
}

 double getdbl(string val, bool pos)
 {
 	double ret = numeric_limits<double>::infinity();
 	if(!pos)
 		ret = -numeric_limits<double>::infinity();
 	if(!val.empty())
 		ret = atof(val.c_str());
 	return ret;
 }

string SqlCaller::GetDbName()
{
	return dbname;
}

long SqlCaller::DoRequest(string request, Recordset* prs, long indcol)
{
    int result = 0;
    result = SQLExecDirect(Statement, (PLATFORMSQLCHAR *) request.c_str(), SQL_NTS);

    if (result != SQL_SUCCESS && result != SQL_NO_DATA)
    {
        printf("Unable to execute statement %d\n", result);
        return 1;
    }

    prs->Load(Statement, indcol);

	if (result != SQL_SUCCESS && result != SQL_NO_DATA)
	{
	   printf("a) Unable to execute statement %d\n", result);
	   return 1;
	}
	SQLFreeStmt(Statement, SQL_DROP);
	Statement = SQL_NULL_HSTMT;
	if (SQLAllocStmt(Connection, &Statement) != SQL_SUCCESS)
	{
	   printf("b) Unable to execute statement %d\n", result);
	   return 1;
	}

    return 0;
}

long SqlCaller::DoNoRecRequest(string request)
{
    int result = 0;
    result = SQLExecDirect(Statement, (PLATFORMSQLCHAR *) request.c_str(), SQL_NTS);

    if (result != SQL_SUCCESS && result != SQL_NO_DATA)
    {
        printf("Unable to execute statement %d\n", result);
        return 1;
    }

	if (result != SQL_SUCCESS && result != SQL_NO_DATA)
	{
	   printf("a) Unable to execute statement %d\n", result);
	   return 1;
	}
	SQLFreeStmt(Statement, SQL_DROP);
	Statement = SQL_NULL_HSTMT;
	if (SQLAllocStmt(Connection, &Statement) != SQL_SUCCESS)
	{
	   printf("b) Unable to execute statement %d\n", result);
	   return 1;
	}

    return 0;
}

long Recordset::Load(HSTMT& Statement, long indcol)
{
    char val[2048];
    SQLSMALLINT col = 1;
    SQLSMALLINT length;
    SQLSMALLINT datatype;
    SQLULEN colsize;
    SQLSMALLINT NbDecimal;
    SQLSMALLINT isNullable;

    while(true)
    {
        //SQL_NTS, (PLATFORMSQLCHAR *) username.c_str()

        if(SQLDescribeCol(Statement, col, (PLATFORMSQLCHAR *)val, 2048, &length, &datatype, &colsize, &NbDecimal, &isNullable) != SQL_SUCCESS)
        {
            break;
        }
        colname.push_back(val);
        coltype.push_back(datatype);

        col++;
    }
    nbcol = col-1;

    long linenum = 0;
    while(true)
    {
        if (SQLFetch(Statement) != SQL_SUCCESS)
        {
            //printf("Unable to fetch row");
            break;
        }
        data.push_back(vector<string>());

        col = 1;
        while(true)
        {
            SQLLEN outlen;
          //  if (SQLGetData(Statement, 1, SQL_C_SLONG, &int_buf, 0, NULL) != SQL_SUCCESS)
            if (SQLGetData(Statement, col,  SQL_C_CHAR, (PLATFORMSQLCHAR *)val, 2048, &outlen) != SQL_SUCCESS)
            {
                //printf("Unable to get data");
                break;
            }

            data.back().push_back(val);
            if (outlen <= 0)
            	data.back().back().clear();

            if(indcol != -1 && col == indcol)
            {
                long cval = atol(val);
                index[cval] = linenum;
            }
            col++;
        }
        linenum++;
       // printf("\n");
    }
    nbrow = linenum;

    return 0;
}

long Recordset::Print()
{
    vector<string>::iterator itcn;
    vector<vector<string> >::iterator itl;
    vector<string>::iterator itcv;

    for(itcn = colname.begin(); itcn != colname.end(); itcn++)
        printf("| %s |", itcn->c_str());
    printf("\n");
    for(itl = data.begin(); itl != data.end(); itl++)
    {
        for(itcv = itl->begin(); itcv != itl->end(); itcv++)
        {
        	if(!itcv->empty())
        		printf("| %s |", itcv->c_str());
        	else
        		printf("| NULL |");
        }
        printf("\n");
    }

    return 0;
}

long SqlCaller::Connect()
{
    SQLRETURN res;

    char command[4096];

    if (SQLAllocEnv(&Environment) != SQL_SUCCESS)
    {
        printf("Unable to allocate env\n");
        return 1;
    }
    if (SQLAllocConnect(Environment, &Connection) != SQL_SUCCESS)
    {
        printf("Unable to allocate connection\n");
        SQLFreeEnv(Environment);
        return 1;
    }
    if (SQLAllocEnv(&Environment) != SQL_SUCCESS)
    {
        printf("Unable to allocate env\n");
        return 1;
    }

    printf("Connecting to %s... ", servername.c_str());

    res = 0;
    res = SQLConnect(Connection, (PLATFORMSQLCHAR *) servername.c_str(), SQL_NTS, (PLATFORMSQLCHAR *) username.c_str(), SQL_NTS, (PLATFORMSQLCHAR *) password.c_str(), SQL_NTS);
    if (!SQL_SUCCEEDED(res))
    {
        printf("\nUnable to open data source (ret=%d)\n", res);
        return 1;
    }

    if (SQLAllocStmt(Connection, &Statement) != SQL_SUCCESS)
    {
	printf("Unable to allocate statement\n");
        return 1;
    }

    snprintf(command, 4096, "use %s", dbname.c_str());
    printf("%s\n", command);

    if (!SQL_SUCCEEDED(SQLExecDirect(Statement, (PLATFORMSQLCHAR *) command, SQL_NTS)))
    {
	printf("Unable to execute statement\n");
        return 1;
    }

    printf("done!\n");

    return 0;
}

long SqlCaller::DrConnect()
{
    SQLRETURN res;
    SQLSMALLINT len;
    char command[4096];

    if (SQLAllocEnv(&Environment) != SQL_SUCCESS)
    {
        printf("Unable to allocate env\n");
        return 1;
    }
    if (SQLAllocConnect(Environment, &Connection) != SQL_SUCCESS)
    {
        printf("Unable to allocate connection\n");
        SQLFreeEnv(Environment);
        return 1;
    }
    if (SQLAllocEnv(&Environment) != SQL_SUCCESS)
    {
        printf("Unable to allocate env\n");
        return 1;
    }

    printf("Connecting to %s... ", servername.c_str());

    res = 0;

    char tmp[4096];
    char tmp2[4096];

    if(port == -1)
    	snprintf(tmp, 4096, "DRIVER=%s;SERVER=%s;UID=%s;PWD=%s;DATABASE=%s;TDS_Version=8.0;",
    		drivername.c_str(), servername.c_str(), username.c_str(),
    		password.c_str(), dbname.c_str());
    else
    	snprintf(tmp, 4096, "DRIVER=%s;SERVER=%s;UID=%s;PWD=%s;DATABASE=%s;TDS_Version=8.0;Port=%ld;",
    	    drivername.c_str(), servername.c_str(), username.c_str(),
    	    password.c_str(), dbname.c_str(), port);
    res = SQLDriverConnect(Connection, NULL, (PLATFORMSQLCHAR *) tmp, SQL_NTS, (PLATFORMSQLCHAR *) tmp2, sizeof(tmp2), &len, SQL_DRIVER_NOPROMPT);

    if (!SQL_SUCCEEDED(res))
    {
        printf("\nUnable to open data source (ret=%d)\n", res);
        return 1;
    }

    if (SQLAllocStmt(Connection, &Statement) != SQL_SUCCESS)
    {
	printf("Unable to allocate statement\n");
        return 1;
    }

    snprintf(command, 4096, "use %s", dbname.c_str());
    printf("%s\n", command);

    if (!SQL_SUCCEEDED(SQLExecDirect(Statement, (PLATFORMSQLCHAR *) command, SQL_NTS)))
    {
	printf("Unable to execute statement\n");
        return 1;
    }

    printf("done!\n");

    return 0;
}

long SqlCaller::Disconnect()
{
    printf("Disconnecting... ");

    if (Statement)
    {
        SQLFreeStmt(Statement, SQL_DROP);
        Statement = SQL_NULL_HSTMT;
    }
    if (Connection)
    {
        SQLDisconnect(Connection);
        SQLFreeConnect(Connection);
        Connection = SQL_NULL_HDBC;
    }

    if (Environment)
    {
        SQLFreeEnv(Environment);
        Environment = SQL_NULL_HENV;
    }
     printf("done!\n");

    return 0;
}

 SqlCaller::SqlCaller(string sn, string un, string dn, string pw)
 {
	 Init(sn, un, dn, pw);
 }

 SqlCaller::SqlCaller(string sn, string un, string dn, string pw, string dr)
 {
	 Init(sn, un, dn, pw, dr);
 }

 SqlCaller::SqlCaller(string sn, string un, string dn, string pw, string dr, long p)
 {
 	 Init(sn, un, dn, pw, dr, p);
 }

 long SqlCaller::Init(string sn, string un, string dn, string pw, string dr, long p)
 {
     servername = sn;
     username = un;
     dbname = dn;
     password = pw;
     drivername = dr;
     port = p;

     return 0;
 }
