#ifndef DOC_H
#define DOC_H

#include <mupdf/fitz.h>

// Qt related
#include <QList>
#include <QPixmap>
#include <QRectF>

// Extra
#include <string>


/**
 * @brief Bridging mupdf and Qt/C++ : All bits that related to mupdf belong to here! \n
 * Note: mupdf uses reference counting for memory management.
 */
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

    /**
     * @brief A very raw wrapper that will modify the last param
     * @param page_num
     * @param pointA De-scaled before passing
     * @param pointB
     * @param hl_quads Constructed by caller, and modify it directly.
     * @return Number of quads
     */
    int highlight_selection(int page_num, QPointF pointA, QPointF pointB, QList<QRectF> & hl_quads);
    QString get_selection_text(int page_num, QPointF pointA, QPointF pointB);

    std::string get_metadata_string();

private:
    fz_context * ctx;

    // Utils
    static fz_point QPointF_to_fz_point(QPointF p);
    static QRectF fz_quad_to_QRectF(fz_quad q);
};

#endif// DOC_H
