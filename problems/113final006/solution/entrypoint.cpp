#define CATCH_CONFIG_RUNNER
#include <core.h>

int main(int argc, char **argv) {
    Catch::Session session;
    session.configData().reporterName = "tap";
    const char* custom_argv[] = { "test_exe", "-s" }; 
    int custom_argc = sizeof(custom_argv) / sizeof(char*);
    int returnCode = session.applyCommandLine(custom_argc, custom_argv);
    if (returnCode != 0) return returnCode;
    return session.run();
}