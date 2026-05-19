#define DOCTEST_CONFIG_IMPLEMENT
#include <case.h>
#include <test.h>

// clang-format off
/**
 * @brief description of options  
 * | Option Name        | Description                                                     |
 * | ------------------ | --------------------------------------------------------------- |
 * | no-intro           | Disables the intro banner shown before test execution           |
 * | no-version         | Suppresses the doctest version number in output                 |
 * | no-path-filenames  | Strips directory paths from filenames in reported test failures |
 * | no-line-numbers    | Omits line numbers from reported test failure locations         |
 * | no-breaks          | Disables breaking into the debugger on test failures            |
 * | success            | Enables printing of passed test cases                           |
 * | minimal            | Enables minimal output mode: only test summary is printed       |
 */


int main(int argc, char **argv) {
  doctest::Context ctx;
  ctx.setOption("no-intro", true);
  ctx.setOption("no-version", true);
  ctx.setOption("no-path-filenames", false);
  ctx.setOption("no-line-numbers", false);
  ctx.setOption("no-breaks", true);
  ctx.setOption("success", true);
  ctx.setOption("minimal", false);
  return ctx.run();
}