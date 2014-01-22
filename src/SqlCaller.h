/* 
 * File:   SqlCaller.h
 * Author: Mathieu Bouchard
 *
 * Created on November 13, 2008, 9:55 AM
 */

#ifndef _SQLCALLER_H
#define	_SQLCALLER_H

#if defined WIN64 || defined WIN32
	#if defined WIN32
		#define PLATFORMSQLCHAR SQLCHAR
	#else
		#define PLATFORMSQLCHAR SQLCHAR
	#endif
	#define SQL_DRIVER "{SQL Server}"
	#ifndef snprintf
		#define snprintf sprintf_s
	#endif
	#include "windows.h"
#else
	#define SQL_DRIVER "FreeTDS"
	#define PLATFORMSQLCHAR SQLCHAR
#endif

#include "sql.h"
#include "sqlext.h"
#include <string>
#include <map>
#include <vector>
//#include "ShpTools.h"



using namespace std;

class Recordset
{
public:
    Recordset() {}
    ~Recordset() {}
    
    long Load(HSTMT& Statement, long indcol = -1);
    long Print();
   
    long nbcol;
    long nbrow;
    
    vector<string> colname;
    vector<int> coltype;
    vector<vector<string> > data;
    map<long, long> index;
};

class SqlCaller
{
public:
	SqlCaller() {}
    SqlCaller(string sn, string un, string dn, string pw);
    SqlCaller(string sn, string un, string dn, string pw, string dr);
    SqlCaller(string sn, string un, string dn, string pw, string dr, long p);
    ~SqlCaller() {}
    
    long Init(string sn, string un, string dn, string pw, string dr  = "", long p=-1);
        
    long Connect();
    long DrConnect();
    long Disconnect();
    long DoRequest(string request, Recordset* prs, long indcol = -1);
    long DoNoRecRequest(string request);
    string GetDbName();
    
private:
    HENV Environment;
    HDBC Connection;
    HSTMT Statement;
    
    string servername;
    string username;
    string dbname;
    string password;
    string drivername;
    long port;
};

long getlong(string val, bool pos=true);
double getdbl(string val, bool pos=true);

#endif	/* _SQLCALLER_H */

