#include "document.h"
#include <QDebug>
#include <iostream>
document::document(const std::string & path) {
    ctx = fz_new_context(nullptr, nullptr, FZ_STORE_DEFAULT);
    ctm = fz_scale(1, 1);

    fz_try(ctx) {
        fz_register_document_handlers(ctx);
        m_doc = fz_open_document(ctx, path.c_str());
    }
    fz_catch(ctx) {
        qDebug() << fz_caught_message(ctx);
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

QPixmap document::get_QPixmap_from_page_number(int n) {
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
