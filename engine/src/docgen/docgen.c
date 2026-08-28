// FILENAME: docgen.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe (desktop.c)
// NEEDS: libcore (docgen.h, hal.h, memops.h, memops.c)
// NEEDS: libcore (micro_lib_metadata.h, micro_lib_metadata.c)
// NEEDS: libcore (snprintf.h, snprintf.c, strops.h, strops.c)
// NEEDS: libengine (version.c)
// NEEDS: libkernel (version.h)
// Provides core logic and interface definitions for docgen within BASIC++.
//
// ---- Includes ----

#include "docgen/docgen.h"
#include "runtime/micro_lib_metadata.h"
#include "types/version.h"
#include "hal/hal.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/format/snprintf.h"
#include <stdarg.h>

static void docgen_puts(IoHandle f, const char *str) {
    if (f == IO_HANDLE_INVALID || !str) return;
    HalContext *hal = hal_get();
    if (!hal || !hal->io.file_write) return;
    size_t len = runtime_strlen(str);
    if (len > 0) {
        hal->io.file_write(f, str, 1, len);
    }
}

static void docgen_putc(IoHandle f, char c) {
    if (f == IO_HANDLE_INVALID) return;
    HalContext *hal = hal_get();
    if (!hal || !hal->io.file_write) return;
    hal->io.file_write(f, &c, 1, 1);
}

static void docgen_printf(IoHandle f, const char *fmt, ...) {
    if (f == IO_HANDLE_INVALID || !fmt) return;
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    int len = runtime_vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    if (len > 0) {
        docgen_puts(f, buf);
    }
}

static void fprint_json_escaped(IoHandle f, const char *str) {
    if (f == IO_HANDLE_INVALID) return;
    if (!str) {
        docgen_puts(f, "\"\"");
        return;
    }
    docgen_putc(f, '"');
    for (const char *p = str; *p != '\0'; ++p) {
        switch (*p) {
            case '"':  docgen_puts(f, "\\\""); break;
            case '\\': docgen_puts(f, "\\\\"); break;
            case '\b': docgen_puts(f, "\\b");  break;
            case '\f': docgen_puts(f, "\\f");  break;
            case '\n': docgen_puts(f, "\\n");  break;
            case '\r': docgen_puts(f, "\\r");  break;
            case '\t': docgen_puts(f, "\\t");  break;
            default:
                if ((unsigned char)*p < 0x20) {
                    docgen_printf(f, "\\u%04x", (unsigned char)*p);
                } else {
                    docgen_putc(f, *p);
                }
                break;
        }
    }
    docgen_putc(f, '"');
}

static void fprint_html_escaped(IoHandle f, const char *str) {
    if (f == IO_HANDLE_INVALID || !str) return;
    for (const char *p = str; *p != '\0'; ++p) {
        switch (*p) {
            case '&':  docgen_puts(f, "&amp;");  break;
            case '<':  docgen_puts(f, "&lt;");   break;
            case '>':  docgen_puts(f, "&gt;");   break;
            case '"':  docgen_puts(f, "&quot;"); break;
            case '\'': docgen_puts(f, "&#39;");  break;
            default:   docgen_putc(f, *p);       break;
        }
    }
}

bool docgen_export_json(const char *filepath) {
    if (!filepath) return false;
    HalContext *hal = hal_get();
    if (!hal || !hal->io.file_open || !hal->io.file_close) return false;

    IoHandle f = hal->io.file_open(filepath, "wb");
    if (f == IO_HANDLE_INVALID) return false;

    int total = microlib_count();

    docgen_printf(f, "{\n");
    docgen_printf(f, "  \"engine\": \"BASIC++\",\n");
    docgen_printf(f, "  \"version\": \"%s\",\n", BASIC_VERSION_STRING);
    docgen_printf(f, "  \"edition\": \"Standard & Lite Unified\",\n");
    docgen_printf(f, "  \"license\": \"Copyleft (c) 2026 BASIC++ Community\",\n");
    docgen_printf(f, "  \"total_keywords\": %d,\n", total);
    docgen_printf(f, "  \"keywords\": [\n");

    for (int i = 0; i < total; i++) {
        const MicroLibMetadata *meta = microlib_get(i);
        if (!meta) continue;

        docgen_printf(f, "    {\n");
        docgen_printf(f, "      \"name\": ");
        fprint_json_escaped(f, meta->name);
        docgen_printf(f, ",\n      \"category\": ");
        fprint_json_escaped(f, meta->category);
        docgen_printf(f, ",\n      \"syntax\": ");
        fprint_json_escaped(f, meta->syntax);
        docgen_printf(f, ",\n      \"help_text\": ");
        fprint_json_escaped(f, meta->help_text);
        docgen_printf(f, ",\n      \"error_codes\": ");
        fprint_json_escaped(f, meta->error_codes);
        docgen_printf(f, ",\n      \"environments\": [\"desktop\", \"server\", \"iob\", \"wap\", \"lite\", \"script\", \"embedded\", \"mobile\"]");
        docgen_printf(f, "\n    }%s\n", (i < total - 1) ? "," : "");
    }

    docgen_printf(f, "  ]\n}\n");
    hal->io.file_close(f);
    return true;
}

