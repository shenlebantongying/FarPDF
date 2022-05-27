#ifndef DOC_H
#define DOC_H

#include <QPixmap>
#include <mupdf/fitz.h>
#include <string>

// Note: mupdf uses reference counting for memory management.
// To avoid problems, all fz_ related code should belong here.

class document {
public:
    explicit document(const std::string & path);
    ~document();
    QPixmap get_QPixmap_from_page_number(int n, float zoom_factor);

    fz_document * m_doc;

    fz_outline * get_outline();

    // Note: without render, this consumes very little memory
    std::vector<fz_page *> pages;

    // internally we always store page number as 0-indexed.
    // We only do the +1 when present them in UI.
    int pageCount;

    float max_page_width;

    // Accumulated page heights
    std::vector<float> page_acc_h;

private:
    fz_context * ctx;
};

#endif// DOC_H
