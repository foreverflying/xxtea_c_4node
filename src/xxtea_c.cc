#include <node.h>
#include <v8.h>
#include <node_buffer.h>
#include <stdlib.h>
#include <string.h>
extern "C" {
#include "xxtea1.h"
}

using namespace v8;
using namespace node;

typedef unsigned char *(*xxtea_api)(unsigned char *, xxtea_long, unsigned char *, xxtea_long *);

Handle<Value> xxtea( const Arguments& args, xxtea_api func )
{
    HandleScope scope;
    if( args.Length() != 2 || !Buffer::HasInstance( args[0] ) || !Buffer::HasInstance( args[1] ) ){
        ThrowException( Exception::TypeError( String::New( "Wrong arguments" ) ) );
        return scope.Close( Undefined() );
    }
    char* plainBuf = Buffer::Data( args[0] );
    size_t plainLen = Buffer::Length( args[0] );
    char* keyBuf = Buffer::Data( args[1] );
    size_t keyLen = Buffer::Length( args[1] );
    xxtea_long retLen = 0;
    unsigned char* ret;
    if( keyLen < 16 ){
        char key[16];
        memcpy( key, keyBuf, keyLen );
        memset( key + keyLen, 0, 16 - keyLen );
        ret = func( (unsigned char*)plainBuf, (xxtea_long)plainLen, (unsigned char*)key, &retLen );
    }else{
        ret = func( (unsigned char*)plainBuf, (xxtea_long)plainLen, (unsigned char*)keyBuf, &retLen );
    }
    Buffer* slowBuffer = Buffer::New( retLen );
    memcpy( Buffer::Data( slowBuffer ), ret, retLen );
    free( ret );
    Local<Object> globalObj = Context::GetCurrent()->Global();
    Local<Function> bufferConstructor = Local<Function>::Cast( globalObj->Get( String::New( "Buffer" ) ) );
    Handle<Value> constructorArgs[3] = { slowBuffer->handle_, Integer::New( retLen ), Integer::New( 0 ) };
    Local<Object> actualBuffer = bufferConstructor->NewInstance( 3, constructorArgs );
    return scope.Close( actualBuffer );
}

Handle<Value> encrypt( const Arguments& args )
{
    return xxtea( args, do_xxtea_encrypt );
}

Handle<Value> decrypt( const Arguments& args )
{
    return xxtea( args, do_xxtea_decrypt );
}

void init( Handle<Object> target )
{
    NODE_SET_METHOD( target, "encrypt", encrypt );
    NODE_SET_METHOD( target, "decrypt", decrypt );
}

NODE_MODULE( xxtea_c, init );