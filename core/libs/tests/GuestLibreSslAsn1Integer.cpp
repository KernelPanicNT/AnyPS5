#include "prx/libc/include/general/VabiMacros.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <source_location>

struct Asn1String {
    std::int32_t length;
    std::int32_t type;
    unsigned char* data;
    std::int64_t flags;
};

static_assert(sizeof(Asn1String) == 24);
static_assert(offsetof(Asn1String, data) == 8);
static_assert(offsetof(Asn1String, flags) == 16);

extern "C" {
std::int64_t APS5_VABI ASN1_INTEGER_get_nid_postfix(const Asn1String* a);
}

static void Require(bool condition, std::source_location location = std::source_location::current()) {
    if (condition) return;
    std::fprintf(stderr, "LibreSSL ASN1_INTEGER_get check failed at line %u\n", location.line());
    std::abort();
}

static std::int64_t Get(std::int32_t type, unsigned char* data, std::int32_t length, std::int64_t flags = 0) {
    Asn1String integer{length, type, data, flags};
    return ASN1_INTEGER_get_nid_postfix(&integer);
}

int main() {
    constexpr std::int32_t integer = 2;
    constexpr std::int32_t negativeInteger = 0x102;
    Require(ASN1_INTEGER_get_nid_postfix(nullptr) == 0);

    unsigned char small[] = {0x01, 0x00};
    Require(Get(integer, small, 2) == 256);
    Require(Get(negativeInteger, small, 2) == -256);
    Require(Get(integer, small, 1) == 1);
    Require(Get(integer, small, 2, 0x08) == 256);

    unsigned char leadingZeros[] = {0x00, 0x00, 0x2a};
    Require(Get(integer, leadingZeros, 3) == 42);
    Require(Get(negativeInteger, leadingZeros, 3) == -42);

    Require(Get(integer, small, 0) == 0);
    Require(Get(negativeInteger, small, 0) == 0);
    Require(Get(integer, small, -1) == 0);
    Require(Get(integer, nullptr, 3) == 0);
    Require(Get(negativeInteger, nullptr, 8) == 0);

    Require(Get(4, small, 2) == -1);
    Require(Get(10, small, 2) == -1);
    Require(Get(0x10a, small, 2) == -1);
    Require(Get(0x100, small, 2) == -1);

    unsigned char nine[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
    Require(Get(integer, nine, 9) == -1);
    Require(Get(negativeInteger, nine, 9) == -1);
    Require(Get(integer, nullptr, 9) == -1);

    unsigned char max[] = {0x7f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    Require(Get(integer, max, 8) == std::numeric_limits<std::int64_t>::max());
    Require(Get(negativeInteger, max, 8) == -std::numeric_limits<std::int64_t>::max());

    unsigned char wide[] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef};
    Require(Get(integer, wide, 8) == std::int64_t{0x0123456789abcdef});
    Require(Get(negativeInteger, wide, 8) == -std::int64_t{0x0123456789abcdef});

    unsigned char highBit[] = {0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    Require(Get(integer, highBit, 8) == std::numeric_limits<std::int64_t>::min());
    Require(Get(negativeInteger, highBit, 8) == std::numeric_limits<std::int64_t>::min());

    unsigned char allOnes[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    Require(Get(integer, allOnes, 8) == -1);
    Require(Get(negativeInteger, allOnes, 8) == 1);
    return 0;
}
