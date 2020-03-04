#pragma once

#include "TreeFilter.h"
#include "TreeFilterWidget.h"

#include <QtWidgets/qwidget.h>
#include <QtWidgets/qscrollarea.h>

#include <QtCore/qpoint.h>

class QDragEnterEvent;
class QDropEvent;
class QMouseEvent;
class QVBoxLayout;

namespace TreeReaderApp
{
   using TreeFilterPtr = TreeReader::TreeFilterPtr;

   struct TreeFilterListWidget : public QScrollArea
   {
      // Callback signature when the user wants to delete a filter.
      // Deleting the panel removes it from the list panel that contains it.
      using DeleteCallbackFunction = TreeFilterWidget::DeleteCallbackFunction;

      DeleteCallbackFunction DeleteCallback;

      // Create a tree filter list widget.
      TreeFilterListWidget(DeleteCallbackFunction callback = {}, QWidget* parent = nullptr);

      // Clears the list panel of all filters.
      void Clear();

      // Add a filter panel UI to the given list panel, with an optional deletion callback.
      TreeFilterWidget* AddTreeFilter(const TreeFilterPtr& filter, int index = -1);
      TreeFilterWidget* AddTreeFilter(TreeFilterWidget* filter, int index = -1);

      // Remove a filter panel.
      void RemoveTreeFilter(TreeFilterWidget* filter);

   protected:
      void dragEnterEvent(QDragEnterEvent* event) override;
      void dragLeaveEvent(QDragLeaveEvent* event) override;
      void dragMoveEvent(QDragMoveEvent* event) override;
      void dropEvent(QDropEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;

      TreeFilterWidget* FindWidgetAt(const QPoint& pt) const;

      QVBoxLayout* _availLayout = nullptr;
   };
}
