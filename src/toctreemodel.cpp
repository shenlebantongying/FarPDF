#include "toctreemodel.h"
#include <QColor>

toc_item::toc_item(const QList<QVariant> & data, toc_item * parentItem)
    : m_itemData(data),
      m_parent_item(parentItem) {
}

void toc_item::appendChild(toc_item * item) {
    m_childItems.append(item);
}

toc_item * toc_item::child(int row) {
    if (row < 0 || row >= m_childItems.size())
        return nullptr;
    return m_childItems.at(row);
}

int toc_item::childCount() const {
    return m_childItems.count();
}

int toc_item::columnCount() const {
    return m_itemData.count();
}

QVariant toc_item::data(int column) const {
    if (column < 0 || column >= m_itemData.size())
        return {};
    return m_itemData.at(column);
}

toc_item * toc_item::parentItem() {
    return m_parent_item;
}

int toc_item::row() const {
    if (m_parent_item)
        return m_parent_item->m_childItems.indexOf(const_cast<toc_item *>(this));
    return 0;
}

toc_item::~toc_item() = default;

constexpr int n_of_history = 7;

tocTreeModel::tocTreeModel(fz_outline * outline, QObject * parent)
    : QAbstractItemModel(parent) {
    update_outline(outline);
    user_toc_jumping_history = QQueue<QModelIndex>();
    user_toc_jumping_history.fill(QModelIndex(), n_of_history);
}

void tocTreeModel::update_outline(fz_outline * outline) {
    beginResetModel();
    rootItem = new toc_item({"Titles"});
    setupModelData(outline, rootItem);
    endResetModel();
}

int tocTreeModel::columnCount(const QModelIndex & parent) const {
    if (parent.isValid())
        return static_cast<toc_item *>(parent.internalPointer())->columnCount();
    return rootItem->columnCount();
}


// TODO: exactly where this is used?
QVariant tocTreeModel::data(const QModelIndex & index, int role) const {
    if (!index.isValid())
        return {};

    toc_item * i = static_cast<toc_item *>(index.internalPointer());

    if (role == Qt::DisplayRole) {
        return i->data(index.column());
    };

    if (Qt::BackgroundRole == role) {
        auto rank = (int)user_toc_jumping_history.indexOf(index);
        auto slice = 255 / (n_of_history + 5);
        if (rank >= 0) {
            return QVariant(QColor(61, 174, 233, slice * rank));// Note: rank is 0-based
        };
    }

    return {};
}

void tocTreeModel::add_user_toc_jumping_history(const QModelIndex & data) {
    // check if data already exist in history, if yes, remove it.
    user_toc_jumping_history.removeOne(data);

    if (user_toc_jumping_history.size() > n_of_history) {
        user_toc_jumping_history.dequeue();
    }

    user_toc_jumping_history.enqueue(data);
};

int tocTreeModel::page_num_from_index(const QModelIndex & index) {
    if (!index.isValid())
        return {};

    toc_item * i = static_cast<toc_item *>(index.internalPointer());

    return i->page_number;
}

Qt::ItemFlags tocTreeModel::flags(const QModelIndex & index) const {
    if (!index.isValid())
        return Qt::NoItemFlags;
    return QAbstractItemModel::flags(index);
}

QModelIndex tocTreeModel::index(int row, int column, const QModelIndex & parent) const {
    if (!hasIndex(row, column, parent))
        return {};

    toc_item * parentItem = nullptr;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<toc_item *>(parent.internalPointer());

    toc_item * childItem = parentItem->child(row);

    if (childItem)
        return createIndex(row, column, childItem);
    return {};
}

QModelIndex tocTreeModel::parent(const QModelIndex & index) const {
    if (!index.isValid())
        return {};

    toc_item * childItem = static_cast<toc_item *>(index.internalPointer());
    toc_item * parentItem = childItem->parentItem();

    if (parentItem == rootItem)
        return {};

    return createIndex(parentItem->row(), 0, parentItem);
}

int tocTreeModel::rowCount(const QModelIndex & parent) const {
    toc_item * parentItem = nullptr;
    if (parent.column() > 0)
        return 0;
    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<toc_item *>(parent.internalPointer());
    return parentItem->childCount();
}

void tocTreeModel::setupModelData(fz_outline * outline, toc_item * parent) {
    // Use a named lambda to recursively read the fz_outline structure

    std::function<void(fz_outline *, toc_item *)> rec_outline;
    rec_outline = [&rec_outline](fz_outline * outline, toc_item * parent) {
        for (fz_outline * o = outline; o != nullptr; o = o->next) {
            QList<QVariant> columnData;
            columnData << o->title;
            auto temp_item = new toc_item(columnData, parent);

            // Note: inside mupdf, the page num is 0-based, this page_number will be
            // accessed as human-readable way, thus we plus 1.
            temp_item->page_number = o->page + 1;

            parent->appendChild(temp_item);

            if (o->down != nullptr) {
                rec_outline(o->down, temp_item);
            }
        }
    };

    rec_outline(outline, parent);
}

tocTreeModel::~tocTreeModel() = default;
