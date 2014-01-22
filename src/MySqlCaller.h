/* 
 * File:   MySqlCaller.h
 * Author: Mathieu Bouchard
 *
 * Created on September 25, 2012, 9:55 AM
 */

#ifndef _MYSQLCALLER_H
#define	_MYSQLCALLER_H

#include "mysql/mysql.h"
#include "SqlCaller.h"
#include <string>
#include <map>
#include <vector>

using namespace std;

MYSQL_TIME str2mytime(string s);

class MyRecordset
{
public:
    MyRecordset() {}
    ~MyRecordset() {}
    
    long Load(MYSQL_RES *result, long indcol = -1);
    long LoadStmt(MYSQL_STMT *stmt, long indcol = -1);
    long Print();
   
    long nbcol;
    long nbrow;
    
    vector<string> colname;
    vector<enum_field_types> coltype;
    vector<vector<string> > data;
    map<long, long> index;
};

class PreparedStatement
{
public:
    PreparedStatement() {stmt = NULL; size = 0;}
    ~PreparedStatement() {if(stmt != NULL) mysql_stmt_close(stmt);}
    
    long Init(MYSQL *conn, string call);
    long Close();
    long AddParam(enum_field_types t, string v);
    long NoRecExecute();
    long Execute(MyRecordset* prs, long indcol = -1);

    int size;
    vector<enum_field_types> type;
    vector<string> val;
    vector<int> pos;
    
    MYSQL *connection;
    MYSQL_STMT* stmt;
};

class MySqlCaller
{
public:
    MySqlCaller() {}
    MySqlCaller(string sn, string un, string dn, string pw);
    ~MySqlCaller() {}
    
    long Init(string sn, string un, string dn, string pw);
        
    long Connect();
    long DrConnect();
    long Disconnect();
    long DoRequest(string request, MyRecordset* prs, long indcol = -1);
    long DoNoRecRequest(string request);
    string GetDbName();
    
    MYSQL *connection, mysql;
    
private:
    MYSQL_RES *result;
    MYSQL_ROW row;
    
    
    string servername;
    string username;
    string dbname;
    string password;
};


#endif	/* _MYSQLCALLER_H */

