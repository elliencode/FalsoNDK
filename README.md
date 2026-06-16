# FalsoNDK (alpha)
FalsoNDK is a library that implements a certain part of Android NDK and enables running Android apps that rely on it (android_main / ANativeActivity-based apps).

This implements eventfd, pipe, epoll that VitaSDK lacks in a very basic way that is compliant enough to serve client Android apps but not good enough for proper usage of those features in other, heavier applications, at least because it will be SLOW. 

Game ports using FalsoNDK:
- https://github.com/Rinnegatamante/soulcalibur_vita
- https://github.com/Nevak/bgda-vita
- https://github.com/v-atamanenko/gof2-vita
- https://github.com/elliencode/actionsquad-psv-internal
- And others.

### Usage

```c

#include <FalsoJNI/FalsoJNI.h>
#include <FalsoNDK/FalsoNDK.h>

void main() {
	soloader_init_all();

    int (* JNI_OnLoad)(void *jvm) = (void *)so_symbol(&so_mod, "JNI_OnLoad");
    JNI_OnLoad(&jvm);

    gl_init();

	ANativeActivity * activity = (ANativeActivity *) malloc(sizeof(ANativeActivity));
	activity->callbacks = (ANativeActivityCallbacks *) malloc(sizeof(ANativeActivityCallbacks));
	activity->env = &jni; // From FalsoJNI
	activity->vm = &jvm; // From FalsoJNI
	activity->clazz = (jclass) 0x42424242;
	activity->internalDataPath = DATA_PATH"assets/";
	activity->externalDataPath = DATA_PATH"assets/";
	activity->sdkVersion = 14;
	activity->instance = nullptr;
	l_success("Created the NativeActivity object.");

	int (*ANativeActivity_onCreate)(ANativeActivity *activity, void *savedState, size_t savedStateSize) =
        (void *) so_symbol(&so_mod, "ANativeActivity_onCreate");
    ANativeActivity_onCreate(activity, NULL, 0);
    l_success("ANativeActivity_onCreate() passed.");

    activity->callbacks->onStart(activity);
    l_success("onStart() passed.");

    activity->callbacks->onResume(activity);
    l_success("onResume() passed.");

    AInputQueue *aInputQueue = AInputQueue_create();
    activity->callbacks->onInputQueueCreated(activity, aInputQueue);
    l_success("onInputQueueCreated() passed.");

    ANativeWindow *aNativeWindow = ANativeWindow_create();
    activity->callbacks->onNativeWindowCreated(activity, aNativeWindow);
    l_success("onNativeWindowCreated() passed.");

    activity->callbacks->onWindowFocusChanged(activity, 1);
    l_success("onWindowFocusChanged() passed.");

    l_info("The main thread is shutting down.");
    sceKernelExitDeleteThread(0);
}
```


## Flags

Define `FNDK_SAFER_SLOWER` to do proper checks for pipefd / eventfd in fndk_read / fndk_write if you're experiencing issues. Otherwise, a fast check is used.

There's also various `DEBUG_*` like DEBUG_EPOLL, DEBUG_EVENTFD, etc. that you can define to get verbose logs if something isn't working right.