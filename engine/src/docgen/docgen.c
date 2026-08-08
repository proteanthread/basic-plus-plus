/* =====================================================================
 * What it does: Implementation of the BASIC++ Documentation Generator (docgen).
 * Why it exists: Provides dynamic extraction of runtime MicroLib metadata and C API signatures to JSON, Markdown, and HTML formats.
 * Why it works this way: Queries microlib_get() to inspect registered statement and function metadata, pre-rendering full HTML cards and JSON schemas.
 * Dependencies & compilation target: Target C17, depends on <stdio.h>, <stdlib.h>, <string.h>, <ctype.h>, <stdbool.h>, "docgen/docgen.h", "runtime/micro_lib_metadata.h", "types/version.h".
 * Edition inclusion/exclusion: Included in baspp, bpp, and bs targets (libbasicpp and libbasicpp_lite).
 * How to modify or extend it: Add new JSON/HTML sections for specialized subsystems or language extensions.
 * What cannot be changed: JSON property formatting, C17 memory safety, bounded string operations.
 * What to expect: Fast, non-blocking file creation. Returns true on success, false on I/O failure.
 * What to do if something breaks: Check file write permissions, destination path validity, and memory allocation.
 * Assumptions & preconditions: microlib_init() has been invoked and populated before docgen functions are called.
 * Portability & C17 concerns: Strict C17 portability across Windows (MSVC) and Linux (GCC/Clang). Uses fopen_s / standard fopen.
 * Component dependencies & prerequisite source files:
 *   - engine/src/runtime/metadata.c
 *   - engine/include/docgen/docgen.h
 *   - engine/include/runtime/micro_lib_metadata.h
 *   - engine/include/types/version.h
 * ===================================================================== */

#include "docgen/docgen.h"
#include "runtime/micro_lib_metadata.h"
#include "types/version.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static void fprint_json_escaped(FILE *f, const char *str) {
    if (!f) return;
    if (!str) {
        fputs("\"\"", f);
        return;
    }
    fputc('"', f);
    for (const char *p = str; *p != '\0'; ++p) {
        switch (*p) {
            case '"':  fputs("\\\"", f); break;
            case '\\': fputs("\\\\", f); break;
            case '\b': fputs("\\b", f);  break;
            case '\f': fputs("\\f", f);  break;
            case '\n': fputs("\\n", f);  break;
            case '\r': fputs("\\r", f);  break;
            case '\t': fputs("\\t", f);  break;
            default:
                if ((unsigned char)*p < 0x20) {
                    fprintf(f, "\\u%04x", (unsigned char)*p);
                } else {
                    fputc(*p, f);
                }
                break;
        }
    }
    fputc('"', f);
}

static void fprint_html_escaped(FILE *f, const char *str) {
    if (!f || !str) return;
    for (const char *p = str; *p != '\0'; ++p) {
        switch (*p) {
            case '&':  fputs("&amp;", f);  break;
            case '<':  fputs("&lt;", f);   break;
            case '>':  fputs("&gt;", f);   break;
            case '"':  fputs("&quot;", f); break;
            case '\'': fputs("&#39;", f);  break;
            default:   fputc(*p, f);       break;
        }
    }
}

bool docgen_export_json(const char *filepath) {
    if (!filepath) return false;
    FILE *f = fopen(filepath, "w");
    if (!f) return false;

    int total = microlib_count();

    fprintf(f, "{\n");
    fprintf(f, "  \"engine\": \"BASIC++\",\n");
    fprintf(f, "  \"version\": \"%s\",\n", BASIC_VERSION_STRING);
    fprintf(f, "  \"edition\": \"Standard & Lite Unified\",\n");
    fprintf(f, "  \"license\": \"Copyleft (c) 2026 BASIC++ Community\",\n");
    fprintf(f, "  \"total_keywords\": %d,\n", total);
    fprintf(f, "  \"keywords\": [\n");

    for (int i = 0; i < total; i++) {
        const MicroLibMetadata *meta = microlib_get(i);
        if (!meta) continue;

        fprintf(f, "    {\n");
        fprintf(f, "      \"name\": ");
        fprint_json_escaped(f, meta->name);
        fprintf(f, ",\n      \"category\": ");
        fprint_json_escaped(f, meta->category);
        fprintf(f, ",\n      \"syntax\": ");
        fprint_json_escaped(f, meta->syntax);
        fprintf(f, ",\n      \"help_text\": ");
        fprint_json_escaped(f, meta->help_text);
        fprintf(f, ",\n      \"error_codes\": ");
        fprint_json_escaped(f, meta->error_codes);
        fprintf(f, ",\n      \"environments\": [\"desktop\", \"server\", \"iob\", \"wap\", \"lite\", \"script\", \"embedded\", \"mobile\"]");
        fprintf(f, "\n    }%s\n", (i < total - 1) ? "," : "");
    }

    fprintf(f, "  ]\n}\n");
    fclose(f);
    return true;
}

