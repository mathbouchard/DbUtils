#include "MongoCaller.h"
#include <limits>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <string>
#include <iterator>
#include <cerrno>
#include <math.h>
using namespace std;

TRETURN callback(void* p)
{
    static_cast<MongoTrigger*>(p)->Loop();
    THREADEND;
}

int getint(void* c)
{
    return *(static_cast<int*>(c));
}

int reverseint(int i)
{
    int j = ((i>>24)&0xff) | // move byte 3 to byte 0
    ((i<<8)&0xff0000) | // move byte 1 to byte 2
    ((i>>8)&0xff00) | // move byte 2 to byte 1
    ((i<<24)&0xff000000); // byte 0 to byte 3
    return j;
}

pair<char,int32_t> Bson2stringRecursive(string& s, size_t& spos, char* v, int32_t isize, int32_t w, char* tmp)
{
    int32_t size;
    pair<size_t,int> p;
    size_t ssize = s.size();
    
    if(s[spos] == '{')
    {
        if(w==2)
            printf("{\n");
        size=4;
        while(true)
        {
            ++spos;
            int32_t typepos = isize+size;
            ++size;
            
            int in = -1;
            while(spos < ssize && s[spos]!=':')
            {
                if(s[spos] == ' ' || s[spos] == '\n' || s[spos] == '\t')
                {
                    if(in == 0)
                        ++in;
                }
                else
                {
                    if(s[spos]=='}')
                        break;
                    if(in==-1)
                        ++in;
                    if(in==0)
                    {
                        v[w*(isize+size++)] = s[spos];
                        if(w==2)
                            printf("%c", v[w*(isize+size-1)]);
                    }
                }
                ++spos;
            }
            if(s[spos]=='}')
            {
                --size;
                break;
            }
            if(w==2)
                printf(" : ");
            v[w*(isize+size)] = '\0';
            ++spos;
            ++size;
            
            while(spos < ssize && (s[spos]=='\n' || s[spos]=='\t' || s[spos]==' ') )
                ++spos;
            if(s[spos] == '}')
            {
                printf("corrupted file [\n");
                break;
            }
            
            p = Bson2stringRecursive(s, spos, v, isize+size, w, tmp);
            size+=p.second;
            v[w*typepos] = p.first;
            
            ++spos;

            while(spos < ssize && s[spos]!=',' && s[spos]!='}')
                ++spos;
            if(spos == ssize)
            {
                printf("corrupted file [\n");
                break;
            }
            else if(s[spos] == '}')
                break;
        }
        ++size;
  
        char* c = reinterpret_cast<char*>(&size);
        for(int i = 0; i < 4; ++i)
        {
            v[w*isize+i] = c[i];
            if(w==2)
                printf("---%d|%d---\n", isize+i, v[w*isize+i]);
        }
        if(w==2)
            printf("}\n");
        return pair<char,int32_t>(3, size);
    }
    else if(s[spos] == '[')
    {
        if(w==2)
            printf("[\n");
        size=4;
        int index = 0;
        while(true)
        {
            ++spos;
            while(spos < ssize && (s[spos]=='\n' || s[spos]=='\t' || s[spos]==' ') )
                ++spos;
            if(s[spos] == ']')
                break;
            
            int typepos = isize+size;
            ++size;

            snprintf(tmp,33,"%d", index);
            int len = strlen(tmp);
            for(int i = 0; i < len; i++)
                v[w*(isize+size++)] = tmp[i];
            v[w*(isize+size++)] = '\0';
            
            p = Bson2stringRecursive(s, spos, v, isize+size, w, tmp);
            size+=p.second;
            v[w*typepos] = p.first;
                
            ++spos;
            while(spos < ssize && s[spos]!=',' && s[spos]!=']')
                ++spos;
            if(spos == ssize)
            {
                printf("corrupted file [\n");
                break;
            }
            else if(s[spos] == ']')
                break;
            ++index;
            if(w==2)
                printf(",");
        }
        ++size;

        char* c = reinterpret_cast<char*>(&size);
        for(int i = 0; i < 4; ++i)
            v[w*(isize+i)] = c[i];
        if(w==2)
            printf("]\n");
        return pair<char,int32_t>(4, size);
    }
    else if(s[spos] == '"' || s[spos] == '\'' )
    {
        size=4;
        char comp = '\'';
        if(s[spos] != '\'')
            comp='"';
        ++spos;
        while(spos < ssize && s[spos]!=comp)
        {
            v[w*(isize+size++)] = s[spos];
            if(w==2)
                printf("%c", v[w*(isize+size-1)]);
            ++spos;
        }
        ++size;
        //++size;
        int strsize = size-4;
        char* c = reinterpret_cast<char*>(&strsize);
        for(int i = 0; i < 4; ++i)
            v[w*(isize+i)] = c[i];
        if(w==2)
            printf("\n");
        --spos;
        return pair<char,int32_t>(2, size);
    }
    else if(strncmp(&(s[spos]), "ObjectId(", 9) == 0)
    {
        spos+=9;
        while(spos < ssize && s[spos]!='"' && s[spos]=='\'' )
            ++spos;
        ++spos;
        for(int i = 0; i < 12; ++i)
        {
            unsigned char ii = strtol((string("0x")+s[spos]+s[spos+1]).c_str(), NULL, 16);
            v[w*(isize+i)]=ii;
            spos+=2;
        }
        while(spos < ssize && s[spos]!=')' )
            ++spos;
        return pair<char,int32_t>(7, 12);
    }
    else
    {
        char* end;
        double tf = strtod(&(s[spos]), &end);
        int len = end-&(s[spos]);
        if(tf == numeric_limits<double>::infinity() || tf == numeric_limits<double>::quiet_NaN() || tf == numeric_limits<double>::signaling_NaN() || fabs(tf) > double(int(fabs(tf))) )
        {
            char* c = reinterpret_cast<char*>(&tf);
            for(int i = 0; i < 8; ++i)
                v[w*(isize+i)] = c[i];
            if(w==2)
                printf("%f:f\n", tf);
            //getchar();
            spos+=len-1;
            return pair<char,int32_t>(1, 8);
        }
        else if(errno == ERANGE)
        {
            int64_t ll = strtol(&(s[spos]), &end, 0);
            len = end-&(s[spos]);
            char* c = reinterpret_cast<char*>(&ll);
            for(int i = 0; i < 8; ++i)
                v[w*(isize+i)] = c[i];
            spos+=len-1;
            if(w==2)
                printf("%ld:64\n", ll);
            return pair<char,int32_t>(18, 8);
        }
        else if(tf <= numeric_limits<int32_t>::max())
        {
            int32_t ii = int32_t(tf);
            char* c = reinterpret_cast<char*>(&ii);
            for(int i = 0; i < 4; ++i)
                v[w*(isize+i)] = c[i];
            if(w==2)
                printf("%d:32\n", ii);
            return pair<char,int32_t>(16, 4);
        }
    }
    printf("the json string is not well formed\n");
    return pair<size_t,int>(0, -1);
}


