#include "prx/libc/include/general/VabiMacros.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <source_location>

struct Asn1String {
    std::int32_t length;
    std::int32_t type;
    unsigned char* data;
    std::int64_t flags;
};

static_assert(sizeof(Asn1String) == 24);
static_assert(offsetof(Asn1String, data) == 8);

extern "C" {
const unsigned char* APS5_VABI ASN1_STRING_get0_data_nid_postfix(const Asn1String* astr);
}

static void Require(bool condition, std::source_location location = std::source_location::current()) {
    if (condition) return;
    std::fprintf(stderr, "LibreSSL3 ASN1_STRING check failed at line %u\n", location.line());
    std::abort();
}

int main() {
    unsigned char bytes[] = {0x30, 0x03, 0x02, 0x01, 0x05};
    Asn1String octets{static_cast<std::int32_t>(sizeof(bytes)), 4, bytes, 0};
    Require(ASN1_STRING_get0_data_nid_postfix(&octets) == bytes);
    Require(octets.length == 5 && octets.type == 4 && octets.data == bytes && octets.flags == 0);

    Asn1String bits{2, 3, bytes + 2, 0x08 | 0x07};
    Require(ASN1_STRING_get0_data_nid_postfix(&bits) == bytes + 2);

    Asn1String negative{-1, 0x102, bytes + 4, -1};
    Require(ASN1_STRING_get0_data_nid_postfix(&negative) == bytes + 4);

    Asn1String empty{0, 4, nullptr, 0};
    Require(ASN1_STRING_get0_data_nid_postfix(&empty) == nullptr);
    return 0;
}
