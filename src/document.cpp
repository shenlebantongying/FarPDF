#include "document.h"
#include <QDebug>
#include <iostream>
#include <sstream>
document::document(const std::string & path) {
    m_doc = nullptr;
    ctx = fz_new_context(nullptr, nullptr, FZ_STORE_DEFAULT);

    fz_try(ctx) {
        fz_register_document_handlers(ctx);
        m_doc = fz_open_document(ctx, path.c_str());
    }
    fz_catch(ctx) {
        qFatal("%s", fz_caught_message(ctx));
    }

    pageCount = fz_count_pages(ctx, m_doc);
    max_page_width = 0;
    page_acc_h.push_back(0);
    for (int i = 0; i < pageCount; ++i) {
        pages.emplace_back(fz_load_page(ctx, m_doc, i));
        page_acc_h.emplace_back(page_acc_h.back() + fz_bound_page(ctx, pages.at(i)).y1);
        if (fz_bound_page(ctx, pages.at(i)).x1 > max_page_width) {
            // Note: Here i made an assumption that (x0,y0) of pdf file are
            // always (0,0) thus x1 value = width. Is this always true??
            max_page_width = fz_bound_page(ctx, pages.at(i)).x1;
        }
    }
    max_page_width += 50;
}

QPixmap document::get_QPixmap_from_page_number(int n, float zoom_factor) {

    auto ctm = fz_scale(zoom_factor, zoom_factor);
    fz_pixmap * pix = fz_new_pixmap_from_page(ctx, pages[n], ctm, fz_device_rgb(ctx), 0);

    // Without new, the QImg will be auto destroyed when goes out of scope
    // Note: The data (pix) will not be deleted!
    QImage Parsed_Qimg(pix->samples, pix->w, pix->h, pix->stride, QImage::Format_RGB888);
    auto pixmap = QPixmap::fromImage(Parsed_Qimg);

    // recycle allocated pixmap
    fz_drop_pixmap(ctx, pix);

    return pixmap;
}

fz_outline * document::get_outline() {
    return fz_load_outline(ctx, m_doc);
}

document::~document() {
    // TODO: does everything recycled?
    fz_drop_document(ctx, m_doc);
    fz_drop_context(ctx);
}
int document::highlight_selection(int page_num, const QPointF & pointA, const QPointF & pointB, QList<QRectF> & hl_quads) {

    auto temp_stext_page = fz_new_stext_page_from_page(ctx, pages[page_num], nullptr);

    fz_quad hits[500];

    auto n_of_quads = fz_highlight_selection(ctx,
                                             temp_stext_page,
                                             QPointF_to_fz_point(pointA),
                                             QPointF_to_fz_point(pointB),
                                             hits,
                                             nelem(hits));
    for (int i = 0; i < n_of_quads; ++i) {
        hl_quads.append(fz_quad_to_QRectF(hits[i]));
    }

    // Memory cleanup
    fz_drop_stext_page(ctx, temp_stext_page);

    return n_of_quads;
}

QString document::get_selection_text(int page_num, const QPointF & pointA, const QPointF & pointB) {
    auto temp_stext_page = fz_new_stext_page_from_page(ctx, pages[page_num], nullptr);
    auto x = QString(fz_copy_selection(ctx, temp_stext_page, QPointF_to_fz_point(pointA), QPointF_to_fz_point(pointB), 0));
    fz_drop_stext_page(ctx, temp_stext_page);
    return x;
}

fz_point document::QPointF_to_fz_point(const QPointF & p) {
    return fz_make_point((float)p.x(), (float)p.y());
}


QRectF document::fz_quad_to_QRectF(const fz_quad & q) {
    QPointF tl = QPointF(q.ul.x, q.ul.y);
    QPointF br = QPointF(q.lr.x, q.lr.y);
    //QRectF(const QPointF &topLeft, const QPointF &bottomRight)
    return {tl, br};
}

std::string document::get_metadata_string() {
    char buf[100];

    std::stringstream result;

    fz_lookup_metadata(ctx, m_doc, FZ_META_INFO_TITLE, buf, sizeof buf);
    result << "Title: " << buf << std::endl;
    fz_lookup_metadata(ctx, m_doc, FZ_META_FORMAT, buf, sizeof buf);
    result << "Format: " << buf << std::endl;
    fz_lookup_metadata(ctx, m_doc, FZ_META_INFO_AUTHOR, buf, sizeof buf);
    result << "Author: " << buf << std::endl;
    fz_lookup_metadata(ctx, m_doc, FZ_META_INFO_CREATOR, buf, sizeof buf);
    result << "Creator: " << buf << std::endl;
    fz_lookup_metadata(ctx, m_doc, FZ_META_INFO_PRODUCER, buf, sizeof buf);
    result << "Producer: " << buf << std::endl;
    fz_lookup_metadata(ctx, m_doc, FZ_META_ENCRYPTION, buf, sizeof buf);
    result << "Encryption: " << buf << std::endl;

    return result.str();
}

float document::get_page_height(int page_num) {
    auto b = fz_bound_page(ctx, pages[page_num]);
    return b.y1 - b.y0;
}

int document::query_needle_at(const std::string & needle, int page_num, QList<QRectF> & hl_quads) {

    fz_quad hits[500];
    int * hitnum = 0;

    auto n_of_quads = fz_search_page(ctx, pages[page_num], needle.c_str(), hitnum ,hits, nelem(hits));
    for (int i = 0; i < n_of_quads; ++i) {
        hl_quads.append(fz_quad_to_QRectF(hits[i]));
    }
    return n_of_quads;
}