/* =====================================================================
 * What it does: Header file for the BASIC++ Engine Documentation Generator (docgen).
 * Why it exists: Provides C API interfaces to extract and export structured MicroLib metadata, JSON schema (api_schema.json), Markdown manuals, and offline HTML references.
 * Why it works this way: Iterates over the runtime MicroLibMetadata registry and header docstrings, emitting formatted representations.
 * Dependencies & compilation target: Target C17, depends on <stdbool.h>, <stddef.h>.
 * Edition inclusion/exclusion: Included in baspp, bpp, and bs targets.
 * How to modify or extend it: Add new exporter formats (e.g., docgen_export_openapi).
 * What cannot be changed: Function signatures, C17 standard compliance, JSON schema property names.
 * What to expect: Thread-safe, non-recursive file generation. Returns true on success, false on error.
 * What to do if something breaks: Verify output directory write permissions and ensure boot_system() has run.
 * Assumptions & preconditions: microlib_init() and micro-library metadata registrations are complete before invocation.
 * Portability & C17 concerns: Strict C17 compliance. Uses standard file I/O abstraction.
 * Component dependencies & prerequisite source files:
 *   - engine/src/docgen/docgen.c
 *   - engine/src/runtime/metadata.c
 *   - engine/include/runtime/micro_lib_metadata.h
 * ===================================================================== */

#ifndef DOCGEN_DOCGEN_H
#define DOCGEN_DOCGEN_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Exports the canonical API schema as a JSON file.
 * @param filepath Path to destination JSON file (e.g. "api_schema.json").
 * @return True on success, false on I/O error.
 */
bool docgen_export_json(const char *filepath);

/**
 * @brief Exports per-category and master Markdown documentation files.
 * @param output_dir Destination directory for markdown files (e.g. "docs/api").
 * @return True on success, false on error.
 */
bool docgen_export_markdown(const char *output_dir);

/**
 * @brief Exports a standalone, zero-dependency offline HTML documentation manual.
 * @param filepath Path to destination HTML file (e.g. "api_reference.html").
 * @return True on success, false on error.
 */
bool docgen_export_html(const char *filepath);

/**
 * @brief Exports JSON, Markdown, and HTML documentation packages in one step.
 * @param output_dir Target base directory.
 * @return True on success, false on error.
 */
bool docgen_export_all(const char *output_dir);

#ifdef __cplusplus
}
#endif

#endif /* DOCGEN_DOCGEN_H */
