// Compiles a redscript scripts folder against a scripts blob through the scc C API, with the error popup disabled,
// exactly the way the CyPort shim would on the iPhone.
//
//   compile_test <r6 dir> [<input blob> [<output blob>]]
//
// <r6 dir>      holds scripts/ (the .reds), cache/ and gets logs/redscript_rCURRENT.log
// <input blob>  pristine blob to compile against (default: <r6>/cache/final.redscripts, backed up to .bk by scc)
// <output blob> where the compiled blob goes (default: over the input)
// Exit code 0 only if scc reports success, the log has "Compilation complete" and no "[ERROR", and the output
// blob is not empty.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include "../scc/lib/include/scc_ios.h"

static char *slurp(const char *path) {
  FILE *f = fopen(path, "rb");
  if (!f) return NULL;
  fseek(f, 0, SEEK_END);
  long n = ftell(f);
  fseek(f, 0, SEEK_SET);
  char *buf = malloc((size_t)n + 1);
  if (fread(buf, 1, (size_t)n, f) != (size_t)n) n = 0;
  buf[n] = 0;
  fclose(f);
  return buf;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    fprintf(stderr, "usage: %s <r6 dir> [<input blob> [<output blob>]]\n", argv[0]);
    return 2;
  }
  const char *r6 = argv[1];
  char out[4096], logp[4096];
  snprintf(out, sizeof out, "%s/cache/final.redscripts", r6);
  snprintf(logp, sizeof logp, "%s/logs/redscript_rCURRENT.log", r6);
  remove(logp); // only this run's log counts

  SccSettings *settings = scc_settings_new(r6);
  scc_settings_disable_error_popup(settings); // first thing, always
  if (argc > 2) scc_settings_set_custom_cache_file(settings, argv[2]);
  if (argc > 3) {
    scc_settings_set_output_cache_file(settings, argv[3]);
    snprintf(out, sizeof out, "%s", argv[3]);
  } else if (argc > 2) {
    snprintf(out, sizeof out, "%s", argv[2]);
  }

  SccResult *result = scc_compile(settings); // consumes the settings
  SccOutput *success = scc_get_success(result);
  int ok = success != NULL;
  if (ok) {
    printf("scc: success, %zu source refs\n", scc_output_source_ref_count(success));
  } else {
    static char err[1 << 16];
    scc_copy_error(result, err, sizeof err);
    printf("scc: FAILED\n%s\n", err);
  }
  scc_free_result(result);

  char *log = slurp(logp);
  if (!log) { printf("check: no log at %s\n", logp); ok = 0; }
  else {
    if (!strstr(log, "Compilation complete")) { printf("check: log lacks \"Compilation complete\"\n"); ok = 0; }
    if (strstr(log, "[ERROR")) { printf("check: log has [ERROR\n"); ok = 0; }
    free(log);
  }
  struct stat st;
  if (stat(out, &st) != 0 || st.st_size == 0) { printf("check: output blob %s missing or empty\n", out); ok = 0; }
  else printf("output: %s (%lld bytes)\n", out, (long long)st.st_size);

  printf(ok ? "RESULT: OK\n" : "RESULT: FAIL\n");
  return ok ? 0 : 1;
}
