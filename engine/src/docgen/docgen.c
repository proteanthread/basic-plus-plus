// FILENAME: docgen.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe (desktop.c)
// NEEDS: libcore (docgen.h, hal.h, memops.h, memops.c)
// NEEDS: libcore (language_descriptor.h)
// NEEDS: libcore (runtime_snprintf.h, runtime_snprintf.c, strops.h, strops.c)
// NEEDS: libengine (version.c)
// NEEDS: libkernel (version.h)
// Provides core logic and interface definitions for docgen within BASIC++.
//
// ---- Includes ----

#include "docgen/docgen.h"
#include "runtime/language_descriptor.h"
#include "types/version.h"
#include "hal/hal.h"
#include "runtime/string/strops.h"
#include "runtime/string/memops.h"
#include "runtime/format/snprintf.h"
#include "platform/platform.h"
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

    int total = lang_desc_count();

    docgen_printf(f, "{\n");
    docgen_printf(f, "  \"engine\": \"BASIC++\",\n");
    docgen_printf(f, "  \"version\": \"%s\",\n", BASIC_VERSION_STRING);
    docgen_printf(f, "  \"edition\": \"Standard & Lite Unified\",\n");
    docgen_printf(f, "  \"license\": \"Copyleft (c) 2026 BASIC++ Community\",\n");
    docgen_printf(f, "  \"total_keywords\": %d,\n", total);
    docgen_printf(f, "  \"keywords\": [\n");

    for (int i = 0; i < total; i++) {
        const LanguageDescriptor *meta = lang_desc_get(i);
        if (!meta) continue;

        docgen_printf(f, "    {\n");
        docgen_printf(f, "      \"name\": ");
        fprint_json_escaped(f, meta->name);
        docgen_printf(f, ",\n      \"category\": ");
        fprint_json_escaped(f, meta->category);
        docgen_printf(f, ",\n      \"syntax\": ");
        fprint_json_escaped(f, meta->syntax);
        docgen_printf(f, ",\n      \"help_text\": ");
        fprint_json_escaped(f, meta->description);
        docgen_printf(f, ",\n      \"error_codes\": ");
        fprint_json_escaped(f, meta->error_summary);
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

    int total = lang_desc_count();

    docgen_printf(f, "# BASIC++ v%s Complete API Reference Manual\n\n", BASIC_VERSION_STRING);
    docgen_printf(f, "> **Generated**: Engine Dynamic Introspection Exporter (`baspp --export-docs`)\n");
    docgen_printf(f, "> **Total Registered Micro-Libraries**: %d\n", total);
    docgen_printf(f, "> **Target Environments**: Desktop (`basicpp.dll`/`so`), Server (`baspp`), IoB (`libiob.so`), WAP (`wap.wasm`), Script (`bs`), Embedded (`basstub.c`), Mobile (`libbasicpp_mobile.so`)\n\n");
    docgen_printf(f, "---\n\n");

    for (int i = 0; i < total; i++) {
        const LanguageDescriptor *meta = lang_desc_get(i);
        if (!meta) continue;

        docgen_printf(f, "### `%s`\n\n", meta->name ? meta->name : "UNKNOWN");
        docgen_printf(f, "- **Category**: %s\n", meta->category ? meta->category : "General");
        docgen_printf(f, "- **Syntax**: `%s`\n", meta->syntax ? meta->syntax : meta->name);
        docgen_printf(f, "- **Environments**: Desktop, Server, IoB (IoT), WAP (WASM), Script, Embedded, Mobile\n");
        if (meta->error_summary && runtime_strlen(meta->error_summary) > 0) {
            docgen_printf(f, "- **Error Codes**: %s\n", meta->error_summary);
        }
        docgen_printf(f, "\n**Description**:\n%s\n\n", meta->description ? meta->description : "");
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

    int total = lang_desc_count();

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
        const LanguageDescriptor *meta = lang_desc_get(i);
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
        fprint_html_escaped(f, meta->description ? meta->description : "");
        docgen_printf(f, "</div>\n");

        if (meta->error_summary && runtime_strlen(meta->error_summary) > 0) {
            docgen_printf(f, "    <div class=\"errors\">Errors: ");
            fprint_html_escaped(f, meta->error_summary);
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

//
// ---- Keyword & Catalog Documentation Generation ----
//

static void ensure_dir(const char *path) {
    if (!path || path[0] == '\0') return;
    char tmp[512];
    runtime_strncpy(tmp, path, sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0';
    for (char *p = tmp + 1; *p != '\0'; ++p) {
        if (*p == '/' || *p == '\\') {
            char orig = *p;
            *p = '\0';
            platform_mkdir(tmp);
            *p = orig;
        }
    }
    platform_mkdir(tmp);
}

static const char *subsystem_to_str(uint32_t sub) {
    switch (sub) {
        case SUBSYSTEM_BOOT: return "SUBSYSTEM_BOOT";
        case SUBSYSTEM_PLATFORM: return "SUBSYSTEM_PLATFORM";
        case SUBSYSTEM_KERNEL: return "SUBSYSTEM_KERNEL";
        case SUBSYSTEM_ENGINE: return "SUBSYSTEM_ENGINE";
        case SUBSYSTEM_HARDWARE: return "SUBSYSTEM_HARDWARE";
        case SUBSYSTEM_SERVER: return "SUBSYSTEM_SERVER";
        case SUBSYSTEM_SCRIPT: return "SUBSYSTEM_SCRIPT";
        case SUBSYSTEM_CORE: return "SUBSYSTEM_CORE";
        case SUBSYSTEM_FLEX: return "SUBSYSTEM_FLEX";
        case SUBSYSTEM_STANDARD: return "SUBSYSTEM_STANDARD";
        case SUBSYSTEM_ADVANCED: return "SUBSYSTEM_ADVANCED";
        case SUBSYSTEM_EXT: return "SUBSYSTEM_EXT";
        default: return "SUBSYSTEM_ENGINE";
    }
}

static const char *safety_to_str(SafetyLevel s) {
    switch (s) {
        case SAFETY_PURE: return "SAFETY_PURE";
        case SAFETY_SAFE: return "SAFETY_SAFE";
        case SAFETY_IO: return "SAFETY_IO";
        case SAFETY_SYSTEM: return "SAFETY_SYSTEM";
        case SAFETY_UNSAFE: return "SAFETY_UNSAFE";
        default: return "SAFETY_SAFE";
    }
}

static const char *type_to_str(FeatureType t) {
    switch (t) {
        case FEATURE_STATEMENT: return "FEATURE_STATEMENT";
        case FEATURE_FUNCTION: return "FEATURE_FUNCTION";
        case FEATURE_COMMAND: return "FEATURE_COMMAND";
        case FEATURE_VARIABLE: return "FEATURE_VARIABLE";
        case FEATURE_MODULE: return "FEATURE_MODULE";
        case FEATURE_OPERATOR: return "FEATURE_OPERATOR";
        case FEATURE_DEVICE: return "FEATURE_DEVICE";
        case FEATURE_WIDGET: return "FEATURE_WIDGET";
        default: return "FEATURE_STATEMENT";
    }
}

static const char *type_to_display(FeatureType t) {
    switch (t) {
        case FEATURE_STATEMENT: return "Statement";
        case FEATURE_FUNCTION: return "Function";
        case FEATURE_COMMAND: return "Command";
        case FEATURE_VARIABLE: return "Variable";
        case FEATURE_MODULE: return "Module";
        case FEATURE_OPERATOR: return "Operator";
        case FEATURE_DEVICE: return "Virtual Device";
        case FEATURE_WIDGET: return "Widget";
        default: return "Keyword";
    }
}

static const char *type_to_subdir(FeatureType t) {
    switch (t) {
        case FEATURE_STATEMENT: return "statements";
        case FEATURE_FUNCTION: return "functions";
        case FEATURE_COMMAND: return "commands";
        case FEATURE_VARIABLE: return "variables";
        case FEATURE_MODULE: return "modules";
        case FEATURE_OPERATOR: return "operators";
        case FEATURE_DEVICE: return "devices";
        case FEATURE_WIDGET: return "widgets";
        default: return "statements";
    }
}

static void format_delim_mask(uint32_t mask, char *out, size_t out_sz) {
    if (mask == DELIM_NONE) {
        runtime_snprintf(out, out_sz, "DELIM_NONE");
        return;
    }
    if ((mask & DELIM_ALL) == DELIM_ALL) {
        runtime_snprintf(out, out_sz, "DELIM_PAREN | DELIM_BRACKET | DELIM_BRACE");
        return;
    }
    char buf[128];
    buf[0] = '\0';
    if (mask & DELIM_PAREN) runtime_strcat(buf, "DELIM_PAREN");
    if (mask & DELIM_BRACKET) {
        if (buf[0] != '\0') runtime_strcat(buf, " | ");
        runtime_strcat(buf, "DELIM_BRACKET");
    }
    if (mask & DELIM_BRACE) {
        if (buf[0] != '\0') runtime_strcat(buf, " | ");
        runtime_strcat(buf, "DELIM_BRACE");
    }
    runtime_snprintf(out, out_sz, "%s", buf);
}

static void sanitize_var_name(const char *name, char *out, size_t out_sz) {
    if (!name || out_sz == 0) return;
    size_t j = 0;
    for (size_t i = 0; name[i] != '\0' && j < out_sz - 1; ++i) {
        char c = name[i];
        if (c >= 'A' && c <= 'Z') {
            out[j++] = (char)(c + ('a' - 'A'));
        } else if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_') {
            out[j++] = c;
        } else if (c == '$') {
            if (j + 4 < out_sz) {
                out[j++] = '_';
                out[j++] = 's';
                out[j++] = 't';
                out[j++] = 'r';
            }
        } else if (c == '%') {
            if (j + 4 < out_sz) {
                out[j++] = '_';
                out[j++] = 'i';
                out[j++] = 'n';
                out[j++] = 't';
            }
        } else {
            out[j++] = '_';
        }
    }
    out[j] = '\0';
}

static void docgen_print_wrapped_plaintext(IoHandle f, const char *text, int indent, int width) {
    if (!text || f == IO_HANDLE_INVALID) return;
    int col = indent;
    for (int i = 0; i < indent; i++) docgen_putc(f, ' ');
    const char *p = text;
    while (*p) {
        if (*p == '\r') {
            p++;
            continue;
        }
        if (*p == '\n') {
            docgen_puts(f, "\r\n");
            col = indent;
            for (int i = 0; i < indent; i++) docgen_putc(f, ' ');
            p++;
            continue;
        }
        if (*p == ' ' || *p == '\t') {
            if (col >= width - 1) {
                docgen_puts(f, "\r\n");
                col = indent;
                for (int i = 0; i < indent; i++) docgen_putc(f, ' ');
            } else {
                docgen_putc(f, ' ');
                col++;
            }
            p++;
            continue;
        }
        const char *word_start = p;
        while (*p && *p != ' ' && *p != '\t' && *p != '\n' && *p != '\r') p++;
        int word_len = (int)(p - word_start);
        if (col > indent && col + word_len > width) {
            docgen_puts(f, "\r\n");
            col = indent;
            for (int i = 0; i < indent; i++) docgen_putc(f, ' ');
        }
        for (int i = 0; i < word_len; i++) {
            char c = word_start[i];
            if ((unsigned char)c > 127) c = '?';
            docgen_putc(f, c);
        }
        col += word_len;
    }
    docgen_puts(f, "\r\n");
}

bool docgen_export_keywords(const char *docs_dir, const char *help_dir) {
    if (!docs_dir || !help_dir) return false;
    HalContext *hal = hal_get();
    if (!hal || !hal->io.file_open || !hal->io.file_close) return false;

    int total = lang_desc_count();
    if (total <= 0) return true;

    for (int i = 0; i < total; i++) {
        const LanguageDescriptor *meta = lang_desc_get(i);
        if (!meta || !meta->name || meta->name[0] == '\0') continue;

        const char *subdir = type_to_subdir(meta->type);
        const char *type_name = type_to_display(meta->type);
        const char *source_file = meta->source_file ? meta->source_file : "engine/include/runtime/language_descriptor.h";
        const char *compat = meta->compat ? meta->compat : "BASIC++ Standard";
        const char *since_ver = meta->since_version ? meta->since_version : "6.0.0";
        const char *category = meta->category ? meta->category : "General";

        char var_name[128];
        sanitize_var_name(meta->name, var_name, sizeof(var_name));

        char delim_buf[128];
        format_delim_mask(meta->delim_mask, delim_buf, sizeof(delim_buf));

        // Format directory paths
        char md_dir[512], txt_dir[512];
        if (runtime_strstr(docs_dir, "keywords")) {
            runtime_snprintf(md_dir, sizeof(md_dir), "%s/%s", docs_dir, subdir);
        } else {
            runtime_snprintf(md_dir, sizeof(md_dir), "%s/keywords/%s", docs_dir, subdir);
        }
        if (runtime_strstr(help_dir, "keywords")) {
            runtime_snprintf(txt_dir, sizeof(txt_dir), "%s/%s", help_dir, subdir);
        } else {
            runtime_snprintf(txt_dir, sizeof(txt_dir), "%s/keywords/%s", help_dir, subdir);
        }
        ensure_dir(md_dir);
        ensure_dir(txt_dir);

        // --- Export Markdown File (.md) ---
        char md_path[512];
        runtime_snprintf(md_path, sizeof(md_path), "%s/%s.md", md_dir, meta->name);
        IoHandle f_md = hal->io.file_open(md_path, "wb");
        if (f_md != IO_HANDLE_INVALID) {
            docgen_printf(f_md, "<!--\n");
            docgen_printf(f_md, "Title:        %s\n", meta->name);
            docgen_printf(f_md, "Tier:         3\n");
            docgen_printf(f_md, "Applies to:   BASIC++ v%s (baspp, bpp, bs, iot)\n", BASIC_VERSION_STRING);
            docgen_printf(f_md, "Authority:    %s\n", source_file);
            docgen_printf(f_md, "Generated:    yes, from LanguageDescriptor\n");
            docgen_printf(f_md, "Status:       current\n");
            docgen_printf(f_md, "-->\n\n");

            docgen_printf(f_md, "# `%s` %s\n\n", meta->name, type_name);
            docgen_printf(f_md, "## 1. Description & Usage\n\n");
            docgen_printf(f_md, "%s\n\n", meta->description ? meta->description : "No description provided.");
            docgen_printf(f_md, "### Syntax:\n```basic\n%s\n```\n\n", meta->syntax ? meta->syntax : meta->name);

            if (meta->rationale && meta->rationale[0] != '\0') {
                docgen_printf(f_md, "## 2. Why It Exists\n\n%s\n\n", meta->rationale);
            }
            if (meta->design_notes && meta->design_notes[0] != '\0') {
                docgen_printf(f_md, "## 3. Design Notes & Semantics\n\n%s\n\n", meta->design_notes);
            }
            if (meta->examples && meta->examples[0] != '\0') {
                docgen_printf(f_md, "## 4. Code Examples\n\n```basic\n%s\n```\n\n", meta->examples);
            }
            if (meta->error_summary && meta->error_summary[0] != '\0') {
                docgen_printf(f_md, "## 5. Error Conditions\n\n%s\n\n", meta->error_summary);
            }

            docgen_printf(f_md, "## 6. Compatibility & Lineage\n\n");
            docgen_printf(f_md, "- **Compatibility**: %s\n", compat);
            docgen_printf(f_md, "- **Since Version**: %s\n", since_ver);
            docgen_printf(f_md, "- **Category**: %s\n", category);
            docgen_printf(f_md, "- **Subsystem**: %s\n", subsystem_to_str(meta->subsystem));
            docgen_printf(f_md, "- **Safety Level**: %s\n\n", safety_to_str(meta->safety));

            docgen_printf(f_md, "---\n\n");
            docgen_printf(f_md, "## LanguageDescriptor (LangDesc) Quick Reference\n\n");
            docgen_printf(f_md, "```c\n");
            docgen_printf(f_md, "static const LangDesc g_%s_desc = {\n", var_name);
            docgen_printf(f_md, "    .name = \"%s\",\n", meta->name);
            docgen_printf(f_md, "    .category = \"%s\",\n", category);
            docgen_printf(f_md, "    .syntax = \"%s\",\n", meta->syntax ? meta->syntax : meta->name);
            docgen_printf(f_md, "    .description = \"%s\",\n", meta->description ? meta->description : "");
            docgen_printf(f_md, "    .error_summary = \"%s\",\n", meta->error_summary ? meta->error_summary : "None");
            docgen_printf(f_md, "    .subsystem = %s,\n", subsystem_to_str(meta->subsystem));
            docgen_printf(f_md, "    .safety = %s,\n", safety_to_str(meta->safety));
            docgen_printf(f_md, "    .type = %s,\n", type_to_str(meta->type));
            docgen_printf(f_md, "    .delim_mask = %s,\n", delim_buf);
            docgen_printf(f_md, "    .compat = \"%s\",\n", compat);
            docgen_printf(f_md, "    .since_version = \"%s\",\n", since_ver);
            docgen_printf(f_md, "    .source_file = \"%s\"\n", source_file);
            docgen_printf(f_md, "};\n");
            docgen_printf(f_md, "```\n");

            hal->io.file_close(f_md);
        }

        // --- Export Plaintext File (.TXT) ---
        char txt_path[512];
        runtime_snprintf(txt_path, sizeof(txt_path), "%s/%s.TXT", txt_dir, meta->name);
        IoHandle f_txt = hal->io.file_open(txt_path, "wb");
        if (f_txt != IO_HANDLE_INVALID) {
            docgen_printf(f_txt, "Title:        %s\r\n", meta->name);
            docgen_printf(f_txt, "Tier:         3\r\n");
            docgen_printf(f_txt, "Applies to:   BASIC++ v%s (baspp, bpp, bs, iot)\r\n", BASIC_VERSION_STRING);
            docgen_printf(f_txt, "Authority:    %s\r\n", source_file);
            docgen_printf(f_txt, "Generated:    yes, from LanguageDescriptor\r\n");
            docgen_printf(f_txt, "Status:       current\r\n\r\n");

            docgen_puts(f_txt, "==============================================================================\r\n");
            docgen_printf(f_txt, "%s %s\r\n", meta->name, type_name);
            docgen_puts(f_txt, "==============================================================================\r\n\r\n");

            docgen_puts(f_txt, "------------------------------------------------------------------------------\r\n");
            docgen_puts(f_txt, "1. DESCRIPTION & USAGE\r\n");
            docgen_puts(f_txt, "------------------------------------------------------------------------------\r\n");
            docgen_print_wrapped_plaintext(f_txt, meta->description ? meta->description : "No description provided.", 0, 78);
            docgen_puts(f_txt, "\r\nSyntax:\r\n  ");
            docgen_puts(f_txt, meta->syntax ? meta->syntax : meta->name);
            docgen_puts(f_txt, "\r\n\r\n");

            if (meta->rationale && meta->rationale[0] != '\0') {
                docgen_puts(f_txt, "------------------------------------------------------------------------------\r\n");
                docgen_puts(f_txt, "2. WHY IT EXISTS\r\n");
                docgen_puts(f_txt, "------------------------------------------------------------------------------\r\n");
                docgen_print_wrapped_plaintext(f_txt, meta->rationale, 0, 78);
                docgen_puts(f_txt, "\r\n");
            }

            if (meta->examples && meta->examples[0] != '\0') {
                docgen_puts(f_txt, "------------------------------------------------------------------------------\r\n");
                docgen_puts(f_txt, "3. CODE EXAMPLES\r\n");
                docgen_puts(f_txt, "------------------------------------------------------------------------------\r\n");
                docgen_print_wrapped_plaintext(f_txt, meta->examples, 2, 78);
                docgen_puts(f_txt, "\r\n");
            }

            if (meta->error_summary && meta->error_summary[0] != '\0') {
                docgen_puts(f_txt, "------------------------------------------------------------------------------\r\n");
                docgen_puts(f_txt, "4. ERROR CONDITIONS\r\n");
                docgen_puts(f_txt, "------------------------------------------------------------------------------\r\n");
                docgen_print_wrapped_plaintext(f_txt, meta->error_summary, 0, 78);
                docgen_puts(f_txt, "\r\n");
            }

            docgen_puts(f_txt, "------------------------------------------------------------------------------\r\n");
            docgen_puts(f_txt, "5. COMPATIBILITY & LINEAGE\r\n");
            docgen_puts(f_txt, "------------------------------------------------------------------------------\r\n");
            docgen_printf(f_txt, "Compatibility: %s\r\n", compat);
            docgen_printf(f_txt, "Since Version: %s\r\n", since_ver);
            docgen_printf(f_txt, "Category:      %s\r\n", category);
            docgen_printf(f_txt, "Subsystem:     %s\r\n", subsystem_to_str(meta->subsystem));
            docgen_printf(f_txt, "Safety Level:  %s\r\n\r\n", safety_to_str(meta->safety));

            docgen_puts(f_txt, "------------------------------------------------------------------------------\r\n");
            docgen_puts(f_txt, "LANGUAGEDESCRIPTOR (LANGDESC) QUICK REFERENCE\r\n");
            docgen_puts(f_txt, "------------------------------------------------------------------------------\r\n");
            docgen_printf(f_txt, "static const LangDesc g_%s_desc = {\r\n", var_name);
            docgen_printf(f_txt, "    .name = \"%s\",\r\n", meta->name);
            docgen_printf(f_txt, "    .category = \"%s\",\r\n", category);
            docgen_printf(f_txt, "    .syntax = \"%s\",\r\n", meta->syntax ? meta->syntax : meta->name);
            docgen_printf(f_txt, "    .description = \"%s\",\r\n", meta->description ? meta->description : "");
            docgen_printf(f_txt, "    .error_summary = \"%s\",\r\n", meta->error_summary ? meta->error_summary : "None");
            docgen_printf(f_txt, "    .subsystem = %s,\r\n", subsystem_to_str(meta->subsystem));
            docgen_printf(f_txt, "    .safety = %s,\r\n", safety_to_str(meta->safety));
            docgen_printf(f_txt, "    .type = %s,\r\n", type_to_str(meta->type));
            docgen_printf(f_txt, "    .delim_mask = %s,\r\n", delim_buf);
            docgen_printf(f_txt, "    .compat = \"%s\",\r\n", compat);
            docgen_printf(f_txt, "    .since_version = \"%s\",\r\n", since_ver);
            docgen_printf(f_txt, "    .source_file = \"%s\"\r\n", source_file);
            docgen_puts(f_txt, "};\r\n");

            hal->io.file_close(f_txt);
        }
    }

    return true;
}

bool docgen_export_catalogs(const char *docs_dir, const char *help_dir) {
    if (!help_dir) return false;
    HalContext *hal = hal_get();
    if (!hal || !hal->io.file_open || !hal->io.file_close) return false;

    ensure_dir(help_dir);
    if (docs_dir) ensure_dir(docs_dir);

    int total = lang_desc_count();

    // 1. Export help/help.TXT
    char help_path[512];
    runtime_snprintf(help_path, sizeof(help_path), "%s/help.TXT", help_dir);
    IoHandle f_help = hal->io.file_open(help_path, "wb");
    if (f_help != IO_HANDLE_INVALID) {
        docgen_puts(f_help, "Title:        help\r\n");
        docgen_puts(f_help, "Tier:         4\r\n");
        docgen_printf(f_help, "Applies to:   BASIC++ v%s (baspp, bpp, bs, iot)\r\n", BASIC_VERSION_STRING);
        docgen_puts(f_help, "Authority:    engine/include/runtime/language_descriptor.h\r\n");
        docgen_puts(f_help, "Generated:    yes, from LanguageDescriptor\r\n");
        docgen_puts(f_help, "Status:       current\r\n\r\n");

        docgen_puts(f_help, "==============================================================================\r\n");
        docgen_printf(f_help, "BASIC++ v%s QUICK-REFERENCE MASTER HELP CATALOG\r\n", BASIC_VERSION_STRING);
        docgen_puts(f_help, "==============================================================================\r\n\r\n");
        docgen_printf(f_help, "Total Registered Vocabulary Items: %d\r\n\r\n", total);

        for (int i = 0; i < total; i++) {
            const LanguageDescriptor *meta = lang_desc_get(i);
            if (!meta || !meta->name) continue;

            docgen_printf(f_help, "* %-14s [%-9s] %s\r\n",
                meta->name,
                type_to_display(meta->type),
                meta->syntax ? meta->syntax : meta->name);
            if (meta->description && meta->description[0] != '\0') {
                docgen_print_wrapped_plaintext(f_help, meta->description, 4, 78);
            }
            docgen_puts(f_help, "\r\n");
        }
        hal->io.file_close(f_help);
    }

    // 2. Export help/catalog.TXT
    char cat_path[512];
    runtime_snprintf(cat_path, sizeof(cat_path), "%s/catalog.TXT", help_dir);
    IoHandle f_cat = hal->io.file_open(cat_path, "wb");
    if (f_cat != IO_HANDLE_INVALID) {
        docgen_puts(f_cat, "Title:        catalog\r\n");
        docgen_puts(f_cat, "Tier:         4\r\n");
        docgen_printf(f_cat, "Applies to:   BASIC++ v%s (baspp, bpp, bs, iot)\r\n", BASIC_VERSION_STRING);
        docgen_puts(f_cat, "Authority:    engine/include/runtime/language_descriptor.h\r\n");
        docgen_puts(f_cat, "Generated:    yes, from LanguageDescriptor\r\n");
        docgen_puts(f_cat, "Status:       current\r\n\r\n");

        docgen_puts(f_cat, "==============================================================================\r\n");
        docgen_printf(f_cat, "BASIC++ v%s MASTER INVENTORY CATALOG\r\n", BASIC_VERSION_STRING);
        docgen_puts(f_cat, "==============================================================================\r\n\r\n");
        docgen_puts(f_cat, "Name             Type         Category             Syntax\r\n");
        docgen_puts(f_cat, "------------------------------------------------------------------------------\r\n");

        for (int i = 0; i < total; i++) {
            const LanguageDescriptor *meta = lang_desc_get(i);
            if (!meta || !meta->name) continue;

            docgen_printf(f_cat, "%-16s %-12s %-20s %s\r\n",
                meta->name,
                type_to_display(meta->type),
                meta->category ? meta->category : "General",
                meta->syntax ? meta->syntax : meta->name);
        }
        hal->io.file_close(f_cat);
    }

    // 3. Export Markdown Catalogs (docs/help.md, docs/catalog.md) if docs_dir specified
    if (docs_dir) {
        char md_help[512];
        runtime_snprintf(md_help, sizeof(md_help), "%s/help.md", docs_dir);
        IoHandle f_md_h = hal->io.file_open(md_help, "wb");
        if (f_md_h != IO_HANDLE_INVALID) {
            docgen_puts(f_md_h, "<!--\n");
            docgen_puts(f_md_h, "Title:        help\n");
            docgen_puts(f_md_h, "Tier:         4\n");
            docgen_printf(f_md_h, "Applies to:   BASIC++ v%s (baspp, bpp, bs, iot)\n", BASIC_VERSION_STRING);
            docgen_puts(f_md_h, "Authority:    engine/include/runtime/language_descriptor.h\n");
            docgen_puts(f_md_h, "Generated:    yes, from LanguageDescriptor\n");
            docgen_puts(f_md_h, "Status:       current\n");
            docgen_puts(f_md_h, "-->\n\n");

            docgen_printf(f_md_h, "# BASIC++ v%s Quick-Reference Master Help Catalog\n\n", BASIC_VERSION_STRING);
            docgen_printf(f_md_h, "**Total Registered Vocabulary Items**: %d\n\n", total);

            for (int i = 0; i < total; i++) {
                const LanguageDescriptor *meta = lang_desc_get(i);
                if (!meta || !meta->name) continue;

                docgen_printf(f_md_h, "### `%s` (%s)\n", meta->name, type_to_display(meta->type));
                docgen_printf(f_md_h, "```basic\n%s\n```\n", meta->syntax ? meta->syntax : meta->name);
                if (meta->description && meta->description[0] != '\0') {
                    docgen_printf(f_md_h, "%s\n\n", meta->description);
                } else {
                    docgen_puts(f_md_h, "\n");
                }
            }
            hal->io.file_close(f_md_h);
        }

        char md_cat[512];
        runtime_snprintf(md_cat, sizeof(md_cat), "%s/catalog.md", docs_dir);
        IoHandle f_md_c = hal->io.file_open(md_cat, "wb");
        if (f_md_c != IO_HANDLE_INVALID) {
            docgen_puts(f_md_c, "<!--\n");
            docgen_puts(f_md_c, "Title:        catalog\n");
            docgen_puts(f_md_c, "Tier:         4\n");
            docgen_printf(f_md_c, "Applies to:   BASIC++ v%s (baspp, bpp, bs, iot)\n", BASIC_VERSION_STRING);
            docgen_puts(f_md_c, "Authority:    engine/include/runtime/language_descriptor.h\n");
            docgen_puts(f_md_c, "Generated:    yes, from LanguageDescriptor\n");
            docgen_puts(f_md_c, "Status:       current\n");
            docgen_puts(f_md_c, "-->\n\n");

            docgen_printf(f_md_c, "# BASIC++ v%s Master Inventory Catalog\n\n", BASIC_VERSION_STRING);
            docgen_puts(f_md_c, "| Name | Type | Category | Syntax |\n");
            docgen_puts(f_md_c, "| :--- | :--- | :--- | :--- |\n");

            for (int i = 0; i < total; i++) {
                const LanguageDescriptor *meta = lang_desc_get(i);
                if (!meta || !meta->name) continue;

                docgen_printf(f_md_c, "| `%s` | %s | %s | `%s` |\n",
                    meta->name,
                    type_to_display(meta->type),
                    meta->category ? meta->category : "General",
                    meta->syntax ? meta->syntax : meta->name);
            }
            hal->io.file_close(f_md_c);
        }
    }

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
    bool res3 = docgen_export_keywords("docs", "help");
    bool res4 = docgen_export_catalogs("docs", "help");
    bool res5 = docgen_export_html(html_path);

    return res1 && res2 && res3 && res4 && res5;
}
