#include "native_jvm.hpp"

namespace native_jvm::utils {

    union __fi_conv { 
        jfloat m_jfloat; 
        jint m_jint; 
    };

    jint cfi(jfloat f) { 
        __fi_conv fi; 
        fi.m_jfloat = f;
        return fi.m_jint; 
    }

    jfloat cif(jint i) { 
        __fi_conv fi; 
        fi.m_jint = i; 
        return fi.m_jfloat; 
    }


    union __dl_conv { 
        jdouble m_jdouble; 
        jlong m_jlong; 
    };

    jlong cdl(jdouble d) { 
        __dl_conv dl; 
        dl.m_jdouble = d; 
        return dl.m_jlong;
    }

    jdouble cld(jlong l) { 
        __dl_conv dl; 
        dl.m_jlong = l; 
        return dl.m_jdouble; 
    }


    jobjectArray create_multidim_array(JNIEnv *env, jint count, jint *sizes, const char *class_name, int line) {
        if (count == 0)
            return (jobjectArray) nullptr;
        if (*sizes < 0) {
            throw_re(env, "java/lang/NegativeArraySizeException", "MULTIANEWARRAY size < 0", line);
            return (jobjectArray) nullptr;
        }
        jobjectArray result_array = nullptr;
        if (jclass clazz = env->FindClass((std::string(count, '[') + std::string(class_name)).c_str())) {
            result_array = env->NewObjectArray(*sizes, clazz, nullptr);
            env->DeleteLocalRef(clazz);
        }
        else
            return (jobjectArray) nullptr;
        for (jint i = 0; i < *sizes; i++) {
            jobjectArray inner_array = create_multidim_array(env, count - 1, sizes + 1, class_name, line);
            env->SetObjectArrayElement(result_array, i, inner_array);
            if (env->ExceptionCheck())
                return (jobjectArray) nullptr;
            env->DeleteLocalRef(inner_array);
        }
        return result_array;
    }

    jclass find_class_wo_static(JNIEnv *env, const char *class_name) {
        jclass thread_class = env->FindClass("java/lang/Thread");
        jobject current_thread = env->CallStaticObjectMethod(
            thread_class, 
            env->GetStaticMethodID(thread_class, "currentThread", "()Ljava/lang/Thread;")
        );
        jobject classloader = env->CallObjectMethod(
            current_thread,
            env->GetMethodID(thread_class, "getContextClassLoader", "()Ljava/lang/ClassLoader;")
        );
        jclass classloader_class = env->FindClass("java/lang/ClassLoader");
        jstring class_name_string = env->NewStringUTF(class_name);
        env->DeleteLocalRef(current_thread);
        jclass clazz = (jclass) env->CallObjectMethod(
            classloader,
            env->GetMethodID(classloader_class, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;"),
            class_name_string
        );
        env->DeleteLocalRef(classloader);
        env->DeleteLocalRef(classloader_class);
        env->DeleteLocalRef(class_name_string);
        return clazz;
    }

    void throw_re(JNIEnv *env, const char *exception_class, const char *error, int line) {
        jclass exception_class_ptr = env->FindClass(exception_class);
        env->ThrowNew(exception_class_ptr, ("\"" + std::string(error) + "\" on " + std::to_string(line)).c_str());
        env->DeleteLocalRef(exception_class_ptr);
    }
}