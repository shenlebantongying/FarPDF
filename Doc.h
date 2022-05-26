#ifndef DOC_H
#define DOC_H

#include <mupdf/fitz.h>
#include <string>
#include <QPixmap>


// Note: mupdf uses reference counting for memory management.
// To avoid problems, all fz_ related code should belong here.

class Doc
{
public:
    explicit Doc(const std::string &path);

    QPixmap get_QPixmap_from_page_number(int n);

    fz_document * m_doc;

    // Note: without render, this consumes very little memory
    std::vector<fz_page*> pages;

    // internally we always store page number as 0-indexed.
    // We only do the +1 when present them in UI.
    int pageCount;

    float max_page_width;

    // Accumulated page heights
    std::vector<float> page_acc_h;


private:
    fz_context * ctx;
    fz_matrix ctm{};


};

#endif // DOC_H
