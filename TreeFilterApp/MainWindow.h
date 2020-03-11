#pragma once

#include "FilterEditor.h"
#include "TreeFilterListWidget.h"
#include "QWidgetScrollListWidget.h"

#include "TreeFilterCommands.h"

#include <QtWidgets/qmainwindow.h>

class QToolButton;
class QAction;
class QTreeView;
class QDockWidget;
class QTimer;
class QLineEdit;

namespace TreeReaderApp
{
   using TreeFilter = TreeReader::TreeFilter;
   using TreeFilterPtr = TreeReader::TreeFilterPtr;
   using CommandsContext = TreeReader::CommandsContext;
   using UndoStack = TreeReader::UndoStack;
   using QWidgetScrollListWidget = QtAdditions::QWidgetScrollListWidget;

   ////////////////////////////////////////////////////////////////////////////
   //
   // The main window of the tree filter app.

   class MainWindow : public QMainWindow
   {
   public:
      // Create the main window.
      MainWindow();

   protected:
      // Create the UI elements.
      void BuildUI();

      // Connect the signals of the UI elements.
      void ConnectUI();

      // Fill the UI with the intial data.
      void FillUI();
      void FillTextTreeUI();
      void FillFilterEditorUI();
      void FillAvailableFiltersUI();

      // Undo / redo tool-bar buttons.
      void UpdateUndoRedoActions();

      // Closing, loading and saving.
      void closeEvent(QCloseEvent* ev);
      bool SaveIfRequired(const std::wstring& action, const std::wstring& actioning);
      void LoadTree();
      bool SaveFilteredTree();

      // Main window state.
      void SaveState();
      void LoadState();

      // Tree filtering.
      void FilterTree();
      void verifyAsyncFiltering();
      void SearchInTree(const QString& text);
      void NameFilter();
      void AddNamedFilterToAvailable(const TreeFilterPtr& filter);

      // Options.
      void OpenOptions();

      // Data.
      CommandsContext _data;

      // Toolbar buttons.
      QAction* _undoAction = nullptr;
      QToolButton* _undoButton = nullptr;

      QAction* _redoAction = nullptr;
      QToolButton* _redoButton = nullptr;

      QAction* _loadTreeAction = nullptr;
      QToolButton* _loadTreeButton = nullptr;

      QAction* _saveTreeAction = nullptr;
      QToolButton* _saveTreeButton = nullptr;

      QAction* _applyFilterAction = nullptr;
      QToolButton* _applyFilterButton = nullptr;

      QAction* _nameFilterAction = nullptr;
      QToolButton* _nameFilterButton = nullptr;

      QAction* _optionsAction = nullptr;
      QToolButton* _optionsButton = nullptr;

      // UI elements.
      QTreeView* _treeView = nullptr;
      QLineEdit* _simpleSearch = nullptr;
      FilterEditor* _filterEditor = nullptr;
      TreeFilterListWidget* _availableFiltersList = nullptr;
      QWidgetScrollListWidget* _scrollFiltersList = nullptr;
      QTimer* _filteringTimer = nullptr;
   };
}

// vim: sw=3 : sts=3 : et : sta : 
