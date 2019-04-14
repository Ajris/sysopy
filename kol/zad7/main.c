#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>
#include <dlfcn.h>

double (*get_atan(void))(double){
    void * handle = dlopen("libm.so", RTLD_LAZY);
    if(!handle){
        exit(EXIT_FAILURE);
    }
    double (*atan)(double);
    *(void**) (&atan) = dlsym(handle, "atan");
    dlclose(handle);
    return atan;
}

int main(int argc, char *argv[]) {

    return 0;
}
