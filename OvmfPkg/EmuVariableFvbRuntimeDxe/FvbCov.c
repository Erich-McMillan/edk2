#include "FvbCov.h"
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <gcov.h>

/* The start and end symbols are provided by the linker script.  We use the
   array notation to avoid issues with a potential small-data area.  */

extern const struct gcov_info *const __gcov_info_start[];
extern const struct gcov_info *const __gcov_info_end[];

// #define __gcov_filename_to_gcfn(f,d,a)          __builtin___gcov_filename_to_gcfn ((f), (d), (a))
// #define __gcov_info_to_gcda(i,f,d,m,a)          __builtin___gcov_info_to_gcda ((i), (f), (d), (m), (a))

static const unsigned char a = 'a';

static inline unsigned char *
encode (unsigned char c, unsigned char buf[2])
{
  buf[0] = c % 16 + a;
  buf[1] = (c / 16) % 16 + a;
  return buf;
}

/* This function shall produce a reliable in order byte stream to transfer the
   gcov information from the target to the host system.  */

static void
dump (const void *d, unsigned n, void *arg)
{
  (void)arg;
//   const unsigned char *c = d;
  unsigned char buf[3];

  buf[2] = 0u;

  for (unsigned i = 0; i < n; ++i)
    DEBUG ((DEBUG_INFO, "%s", buf));
}

/* The filename is serialized to a gcfn data stream by the
   __gcov_filename_to_gcfn() function.  The gcfn data is used by the
   "merge-stream" subcommand of the "gcov-tool" to figure out the filename
   associated with the gcov information. */

static void
filename (const char *f, void *arg)
{
  __gcov_filename_to_gcfn (f, dump, arg);
}

/* The __gcov_info_to_gcda() function may have to allocate memory under
   certain conditions.  Simply try it out if it is needed for your application
   or not.  */

static void *
allocate (unsigned length, void *arg)
{
  (void)arg;
  return AllocateZeroPool (length);
}

/* Dump the gcov information of all translation units.  */

void
dump_gcov_info (void)
{
  const struct gcov_info *const *info = __gcov_info_start;
  const struct gcov_info *const *end = __gcov_info_end;

  /* Obfuscate variable to prevent compiler optimizations.  */
  __asm__ ("" : "+r" (info));

  while (info != end)
  {
    void *arg = NULL;
    __gcov_info_to_gcda (*info, filename, dump, allocate, arg);
    ++info;
  }
}