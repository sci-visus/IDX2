// WARNING: idx2.hpp is auto-generated. Do not modify it manually.
// The command to generate idx2.hpp is
// quom idx2/src/idx2_lib.hpp idx2/src/idx2.hpp
// To use the library, just do the following
// #define idx2_Implementation
// #include <idx2.hpp>
//
// idx2.hpp can be included in multiple places, but the #define line should appear only once in your code
// Using MSVC (cl.exe), you need these flags: /std:c++17 /Zc:preprocessor
// Using Clang and GCC, you need -std=gnu++17

#pragma once

#include "idx2_all.h"

namespace idx2 {

error<idx2_file_err_code>
Init(idx2_file* Idx2, const params& P);

idx2::grid
GetOutputGrid(const idx2_file& Idx2, const params& P);

error<idx2_file_err_code>
Decode(idx2_file* Idx2, const params& P, buffer* OutBuf);

error<idx2_file_err_code>
Destroy(idx2_file* Idx2);

}

#ifdef idx2_Implementation

#include "idx2_all.cpp"

namespace idx2 {

error<idx2_file_err_code>
Init(idx2_file* Idx2, const params& P) {
  SetDir(Idx2, P.InDir);
  idx2_PropagateIfError(ReadMetaFile(Idx2, idx2_PrintScratch("%s", P.InputFile)));
  idx2_PropagateIfError(Finalize(Idx2));
  return idx2_Error(idx2_file_err_code::NoError);
}

idx2::grid
GetOutputGrid(const idx2_file& Idx2, const params& P) {
  u8 OutMask = P.DecodeLevel == P.OutputLevel ? P.DecodeMask : 128; // TODO: check this
  return GetGrid(P.DecodeExtent, P.OutputLevel, OutMask, Idx2.Subbands);
}

error<idx2_file_err_code>
Decode(idx2_file* Idx2, const params& P, buffer* OutBuf) {
  Decode(*Idx2, P, OutBuf);
  return idx2_Error(idx2_file_err_code::NoError);
}

error<idx2_file_err_code>
Destroy(idx2_file* Idx2) {
  Dealloc(Idx2);
  return idx2_Error(idx2_file_err_code::NoError);
}

} // end namespace idx2

#endif // idx2_Implementation
