// FILENAME: vprinter.c
// LICENSE: Copyleft (c) 2026 BASIC++ Community — All Wrongs Reserved
// VERSION: 6.5.2.0
// NEEDED BY: libboot (common_internal.h)
// NEEDED BY: libcore (print_using.c)
// NEEDED BY: libengine (context.c, exec_control_internal.h, exec_dispatch.c)
// NEEDED BY: libengine (exec_internal.h, exec_interrupt.c, llist.c, lprint.c)
// NEEDS: libcore (alloc.h, alloc.c, ctype.h, ctype.c, hal.h)
// NEEDS: libcore (memops.h, memops.c, snprintf.h, snprintf.c)
// NEEDS: libcore (strops.h, strops.c)
// NEEDS: libkernel (pdf_writer.h, pdf_writer.c, vprinter.h)
// Implements virtual device and graphics rendering logic for vprinter.
//
// ---- Includes ----

#include "device/vprinter.h"
#include "device/pdf_writer.h"
#include "runtime/memory/alloc.h"
#include "runtime/string/memops.h"
#include "runtime/string/strops.h"
#include "runtime/ctype/ctype.h"
#include "runtime/format/snprintf.h"
#include "hal/hal.h"

typedef struct {
    PdfDoc *doc;
    char    target_filename[512];
    bool    has_printed;
    bool    is_custom_file;
} VPrinterChannel;

static PdfDoc *g_printer_doc = NULL;
static char    g_prog_path[512] = {0};
static char    g_output_pdf[512] = "OUTPUT.PDF";
static bool    g_has_printed = false;

static void update_output_pdf_name(void) {
    if (g_prog_path[0] == '\0') {
        runtime_strncpy(g_output_pdf, "OUTPUT.PDF", sizeof(g_output_pdf) - 1);
        g_output_pdf[sizeof(g_output_pdf) - 1] = '\0';
        return;
    }

    // Extract filename component from path
    const char *p = g_prog_path;
    const char *last_sep = p;
    while (*p) {
        if (*p == '/' || *p == '\\') {
            last_sep = p + 1;
        }
        p++;
    }

    char base_name[256];
    runtime_strncpy(base_name, last_sep, sizeof(base_name) - 1);
    base_name[sizeof(base_name) - 1] = '\0';

    // Strip extension if present
    char *dot = runtime_strrchr(base_name, '.');
    if (dot) {
        *dot = '\0';
    }

    if (base_name[0] == '\0') {
        runtime_strncpy(g_output_pdf, "OUTPUT.PDF", sizeof(g_output_pdf) - 1);
    } else {
        runtime_snprintf(g_output_pdf, sizeof(g_output_pdf), "%s.pdf", base_name);
    }
}

static void resolve_channel_target_filename(const char *path, char *out_buf, size_t max_len) {
    if (!path || path[0] == '\0') {
        runtime_strncpy(out_buf, g_output_pdf, max_len - 1);
        out_buf[max_len - 1] = '\0';
        return;
    }

    size_t len = runtime_strlen(path);
    if (len >= 4 && (runtime_strcasecmp(path + len - 4, ".pdf") == 0)) {
        runtime_strncpy(out_buf, path, max_len - 1);
        out_buf[max_len - 1] = '\0';
    } else {
        runtime_snprintf(out_buf, max_len, "%s.pdf", path);
    }
}

void vprinter_init(void) {
    if (!g_printer_doc) {
        g_printer_doc = pdf_doc_create(PDF_PAGE_LETTER_W, PDF_PAGE_LETTER_H);
    }
    g_has_printed = false;
    update_output_pdf_name();
}

void vprinter_shutdown(void) {
    if (g_has_printed) {
        vprinter_flush_pdf(NULL);
    }
    if (g_printer_doc) {
        pdf_doc_destroy(g_printer_doc);
        g_printer_doc = NULL;
    }
    g_has_printed = false;
}

void vprinter_set_program_path(const char *prog_path) {
    if (prog_path && prog_path[0] != '\0') {
        runtime_strncpy(g_prog_path, prog_path, sizeof(g_prog_path) - 1);
        g_prog_path[sizeof(g_prog_path) - 1] = '\0';
    } else {
        g_prog_path[0] = '\0';
    }
    update_output_pdf_name();
}

const char *vprinter_get_output_filename(void) {
    return g_output_pdf;
}

void vprinter_write_str(const char *str) {
    if (!str) return;
    if (!g_printer_doc) {
        g_printer_doc = pdf_doc_create(PDF_PAGE_LETTER_W, PDF_PAGE_LETTER_H);
    }
    if (g_printer_doc) {
        pdf_doc_write_text(g_printer_doc, str);
        g_has_printed = true;
    }
}

void vprinter_write_char(char c) {
    char buf[2] = {c, '\0'};
    vprinter_write_str(buf);
}

bool vprinter_has_output(void) {
    return g_has_printed || (g_printer_doc && pdf_doc_has_content(g_printer_doc));
}

