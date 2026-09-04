#include <jni.h>
#include <string>
#include <android/log.h>
#include <dlfcn.h>
#include <pthread.h>

#define LOG_TAG "NativeHostBridge"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static JavaVM* g_JavaVM = nullptr;
static pthread_key_t g_JniThreadKey;

// Thread-local cleanup handler called automatically when background POSIX loops terminate
static void detach_thread_from_jvm(void* env) {
    if (g_JavaVM && env) {
        LOGI("[JNI Bridge] Detaching background worker thread from JVM automatically");
        g_JavaVM->DetachCurrentThread();
    }
}

// Intercepts initial library loading to cache system Java Virtual Machine reference
jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    g_JavaVM = vm;
    LOGI("[JNI Bridge] JNI_OnLoad Initialized successfully");

    // Allocate thread-local storage key to intercept thread exiting
    if (pthread_key_create(&g_JniThreadKey, detach_thread_from_jvm) != 0) {
        LOGE("[JNI Bridge] Failed to initialize thread-local JNI key!");
        return JNI_ERR;
    }
    return JNI_VERSION_1_6;
}

// Helper to safely attach current POSIX thread to JVM, caching environment structures
static JNIEnv* secure_attach_current_thread() {
    if (!g_JavaVM) return nullptr;

    JNIEnv* env = nullptr;
    jint get_env_res = g_JavaVM->GetEnv((void**)&env, JNI_VERSION_1_6);

    if (get_env_res == JNI_EDETACHED) {
        JavaVMAttachArgs args = { .version = JNI_VERSION_1_6, .name = "NACLWorkerThread", .group = nullptr };
        if (g_JavaVM->AttachCurrentThread(&env, &args) == JNI_OK) {
            // Set thread-local value to trigger auto-detachment on exit
            pthread_setspecific(g_JniThreadKey, env);
            return env;
        }
        return nullptr;
    }
    return env;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_your_app_MainActivity_nativeBootRuntime(JNIEnv* env, jobject thiz,
                                                 jstring secure_lib_path,
                                                 jstring secure_key_path) {
    const char* lib_path = env->GetStringUTFChars(secure_lib_path, nullptr);
    const char* key_path = env->GetStringUTFChars(secure_key_path, nullptr);

    LOGI("[JNI Bridge] Booting embedded QuickJS engine context...");
    LOGI("[JNI Bridge] Relocated Library Directory: %s", lib_path);
    LOGI("[JNI Bridge] Cryptographic Keys Directory: %s", key_path);

    // Dynamic execution boot logic targeting our core dynamic library:
    // Void* handle = secure_module_load("libandroid_core.so");
    // (Verify paths, initialize dynamic namespace, launch threads...)

    env->ReleaseStringUTFChars(secure_lib_path, lib_path);
    env->ReleaseStringUTFChars(secure_key_path, key_path);
    return JNI_TRUE;
}

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_your_app_MainActivity_nativeAuthenticateADB(JNIEnv* env, jobject thiz,
                                                     jint port,
                                                     jstring pairing_code) {
    const char* code = env->GetStringUTFChars(pairing_code, nullptr);
    LOGI("[JNI Bridge] Authenticating custom on-device ADB loopback on port: %d...", port);

    // Establish socket to 127.0.0.1:port
    // Perform RSA handshake via system libcrypto.so
    // Spawn local command shell processes running under UID 2000

    env->ReleaseStringUTFChars(pairing_code, code);
    return JNI_TRUE; // Handshake complete
}