bool docgen_export_markdown(const char *output_dir) {
    if (!output_dir) return false;
    HalContext *hal = hal_get();
    if (!hal || !hal->io.file_open || !hal->io.file_close) return false;

    char pathbuf[512];
    runtime_snprintf(pathbuf, sizeof(pathbuf), "%s/PUBLIC_API_REFERENCE.md", output_dir);

    IoHandle f = hal->io.file_open(pathbuf, "wb");
    if (f == IO_HANDLE_INVALID) return false;

    int total = microlib_count();

    docgen_printf(f, "# BASIC++ v%s Complete API Reference Manual\n\n", BASIC_VERSION_STRING);
    docgen_printf(f, "> **Generated**: Engine Dynamic Introspection Exporter (`baspp --export-docs`)\n");
    docgen_printf(f, "> **Total Registered Micro-Libraries**: %d\n", total);
    docgen_printf(f, "> **Target Environments**: Desktop (`basicpp.dll`/`so`), Server (`baspp`), IoB (`libiob.so`), WAP (`wap.wasm`), Script (`bs`), Embedded (`basstub.c`), Mobile (`libbasicpp_mobile.so`)\n\n");
    docgen_printf(f, "---\n\n");

    for (int i = 0; i < total; i++) {
        const MicroLibMetadata *meta = microlib_get(i);
        if (!meta) continue;

        docgen_printf(f, "### `%s`\n\n", meta->name ? meta->name : "UNKNOWN");
        docgen_printf(f, "- **Category**: %s\n", meta->category ? meta->category : "General");
        docgen_printf(f, "- **Syntax**: `%s`\n", meta->syntax ? meta->syntax : meta->name);
        docgen_printf(f, "- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile\n");
        if (meta->error_codes && runtime_strlen(meta->error_codes) > 0) {
            docgen_printf(f, "- **Error Codes**: %s\n", meta->error_codes);
        }
        docgen_printf(f, "\n**Description**:\n%s\n\n", meta->help_text ? meta->help_text : "");
        docgen_printf(f, "---\n\n");
    }

    hal->io.file_close(f);
    return true;
}

