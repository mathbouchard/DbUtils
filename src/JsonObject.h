/* 
 * File:   JsonObject.h
 * Author: Mathieu Bouchard
 *
 * Created on September 11, 2013, 9:55 AM
 */

#ifndef _JSONOBJECT_H
#define	_JSONOBJECT_H

#include <string>
#include <map>
#include <vector>

typedef enum {
    JO_EOO = 0,
    JO_DOUBLE = 1,
    JO_STRING = 2,
    JO_OBJECT = 3,
    JO_ARRAY = 4,
    JO_BINDATA = 5,
    JO_UNDEFINED = 6,
    JO_OID = 7,
    JO_BOOL = 8,
    JO_DATE = 9,
    JO_NULL = 10,
    JO_REGEX = 11,
    JO_DBREF = 12,
    JO_CODE = 13,
    JO_SYMBOL = 14,
    JO_CODEWSCOPE = 15,
    JO_INT = 16,
    JO_TIMESTAMP = 17,
    JO_LONG = 18,
    JO_MAXKEY = 127,
    JO_MINKEY = 255
} jo_type;

using namespace std;


class JsonObject
{
public:
    JsonObject() {}
    ~JsonObject() {}
    
    int InitFromStr(string str);
    int InitFromFile(string filename);
    int InitFromBson();
    int ExportToStr(string* str);
    int ExportToFile();
    int ExportToBson(json* j);
    int __ExportToBson(vector<bson>);
    int GetNext();
    int GetPrev();
    int GetFirst();
    int GetLast();
    int GetKey();

    void* elem;
    void* next;
    int type;
};

#endif	/* _MYSQLCALLER_H */

