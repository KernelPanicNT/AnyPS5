# libSceLibreSsl technical debt

### Unknown function info

- [libSceLibreSsl pending exports](../../../core/libs/prx/libSceLibreSsl/Export.cpp) - 396 named callable exports are listed in the [PS5 3.20 export table](https://github.com/DNNDHH/PS5-3.20_Libs/blob/b2a77c09cd71c01f34c790cfbe98c751a13262ad/libSceLibreSsl.c) and were cross-checked against the same module in PS5 firmware 13.60. Their signatures and behaviour are unverified; the declarations use the no-parameter unknown-signature convention and throw through `NotImplemented_nid_no_patch`.

### Functional

- [ASN1_STRING_length, ASN1_STRING_type and ASN1_STRING_data](../../../core/libs/prx/libSceLibreSsl/Export.cpp) dereference a null `ASN1_STRING` and fault, as LibreSSL 2.5.4 [asn1_lib.c](https://github.com/libressl/openbsd/blob/2e2c612066aa993717528a0249c15c51128d61cb/src/lib/libcrypto/asn1/asn1_lib.c#L468-L490) does; whether the console build checks for null is not measured. Closes when: a null call is measured on a PS5
