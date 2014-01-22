/*
 * File:   MySqlCaller.cpp
 * Author: mbouchard
 *
 * Created on October 25, 2012, 10:25 AM
 */

#include "MySqlCaller.h"
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <limits>

static void test_stmt_error(MYSQL_STMT *stmt, int status)
{
    if (status)
    {
        fprintf(stderr, "Error: %s (errno: %d)\n",
                mysql_stmt_error(stmt), mysql_stmt_errno(stmt));
        exit(1);
    }
}

MYSQL_TIME str2mytime(string s)
{
    string date = s.substr(0,s.find_first_of(" "));
    string time = s.substr(s.find_first_of(" ")+1,string::npos);
    string year = date.substr(0,date.find_first_of("-"));
    string month = date.substr(date.find_first_of("-")+1,string::npos);
    month = month.substr(0,month.find_first_of("-"));
    string day = date.substr(date.find_last_of("-")+1,string::npos);
    string hour = time.substr(0,time.find_first_of(":"));
    string minute = time.substr(time.find_first_of(":")+1, string::npos);
    minute = minute.substr(0,month.find_first_of(":"));
    string second = time.substr(time.find_last_of(":")+1, string::npos);
    
    MYSQL_TIME mst;
    mst.year = atol(year.c_str());
    mst.month = atol(month.c_str());
    mst.day = atol(day.c_str());
    mst.hour = atol(hour.c_str());
    mst.minute = atol(minute.c_str());
    mst.second = atol(second.c_str());
    mst.second_part = mst.second;
    mst.time_type = MYSQL_TIMESTAMP_NONE;
    mst.neg = false;
    
    return mst;
}


long PreparedStatement::Init(MYSQL *conn, string call)
{
    stmt = mysql_stmt_init(conn);
    if (!stmt)
    {
        printf("Could not initialize statement\n");
        exit(1);
    }
    int status = mysql_stmt_prepare(stmt,call.c_str(), call.length());
    test_stmt_error(stmt, status);
    connection = conn;
    
    return 0;
}

long PreparedStatement::Close()
{
    mysql_stmt_close(stmt);
    stmt = NULL;
    
    return 0;
}




