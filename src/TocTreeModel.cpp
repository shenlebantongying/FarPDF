#include "TocTreeModel.h"
#include <QColor>

TocItem::TocItem(const QList<QVariant>& data, TocItem* parentItem) :
    m_itemData(data),
    m_parent_item(parentItem)
{
}

void TocItem::appendChild(TocItem* item)
{
    m_childItems.append(item);
}

TocItem* TocItem::child(int row)
{
    if (row < 0 || row >= m_childItems.size()) {
        return nullptr;
    }
    return m_childItems.at(row);
}

qsizetype TocItem::childCount() const
{
    return m_childItems.count();
}

qsizetype TocItem::columnCount() const
{
    return m_itemData.count();
}

QVariant TocItem::data(int column) const
{
    if (column < 0 || column >= m_itemData.size()) {
        return {};
    }
    return m_itemData.at(column);
}

TocItem* TocItem::parentItem()
{
    return m_parent_item;
}

int TocItem::row() const
{
    if (m_parent_item != nullptr) {
        return m_parent_item->m_childItems.indexOf(const_cast<TocItem*>(this));
    }
    return 0;
}

constexpr int n_of_history = 7;

TocTreeModel::TocTreeModel(fz_outline* outline, QObject* parent) :
    QAbstractItemModel(parent)
{
    update_outline(outline);
    user_toc_jumping_history = QQueue<QModelIndex>();
    user_toc_jumping_history.fill(QModelIndex(), n_of_history);
}

void TocTreeModel::update_outline(fz_outline* outline)
{
    beginResetModel();
    rootItem = new TocItem({ "Titles" });
    setupModelData(outline, rootItem);
    endResetModel();
}

int TocTreeModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid()) {
        return static_cast<TocItem*>(parent.internalPointer())->columnCount();
    }
    return rootItem->columnCount();
}

// TODO: exactly where this is used?
QVariant TocTreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    TocItem* i = static_cast<TocItem*>(index.internalPointer());

    if (role == Qt::DisplayRole) {
        return i->data(index.column());
    };

    if (Qt::BackgroundRole == role) {
        auto rank = (int)user_toc_jumping_history.indexOf(index);
        auto slice = 255 / (n_of_history + 5);
        if (rank >= 0) {
            return QVariant(QColor(61, 174, 233, slice * rank)); // Note: rank is 0-based
        };
    }

    return {};
}

void TocTreeModel::add_user_toc_jumping_history(const QModelIndex& data)
{
    // check if data already exist in history, if yes, remove it.
    user_toc_jumping_history.removeOne(data);

    if (user_toc_jumping_history.size() > n_of_history) {
        user_toc_jumping_history.dequeue();
    }

    user_toc_jumping_history.enqueue(data);
};

int TocTreeModel::page_num_from_index(const QModelIndex& index)
{
    if (!index.isValid()) {
        return {};
    }

    TocItem* i = static_cast<TocItem*>(index.internalPointer());

    return i->page_number;
}

Qt::ItemFlags TocTreeModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }
    return QAbstractItemModel::flags(index);
}

QModelIndex TocTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent)) {
        return {};
    }

    TocItem* parentItem = nullptr;

    if (!parent.isValid()) {
        parentItem = rootItem;
    } else {
        parentItem = static_cast<TocItem*>(parent.internalPointer());
    }

    TocItem* childItem = parentItem->child(row);

    if (childItem != nullptr) {
        return createIndex(row, column, childItem);
    }
    return {};
}

QModelIndex TocTreeModel::parent(const QModelIndex& index) const
{
    if (!index.isValid()) {
        return {};
    }

    TocItem* childItem = static_cast<TocItem*>(index.internalPointer());
    TocItem* parentItem = childItem->parentItem();

    if (parentItem == rootItem) {
        return {};
    }

    return createIndex(parentItem->row(), 0, parentItem);
}

int TocTreeModel::rowCount(const QModelIndex& parent) const
{
    TocItem* parentItem = nullptr;
    if (parent.column() > 0) {
        return 0;
    }
    if (!parent.isValid()) {
        parentItem = rootItem;
    } else {
        parentItem = static_cast<TocItem*>(parent.internalPointer());
    }
    return parentItem->childCount();
}

void TocTreeModel::setupModelData(fz_outline* outline, TocItem* parent)
{
    // Use a named lambda to recursively read the fz_outline structure

    std::function<void(fz_outline*, TocItem*)> rec_outline;
    rec_outline = [&rec_outline](fz_outline* outline, TocItem* parent) {
        for (fz_outline* o = outline; o != nullptr; o = o->next) {
            QList<QVariant> columnData;
            columnData << QString::fromLatin1(o->title);
            auto* temp_item = new TocItem(columnData, parent);

            // Note: inside mupdf, the page num is 0-based, this page_number will be
            // accessed as human-readable way, thus we plus 1.
            temp_item->page_number = o->page.page + 1;

            parent->appendChild(temp_item);

            if (o->down != nullptr) {
                rec_outline(o->down, temp_item);
            }
        }
    };

    rec_outline(outline, parent);
}
