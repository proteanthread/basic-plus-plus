// FILENAME: pdf_writer.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libkernel (vprinter.c)
// NEEDS: libcore (alloc.h, alloc.c, hal.h, memops.h, memops.c)
// NEEDS: libcore (snprintf.h, snprintf.c, strops.h, strops.c)
// NEEDS: libkernel (pdf_writer.h)
// Implements virtual device and graphics rendering logic for pdf_writer.
//
// ---- Includes ----

#include "device/pdf_writer.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/format/snprintf.h"
#include "hal/hal.h"

#define INITIAL_STREAM_CAP 4096
#define INITIAL_PAGE_CAP   16

typedef struct {
    char   *data;
    size_t  size;
    size_t  capacity;
    int     line_count;
} PdfPage;

struct PdfDoc {
    double    width;
    double    height;
    double    font_size;
    double    line_height;
    double    margin_left;
    double    margin_top;
    double    margin_bottom;
    int       max_lines_per_page;

    PdfPage **pages;
    size_t    page_count;
    size_t    page_capacity;

    char     *current_line_buf;
    size_t    current_line_len;
    size_t    current_line_cap;
    bool      has_content;
};

static bool page_append_bytes(PdfPage *page, const char *bytes, size_t len) {
    if (!page || !bytes || len == 0) return true;
    HalContext *hal = hal_get();
    if (page->size + len + 1 > page->capacity) {
        size_t old_cap = page->capacity;
        size_t new_cap = page->capacity ? page->capacity * 2 : INITIAL_STREAM_CAP;
        while (new_cap < page->size + len + 1) new_cap *= 2;
        char *new_data = NULL;
        if (hal && hal->mem.realloc) {
            new_data = (char *)hal->mem.realloc(page->data, new_cap);
        } else if (hal && hal->mem.alloc) {
            new_data = (char *)hal->mem.alloc(new_cap);
            if (new_data && page->data) {
                runtime_memcpy(new_data, page->data, page->size);
                if (hal->mem.free) hal->mem.free(page->data);
            }
        }
        if (!new_data) return false;
        page->data = new_data;
        page->capacity = new_cap;
        (void)old_cap;
    }
    runtime_memcpy(page->data + page->size, bytes, len);
    page->size += len;
    page->data[page->size] = '\0';
    return true;
}

static bool page_append_str(PdfPage *page, const char *str) {
    if (!str) return true;
    return page_append_bytes(page, str, runtime_strlen(str));
}

PdfDoc *pdf_doc_create(double width, double height) {
    HalContext *hal = hal_get();
    PdfDoc *doc = NULL;
    if (hal && hal->mem.alloc) {
        doc = (PdfDoc *)hal->mem.alloc(sizeof(PdfDoc));
    }
    if (!doc) return NULL;
    runtime_memset(doc, 0, sizeof(PdfDoc));

    doc->width = (width > 0.0) ? width : PDF_PAGE_LETTER_W;
    doc->height = (height > 0.0) ? height : PDF_PAGE_LETTER_H;
    doc->font_size = PDF_DEFAULT_FONT_SIZE;
    doc->line_height = PDF_DEFAULT_LINE_HEIGHT;
    doc->margin_left = PDF_DEFAULT_MARGIN_LEFT;
    doc->margin_top = PDF_DEFAULT_MARGIN_TOP;
    doc->margin_bottom = PDF_DEFAULT_MARGIN_BOT;
    doc->max_lines_per_page = (int)((doc->height - doc->margin_top - doc->margin_bottom) / doc->line_height);
    if (doc->max_lines_per_page < 10) doc->max_lines_per_page = 10;

    doc->current_line_cap = 512;
    if (hal && hal->mem.alloc) {
        doc->current_line_buf = (char *)hal->mem.alloc(doc->current_line_cap);
    }
    if (!doc->current_line_buf) {
        if (hal && hal->mem.free) hal->mem.free(doc);
        return NULL;
    }
    doc->current_line_buf[0] = '\0';
    doc->current_line_len = 0;
    doc->has_content = false;

    return doc;
}

