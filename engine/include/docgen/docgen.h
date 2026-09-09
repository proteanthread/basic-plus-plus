// FILENAME: docgen.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: baspp.exe (desktop.c)
// NEEDED BY: libcore (docgen.c)
// NEEDS: platform, memory
// Provides core logic and interface definitions for docgen within BASIC++.
//
// ---- Includes ----

#ifndef DOCGEN_DOCGEN_H
#define DOCGEN_DOCGEN_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// @brief Exports the canonical API schema as a JSON file.
// @param filepath Path to destination JSON file (e.g. "api_schema.json").
// @return True on success, false on I/O error.
bool docgen_export_json(const char *filepath);

// @brief Exports per-category and master Markdown documentation files.
// @param output_dir Destination directory for markdown files (e.g. "docs/api").
// @return True on success, false on error.
bool docgen_export_markdown(const char *output_dir);

// @brief Exports per-keyword reference pages to docs/keywords/ and help/keywords/.
// @param docs_dir Base path for Markdown output (e.g. "docs" or "docs/keywords").
// @param help_dir Base path for Plaintext output (e.g. "help" or "help/keywords").
// @return True on success, false on error.
bool docgen_export_keywords(const char *docs_dir, const char *help_dir);

// @brief Exports Tier 4 catalogs in Markdown (docs/) and Plaintext (help/).
// @param docs_dir Base path for docs output directory (e.g. "docs").
// @param help_dir Base path for help output directory (e.g. "help").
// @return True on success, false on error.
bool docgen_export_catalogs(const char *docs_dir, const char *help_dir);

// @brief Exports a standalone, zero-dependency offline HTML documentation manual.
// @param filepath Path to destination HTML file (e.g. "api_reference.html").
// @return True on success, false on error.
bool docgen_export_html(const char *filepath);

// @brief Exports JSON, Markdown, and HTML documentation packages in one step.
// @param output_dir Target base directory.
// @return True on success, false on error.
bool docgen_export_all(const char *output_dir);

#ifdef __cplusplus
}
#endif

#endif // DOCGEN_DOCGEN_H
