
#include <jni.h>

#ifndef _WINDOWSCLASSICJNI_H_
#define _WINDOWSCLASSICJNI_H_

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL Java_WindowsClassic_MessageBox(JNIEnv *, jobject);

#ifdef __cplusplus
}
#endif

#endif // _WINDOWSCLASSICJNI_H_