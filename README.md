# FalsoNDK (alpha)
FalsoNDK is a library that implements a certain part of Android NDK and enables running Android apps that rely on it (android_main / ANativeActivity-based apps).

Now it is heavily tied to VitaSDK and implements eventfd, pipe, epoll that VitaSDK lacks. In future these things will be abstracted and the library will become possible to use on other platforms as well.

Documentation and usage tutorial is in progress.

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


Define `FNDK_SAFER_SLOWER` to do proper checks for pipefd / eventfd in fndk_read / fndk_write if you're experiencing issues. Otherwise a fast check is used.