void pdf_doc_destroy(PdfDoc *doc) {
    if (!doc) return;
    HalContext *hal = hal_get();
    if (doc->pages) {
        for (size_t i = 0; i < doc->page_count; ++i) {
            if (doc->pages[i]) {
                if (doc->pages[i]->data && hal && hal->mem.free) {
                    hal->mem.free(doc->pages[i]->data);
                }
                if (hal && hal->mem.free) hal->mem.free(doc->pages[i]);
            }
        }
        if (hal && hal->mem.free) hal->mem.free(doc->pages);
    }
    if (doc->current_line_buf && hal && hal->mem.free) {
        hal->mem.free(doc->current_line_buf);
    }
    if (hal && hal->mem.free) {
        hal->mem.free(doc);
    }
}

void pdf_doc_add_page(PdfDoc *doc) {
    if (!doc) return;
    HalContext *hal = hal_get();

    if (doc->page_count + 1 > doc->page_capacity) {
        size_t old_cap = doc->page_capacity;
        size_t new_cap = doc->page_capacity ? doc->page_capacity * 2 : INITIAL_PAGE_CAP;
        PdfPage **new_pages = NULL;
        if (hal && hal->mem.realloc) {
            new_pages = (PdfPage **)hal->mem.realloc(doc->pages, sizeof(PdfPage *) * new_cap);
        } else if (hal && hal->mem.alloc) {
            new_pages = (PdfPage **)hal->mem.alloc(sizeof(PdfPage *) * new_cap);
            if (new_pages && doc->pages) {
                runtime_memcpy(new_pages, doc->pages, sizeof(PdfPage *) * old_cap);
                if (hal->mem.free) hal->mem.free(doc->pages);
            }
        }
        if (!new_pages) return;
        doc->pages = new_pages;
        doc->page_capacity = new_cap;
    }

    PdfPage *page = NULL;
    if (hal && hal->mem.alloc) {
        page = (PdfPage *)hal->mem.alloc(sizeof(PdfPage));
    }
    if (!page) return;
    runtime_memset(page, 0, sizeof(PdfPage));

    page->capacity = INITIAL_STREAM_CAP;
    if (hal && hal->mem.alloc) {
        page->data = (char *)hal->mem.alloc(page->capacity);
    }
    if (!page->data) {
        if (hal && hal->mem.free) hal->mem.free(page);
        return;
    }
    page->data[0] = '\0';
    page->size = 0;
    page->line_count = 0;

    // Initialize Text Object with font and start position
    char init_buf[128];
    double start_y = doc->height - doc->margin_top - doc->font_size;
    runtime_snprintf(init_buf, sizeof(init_buf),
             "BT\n/F1 %.2f Tf\n%.2f TL\n%.2f %.2f Td\n",
             doc->font_size, doc->line_height, doc->margin_left, start_y);
    page_append_str(page, init_buf);

    doc->pages[doc->page_count++] = page;
}

static void escape_and_append_pdf_string(PdfPage *page, const char *str) {
    if (!page || !str) return;
    page_append_str(page, "(");
    while (*str) {
        unsigned char c = (unsigned char)*str;
        if (c == '(' || c == ')' || c == '\\') {
            char esc[2] = {'\\', (char)c};
            page_append_bytes(page, esc, 2);
        } else if (c >= 32 && c <= 126) {
            char ch = (char)c;
            page_append_bytes(page, &ch, 1);
        } else if (c == '\t') {
            // Expand tab to 4 spaces
            page_append_str(page, "    ");
        } else {
            // Octal escape for other bytes
            char oct[5];
            runtime_snprintf(oct, sizeof(oct), "\\%03o", c);
            page_append_str(page, oct);
        }
        str++;
    }
    page_append_str(page, ") Tj\n");
}