bool docgen_export_html(const char *filepath) {
    if (!filepath) return false;
    HalContext *hal = hal_get();
    if (!hal || !hal->io.file_open || !hal->io.file_close) return false;

    IoHandle f = hal->io.file_open(filepath, "wb");
    if (f == IO_HANDLE_INVALID) return false;

    int total = microlib_count();

    docgen_printf(f, "<!DOCTYPE html>\n<html lang=\"en\">\n<head>\n");
    docgen_printf(f, "<meta charset=\"UTF-8\">\n");
    docgen_printf(f, "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n");
    docgen_printf(f, "<title>BASIC++ v%s API Developer Portal</title>\n", BASIC_VERSION_STRING);
    docgen_printf(f, "<style>\n");
    docgen_printf(f, "  :root { --bg: #090d16; --card-bg: rgba(22, 27, 34, 0.7); --glass-border: rgba(255, 255, 255, 0.08); --accent: #58a6ff; --green: #3fb950; --purple: #bc8cff; --orange: #d29922; --text: #e6edf3; --text-muted: #8b949e; }\n");
    docgen_printf(f, "  body { font-family: system-ui, -apple-system, sans-serif; background: var(--bg); color: var(--text); margin: 0; padding: 20px; line-height: 1.6; }\n");
    docgen_printf(f, "  header { text-align: center; margin-bottom: 30px; border-bottom: 1px solid var(--glass-border); padding-bottom: 20px; }\n");
    docgen_printf(f, "  h1 { color: var(--accent); margin-bottom: 5px; }\n");
    docgen_printf(f, "  .filter-bar { display: flex; justify-content: center; gap: 8px; margin-bottom: 20px; flex-wrap: wrap; }\n");
    docgen_printf(f, "  .btn-filter { background: #21262d; color: var(--text-muted); border: 1px solid var(--glass-border); padding: 6px 14px; border-radius: 20px; cursor: pointer; font-size: 13px; font-weight: 600; }\n");
    docgen_printf(f, "  .btn-filter.active, .btn-filter:hover { background: var(--accent); color: #000; border-color: var(--accent); }\n");
    docgen_printf(f, "  .search-box { width: 100%%; max-width: 600px; padding: 12px 18px; font-size: 16px; border-radius: 8px; border: 1px solid var(--glass-border); background: #0d1117; color: #fff; margin: 0 auto 20px auto; display: block; box-sizing: border-box; }\n");
    docgen_printf(f, "  .grid { display: grid; grid-template-columns: repeat(auto-fill, minmax(340px, 1fr)); gap: 20px; }\n");
    docgen_printf(f, "  .card { background: var(--card-bg); border: 1px solid var(--glass-border); border-radius: 10px; padding: 20px; box-shadow: 0 4px 12px rgba(0,0,0,0.3); transition: transform 0.2s, border-color 0.2s; }\n");
    docgen_printf(f, "  .card:hover { transform: translateY(-2px); border-color: var(--accent); }\n");
    docgen_printf(f, "  .card-header { display: flex; justify-content: space-between; align-items: center; margin-bottom: 12px; }\n");
    docgen_printf(f, "  .card-title { font-size: 20px; font-weight: bold; color: var(--accent); margin: 0; }\n");
    docgen_printf(f, "  .badge { background: #21262d; color: var(--green); padding: 4px 10px; border-radius: 12px; font-size: 12px; border: 1px solid var(--glass-border); }\n");
    docgen_printf(f, "  .env-tags { display: flex; gap: 4px; margin-bottom: 10px; flex-wrap: wrap; }\n");
    docgen_printf(f, "  .env-tag { font-size: 10px; padding: 2px 6px; border-radius: 4px; background: rgba(88,166,255,0.15); color: var(--accent); border: 1px solid rgba(88,166,255,0.3); font-weight: 600; }\n");
    docgen_printf(f, "  .syntax { font-family: monospace; background: #000; padding: 8px 12px; border-radius: 6px; border: 1px solid var(--glass-border); font-size: 13px; color: #e6edf3; overflow-x: auto; margin-bottom: 12px; }\n");
    docgen_printf(f, "  .desc { font-size: 14px; color: var(--text-muted); margin-bottom: 10px; }\n");
    docgen_printf(f, "  .errors { font-size: 12px; color: #f85149; font-style: italic; }\n");
    docgen_printf(f, "</style>\n</head>\n<body>\n");

    docgen_printf(f, "<header>\n");
    docgen_printf(f, "  <h1>BASIC++ v%s Complete API Reference Portal</h1>\n", BASIC_VERSION_STRING);
    docgen_printf(f, "  <p>Copyleft &copy; 2026 BASIC++ Community &bull; <strong>%d Registered Micro-Libraries</strong></p>\n", total);
    docgen_printf(f, "  <div class=\"filter-bar\">\n");
    docgen_printf(f, "    <button class=\"btn-filter active\" onclick=\"setEnvFilter('all')\">All Environments</button>\n");
    docgen_printf(f, "    <button class=\"btn-filter\" onclick=\"setEnvFilter('desktop')\">Desktop (basicpp)</button>\n");
    docgen_printf(f, "    <button class=\"btn-filter\" onclick=\"setEnvFilter('server')\">Cloud Server (baspp)</button>\n");
    docgen_printf(f, "    <button class=\"btn-filter\" onclick=\"setEnvFilter('iob')\">IoB (Internet of BASIC)</button>\n");
    docgen_printf(f, "    <button class=\"btn-filter\" onclick=\"setEnvFilter('wap')\">WAP (WebAssembly)</button>\n");
    docgen_printf(f, "    <button class=\"btn-filter\" onclick=\"setEnvFilter('script')\">Script (bs)</button>\n");
    docgen_printf(f, "  </div>\n");
    docgen_printf(f, "  <input type=\"text\" class=\"search-box\" id=\"search\" placeholder=\"Search keywords, functions, syntax, or categories...\" oninput=\"filterCards()\">\n");
    docgen_printf(f, "</header>\n");

    docgen_printf(f, "<div class=\"grid\" id=\"card-grid\">\n");

    for (int i = 0; i < total; i++) {
        const MicroLibMetadata *meta = microlib_get(i);
        if (!meta) continue;

        docgen_printf(f, "  <div class=\"card\" data-name=\"");
        fprint_html_escaped(f, meta->name ? meta->name : "");
        docgen_printf(f, "\" data-cat=\"");
        fprint_html_escaped(f, meta->category ? meta->category : "");
        docgen_printf(f, "\" data-env=\"desktop server iob wap lite script embedded mobile\">\n");

        docgen_printf(f, "    <div class=\"card-header\">\n");
        docgen_printf(f, "      <h2 class=\"card-title\">");
        fprint_html_escaped(f, meta->name ? meta->name : "");
        docgen_printf(f, "</h2>\n");
        docgen_printf(f, "      <span class=\"badge\">");
        fprint_html_escaped(f, meta->category ? meta->category : "General");
        docgen_printf(f, "</span>\n");
        docgen_printf(f, "    </div>\n");

        docgen_printf(f, "    <div class=\"env-tags\">\n");
        docgen_printf(f, "      <span class=\"env-tag\">Desktop</span>\n");
        docgen_printf(f, "      <span class=\"env-tag\">Server</span>\n");
        docgen_printf(f, "      <span class=\"env-tag\">IoB</span>\n");
        docgen_printf(f, "      <span class=\"env-tag\">WAP</span>\n");
        docgen_printf(f, "      <span class=\"env-tag\">Script</span>\n");
        docgen_printf(f, "    </div>\n");

        docgen_printf(f, "    <div class=\"syntax\"><code>");
        fprint_html_escaped(f, meta->syntax ? meta->syntax : meta->name);
        docgen_printf(f, "</code></div>\n");

        docgen_printf(f, "    <div class=\"desc\">");
        fprint_html_escaped(f, meta->help_text ? meta->help_text : "");
        docgen_printf(f, "</div>\n");

        if (meta->error_codes && runtime_strlen(meta->error_codes) > 0) {
            docgen_printf(f, "    <div class=\"errors\">Errors: ");
            fprint_html_escaped(f, meta->error_codes);
            docgen_printf(f, "</div>\n");
        }

        docgen_printf(f, "  </div>\n");
    }

    docgen_printf(f, "</div>\n");

    docgen_printf(f, "<script>\n");
    docgen_printf(f, "let currentEnv = 'all';\n");
    docgen_printf(f, "function setEnvFilter(env) {\n");
    docgen_printf(f, "  currentEnv = env;\n");
    docgen_printf(f, "  document.querySelectorAll('.btn-filter').forEach(btn => btn.classList.remove('active'));\n");
    docgen_printf(f, "  event.target.classList.add('active');\n");
    docgen_printf(f, "  filterCards();\n");
    docgen_printf(f, "}\n");
    docgen_printf(f, "function filterCards() {\n");
    docgen_printf(f, "  const query = document.getElementById('search').value.toLowerCase();\n");
    docgen_printf(f, "  const cards = document.querySelectorAll('.card');\n");
    docgen_printf(f, "  cards.forEach(card => {\n");
    docgen_printf(f, "    const envs = card.getAttribute('data-env') || '';\n");
    docgen_printf(f, "    const envMatch = (currentEnv === 'all' || envs.includes(currentEnv));\n");
    docgen_printf(f, "    const text = (card.getAttribute('data-name') + ' ' + card.getAttribute('data-cat') + ' ' + card.innerText).toLowerCase();\n");
    docgen_printf(f, "    const textMatch = text.includes(query);\n");
    docgen_printf(f, "    card.style.display = (envMatch && textMatch) ? 'block' : 'none';\n");
    docgen_printf(f, "  });\n");
    docgen_printf(f, "}\n");
    docgen_printf(f, "</script>\n");

    docgen_printf(f, "</body>\n</html>\n");

    hal->io.file_close(f);
    return true;
}

bool docgen_export_all(const char *output_dir) {
    if (!output_dir) return false;
    char json_path[512];
    char html_path[512];

    runtime_snprintf(json_path, sizeof(json_path), "%s/api_schema.json", output_dir);
    runtime_snprintf(html_path, sizeof(html_path), "%s/index.html", output_dir);

    bool res1 = docgen_export_json(json_path);
    bool res2 = docgen_export_markdown(output_dir);
    bool res3 = docgen_export_html(html_path);

    return res1 && res2 && res3;
}
