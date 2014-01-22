/* 
 * File:   MongoTrigger.h
 * Author: Mathieu Bouchard
 *
 * Created on September 11, 2013, 9:55 AM
 */

#ifndef _MONGOCALLER_H
#define	_MONGOCALLER_H

#include <string>
#include <map>
#include <vector>

/*
#ifndef __cplusplus
#define __cplusplus
#endif
*/

#ifndef MONGO_HAVE_STDINT
#define MONGO_HAVE_STDINT
#endif

#if defined WIN64 || defined WIN32
    #include <windows.h>
    #define TRETURN DWORD WINAPI
    #define THREADEND return 0;
#else
    #include <pthread.h>
    #define TRETURN void*
    #define THREADEND return NULL;
#endif

#include "mongo.h"
#include "bcon.h"
#include <time.h>
#include <vector>
#include <list>

typedef void (*cbmongotype)(mongo_cursor*, void*);

using namespace std;

class PosInfo
{
public:
    PosInfo() {}
    ~PosInfo() {}
    
    PosInfo* parent;
    void* data;
    void* obj;
    void* next;
    void* key;
    int type;
    
};

class BsonUtil
{
public:
    BsonUtil() {}
    ~BsonUtil() {}
    
    static string Bson2string(const bson* b, bool oneline=true, bool compact=true);
    static bson String2bson(string s);
    static bson File2bson(string filename);
};

class MongoObj
{
public:
    MongoObj() {conn.sock = 0;}
    ~MongoObj();
    int Connect(string _server, int _port);
    int Auth(string db, string _user, string _pwd);
    int ConnectAndAuth(string _server, int _port, string db, string _user, string _pwd);
    
    mongo conn;
    char tmp[4096];
};

class MongoQuery
{
public:
    MongoQuery() {currind = -1; currpos = NULL; depth=0; pmo = NULL; cursor.conn = NULL;}
    MongoQuery(MongoObj* _pmo) {currind = -1; currpos = NULL; depth=0; pmo = _pmo; cursor.conn = NULL;}
    ~MongoQuery();
    int Query(string db, string collection, bson* query);
    int QueryBson(const bson* data);
    //int Query(string db, string collection, string query);
    
    int GetCurrentBson(bson* b);
    MongoQuery GetCurrentQuery();
    int SetPosInfo(void* c, PosInfo* parent);
    int FindKey(string key);
    int GetIn();
    int GetOut();
    int NextItem();
    int Next();
    string GetKey();
    int GetType();
    string GetStrValue();
    int GetIntValue();
    double GetDoubleValue();
    
    MongoObj* pmo;
    mongo_cursor cursor;
    int currind;
    int depth;
    PosInfo* currpos;
    vector<PosInfo*> browse;
};

class MongoTrigger
{
public:
    MongoTrigger() {}
    ~MongoTrigger() {}
    
    int Start(MongoObj* _pmo, string _db, string _coll, string _filter, cbmongotype _cb, void* _obj, int delay_sec, int delay_nano);
    int Stop();
    int Loop();

private:
    void* obj;
    MongoObj* pmo;
    string db;
    string coll;
    string filter;
    timespec ts;
    cbmongotype cb;
    bool mutex;
};

#endif	/* _MONGOCALLER_H */