void pdf_doc_write_line(PdfDoc *doc, const char *text) {
    if (!doc) return;

    if (doc->page_count == 0 ||
        doc->pages[doc->page_count - 1]->line_count >= doc->max_lines_per_page) {
        pdf_doc_add_page(doc);
    }

    PdfPage *cur_page = doc->pages[doc->page_count - 1];

    if (cur_page->line_count > 0) {
        page_append_str(cur_page, "T*\n");
    }

    escape_and_append_pdf_string(cur_page, text ? text : "");
    cur_page->line_count++;
    doc->has_content = true;
}

void pdf_doc_write_text(PdfDoc *doc, const char *text) {
    if (!doc || !text) return;
    HalContext *hal = hal_get();

    while (*text) {
        char c = *text++;
        if (c == '\r') {
            continue;
        } else if (c == '\n') {
            doc->current_line_buf[doc->current_line_len] = '\0';
            pdf_doc_write_line(doc, doc->current_line_buf);
            doc->current_line_len = 0;
            doc->current_line_buf[0] = '\0';
        } else if (c == '\f') {
            // Form Feed: flush current line and start a new page
            if (doc->current_line_len > 0) {
                doc->current_line_buf[doc->current_line_len] = '\0';
                pdf_doc_write_line(doc, doc->current_line_buf);
                doc->current_line_len = 0;
                doc->current_line_buf[0] = '\0';
            }
            pdf_doc_add_page(doc);
            doc->has_content = true;
        } else {
            if (doc->current_line_len + 2 > doc->current_line_cap) {
                size_t old_cap = doc->current_line_cap;
                size_t new_cap = doc->current_line_cap * 2;
                char *new_buf = NULL;
                if (hal && hal->mem.realloc) {
                    new_buf = (char *)hal->mem.realloc(doc->current_line_buf, new_cap);
                } else if (hal && hal->mem.alloc) {
                    new_buf = (char *)hal->mem.alloc(new_cap);
                    if (new_buf && doc->current_line_buf) {
                        runtime_memcpy(new_buf, doc->current_line_buf, old_cap);
                        if (hal->mem.free) hal->mem.free(doc->current_line_buf);
                    }
                }
                if (!new_buf) return;
                doc->current_line_buf = new_buf;
                doc->current_line_cap = new_cap;
            }
            doc->current_line_buf[doc->current_line_len++] = c;
            doc->current_line_buf[doc->current_line_len] = '\0';
            doc->has_content = true;
        }
    }
}

bool pdf_doc_has_content(const PdfDoc *doc) {
    if (!doc) return false;
    return doc->has_content || (doc->current_line_len > 0);
}

static void pdf_write_str(IoHandle handle, const char *s) {
    if (handle == IO_HANDLE_INVALID || !s) return;
    HalContext *hal = hal_get();
    if (hal && hal->io.file_write) {
        hal->io.file_write(handle, s, 1, runtime_strlen(s));
    }
}