long PreparedStatement::AddParam(enum_field_types t, string v)
{
    type.push_back(t);
    val.push_back(v);
    pos.push_back(size);
    
    switch (t) {
        case MYSQL_TYPE_TINY:
            size+=sizeof(signed char); break;
        case MYSQL_TYPE_SHORT:
            size+=sizeof(short int); break;
        case MYSQL_TYPE_LONG:
            size+=sizeof(int); break;
        case MYSQL_TYPE_LONGLONG:
            size+=sizeof(long long int); break;
        case MYSQL_TYPE_DECIMAL:
            size+=(v.length()+1)*sizeof(char); break;
        case MYSQL_TYPE_NEWDECIMAL:
            size+=(v.length()+1)*sizeof(char); break;
        case MYSQL_TYPE_FLOAT:
            size+=sizeof(float); break;
        case MYSQL_TYPE_DOUBLE:
            size+=sizeof(double); break;
        case MYSQL_TYPE_TIME:
            size+=sizeof(MYSQL_TIME); break;
        case MYSQL_TYPE_DATE:
            size+=sizeof(MYSQL_TIME); break;
        case MYSQL_TYPE_DATETIME:
            size+=sizeof(MYSQL_TIME); break;
        case MYSQL_TYPE_TIMESTAMP:
            size+=sizeof(MYSQL_TIME); break;
        case MYSQL_TYPE_STRING:
            size+=v.length()*sizeof(char); break;
        case MYSQL_TYPE_VAR_STRING:
            size+=(v.length()+1)*sizeof(char); break;
        case MYSQL_TYPE_BLOB:
            size+=(v.length()+1)*sizeof(char); break;
        case MYSQL_TYPE_BIT:
            size+=(v.length()+1)*sizeof(char); break;
        case MYSQL_TYPE_NULL:
            size++; break;
        default:
            break;
    }
    
    return 0;
}
long PreparedStatement::NoRecExecute()
{
    int n = int(type.size());
    MYSQL_BIND* params = new MYSQL_BIND[n];
    char* buffer = new char[size];
    memset(params, 0, n*sizeof(MYSQL_BIND));
    
    signed char c;
    short int si;
    int ii;
    long long int lli;
    float f;
    double d;
    unsigned long ul;
    
    for(int i = 0; i < n; i++)
    {
        params[i].buffer_type = type[i];
        params[i].buffer = (char *)&buffer[pos[i]];
        params[i].length = 0;
        params[i].is_null = 0;
        
        MYSQL_TIME mt;
        
        switch (type[i]) {
            case MYSQL_TYPE_TINY:
                c = (signed char)(atol(val[i].c_str()));
                memcpy(&buffer[pos[i]], &c, sizeof(signed char)); break;
            case MYSQL_TYPE_SHORT:
                si = (short int)(atol(val[i].c_str()));
                memcpy(&buffer[pos[i]], &si, sizeof(short int)); break;
            case MYSQL_TYPE_LONG:
                ii = (int)(atol(val[i].c_str()));
                memcpy(&buffer[pos[i]], &ii, sizeof(int)); break;
            case MYSQL_TYPE_LONGLONG:
                lli = (long long int)(atol(val[i].c_str()));
                memcpy(&buffer[pos[i]], &lli, sizeof(long long int)); break;
            case MYSQL_TYPE_DECIMAL:
                snprintf((char*)params[i].buffer, (val[i].length()+1), "%s", val[i].c_str());
                ul = val[i].length()+1; params[i].length = &ul; break;
            case MYSQL_TYPE_NEWDECIMAL:
                snprintf((char*)params[i].buffer, (val[i].length()+1), "%s", val[i].c_str());
                ul = val[i].length()+1; params[i].length = &ul; break;
            case MYSQL_TYPE_FLOAT:
                f = (float)(atol(val[i].c_str()));
                memcpy(&buffer[pos[i]], &f, sizeof(float)); break;
            case MYSQL_TYPE_DOUBLE:
                d = (double)(atof(val[i].c_str()));
                memcpy(&buffer[pos[i]], &d, sizeof(double)); break;
            case MYSQL_TYPE_TIME:
                mt = str2mytime(val[i]);
                memcpy(&buffer[pos[i]], &mt, sizeof(MYSQL_TIME)); break;
            case MYSQL_TYPE_DATE:
                mt = str2mytime(val[i]);
                memcpy(&buffer[pos[i]], &mt, sizeof(MYSQL_TIME)); break;
            case MYSQL_TYPE_DATETIME:
                mt = str2mytime(val[i]);
                memcpy(&buffer[pos[i]], &mt, sizeof(MYSQL_TIME)); break;
            case MYSQL_TYPE_TIMESTAMP:
                mt = str2mytime(val[i]);
                memcpy(&buffer[pos[i]], &mt, sizeof(MYSQL_TIME)); break;
            case MYSQL_TYPE_STRING:
                memcpy(&buffer[pos[i]], val[i].c_str(), (val[i].length()+1)*sizeof(char));
                ul = val[i].length()+1; params[i].length = &ul; break;
            case MYSQL_TYPE_VAR_STRING:
                snprintf((char*)params[i].buffer, (val[i].length()+1), "%s", val[i].c_str());
                ul = val[i].length(); params[i].length = &ul; break;
            case MYSQL_TYPE_BLOB:
                snprintf((char*)params[i].buffer, (val[i].length()+1), "%s", val[i].c_str());
                ul = val[i].length()+1; params[i].length = &ul; break;
            case MYSQL_TYPE_BIT:
                snprintf((char*)params[i].buffer, (val[i].length()+1), "%s", val[i].c_str());
                ul = val[i].length()+1; params[i].length = &ul; break;
            case MYSQL_TYPE_NULL:
                memset(&buffer[pos[i]], 0, 1); break;
            default:
                break;
        }
    }
    
    int status = mysql_stmt_bind_param(stmt, params);
    test_stmt_error(stmt, status);
    
    status = mysql_stmt_execute(stmt);
    test_stmt_error(stmt, status);
    
    delete[] params;
    delete[] buffer;
    
    return 0;
}

long PreparedStatement::Execute(MyRecordset* prs, long indcol)
{
    NoRecExecute();
    prs->LoadStmt(stmt, indcol);
   
    return 0;
}

string MySqlCaller::GetDbName()
{
	return dbname;
}

long MySqlCaller::DoRequest(string request, MyRecordset* prs, long indcol)
{
    MYSQL_RES *result = NULL;
    
    int state = mysql_query(connection, request.c_str());
    if (state)
    {
        printf("Unable to execute query: %s\n", mysql_error(connection) );
        return 1;
    }

    result = mysql_store_result(connection);    
    prs->Load(result, indcol);
    mysql_free_result(result);
    
    return 0;
}

