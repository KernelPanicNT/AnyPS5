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
static_assert(offsetof(Asn1String, flags) == 16);

extern "C" {
int APS5_VABI ASN1_STRING_length_nid_postfix(const Asn1String* x);
int APS5_VABI ASN1_STRING_type_nid_postfix(Asn1String* x);
unsigned char* APS5_VABI ASN1_STRING_data_nid_postfix(Asn1String* x);
}

static void Require(bool condition, std::source_location location = std::source_location::current()) {
    if (condition) return;
    std::fprintf(stderr, "LibreSSL ASN1_STRING check failed at line %u\n", location.line());
    std::abort();
}

int main() {
    unsigned char bytes[] = {0x30, 0x03, 0x02, 0x01, 0x05};
    Asn1String octets{5, 4, bytes, 0};
    Require(ASN1_STRING_length_nid_postfix(&octets) == 5);
    Require(ASN1_STRING_type_nid_postfix(&octets) == 4);
    Require(ASN1_STRING_data_nid_postfix(&octets) == bytes);

    Asn1String negative{-1, 258, nullptr, 0x08};
    Require(ASN1_STRING_length_nid_postfix(&negative) == -1);
    Require(ASN1_STRING_type_nid_postfix(&negative) == 258);
    Require(ASN1_STRING_data_nid_postfix(&negative) == nullptr);

    octets.length = 2;
    octets.type = 12;
    octets.data = bytes + 3;
    Require(ASN1_STRING_length_nid_postfix(&octets) == 2);
    Require(ASN1_STRING_type_nid_postfix(&octets) == 12);
    Require(ASN1_STRING_data_nid_postfix(&octets) == bytes + 3);
    Require(octets.flags == 0);
    return 0;
}