string BsonUtil::Bson2string(const bson* b, bool oneline, bool compact)
{
    string def = ":";
    string sep = "";
    string coma = ",";
    if(!compact)
    {
        def = " : ";
        sep = " ";
        coma = ",";
    }
    string add = "";
    
    if(!oneline)
    {
        coma = ",";
        sep="\n";
        add="    ";
    }
    
    string ret = "{"+sep;
    MongoObj mo;
    MongoQuery mq(&mo);
    mq.QueryBson(b);
    list<string> mod;
    list<string> symbols;
    mod.push_back(add);
    symbols.push_back("}");
    
    while(mq.GetType() != 0 || mq.depth > 0)
    {
        //printf("type %d -> %s : %s\n", mq.GetType(), mq.GetKey().c_str(), mq.GetStrValue().c_str());
        //getchar();
        switch(mq.GetType())
        {
            case 0:
                mod.pop_back();
                ret+=mod.back()+symbols.back();
                symbols.pop_back();
                mq.GetOut();
                mq.Next();
                if(mq.GetType() != 0)
                    ret+=coma;
                ret+=sep;
                break;
            case 1:
                if(symbols.back() == "}")
                    ret+=mod.back()+mq.GetKey()+def+mq.GetStrValue();
                else
                    ret+=mod.back()+mq.GetStrValue();
                mq.Next();
                if(mq.GetType() != 0)
                    ret+=coma;
                ret+=sep;
                break;
            case 2:
                if(symbols.back() == "}")
                    ret+=mod.back()+mq.GetKey()+def+string("\"")+mq.GetStrValue()+string("\"");
                else
                    ret+=mod.back()+string("\"")+mq.GetStrValue()+string("\"");
                mq.Next();
                if(mq.GetType() != 0)
                    ret+=coma;
                ret+=sep;
                break;
            case 3:
                if(symbols.back() == "}")
                    ret+=mod.back()+mq.GetKey()+def+string("{")+sep;
                else
                    ret+=mod.back()+string("{")+sep;
                mod.push_back(mod.back()+string(add));
                symbols.push_back("}");
                mq.GetIn();
                break;
            case 4:
                if(symbols.back() == "}")
                    ret+=mod.back()+mq.GetKey()+def+string("[")+sep;
                else
                    ret+=mod.back()+string("[")+sep;
                mod.push_back(mod.back()+string(add));
                symbols.push_back("]");
                mq.GetIn();
                break;
            case 7:
                if(symbols.back() == "}")
                    ret+=mod.back()+mq.GetKey()+def+mq.GetStrValue();
                else
                    ret+=mod.back()+mq.GetStrValue();
                mq.Next();
                if(mq.GetType() != 0)
                    ret+=coma;
                ret+=sep;
                break;
            case 16:
                if(symbols.back() == "}")
                    ret+=mod.back()+mq.GetKey()+def+mq.GetStrValue();
                else
                    ret+=mod.back()+mq.GetStrValue();
                mq.Next();
                if(mq.GetType() != 0)
                    ret+=coma;
                ret+=sep;
                break;
            case 18:
                if(symbols.back() == "}")
                    ret+=mod.back()+mq.GetKey()+def+mq.GetStrValue();
                else
                    ret+=mod.back()+mq.GetStrValue();
                mq.Next();
                if(mq.GetType() != 0)
                    ret+=coma;
                ret+=sep;
                break;
            default:
                break;
        }
    }
    ret+=string("}");

    return ret;
}

