//
// Created by corgi on 2/15/20.
//

#include <string>
#include <jni.h>
using namespace std;
extern "C" {
JNIEXPORT jstring JNICALL Java_com_ctf_toast_MainActivity_keyStringFromJNI( JNIEnv* env, jobject thiz )
{
    int a[] = {52,57,99,57,98,55,49,49,56,101,54,49};
    string b = "";
    for(int i=0; i<sizeof(a)/sizeof(*a); i++){
        b = b + char(a[i]);
    }
    return env->NewStringUTF(b.c_str());
}
JNIEXPORT jstring JNICALL Java_com_ctf_toast_MainActivity_encryptedStringFromJNI( JNIEnv* env, jobject thiz )
{
    return env->NewStringUTF("MwDxTPEvfSLms0PVdgxjYwgpgN8Y8Xj3Hrkw9pFkV6o=");

}
}