bool docgen_export_markdown(const char *output_dir) {
    if (!output_dir) return false;
    char pathbuf[512];
    snprintf(pathbuf, sizeof(pathbuf), "%s/PUBLIC_API_REFERENCE.md", output_dir);

    FILE *f = fopen(pathbuf, "w");
    if (!f) return false;

    int total = microlib_count();

    fprintf(f, "# BASIC++ v%s Complete API Reference Manual\n\n", BASIC_VERSION_STRING);
    fprintf(f, "> **Generated**: Engine Dynamic Introspection Exporter (`baspp --export-docs`)\n");
    fprintf(f, "> **Total Registered Micro-Libraries**: %d\n", total);
    fprintf(f, "> **Target Environments**: Desktop (`basicpp.dll`/`so`), Server (`baspp`), IoB (`libiob.so`), WAP (`wap.wasm`), Script (`bs`), Embedded (`basstub.c`), Mobile (`libbasicpp_mobile.so`)\n\n");
    fprintf(f, "---\n\n");

    for (int i = 0; i < total; i++) {
        const MicroLibMetadata *meta = microlib_get(i);
        if (!meta) continue;

        fprintf(f, "### `%s`\n\n", meta->name ? meta->name : "UNKNOWN");
        fprintf(f, "- **Category**: %s\n", meta->category ? meta->category : "General");
        fprintf(f, "- **Syntax**: `%s`\n", meta->syntax ? meta->syntax : meta->name);
        fprintf(f, "- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile\n");
        if (meta->error_codes && strlen(meta->error_codes) > 0) {
            fprintf(f, "- **Error Codes**: %s\n", meta->error_codes);
        }
        fprintf(f, "\n**Description**:\n%s\n\n", meta->help_text ? meta->help_text : "");
        fprintf(f, "---\n\n");
    }

    fclose(f);
    return true;
}

