#include <node.h>
#include <v8.h>
#include <node_buffer.h>
#include <stdlib.h>
#include <string.h>
extern "C" {
#include "xxtea.h"
}

using namespace v8;
using namespace node;

Handle<Value> xxtea( const Arguments& args, bool encrypt )
{
    HandleScope scope;
    if( args.Length() != 2 || !Buffer::HasInstance( args[0] ) || !Buffer::HasInstance( args[0] ) ){
        ThrowException( Exception::TypeError( String::New( "Wrong arguments" ) ) );
        return scope.Close( Undefined() );
    }
    char* plainBuf = Buffer::Data( args[0] );
    size_t plainLen = Buffer::Length( args[0] );
    char* keyBuf = Buffer::Data( args[1] );
    size_t keyLen = Buffer::Length( args[1] );
    unsigned char* ret;
    xxtea_long retLen;
    if( encrypt ){
        ret = xxtea_encrypt( (unsigned char*)plainBuf, (xxtea_long)plainLen, (unsigned char*)keyBuf, (xxtea_long)keyLen, &retLen );
    }else{
        ret = xxtea_decrypt( (unsigned char*)plainBuf, (xxtea_long)plainLen, (unsigned char*)keyBuf, (xxtea_long)keyLen, &retLen );
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
    return xxtea( args, true );
}

Handle<Value> decrypt( const Arguments& args )
{
    return xxtea( args, false );
}

void init( Handle<Object> target )
{
    NODE_SET_METHOD( target, "encrypt", encrypt );
    NODE_SET_METHOD( target, "decrypt", decrypt );
}

NODE_MODULE( xxtea_c, init );