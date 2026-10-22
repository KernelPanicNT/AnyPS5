# libSceLibreSsl3 technical debt

### Unknown function info

- [libSceLibreSsl3 pending exports](../../../core/libs/prx/libSceLibreSsl3/Export.cpp) - 86 named callable exports are listed in the [PS5 3.20 export table](https://github.com/DNNDHH/PS5-3.20_Libs/blob/b2a77c09cd71c01f34c790cfbe98c751a13262ad/libSceLibreSsl3.c) and were cross-checked against the same module in PS5 firmware 13.60. Their signatures and behaviour are unverified; the declarations use the no-parameter unknown-signature convention and throw through `NotImplemented_nid_no_patch`.

### Functional

- [ASN1_STRING_get0_data](../../../core/libs/prx/libSceLibreSsl3/Export.cpp) (libSceLibreSsl3) - whether the console build checks for a null `ASN1_STRING` is not measured; a null argument is dereferenced and faults, as LibreSSL 4.0.0 [a_string.c](https://github.com/libressl/openbsd/blob/ce488b78e0bd994780569df718596b9776ff681a/src/lib/libcrypto/asn1/a_string.c#L241-L245) does. Closes when: a null call is measured on a PS5
