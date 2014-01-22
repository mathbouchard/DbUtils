#include "JsonObject.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>

#include "mongo.h"
#include "bcon.h"


int JsonObject::InitFromStr(string str)
{
    /*string jsonstr;
    ifstream infile( str ) ;
    if ( !infile )
    {
        std::cout << "file not found!\n" ;
        return 1 ;
    }
    string jsonstr( ( std::istreambuf_iterator<char> ( infile ) ), std::istreambuf_iterator<char> ( ) );
    infile.close( );*/
    
	return 0;
}

int JsonObject::InitFromFile(string filename)
{
	return 0;
}

int JsonObject::InitFromBson()
{
	return 0;
}

int JsonObject::ExportToStr(string* str)
{
	return 0;
}

int JsonObject::ExportToFile(string filename)
{
	return 0;
}

int JsonObject::ExportToBson(string filename, )
{
    vector<bcon> v;
    
    void* curr = elem;
    list<string> stack;
    
    while(curr != NULL)
    {
        switch ( t ) {
            case JO_OBJECT:
                v.push_back("{");
                stack.push_back("}");
                break;
            case JO_ARRAY:
                v.push_back("[");
                stack.push_back("]");
                break;
            case JO_DOUBLE:
                v.push_back(":_f:");
                v.push_back();
                v.back.f = *static_cast<double*>(elem);
                v.push_back(stack.back());
                stack.pop_back();
                break;
            case JO_STRING:
                v.push_back();
                v.back.s = *static_cast<double*>(elem);
                v.push_back( static_cast<string*>(elem)->c_str() );
                v.push_back(stack.back());
                stack.pop_back();
                break;
            case JO_STRING:
                v.push_back();
                v.back.s = *static_cast<double*>(elem);
                v.push_back( static_cast<string*>(elem)->c_str() );
                v.push_back(stack.back());
                stack.pop_back();
                break;
            default:
                v.push_back("<unsupported_type>");
                v.push_back(stack.back());
                stack.pop_back();
        }
        if(type == JO_OBJECT)
        {
            v.push_back("{");
            v.
        }
    }
    
    
	return 0;
}

int JsonObject::GetNext()
{
	return 0;
}

int JsonObject::GetPrev()
{
	return 0;
}

int JsonObject::GetFirst()
{
	return 0;
}

int JsonObject::GetLast()
{
	return 0;
}
