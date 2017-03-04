#include <jni.h>
#include <string>
#include <map>

std::map<std::string , std::string> mapVals;

static const std::string available_chars =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";


static inline bool is_decodable(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}

std::string decode(std::string encoded_string) {
    int in_len = encoded_string.size();
    int i = 0;
    int j = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::string ret;

    while (in_len-- && (encoded_string[in_] != '=') && is_decodable(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_]; in_++;
        if (i ==4) {
            for (i = 0; i <4; i++)
                char_array_4[i] = available_chars.find(char_array_4[i]);

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
                ret += char_array_3[i];
            i = 0;
        }
    }

    if (i) {
        for (j = i; j <4; j++)
            char_array_4[j] = 0;

        for (j = 0; j <4; j++)
            char_array_4[j] = available_chars.find(char_array_4[j]);

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
    }

    return ret;
}

extern "C" {
    JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved);
    JNIEXPORT jstring JNICALL Java_com_u_securekeys_SecureKeys_nativeGetString(JNIEnv *env, jclass instance, jstring key);
    JNIEXPORT void JNICALL Java_com_u_securekeys_SecureKeys_nativeInit(JNIEnv *env, jclass instance, jobjectArray array);
};


JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    return JNI_VERSION_1_6;
}

JNIEXPORT void JNICALL Java_com_u_securekeys_SecureKeys_nativeInit
        (JNIEnv *env, jclass instance, jobjectArray array) {
    int stringCount = env->GetArrayLength(array);

    for (int i = 0; i < stringCount; i++) {
        jstring string = (jstring) (env->GetObjectArrayElement(array, i));
        const jchar *rawChar = env->GetStringChars(string, 0);
        int size = env->GetStringLength(string);

        std::string keyval;

        for (int j = 0 ; j < size ; j++) {
            keyval += *(rawChar);
            ++rawChar;
        }

        unsigned long separator = keyval.find(";;;;");
        if (separator != std::string::npos) {
            mapVals[keyval.substr(0, separator)] = keyval.substr(separator + 4);
        }
    }
}

JNIEXPORT jstring JNICALL Java_com_u_securekeys_SecureKeys_nativeGetString
        (JNIEnv *env, jclass instance, jstring key) {
    const char *rawString = env->GetStringUTFChars(key, 0);
    std::string paramKey(rawString);
    for(std::pair<std::string, std::string> const &pair : mapVals) {
        if (paramKey.compare(decode(pair.first)) == 0) {
            env->ReleaseStringUTFChars(key, rawString);
            return (env)->NewStringUTF(decode(pair.second).c_str());
        }
    }

    env->ReleaseStringUTFChars(key, rawString);

    return (env)->NewStringUTF("");
}