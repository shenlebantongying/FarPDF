#include "Document.h"
#include <QApplication>
#include <QDebug>
#include <iostream>
#include <mupdf/classes2.h>
#include <sstream>
using namespace mupdf;

Document::Document(const std::string& path) :
    m_doc(new FzDocument(path.c_str()))
{
    pageCount = m_doc->fz_count_pages();
    devicePixelRatio = qApp->devicePixelRatio();
    page_acc_h.push_back(0);
    for (int i = 0; i < pageCount; ++i) {
        pages.emplace_back(m_doc->fz_load_page(i));

        page_acc_h.emplace_back(page_acc_h.back() + pages.back().fz_bound_page().y1);

        // Note: Here i made an assumption that (x0,y0) of pdf file are
        // always (0,0) thus x1 value = width. Is this always true??
        max_page_width = std::max(pages.back().fz_bound_page().x1, max_page_width);
    }
    max_page_width += 50;
}

QPixmap Document::get_QPixmap_from_page_number(int n, float zoom_factor)
{
    zoom_factor = zoom_factor * devicePixelRatio;
    FzMatrix ctm = mupdf::fz_scale(zoom_factor, zoom_factor);

    FzPixmap pix = pages[n].fz_new_pixmap_from_page(ctm, FzColorspace(FzColorspace::Fixed_RGB), 0);

    QImage Parsed_Qimg(pix.samples(), pix.w(), pix.h(), pix.stride(), QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(Parsed_Qimg);

    // TODO: the performance implication of this??
    pixmap.setDevicePixelRatio(zoom_factor);
    return pixmap;
}

FzOutline Document::get_outline()
{
    return m_doc->fz_load_outline();
}

int Document::highlight_selection(int page_num, const QPointF& pointA, const QPointF& pointB, QList<QRectF>& hl_quads)
{
    FzStextOptions options {};
    std::unique_ptr<FzStextPage> temp_stext_page = std::make_unique<FzStextPage>(pages[page_num], options);

    FzQuad hits[500];

    auto n_of_quads = temp_stext_page->fz_highlight_selection(*QPointF_to_fz_point(pointA), *QPointF_to_fz_point(pointB), *hits, nelem(hits));
    for (int i = 0; i < n_of_quads; ++i) {
        hl_quads.append(fz_quad_to_QRectF(hits[i]));
    }
    return n_of_quads;
}

QString Document::get_selection_text(int page_num, const QPointF& pointA, const QPointF& pointB)
{
    FzStextOptions options {};
    std::unique_ptr<FzStextPage> temp_stext_page = std::make_unique<FzStextPage>(pages[page_num], options);

    auto x = QString::fromStdString(temp_stext_page->fz_copy_selection(*QPointF_to_fz_point(pointA), *QPointF_to_fz_point(pointB), 0));
    return x;
}

std::unique_ptr<FzPoint> Document::QPointF_to_fz_point(const QPointF& p)
{
    return std::make_unique<FzPoint>((float)p.x(), (float)p.y());
}

QRectF Document::fz_quad_to_QRectF(const FzQuad& q)
{
    QPointF tl = QPointF(q.ul.x, q.ul.y);
    QPointF br = QPointF(q.lr.x, q.lr.y);
    // QRectF(const QPointF &topLeft, const QPointF &bottomRight)
    return { tl, br };
}

std::string Document::get_metadata_string()
{
    char buf[100];

    std::stringstream result;

    m_doc->fz_lookup_metadata(FZ_META_INFO_TITLE, buf, sizeof buf);
    result << "Title: " << buf << std::endl;

    m_doc->fz_lookup_metadata(FZ_META_FORMAT, buf, sizeof buf);
    result << "Format: " << buf << std::endl;

    m_doc->fz_lookup_metadata(FZ_META_INFO_AUTHOR, buf, sizeof buf);
    result << "Author: " << buf << std::endl;

    m_doc->fz_lookup_metadata(FZ_META_INFO_CREATOR, buf, sizeof buf);
    result << "Creator: " << buf << std::endl;

    m_doc->fz_lookup_metadata(FZ_META_INFO_PRODUCER, buf, sizeof buf);
    result << "Producer: " << buf << std::endl;

    m_doc->fz_lookup_metadata(FZ_META_ENCRYPTION, buf, sizeof buf);
    result << "Encryption: " << buf << std::endl;

    return result.str();
}

float Document::get_page_height(int page_num)
{
    auto b = pages[page_num].fz_bound_page();
    return b.y1 - b.y0;
}

int Document::query_needle_at(const std::string& needle, int page_num, QList<QRectF>& hl_quads)
{

    std::vector<FzQuad> hits;
    hits.resize(500);

    int* hitnum = nullptr;

    auto n_of_quads = pages[page_num].fz_search_page(needle.c_str(), hitnum, *hits.data(), hits.size());
    for (int i = 0; i < n_of_quads; ++i) {
        hl_quads.append(fz_quad_to_QRectF(hits[i]));
    }
    return n_of_quads;
}
