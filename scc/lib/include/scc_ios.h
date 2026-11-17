// Plain prototypes of the scc C API, for linking libscc_lib.a / libscc_lib.dylib directly
// (scc.h only declares function *types*, for the table RED4ext fills with GetProcAddress/dlsym).
#ifndef SCC_IOS_H
#define SCC_IOS_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SccSettings SccSettings;
typedef struct SccOutput SccOutput;
typedef struct SccResult SccResult;
typedef struct SccSourceRef SccSourceRef;

typedef struct SccStr {
  const char *str; // not null-terminated
  size_t len;
} SccStr;

// r6_dir: the game's r6 folder (scripts/, cache/, logs/, config/ live under it). UTF-8, null-terminated.
SccSettings *scc_settings_new(const char *r6_dir);
// Blob to compile against instead of <r6>/cache/final.redscripts.
void scc_settings_set_custom_cache_file(SccSettings *settings, const char *cache_file);
// Where to write the compiled blob instead of over the input blob. With this set the input is never modified.
void scc_settings_set_output_cache_file(SccSettings *settings, const char *cache_file);
// Extra folder (or single .reds) searched for scripts, besides <r6>/scripts.
void scc_settings_add_script_path(SccSettings *settings, const char *path);
void scc_settings_disable_error_popup(SccSettings *settings);

// Consumes (frees) the settings. Never returns NULL. Blocking (about 1 s and 155 MB peak for the CyPort mod set on an M-series Mac).
SccResult *scc_compile(SccSettings *settings);
void scc_free_result(SccResult *result);
// NULL when the compilation failed. Owned by the result.
SccOutput *scc_get_success(SccResult *result);
// Copies the null-terminated error text (truncated to fit); returns its length, 0 on success. buffer_size >= 1.
size_t scc_copy_error(SccResult *result, char *buffer, size_t buffer_size);

size_t scc_output_source_ref_count(SccOutput *output);
SccSourceRef *scc_output_get_source_ref(SccOutput *output, size_t index);
uint8_t scc_source_ref_type(SccOutput *output, SccSourceRef *ref);
bool scc_source_ref_is_native(SccOutput *output, SccSourceRef *ref);
SccStr scc_source_ref_name(SccOutput *output, SccSourceRef *ref);
SccStr scc_source_ref_parent_name(SccOutput *output, SccSourceRef *ref);
SccStr scc_source_ref_path(SccOutput *output, SccSourceRef *ref);
size_t scc_source_ref_line(SccOutput *output, SccSourceRef *ref);

#ifdef __cplusplus
}
#endif

#endif
