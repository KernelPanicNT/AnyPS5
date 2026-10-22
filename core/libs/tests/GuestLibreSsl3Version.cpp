#include "prx/libc/include/general/VabiMacros.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <source_location>

extern "C" {
const char* APS5_VABI OpenSSL_version_nid_postfix(int type);
}

static void Require(bool condition, std::source_location location = std::source_location::current()) {
    if (condition) return;
    std::fprintf(stderr, "LibreSSL 3 version check failed at line %u\n", location.line());
    std::abort();
}

static bool Equals(const char* actual, const char* expected) {
    return actual != nullptr && std::strcmp(actual, expected) == 0;
}

int main() {
    Require(Equals(OpenSSL_version_nid_postfix(0), "LibreSSL 4.0.0"));
    Require(Equals(OpenSSL_version_nid_postfix(1), "compiler: information not available"));
    Require(Equals(OpenSSL_version_nid_postfix(2), "built on: date not available"));
    Require(Equals(OpenSSL_version_nid_postfix(3), "platform: information not available"));
    Require(Equals(OpenSSL_version_nid_postfix(4), "OPENSSLDIR: \"/etc/ssl\""));
    Require(Equals(OpenSSL_version_nid_postfix(5), "ENGINESDIR: N/A"));
    Require(Equals(OpenSSL_version_nid_postfix(6), "not available"));
    Require(Equals(OpenSSL_version_nid_postfix(-1), "not available"));
    Require(OpenSSL_version_nid_postfix(0) == OpenSSL_version_nid_postfix(0));
    return 0;
}