long MySqlCaller::DoNoRecRequest(string request)
{
    MYSQL_RES *result = NULL;
    
    int state = mysql_query(connection, request.c_str());
    result = mysql_store_result(connection);
    if (state)
    {
        printf("Unable to execute query: %s\n", mysql_error(connection) );
        return 1;
    }
    
    mysql_free_result(result);
        
    return 0;
}

long MyRecordset::Load(MYSQL_RES *result, long indcol)
{
    MYSQL_FIELD    *fields;
    MYSQL_ROW row;
    
    nbcol = mysql_num_fields(result);
    
    fields = mysql_fetch_fields(result);
    
    for(int i = 0; i < nbcol; i++)
    {
        colname.push_back(fields[i].name);
        coltype.push_back(fields[i].type);
    }
    
    int linenum = 0;
    while ( ( row=mysql_fetch_row(result)) != NULL )
    {
        data.push_back(vector<string>());
        for(int i = 0; i < nbcol; i++)
        {
            data.back().push_back(row[i]);
            if(indcol != -1 && i == indcol)
            {
                int cval = atol(row[i]);
                index[cval] = linenum;
            }
        }
        linenum++;
    }
    nbrow = linenum;

    return 0;
}

long MyRecordset::LoadStmt(MYSQL_STMT* stmt, long indcol)
{
    MYSQL_FIELD    *fields;
    MYSQL_BIND* binds = NULL;  /* for output buffers */
    MYSQL_RES *result = NULL;
    result = mysql_stmt_result_metadata(stmt);
    test_stmt_error(stmt, result == NULL);
    
    char* buffer = NULL;
    
    nbcol = mysql_stmt_field_count(stmt);
        
    fields = mysql_fetch_fields(result);
    
    binds = new MYSQL_BIND[nbcol*sizeof(MYSQL_BIND)];
    
    memset(binds, 0, sizeof (MYSQL_BIND) * nbcol);
    
    int size=0;
    for(int i = 0; i < nbcol; i++)
        size+=fields[i].length;
    
    buffer = new char[size*8];
    
    memset(buffer, 0, sizeof (char) * size*8);
    int pos = 0;
    
    for(int i = 0; i < nbcol; i++)
    {
        colname.push_back(fields[i].name);
        coltype.push_back(fields[i].type);
        binds[i].buffer_type = fields[i].type;
        binds[i].is_null = 0;
        binds[i].buffer = &buffer[pos];
        binds[i].buffer_length = fields[i].length;
        pos+=fields[i].length;
    }

    int status = mysql_stmt_bind_result(stmt, binds);
    test_stmt_error(stmt, status);
    
    int linenum = 0;
    
    char tmp[8000];
    
    while(true)
    {
        status = mysql_stmt_fetch(stmt);
        if (status == 1 || status == MYSQL_NO_DATA)
            break;
        data.push_back(vector<string>());
        MYSQL_TIME ts;
        string stmp;
        for(int i = 0; i < nbcol; i++)
        {
            switch (binds[i].buffer_type) {
                case MYSQL_TYPE_TINY:
                    snprintf(tmp, 8000, "%d", *((signed char*)(binds[i].buffer)));
                    data.back().push_back(string(tmp)); break;
                case MYSQL_TYPE_SHORT:
                    snprintf(tmp, 8000, "%d", *((short int*)(binds[i].buffer)));
                    data.back().push_back(string(tmp)); break;
                case MYSQL_TYPE_LONG:
                    snprintf(tmp, 8000, "%d", *((int*)(binds[i].buffer)));
                    data.back().push_back(string(tmp)); break;
                case MYSQL_TYPE_LONGLONG:
                    snprintf(tmp, 8000, "%lld", *((long long int*)(binds[i].buffer)));
                    data.back().push_back(string(tmp)); break;
                case MYSQL_TYPE_DECIMAL:
                    snprintf(tmp, 8000, "%s", ((char*)(binds[i].buffer)));
                    data.back().push_back(string(tmp)); break;
                case MYSQL_TYPE_NEWDECIMAL:
                    snprintf(tmp, 8000, "%s", (char*)(binds[i].buffer));
                    stmp = string(tmp);
                    data.back().push_back(stmp); break;
                case MYSQL_TYPE_FLOAT:
                    snprintf(tmp, 8000, "%f", *((float*)(binds[i].buffer)));
                    data.back().push_back(string(tmp)); break;
                case MYSQL_TYPE_DOUBLE:
                    snprintf(tmp, 8000, "%f", *((double*)(binds[i].buffer)));
                    data.back().push_back(string(tmp)); break;
                case MYSQL_TYPE_TIME:
                    ts = *((MYSQL_TIME*)(binds[i].buffer));
                    snprintf(tmp, 8000, "%04d-%02d-%02d %02d:%02d:%02d", ts.year, ts.month, ts.day, ts.hour, ts.minute, ts.second);
                    data.back().push_back(string(tmp)); break;
                case MYSQL_TYPE_DATE:
                    ts = *((MYSQL_TIME*)(binds[i].buffer));
                    snprintf(tmp, 8000, "%04d-%02d-%02d %02d:%02d:%02d", ts.year, ts.month, ts.day, ts.hour, ts.minute, ts.second);
                    data.back().push_back(string(tmp)); break;
                case MYSQL_TYPE_DATETIME:
                    ts = *((MYSQL_TIME*)(binds[i].buffer));
                    snprintf(tmp, 8000, "%04d-%02d-%02d %02d:%02d:%02d", ts.year, ts.month, ts.day, ts.hour, ts.minute, ts.second);
                    data.back().push_back(string(tmp)); break;
                case MYSQL_TYPE_TIMESTAMP:
                    ts = *((MYSQL_TIME*)(binds[i].buffer));
                    snprintf(tmp, 8000, "%04d-%02d-%02d %02d:%02d:%02d", ts.year, ts.month, ts.day, ts.hour, ts.minute, ts.second);
                    data.back().push_back(string(tmp)); break;
                case MYSQL_TYPE_STRING:
                    snprintf(tmp, 8000, "%s", (char*)binds[i].buffer);
                    data.back().push_back(string(tmp)); break;
                case MYSQL_TYPE_VAR_STRING:
                    snprintf(tmp, 8000, "%s", (char*)binds[i].buffer);
                    data.back().push_back(string(tmp)); break;
                case MYSQL_TYPE_BLOB:
                    snprintf(tmp, 8000, "%s", (char*)binds[i].buffer);
                    data.back().push_back(string(tmp)); break;
                case MYSQL_TYPE_BIT:
                    snprintf(tmp, 8000, "%s", (char*)binds[i].buffer);
                    data.back().push_back(string(tmp)); break;
                case MYSQL_TYPE_NULL:
                    data.back().push_back(""); break;
                default:
                    printf("UNKNOWN TYPE %s %d\n", fields[i].name, binds[i].buffer_type);
                    exit(1);
                    break;
            }
            
            if(indcol != -1 && i == indcol)
            {
                int cval = *((int*)(binds[i].buffer));
                index[cval] = linenum;
            }
        }
        linenum++;
    }
    nbrow = linenum;
    
    
    while(mysql_stmt_next_result(stmt) == 0)
    {
        linenum+=0;
    }
    
    mysql_free_result(result);
    delete[] binds;
    delete[] buffer;
    
    return 0;
}