int MongoQuery::GetCurrentBson(bson* b)
{
    bson_init_finished_data( b, (char*)((char*)(currpos->data)-4), false );
    return 0;
}

bson BsonUtil::String2bson(string s)
{
    size_t spos = 0;
    int32_t size;
    char tmp[33];
    bson b;
    int isize = 0;
    Bson2stringRecursive(s, spos,  reinterpret_cast<char*>(&size), isize, 0, tmp);
    char* bd = new char[size];
    memset(bd, 0, sizeof(char)*size);
    spos=0;
    Bson2stringRecursive(s, spos, bd, isize, 1, tmp);
    bson_init_finished_data( &b, bd, true );
    
    return b;
}

bson BsonUtil::File2bson(string filename)
{
    ifstream infile(filename.c_str()) ;
    if (!infile)
    {
        printf("file not found!\n");
        return bson();
    }
    string str( (std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>() );
    infile.close();

    return String2bson(str);
}

int MongoQuery::FindKey(string key)
{
    if(currpos == NULL)
        return 0;
    while(GetKey() != key && currpos->type != 0)
        Next();
    
    return 0;
}
int MongoQuery::GetIn()
{
    if(currpos != NULL)
    {
        ++depth;
        if(depth+1 > int(browse.size()))
            browse.resize(depth+1, NULL);
        SetPosInfo(currpos->obj, currpos);
    }
    return 0;
}
int MongoQuery::GetOut()
{
    currpos = currpos->parent;
    --depth;
    return 0;
}

int MongoQuery::SetPosInfo(void* c, PosInfo* parent)
{
    PosInfo* pi = new PosInfo();
    char* _c = (char*)(c);
    pi->type = int(_c[0]);
    pi->parent = parent;
    pi->data = c;
    
    //printf("type=%d\n", pi->type);
    
    if(pi->type!=0)
    {
        pi->key = &(_c[1]);
        
        int len = strlen(&(_c[1]));
        //printf("klen=%d\n", len);
        switch(pi->type)
        {
            case 1:
                pi->next = &(_c[1+len+1+8]);
                pi->obj = &(_c[1+len+1]);
                break;
            case 2:
                //printf("len=%d\n", getint(&(_c[1+len+1])) );
                pi->next = &(_c[1+len+1+4+getint(&(_c[1+len+1]))]);
                pi->obj = &(_c[1+len+1+4]);
                break;
            case 7:
                pi->next = &(_c[1+len+1+12]);
                pi->obj = &(_c[1+len+1]);
                break;
            case 16:
                pi->next = &(_c[1+len+1+4]);
                pi->obj = &(_c[1+len+1]);
                break;
            case 18:
                pi->next = &(_c[1+len+1+8]);
                pi->obj = &(_c[1+len+1]);
                break;
            default:
                pi->next = &(_c[1+len+1+getint(&(_c[1+len+1]))]);
                pi->obj = &(_c[1+len+1+4]);
                break;
        }
    }

    if(browse[depth] != NULL)
        delete browse[depth];
    
    browse[depth] = pi;
    currpos = browse[depth];
    
    return 0;
}
         
int MongoQuery::NextItem()
{
    if( mongo_cursor_next( &cursor ) != MONGO_OK )
        return 1;
    ++currind;
    
    void* v = reinterpret_cast<void*>(mongo_cursor_bson( &cursor )->data);
    
    SetPosInfo(&(((char*)v)[4]), NULL);
    
    return 0;
}

MongoQuery MongoQuery::GetCurrentQuery()
{
    MongoQuery ret;
    ret.browse.resize(1, NULL);
    ret.SetPosInfo(currpos->data, NULL);
    
    return ret;
}

int MongoQuery::Next()
{
    SetPosInfo(currpos->next, currpos->parent);
    if(currpos->type == 0)
        return 1;
    
    return 0;
}

string MongoQuery::GetKey()
{
    if(currpos == NULL)
        return string("<MONGO::NULL>");
    else if(currpos->type == 0)
        return string("<MONGO::EOO>");
    else
        return string( ((char*)(currpos->key)) );
}

int MongoQuery::GetType()
{
    if(currpos == NULL)
        return -1;
    else
        return currpos->type;
}
           
string MongoQuery::GetStrValue()
{
    string ret = "";
    if(currpos == NULL)
        return string("<MONGO::NULL>");
        
    switch ( currpos->type ) {
        case 0:
            return string("<MONGO::EOO>");
            break;
        case 1:
            if(int(*(static_cast<double*>(currpos->obj))) == double(*(static_cast<double*>(currpos->obj))) )
                snprintf(pmo->tmp, 4096, "%.0f", *(static_cast<double*>(currpos->obj)));
            else
                snprintf(pmo->tmp, 4096, "%f", *(static_cast<double*>(currpos->obj)));
            return string(pmo->tmp);
            break;
        case 2:
            ret = string(static_cast<char*>(currpos->obj));
            return ret;
            break;
        case 3:
            return string("<MONGO::OBJECT>");
            break;
        case 4:
            return string("<MONGO::ARRAY>");
            break;
        case 7:
            /*for(int i = 0; i < 12; i++)
            {
                snprintf(pmo->tmp, 4096, "%02x", *(static_cast<unsigned char*>((unsigned char*)(currpos->obj)+i)));
                ret+=string(pmo->tmp);
            }*/
            for(int i = 0; i < 3; i++)
            {
                snprintf(pmo->tmp, 4096, "%08x", reverseint(*(static_cast<int*>((int*)(currpos->obj)+i))));
                ret+=string(pmo->tmp);
            }
            return "ObjectId(\""+ret+"\")";
            break;
        case 16:
            snprintf(pmo->tmp, 4096, "%d", *(static_cast<int32_t*>(currpos->obj)));
            return string(pmo->tmp);
            break;
        case 18:
            snprintf(pmo->tmp, 4096, "%ld", *(static_cast<int64_t*>(currpos->obj)));
            return string(pmo->tmp);
            break;
        default:
            return string("<MONGO::UNKNOWN>");
            break;
    }
    return 0;
}
           
int MongoQuery::GetIntValue()
{
    if(currpos == NULL)
        return numeric_limits<int>::quiet_NaN();
    
    switch ( currpos->type ) {
        case 1:
            return int(*(static_cast<double*>(currpos->obj)));
            break;
        case 2:
            return atol(static_cast<char*>(currpos->obj));
            break;
        case 16:
            return *(static_cast<int*>(currpos->obj));
            break;
        case 18:
            return int(*(static_cast<long*>(currpos->obj)));
            break;
        default:
            return numeric_limits<int>::quiet_NaN();
            break;
    }
                     
    return 0;
}
           
double MongoQuery::GetDoubleValue()
{
    if(currpos == NULL)
        return numeric_limits<int>::quiet_NaN();
    
    switch ( currpos->type ) {
        case 1:
            return *(static_cast<double*>(currpos->obj));
            break;
        case 2:
            return atol(static_cast<char*>(currpos->obj));
            break;
        case 16:
            return double(*(static_cast<int*>(currpos->obj)));
            break;
        case 18:
            return double(*(static_cast<long*>(currpos->obj)));
            break;
        default:
            return numeric_limits<double>::quiet_NaN();
            break;
    }
    
    return 0;
}
           
int MongoQuery::Query(string db, string collection, bson* query)
{
    mongo_cursor_init( &cursor, &(pmo->conn), (db+"."+collection).c_str() );
    mongo_cursor_set_query( &cursor, query );
    browse.resize(1, NULL);
    
    return 0;
}
            
int MongoQuery::QueryBson(const bson* data)
{
    void* v = data->data;
    browse.resize(1, NULL);
    SetPosInfo(&(((char*)v)[4]), NULL);
    return 0;
}

int MongoObj::Connect(string server, int port)
{
    int status = mongo_client( &conn, server.c_str(), port );
    
    if( status != MONGO_OK )
    {
        switch ( conn.err )
        {
            case MONGO_CONN_NO_SOCKET:  printf( "no socket\n" ); return 1;
            case MONGO_CONN_FAIL:       printf( "connection failed\n" ); return 1;
            case MONGO_CONN_NOT_MASTER: printf( "not master\n" ); return 1;
            default: printf( "unhandled error\n" ); return 1;
        }
    }
    else
        printf("Connected to server = %s\n", server.c_str());
    return 0;
}

int MongoObj::Auth(string db, string user, string pwd)
{
    if( mongo_cmd_authenticate( &conn, db.c_str(), user.c_str(), pwd.c_str() ) != MONGO_OK )
    {
        printf( "athentification failed\n" ); return 1;
    }
    return 0;
}

int MongoObj::ConnectAndAuth(string server, int port, string db, string user, string pwd)
{
    int status = Connect(server, port);
    if(status)
        return status;
    return Auth(db, user, pwd);
}

int MongoTrigger::Loop()
{
    mongo_cursor cursor[1];
    
    mongo_cursor_init( cursor, &(pmo->conn), (db+"."+coll).c_str() );
    bson b = BsonUtil::String2bson(filter);
    mongo_cursor_set_query( cursor, &b );
    mongo_cursor_set_options(cursor, MONGO_TAILABLE);
    
    while( true )
    {
        if(mutex)
            break;
        if(mongo_cursor_next( cursor ) == MONGO_OK )
            cb( cursor, obj );
        else
            nanosleep(&ts, NULL);
    }

    mongo_cursor_destroy( cursor );
    delete[] b.data;
    mutex = false;
    return 0;
}

int MongoTrigger::Start(MongoObj* _pmo, string _db, string _coll, string _filter, cbmongotype _cb, void* _obj, int delay_sec, int delay_nano)
{
    pmo = _pmo;
    db = _db;
    coll = _coll;
    filter = _filter;
    cb = _cb;
    obj = _obj;
    mutex = false;
    ts.tv_sec = delay_sec;
    ts.tv_nsec = delay_nano;
    
#if defined WIN64 || defined WIN32
    HANDLE  hThreadArray[1];
    hThreadArray[0] = CreateThread(NULL, 0, callback, info, 0, NULL);
    //WaitForMultipleObjects(Proc, hThreadArray, TRUE, INFINITE);
#else
    pthread_t tsubs[1];
    pthread_create( &tsubs[0], NULL, callback, this);
    //pthread_join(tsubs[0], NULL);
#endif
    
    return 0;
}

int MongoTrigger::Stop()
{
    timespec tts = {0,10000000};
    mutex = true;
    while(mutex)
        nanosleep(&tts, NULL);
    return 0;
}

MongoQuery::~MongoQuery()
{
    for(int i = 0; i < int(browse.size()); i++)
        delete browse[i];
    if(cursor.conn != NULL)
        mongo_cursor_destroy( &cursor );
    return;
}

MongoObj::~MongoObj()
{
    if(conn.sock != 0)
        mongo_destroy( &conn );
    return;
}
