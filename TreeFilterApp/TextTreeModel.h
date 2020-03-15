#include "TextTree.h"

#include <QtCore/qabstractitemmodel.h>

#include <memory>

namespace TreeReaderApp
{
   using TextTreePtr = TreeReader::TextTreePtr;

   ////////////////////////////////////////////////////////////////////////////
   //
   // Tree model containing all lines of a text tree.

   struct TextTreeModel : QAbstractItemModel
   {
      TextTreePtr Tree;
      void Reset();

      QVariant data(const QModelIndex& index, int role) const override;
      QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
      QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
      QModelIndex parent(const QModelIndex& index) const override;
      int rowCount(const QModelIndex& parent = QModelIndex()) const override;
      int columnCount(const QModelIndex& parent = QModelIndex()) const override;
   };
}

