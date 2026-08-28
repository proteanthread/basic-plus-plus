// FILENAME: pdf_writer.h
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libkernel (pdf_writer.c, vprinter.c)
// NEEDS: platform, memory
// Implements virtual device and graphics rendering logic for pdf_writer.
//
// ---- Includes ----

#ifndef DEVICE_PDF_WRITER_H
#define DEVICE_PDF_WRITER_H

#include <stddef.h>
#include <stdbool.h>

#define PDF_PAGE_LETTER_W 612.0
#define PDF_PAGE_LETTER_H 792.0
#define PDF_PAGE_A4_W     595.0
#define PDF_PAGE_A4_H     842.0

#define PDF_DEFAULT_FONT_SIZE   10.0
#define PDF_DEFAULT_LINE_HEIGHT 12.0
#define PDF_DEFAULT_MARGIN_LEFT 36.0
#define PDF_DEFAULT_MARGIN_TOP  36.0
#define PDF_DEFAULT_MARGIN_BOT  36.0

#define PDF_MAX_LINES_PER_PAGE  60

typedef struct PdfDoc PdfDoc;

// Creates a new in-memory PDF document.
// @param width Page width in points (default: 612.0 = Letter)
// @param height Page height in points (default: 792.0 = Letter)
// @return Allocated PdfDoc pointer or NULL on failure.
PdfDoc *pdf_doc_create(double width, double height);

// Frees a PDF document and all its allocated pages and streams.
void pdf_doc_destroy(PdfDoc *doc);

// Starts a new page in the document.
void pdf_doc_add_page(PdfDoc *doc);

// Writes a line of text to the current page. If lines exceed page capacity,
// automatically starts a new page.
void pdf_doc_write_line(PdfDoc *doc, const char *text);

// Writes raw text (handling \n, \r, \t, and \f Form Feed) to the document.
void pdf_doc_write_text(PdfDoc *doc, const char *text);

// Checks if any text has been written to the document.
bool pdf_doc_has_content(const PdfDoc *doc);

// Serializes the PDF document to disk at the specified filepath.
// @return true on success, false on failure.
bool pdf_doc_save_file(const PdfDoc *doc, const char *filepath);

#endif // DEVICE_PDF_WRITER_H