bool docgen_export_html(const char *filepath) {
    if (!filepath) return false;
    FILE *f = fopen(filepath, "w");
    if (!f) return false;

    int total = microlib_count();

    fprintf(f, "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n");
    fprintf(f, "<meta charset=\"UTF-8\">\n");
    fprintf(f, "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n");
    fprintf(f, "<title>BASIC++ v%s API Developer Portal</title>\n", BASIC_VERSION_STRING);
    fprintf(f, "<style>\n");
    fprintf(f, "  :root { --bg: #090d16; --card-bg: rgba(22, 27, 34, 0.7); --glass-border: rgba(255, 255, 255, 0.08); --accent: #58a6ff; --green: #3fb950; --purple: #bc8cff; --orange: #d29922; --text: #e6edf3; --text-muted: #8b949e; }\n");
    fprintf(f, "  body { font-family: system-ui, -apple-system, sans-serif; background: var(--bg); color: var(--text); margin: 0; padding: 20px; line-height: 1.6; }\n");
    fprintf(f, "  header { text-align: center; margin-bottom: 30px; border-bottom: 1px solid var(--glass-border); padding-bottom: 20px; }\n");
    fprintf(f, "  h1 { color: var(--accent); margin-bottom: 5px; }\n");
    fprintf(f, "  .filter-bar { display: flex; justify-content: center; gap: 8px; margin-bottom: 20px; flex-wrap: wrap; }\n");
    fprintf(f, "  .btn-filter { background: #21262d; color: var(--text-muted); border: 1px solid var(--glass-border); padding: 6px 14px; border-radius: 20px; cursor: pointer; font-size: 13px; font-weight: 600; }\n");
    fprintf(f, "  .btn-filter.active, .btn-filter:hover { background: var(--accent); color: #000; border-color: var(--accent); }\n");
    fprintf(f, "  .search-box { width: 100%%; max-width: 600px; padding: 12px 18px; font-size: 16px; border-radius: 8px; border: 1px solid var(--glass-border); background: #0d1117; color: #fff; margin: 0 auto 20px auto; display: block; box-sizing: border-box; }\n");
    fprintf(f, "  .grid { display: grid; grid-template-columns: repeat(auto-fill, minmax(340px, 1fr)); gap: 20px; }\n");
    fprintf(f, "  .card { background: var(--card-bg); border: 1px solid var(--glass-border); border-radius: 10px; padding: 20px; box-shadow: 0 4px 12px rgba(0,0,0,0.3); transition: transform 0.2s, border-color 0.2s; }\n");
    fprintf(f, "  .card:hover { transform: translateY(-2px); border-color: var(--accent); }\n");
    fprintf(f, "  .card-header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 12px; }\n");
    fprintf(f, "  .card-title { font-size: 20px; font-weight: bold; color: var(--accent); margin: 0; }\n");
    fprintf(f, "  .badge { background: #21262d; color: var(--green); padding: 4px 10px; border-radius: 12px; font-size: 12px; border: 1px solid var(--glass-border); }\n");
    fprintf(f, "  .env-tags { display: flex; gap: 4px; margin-bottom: 10px; flex-wrap: wrap; }\n");
    fprintf(f, "  .env-tag { font-size: 10px; padding: 2px 6px; border-radius: 4px; background: rgba(88,166,255,0.15); color: var(--accent); border: 1px solid rgba(88,166,255,0.3); font-weight: 600; }\n");
    fprintf(f, "  .syntax { font-family: monospace; background: #000; padding: 8px 12px; border-radius: 6px; border: 1px solid var(--glass-border); font-size: 13px; color: #e6edf3; overflow-x: auto; margin-bottom: 12px; }\n");
    fprintf(f, "  .desc { font-size: 14px; color: var(--text-muted); margin-bottom: 10px; }\n");
    fprintf(f, "  .errors { font-size: 12px; color: #f85149; font-style: italic; }\n");
    fprintf(f, "</style>\n</head>\n<body>\n");

    fprintf(f, "<header>\n");
    fprintf(f, "  <h1>BASIC++ v%s Complete API Reference Portal</h1>\n", BASIC_VERSION_STRING);
    fprintf(f, "  <p>Copyleft &copy; 2026 BASIC++ Community &bull; <strong>%d Registered Micro-Libraries</strong></p>\n", total);
    fprintf(f, "  <div class=\"filter-bar\">\n");
    fprintf(f, "    <button class=\"btn-filter active\" onclick=\"setEnvFilter('all')\">All Environments</button>\n");
    fprintf(f, "    <button class=\"btn-filter\" onclick=\"setEnvFilter('desktop')\">Desktop (basicpp)</button>\n");
    fprintf(f, "    <button class=\"btn-filter\" onclick=\"setEnvFilter('server')\">Cloud Server (baspp)</button>\n");
    fprintf(f, "    <button class=\"btn-filter\" onclick=\"setEnvFilter('iob')\">IoB (Internet of BASIC)</button>\n");
    fprintf(f, "    <button class=\"btn-filter\" onclick=\"setEnvFilter('wap')\">WAP (WebAssembly)</button>\n");
    fprintf(f, "    <button class=\"btn-filter\" onclick=\"setEnvFilter('script')\">Script (bs)</button>\n");
    fprintf(f, "  </div>\n");
    fprintf(f, "  <input type=\"text\" class=\"search-box\" id=\"search\" placeholder=\"Search keywords, functions, syntax, or categories...\" oninput=\"filterCards()\">\n");
    fprintf(f, "</header>\n");

    fprintf(f, "<div class=\"grid\" id=\"card-grid\">\n");

    for (int i = 0; i < total; i++) {
        const MicroLibMetadata *meta = microlib_get(i);
        if (!meta) continue;

        fprintf(f, "  <div class=\"card\" data-name=\"");
        fprint_html_escaped(f, meta->name ? meta->name : "");
        fprintf(f, "\" data-cat=\"");
        fprint_html_escaped(f, meta->category ? meta->category : "");
        fprintf(f, "\" data-env=\"desktop server iob wap lite script embedded mobile\">\n");

        fprintf(f, "    <div class=\"card-header\">\n");
        fprintf(f, "      <h2 class=\"card-title\">");
        fprint_html_escaped(f, meta->name ? meta->name : "");
        fprintf(f, "</h2>\n");
        fprintf(f, "      <span class=\"badge\">");
        fprint_html_escaped(f, meta->category ? meta->category : "General");
        fprintf(f, "</span>\n");
        fprintf(f, "    </div>\n");

        fprintf(f, "    <div class=\"env-tags\">\n");
        fprintf(f, "      <span class=\"env-tag\">Desktop</span>\n");
        fprintf(f, "      <span class=\"env-tag\">Server</span>\n");
        fprintf(f, "      <span class=\"env-tag\">IoB</span>\n");
        fprintf(f, "      <span class=\"env-tag\">WAP</span>\n");
        fprintf(f, "      <span class=\"env-tag\">Script</span>\n");
        fprintf(f, "    </div>\n");

        fprintf(f, "    <div class=\"syntax\"><code>");
        fprint_html_escaped(f, meta->syntax ? meta->syntax : meta->name);
        fprintf(f, "</code></div>\n");

        fprintf(f, "    <div class=\"desc\">");
        fprint_html_escaped(f, meta->help_text ? meta->help_text : "");
        fprintf(f, "</div>\n");

        if (meta->error_codes && strlen(meta->error_codes) > 0) {
            fprintf(f, "    <div class=\"errors\">Errors: ");
            fprint_html_escaped(f, meta->error_codes);
            fprintf(f, "</div>\n");
        }

        fprintf(f, "  </div>\n");
    }

    fprintf(f, "</div>\n");

    fprintf(f, "<script>\n");
    fprintf(f, "let currentEnv = 'all';\n");
    fprintf(f, "function setEnvFilter(env) {\n");
    fprintf(f, "  currentEnv = env;\n");
    fprintf(f, "  document.querySelectorAll('.btn-filter').forEach(btn => btn.classList.remove('active'));\n");
    fprintf(f, "  event.target.classList.add('active');\n");
    fprintf(f, "  filterCards();\n");
    fprintf(f, "}\n");
    fprintf(f, "function filterCards() {\n");
    fprintf(f, "  const query = document.getElementById('search').value.toLowerCase();\n");
    fprintf(f, "  const cards = document.querySelectorAll('.card');\n");
    fprintf(f, "  cards.forEach(card => {\n");
    fprintf(f, "    const envs = card.getAttribute('data-env') || '';\n");
    fprintf(f, "    const envMatch = (currentEnv === 'all' || envs.includes(currentEnv));\n");
    fprintf(f, "    const text = (card.getAttribute('data-name') + ' ' + card.getAttribute('data-cat') + ' ' + card.innerText).toLowerCase();\n");
    fprintf(f, "    const textMatch = text.includes(query);\n");
    fprintf(f, "    card.style.display = (envMatch && textMatch) ? 'block' : 'none';\n");
    fprintf(f, "  });\n");
    fprintf(f, "}\n");
    fprintf(f, "</script>\n");

    fprintf(f, "</body>\n</html>\n");

    fclose(f);
    return true;
}

bool docgen_export_all(const char *output_dir) {
    if (!output_dir) return false;
    char json_path[512];
    char html_path[512];

    snprintf(json_path, sizeof(json_path), "%s/api_schema.json", output_dir);
    snprintf(html_path, sizeof(html_path), "%s/index.html", output_dir);

    bool res1 = docgen_export_json(json_path);
    bool res2 = docgen_export_markdown(output_dir);
    bool res3 = docgen_export_html(html_path);

    return res1 && res2 && res3;
}