bool pdf_doc_save_file(const PdfDoc *doc_const, const char *filepath) {
    if (!doc_const || !filepath) return false;
    HalContext *hal = hal_get();
    if (!hal || !hal->io.file_open) return false;

    // Flush any pending trailing line
    PdfDoc *doc = (PdfDoc *)doc_const;
    if (doc->current_line_len > 0) {
        doc->current_line_buf[doc->current_line_len] = '\0';
        pdf_doc_write_line(doc, doc->current_line_buf);
        doc->current_line_len = 0;
        doc->current_line_buf[0] = '\0';
    }

    if (doc->page_count == 0) {
        pdf_doc_add_page(doc);
    }

    IoHandle fp = hal->io.file_open(filepath, "wb");
    if (fp == IO_HANDLE_INVALID) return false;

    size_t num_pages = doc->page_count;
    size_t total_objs = 3 + num_pages * 2;

    long *offsets = NULL;
    if (hal->mem.alloc) {
        offsets = (long *)hal->mem.alloc((total_objs + 1) * sizeof(long));
    }
    if (!offsets) {
        if (hal->io.file_close) hal->io.file_close(fp);
        return false;
    }
    runtime_memset(offsets, 0, (total_objs + 1) * sizeof(long));

    char buf[512];

    // 1. Header
    pdf_write_str(fp, "%PDF-1.4\n%\xe2\xe3\xcf\xd3\n");

    // 2. Catalog (Object 1)
    offsets[1] = hal->io.file_tell ? (long)hal->io.file_tell(fp) : 0;
    pdf_write_str(fp, "1 0 obj\n<< /Type /Catalog /Pages 2 0 R >>\nendobj\n");

    // 3. Pages Root (Object 2)
    offsets[2] = hal->io.file_tell ? (long)hal->io.file_tell(fp) : 0;
    pdf_write_str(fp, "2 0 obj\n<< /Type /Pages /Kids [");
    for (size_t i = 0; i < num_pages; ++i) {
        runtime_snprintf(buf, sizeof(buf), " %d 0 R", (int)(4 + 2 * i));
        pdf_write_str(fp, buf);
    }
    runtime_snprintf(buf, sizeof(buf), " ] /Count %d >>\nendobj\n", (int)num_pages);
    pdf_write_str(fp, buf);

    // 4. Font Resource (Object 3)
    offsets[3] = hal->io.file_tell ? (long)hal->io.file_tell(fp) : 0;
    pdf_write_str(fp, "3 0 obj\n<< /Type /Font /Subtype /Type1 /BaseFont /Courier /Encoding /WinAnsiEncoding >>\nendobj\n");

    // 5. Page Objects and Content Streams
    for (size_t i = 0; i < num_pages; ++i) {
        int page_obj_id = (int)(4 + 2 * i);
        int stream_obj_id = (int)(4 + 2 * i + 1);
        PdfPage *page = doc->pages[i];

        // Close BT text block
        page_append_str(page, "ET\n");

        // Page Object
        offsets[page_obj_id] = hal->io.file_tell ? (long)hal->io.file_tell(fp) : 0;
        runtime_snprintf(buf, sizeof(buf), "%d 0 obj\n<< /Type /Page /Parent 2 0 R /MediaBox [ 0 0 %.2f %.2f ]\n"
                    "   /Resources << /Font << /F1 3 0 R >> >>\n"
                    "   /Contents %d 0 R >>\nendobj\n",
                page_obj_id, doc->width, doc->height, stream_obj_id);
        pdf_write_str(fp, buf);

        // Content Stream Object
        offsets[stream_obj_id] = hal->io.file_tell ? (long)hal->io.file_tell(fp) : 0;
        runtime_snprintf(buf, sizeof(buf), "%d 0 obj\n<< /Length %d >>\nstream\n", stream_obj_id, (int)page->size);
        pdf_write_str(fp, buf);
        if (hal->io.file_write) {
            hal->io.file_write(fp, page->data, 1, page->size);
        }
        pdf_write_str(fp, "\nendstream\nendobj\n");
    }

    // 6. XRef Table
    long xref_offset = hal->io.file_tell ? (long)hal->io.file_tell(fp) : 0;
    runtime_snprintf(buf, sizeof(buf), "xref\n0 %d\n0000000000 65535 f \n", (int)(total_objs + 1));
    pdf_write_str(fp, buf);
    for (size_t i = 1; i <= total_objs; ++i) {
        runtime_snprintf(buf, sizeof(buf), "%010ld 00000 n \n", offsets[i]);
        pdf_write_str(fp, buf);
    }

    // 7. Trailer
    runtime_snprintf(buf, sizeof(buf), "trailer\n<< /Size %d /Root 1 0 R >>\nstartxref\n%ld\n%%%%EOF\n", (int)(total_objs + 1), xref_offset);
    pdf_write_str(fp, buf);

    if (hal->mem.free) hal->mem.free(offsets);
    if (hal->io.file_close) hal->io.file_close(fp);
    return true;
}


