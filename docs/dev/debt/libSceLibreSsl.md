# libSceLibreSsl technical debt

### Unknown function info

- [libSceLibreSsl pending exports](../../../core/libs/prx/libSceLibreSsl/Export.cpp) - 398 named callable exports are listed in the [PS5 3.20 export table](https://github.com/DNNDHH/PS5-3.20_Libs/blob/b2a77c09cd71c01f34c790cfbe98c751a13262ad/libSceLibreSsl.c) and were cross-checked against the same module in PS5 firmware 13.60. Their signatures and behaviour are unverified; the declarations use the no-parameter unknown-signature convention and throw through `NotImplemented_nid_no_patch`.

### Functional

- [`ASN1_INTEGER_get`](../../../core/libs/prx/libSceLibreSsl/Export.cpp) results where LibreSSL 2.5.4's C is undefined are not measured: 8 content bytes with the high bit set overflow the signed shift, and a negative integer of magnitude 2^63 negates `LONG_MIN`. The export computes in 64-bit unsigned arithmetic and negates modulo 2^64, so these return the two's complement bit pattern (`80 00 00 00 00 00 00 00` gives `INT64_MIN` for either sign). Closes when: measured on a PS5.