long MyRecordset::Print()
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

long MySqlCaller::Connect()
{
    char command[4096];
    printf("Connecting to %s... ", servername.c_str());
    
    mysql_init(&mysql);
    connection = mysql_real_connect(&mysql,servername.c_str(), username.c_str(), password.c_str() ,dbname.c_str(),0,0,CLIENT_MULTI_STATEMENTS);
    if (connection == NULL)
    {
        printf("\nUnable to open data source: %s\n", mysql_error(connection) );
        return 1;
    }
    
    snprintf(command, 4096, "use %s", dbname.c_str());
    printf("%s\n", command);

    printf("done!\n");

    return 0;
}

long MySqlCaller::DrConnect()
{
    char command[4096];
    printf("Connecting to %s... ", servername.c_str());
    
    mysql_init(&mysql);
    connection = mysql_real_connect(&mysql,servername.c_str(), username.c_str(), password.c_str() ,dbname.c_str(),0,0,CLIENT_MULTI_STATEMENTS);
    if (connection == NULL)
    {
        printf("\nUnable to open data source: %s\n", mysql_error(connection));
        return 1;
    }
    
    snprintf(command, 4096, "use %s", dbname.c_str());
    printf("%s\n", command);
    
    return 0;
}

long MySqlCaller::Disconnect()
{
    printf("Disconnecting... ");
    mysql_close(connection);
    printf("done!\n");

    return 0;
}

 MySqlCaller::MySqlCaller(string sn, string un, string dn, string pw)
 {
	 Init(sn, un, dn, pw);
 }

 long MySqlCaller::Init(string sn, string un, string dn, string pw)
 {
     servername = sn;
     username = un;
     dbname = dn;
     password = pw;

     return 0;
 }
