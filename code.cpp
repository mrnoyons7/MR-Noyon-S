#include <jni.h>
#include <android/log.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <linux/uinput.h>
#include <string.h>

// XOR Encryption for Anti-Decompilation
char XOR_KEY = 0x5A;
std::string decrypt(const char* enc, int len) {
    std::string dec(len, 0);
    for (int i = 0; i < len; i++) dec[i] = enc[i] ^ XOR_KEY;
    return dec;
}

// Encrypted Admin Email: mrnoyonss675@gmail.com
const char ADMIN_EMAIL[] = {
    0x27^XOR_KEY,0x6D^XOR_KEY,0x72^XOR_KEY,0x6E^XOR_KEY,0x6F^XOR_KEY,0x79^XOR_KEY,0x6F^XOR_KEY,0x6E^XOR_KEY,
    0x73^XOR_KEY,0x73^XOR_KEY,0x36^XOR_KEY,0x37^XOR_KEY,0x35^XOR_KEY,0x40^XOR_KEY,0x67^XOR_KEY,0x6D^XOR_KEY,
    0x61^XOR_KEY,0x69^XOR_KEY,0x6C^XOR_KEY,0x2E^XOR_KEY,0x63^XOR_KEY,0x6F^XOR_KEY,0x6D^XOR_KEY,0x00
};

class GoliathV75 {
private:
    float screenHeight;
    bool authorized = false, active = false;
    int inputFd = -1, uinputFd = -1;
    pthread_t worker;
    
    struct PID {
        float kp=2.8f, ki=0.12f, kd=0.9f, integral=0, prev=0;
        float update(float error, float dt) {
            integral += error * dt;
            float deriv = (error - prev) / dt;
            prev = error;
            return kp * error + ki * integral + kd * deriv;
        }
    } pid;

    bool initInput() {
        for (int i = 0; i < 32; i++) {
            char path[32];
            sprintf(path, "/dev/input/event%d", i);
            inputFd = open(path, O_RDONLY | O_NONBLOCK);
            if (inputFd >= 0) return true;
        }
        return initUInput();
    }

    bool initUInput() {
        uinputFd = open("/dev/uinput", O_WRONLY);
        if (uinputFd < 0) return false;
        ioctl(uinputFd, UI_SET_EVBIT, EV_ABS);
        ioctl(uinputFd, UI_SET_ABS, ABS_Y);
        ioctl(uinputFd, UI_SET_EVBIT, EV_SYN);
        struct uinput_setup us = {};
        strcpy(us.name, "GoliathTouch");
        ioctl(uinputFd, UI_DEV_CREATE);
        return true;
    }

    void injectY(float y) {
        struct input_event ev;
        ev.type = EV_ABS;
        ev.code = ABS_Y;
        ev.value = y * 32767.0f / screenHeight;
        write(uinputFd, &ev, sizeof(ev));
        ev.type = EV_SYN; ev.code = 0; ev.value = 0;
        write(uinputFd, &ev, sizeof(ev));
    }

public:
    void setAuthSuccess(bool success) { 
        authorized = success; 
        __android_log_print(ANDROID_LOG_INFO, "GOLIATH", "AUTH: %s", success ? "SUCCESS" : "FAILED");
    }

    bool initGoliath(float height) {
        if (!authorized) {
            __android_log_print(ANDROID_LOG_ERROR, "GOLIATH", "🚫 UNAUTHORIZED - ENGINE LOCKED");
            return false;
        }
        screenHeight = height;
        if (!initInput()) return false;
        
        pthread_create(&worker, NULL, [](void* self) -> void* {
            GoliathV75* g = (GoliathV75*)self;
            pollfd pfd = {g->inputFd, POLLIN};
            while (g->active) {
                if (poll(&pfd, 1, 16) > 0) {
                    input_event events[64];
                    int n = read(g->inputFd, events, sizeof(events));
                    for (int i = 0; i < n/sizeof(input_event); i++) {
                        if (events[i].type == EV_ABS && events[i].code == ABS_Y && events[i].value > 1000) {
                            float y = events[i].value * g->screenHeight / 32767.0f;
                            float target = g->screenHeight * 0.07f; // Head level
                            float correction = g->pid.update(target - y, 0.016f);
                            g->injectY(y - correction);
                        }
                    }
                }
            }
            return NULL;
        }, this);
        return true;
    }

    void toggleGoliath(bool on) { active = on && authorized; }
};

static GoliathV75* goliath = NULL;

extern "C" JNIEXPORT jstring JNICALL
Java_com_goliath_master_v75_MainActivity_getAdminEmail(JNIEnv* env, jobject) {
    return env->NewStringUTF(decrypt(ADMIN_EMAIL, strlen((char*)ADMIN_EMAIL)).c_str());
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_goliath_master_v75_MainActivity_generateLicenseKey(JNIEnv* env, jobject, jstring hwid) {
    const char* hw = env->GetStringUTFChars(hwid, NULL);
    // Simple HWID -> Key hash (SHA256 in production)
    char key[33];
    sprintf(key, "%08X%08X", (unsigned int)hw[0] * 0xDEADBEEF, (unsigned int)hw[8] * 0xCAFEBABE);
    env->ReleaseStringUTFChars(hwid, hw);
    return env->NewStringUTF(key);
}

extern "C" JNIEXPORT void JNICALL
Java_com_goliath_master_v75_MainActivity_setAuthSuccess(JNIEnv*, jobject, jboolean success) {
    if (!goliath) goliath = new GoliathV75();
    goliath->setAuthSuccess(success);
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_goliath_master_v75_MainActivity_initGoliath(JNIEnv*, jobject, jfloat height) {
    if (!goliath) return false;
    return goliath->initGoliath(height);
}

extern "C" JNIEXPORT void JNICALL
Java_com_goliath_master_v75_MainActivity_toggleGoliath(JNIEnv*, jobject, jboolean enable) {
    if (goliath) goliath->toggleGoliath(enable);
}
