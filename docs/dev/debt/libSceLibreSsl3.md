# libSceLibreSsl3 technical debt

### Unknown function info

- [libSceLibreSsl3 pending exports](../../../core/libs/prx/libSceLibreSsl3/Export.cpp) - 86 named callable exports are listed in the [PS5 3.20 export table](https://github.com/DNNDHH/PS5-3.20_Libs/blob/b2a77c09cd71c01f34c790cfbe98c751a13262ad/libSceLibreSsl3.c) and were cross-checked against the same module in PS5 firmware 13.60. Their signatures and behaviour are unverified; the declarations use the no-parameter unknown-signature convention and throw through `NotImplemented_nid_no_patch`.

### Functional

- [OpenSSL_version](../../../core/libs/prx/libSceLibreSsl3/Export.cpp) (libSceLibreSsl3) - the answer for a type other than 0 to 5 is not measured on a console; such a type returns "not available", as [LibreSSL 4.0.0's `OpenSSL_version`](https://github.com/libressl/openbsd/blob/ce488b78e0bd994780569df718596b9776ff681a/src/lib/libcrypto/cversion.c#L90-L108) does. Types 0 to 5 return the strings measured on a PS5 (firmware 13.40) in [#8981](https://github.com/boykopovar/AnyPS5/discussions/8981#discussioncomment-19111214). Closes when: measured on a PS5
