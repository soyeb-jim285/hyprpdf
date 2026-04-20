#include "outlinemodel.h"
#include "services/pdfdoc.h"
#include <QList>

static void buildNodes(const QVector<PdfDoc::TocEntry> &src,
                       OutlineModel::Node *parent, int depth) {
    for (const auto &e : src) {
        auto *n = new OutlineModel::Node;
        n->title = e.title;
        n->page = e.pageIndex;
        n->depth = depth;
        n->parent = parent;
        parent->children.append(n);
        buildNodes(e.children, n, depth + 1);
    }
}

OutlineModel::OutlineModel(QObject *parent)
    : QAbstractItemModel(parent), m_root(new Node) {
    m_root->depth = -1;
}

OutlineModel::~OutlineModel() {
    clearRoot();
    delete m_root;
}

void OutlineModel::clearRoot() {
    for (auto *c : m_root->children) {
        QList<Node*> stack; stack.push_back(c);
        while (!stack.isEmpty()) {
            Node *n = stack.takeLast();
            for (auto *k : n->children) stack.push_back(k);
            delete n;
        }
    }
    m_root->children.clear();
}

void OutlineModel::setDocument(QObject *doc) {
    beginResetModel();
    clearRoot();
    auto *pdf = qobject_cast<PdfDoc*>(doc);
    if (pdf) {
        buildNodes(pdf->toc(), m_root, 0);
    }
    endResetModel();
}

QModelIndex OutlineModel::index(int row, int col, const QModelIndex &parent) const {
    if (col != 0 || row < 0) return {};
    const Node *p = parent.isValid() ? static_cast<const Node*>(parent.internalPointer()) : m_root;
    if (row >= p->children.size()) return {};
    return createIndex(row, 0, p->children[row]);
}

QModelIndex OutlineModel::parent(const QModelIndex &child) const {
    if (!child.isValid()) return {};
    const Node *n = static_cast<const Node*>(child.internalPointer());
    if (!n->parent || n->parent == m_root) return {};
    Node *grand = n->parent->parent ? n->parent->parent : m_root;
    const int row = grand->children.indexOf(n->parent);
    return createIndex(row, 0, n->parent);
}

int OutlineModel::rowCount(const QModelIndex &parent) const {
    const Node *p = parent.isValid() ? static_cast<const Node*>(parent.internalPointer()) : m_root;
    return p->children.size();
}

int OutlineModel::columnCount(const QModelIndex &) const { return 1; }

QVariant OutlineModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) return {};
    const Node *n = static_cast<const Node*>(index.internalPointer());
    switch (role) {
        case TitleRole: case Qt::DisplayRole: return n->title;
        case PageRole: return n->page;
        case DepthRole: return n->depth;
    }
    return {};
}

QHash<int, QByteArray> OutlineModel::roleNames() const {
    return {
        {TitleRole, "title"},
        {PageRole,  "page"},
        {DepthRole, "depth"}
    };
}