bool vprinter_flush_pdf(const char *override_filename) {
    if (!vprinter_has_output() || !g_printer_doc) {
        return false;
    }

    const char *target = (override_filename && override_filename[0] != '\0') ? override_filename : g_output_pdf;
    bool success = pdf_doc_save_file(g_printer_doc, target);

    // Reset for next print job
    pdf_doc_destroy(g_printer_doc);
    g_printer_doc = pdf_doc_create(PDF_PAGE_LETTER_W, PDF_PAGE_LETTER_H);
    g_has_printed = false;

    return success;
}

void vprinter_reset(void) {
    if (g_printer_doc) {
        pdf_doc_destroy(g_printer_doc);
        g_printer_doc = pdf_doc_create(PDF_PAGE_LETTER_W, PDF_PAGE_LETTER_H);
    }
    g_has_printed = false;
}

// --- VDev Device Callbacks ---

static int vprinter_dev_open(VDev *dev, const char *path, int mode) {
    (void)mode;
    if (!dev) return -1;
    HalContext *hal = hal_get();

    VPrinterChannel *chan = NULL;
    if (hal && hal->mem.alloc) {
        chan = (VPrinterChannel *)hal->mem.alloc(sizeof(VPrinterChannel));
    }
    if (!chan) return -1;
    runtime_memset(chan, 0, sizeof(VPrinterChannel));

    chan->doc = pdf_doc_create(PDF_PAGE_LETTER_W, PDF_PAGE_LETTER_H);
    if (!chan->doc) {
        if (hal && hal->mem.free) hal->mem.free(chan);
        return -1;
    }

    resolve_channel_target_filename(path, chan->target_filename, sizeof(chan->target_filename));
    chan->is_custom_file = (path && path[0] != '\0');
    chan->has_printed = false;

    dev->priv = chan;
    return 0;
}

static int vprinter_dev_close(VDev *dev) {
    if (!dev) return -1;
    HalContext *hal = hal_get();

    if (dev->priv) {
        VPrinterChannel *chan = (VPrinterChannel *)dev->priv;
        if (chan->has_printed || pdf_doc_has_content(chan->doc)) {
            pdf_doc_save_file(chan->doc, chan->target_filename);
        }
        pdf_doc_destroy(chan->doc);
        if (hal && hal->mem.free) {
            hal->mem.free(chan);
        }
        dev->priv = NULL;
    } else {
        if (vprinter_has_output()) {
            vprinter_flush_pdf(NULL);
        }
    }
    return 0;
}

static int vprinter_dev_write(VDev *dev, const void *buf, int count) {
    if (!dev || !buf || count <= 0) return 0;
    const char *str = (const char *)buf;

    if (dev->priv) {
        VPrinterChannel *chan = (VPrinterChannel *)dev->priv;
        for (int i = 0; i < count; ++i) {
            char ch[2] = {str[i], '\0'};
            pdf_doc_write_text(chan->doc, ch);
        }
        chan->has_printed = true;
    } else {
        for (int i = 0; i < count; ++i) {
            vprinter_write_char(str[i]);
        }
    }
    return count;
}

static int vprinter_dev_putc(VDev *dev, int c) {
    char buf[2] = {(char)c, '\0'};
    if (dev && dev->priv) {
        VPrinterChannel *chan = (VPrinterChannel *)dev->priv;
        pdf_doc_write_text(chan->doc, buf);
        chan->has_printed = true;
    } else {
        vprinter_write_char((char)c);
    }
    return c;
}

static int vprinter_dev_puts(VDev *dev, const char *s) {
    if (!s) return 0;
    if (dev && dev->priv) {
        VPrinterChannel *chan = (VPrinterChannel *)dev->priv;
        pdf_doc_write_text(chan->doc, s);
        chan->has_printed = true;
    } else {
        vprinter_write_str(s);
    }
    return (int)runtime_strlen(s);
}

static int vprinter_dev_flush(VDev *dev) {
    if (dev && dev->priv) {
        VPrinterChannel *chan = (VPrinterChannel *)dev->priv;
        if (chan->has_printed || pdf_doc_has_content(chan->doc)) {
            pdf_doc_save_file(chan->doc, chan->target_filename);
        }
    } else {
        if (vprinter_has_output()) {
            vprinter_flush_pdf(NULL);
        }
    }
    return 0;
}

VDev vprinter_create_vdev(const char *dev_name) {
    VDev dev;
    runtime_memset(&dev, 0, sizeof(dev));

    dev.name = dev_name ? dev_name : "LPT1:";
    dev.dev_class = VDCLASS_PRINTER;
    dev.dev_version = "1.0";
    dev.dev_description = "Virtual Text-to-PDF Printer";
    dev.dev_req_caps = VDCAP_WRITE | VDCAP_STREAM;
    dev.dev_open = vprinter_dev_open;
    dev.dev_close = vprinter_dev_close;
    dev.dev_write = vprinter_dev_write;
    dev.ops.putc = vprinter_dev_putc;
    dev.ops.puts = vprinter_dev_puts;
    dev.ops.flush = vprinter_dev_flush;

    return dev;
}

