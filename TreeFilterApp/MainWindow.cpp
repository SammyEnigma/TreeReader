#include "MainWindow.h"
#include "MainWindow.h"
#include "TextTreeModel.h"
#include "QtUtilities.h"
#include "TreeFilterMaker.h"

#include <QtGui/qpainter.h>
#include <QtGui/qevent.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qerrormessage.h>
#include <QtWidgets/qtoolbar.h>
#include <QtWinExtras/qwinfunctions.h>

namespace TreeReaderApp
{
   using namespace TreeReader;
   using namespace std;

   namespace
   {
      namespace L
      {
         inline const wchar_t* t(const wchar_t* text)
         {
            return text;
         }
      }
   }

   /////////////////////////////////////////////////////////////////////////
   //
   // Create the main window.

   MainWindow::MainWindow(const MainWindowIcons& icons)
   {
      BuildUI(icons);
      FillUI();
      ConnectUI(icons);
   }

   /////////////////////////////////////////////////////////////////////////
   //
   // Create the UI elements.

   void MainWindow::BuildUI(const MainWindowIcons& icons)
   {
      QToolBar* toolbar = new QToolBar();
         toolbar->setIconSize(QSize(32, 32));

         _loadTreeAction = CreateAction(L::t(L"Load Tree"), icons.TextTreeOpen, QKeySequence(QKeySequence::StandardKey::Open));
         _loadTreeButton = CreateToolButton(_loadTreeAction);
         toolbar->addWidget(_loadTreeButton);

         _saveTreeAction = CreateAction(L::t(L"Save Tree"), icons.TextTreeSave, QKeySequence(QKeySequence::StandardKey::Save));
         _saveTreeButton = CreateToolButton(_saveTreeAction);
         toolbar->addWidget(_saveTreeButton);

         toolbar->addSeparator();

         _undoAction = CreateAction(L::t(L"Undo"), icons.Undo, QKeySequence(QKeySequence::StandardKey::Undo));
         _undoButton = CreateToolButton(_undoAction);
         _undoAction->setEnabled(false);
         toolbar->addWidget(_undoButton);

         _redoAction = CreateAction(L::t(L"Redo"), icons.Redo, QKeySequence(QKeySequence::StandardKey::Redo));
         _redoButton = CreateToolButton(_redoAction);
         _redoAction->setEnabled(false);
         toolbar->addWidget(_redoButton);

      _layersDock = new QDockWidget(QString::fromWCharArray(L::t(L"Layers")));
         _layersDock->setFeatures(QDockWidget::DockWidgetFeature::DockWidgetFloatable | QDockWidget::DockWidgetFeature::DockWidgetMovable);
         QWidget* filters_container = new QWidget();
         QVBoxLayout* filters_layout = new QVBoxLayout(filters_container);

         _filtersList = new FiltersEditor(filters_container, icons.FilterCopy, icons.FilterAdd, icons.FilterDelete, icons.FilterMoveUp, icons.FilterMoveDown);
         filters_layout->addWidget(_filtersList);

         _layersDock->setWidget(filters_container);

      _treeView = new QTreeView;
      _treeView->setUniformRowHeights(true);
      _treeView->setHeaderHidden(true);
      _treeView->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

      _cmdDock = new QDockWidget(QString::fromWCharArray(L::t(L"Commands")));
         _cmdDock->setFeatures(QDockWidget::DockWidgetFeature::DockWidgetFloatable | QDockWidget::DockWidgetFeature::DockWidgetMovable);
         QWidget* cmd_container = new QWidget();
         QVBoxLayout* cmd_layout = new QVBoxLayout(cmd_container);

         _cmdLine = new QLineEdit(cmd_container);
         cmd_layout->addWidget(_cmdLine);

         _cmdDock->setWidget(cmd_container);

      setCentralWidget(_treeView);
      addToolBar(toolbar);
      addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea, _layersDock);
      addDockWidget(Qt::DockWidgetArea::BottomDockWidgetArea, _cmdDock);
      setWindowIcon(QIcon(QtWin::fromHICON((HICON)::LoadImage(GetModuleHandle(0), MAKEINTRESOURCE(icons.AppIcon), IMAGE_ICON, 256, 256, 0))));
   }

   /////////////////////////////////////////////////////////////////////////
   //
   // Connect the signals of the UI elements.

   void MainWindow::ConnectUI(const MainWindowIcons& icons)
   {
      /////////////////////////////////////////////////////////////////////////
      //
      // Undo / redo actions.

      _undoAction->connect(_undoAction, &QAction::triggered, [&]()
      {
         _undoStack.Undo();
         UpdateUndoRedoActions();
      });

      _redoAction->connect(_redoAction, &QAction::triggered, [&]()
      {
         _undoStack.Redo();
         UpdateUndoRedoActions();
      });

      /////////////////////////////////////////////////////////////////////////
      //
      // Load / save actions.

      _loadTreeAction->connect(_loadTreeAction, &QAction::triggered, [self=this]()
      {
         if (!self->SaveIfRequired(L::t(L"load a text tree"), L::t(L"loading a text tree")))
            return;

         filesystem::path path = AskOpen(L::t(L"Load Text Tree"), L::t(L"txt"), self);
         self->_data.LoadTree(path);
         self->_data.ApplyFilterToTree();

         if (self->_data.Trees.size() == 0)
            return;

         self->FillTextTreeUI();
      });

      _saveTreeAction->connect(_saveTreeAction, &QAction::triggered, [self=this]()
      {
         self->SaveFilteredTree();
      });

      /////////////////////////////////////////////////////////////////////////
      //
      // The filter list UI call-backs.

      _filtersList->FiltersChanged = [self=this](const FiltersEditor::Filters& filters)
      {
         self->CommitToUndo();
         self->_data.Filter = self->_filtersList->GetEdited();
      };

      _filtersList->SelectionChanged = [self=this](const FiltersEditor::Filters& filters)
      {
         // Nothing...
      };

      _filtersList->NewFilterRequested = [self=this]()
      {
         self->RequestNewFilter();
      };

      /////////////////////////////////////////////////////////////////////////
      //
      // Command line-edit.

      _cmdLine->connect(_cmdLine, &QLineEdit::editingFinished, [self=this]()
      {
         QString text = self->_cmdLine->text();
         wstring result = ParseCommands(text.toStdWString(), self->_data);

         self->FillTextTreeUI();
         self->FillFiltersUI();
      });
   }

   /////////////////////////////////////////////////////////////////////////
   //
   // Fill the UI with the intial data.

   void MainWindow::FillUI()
   {
      try
      {
         _data.KnownFilters = ReadNamedFilters(L"filters.txt");
         ClearUndoStack();
      }
      catch (const exception &)
      {
         // Ignore.
      }
   }

   void MainWindow::FillTextTreeUI()
   {
      shared_ptr<TextTree> newTree;
      if (_data.Filtered)
      {
         newTree = _data.Filtered;
      }
      else if (_data.Trees.size() > 0)
      {
         newTree = _data.Trees.back();
      }

      if (!_treeView->model() || !dynamic_cast<TextTreeModel*>(_treeView->model()) || dynamic_cast<TextTreeModel*>(_treeView->model())->Tree != newTree)
      {
         TextTreeModel* model = new TextTreeModel;
         model->Tree = newTree;
         _treeView->setModel(model);
      }

      _layersDock->setWindowTitle(QString::fromWCharArray(L::t(L"Filters: ")) + QString::fromWCharArray(_data.TreeFileName.c_str()));

      CommitToUndo();
   }

   void MainWindow::FillFiltersUI()
   {
      _filtersList->SetEdited(_data.Filter);
   }

   /////////////////////////////////////////////////////////////////////////
   //
   // Closing and saving.

   void MainWindow::closeEvent(QCloseEvent* ev)
   {
      if (SaveIfRequired(L::t(L"close the window"), L::t(L"closing the window")))
      {
         QWidget::closeEvent(ev);
         try
         {
            WriteNamedFilters(L"filters.txt", _data.KnownFilters);
         }
         catch (const exception&)
         {
            // Ignore.
         }
      }
      else
      {
         ev->ignore();
      }
   }

   bool MainWindow::SaveIfRequired(const wstring& action, const wstring& actioning)
   {
      if (_undoStack.HasUndo() && _data.Trees.size() > 0 && _data.Filter)
      {
         YesNoCancel answer = AskYesNoCancel(
            L::t(L"Unsaved Text Tree Warning"),
            wstring(L::t(L"The current tree has not been saved.\nDo you want to save it before ")) + actioning + L::t(L"?"),
            this);
         if (answer == YesNoCancel::Cancel)
            return false;
         else if (answer == YesNoCancel::Yes)
            if (!SaveFilteredTree())
               return false;
      }

      return true;
   }

   bool MainWindow::SaveFilteredTree()
   {
      if (_data.Trees.size() == 0)
         return true;

      filesystem::path path = AskSave(L::t(L"Save Text Tree"), L::t(L"txt"), this);
      _data.SaveTree(path);

      return true;
   }


   /////////////////////////////////////////////////////////////////////////
   //
   // Undo / redo tool-bar buttons.

   void MainWindow::DeadedFilters(any& data)
   {
      data = ConvertFiltersToText(_data.Filter);
   }

   void MainWindow::AwakenFilters(const any& data)
   {
      _data.Filter = ConvertTextToFilters(any_cast<wstring>(data), _data.KnownFilters);;

      FillFiltersUI();
   }

   void MainWindow::AwakenToEmptyFilters()
   {
      _data.Filter = nullptr;

      FillFiltersUI();
   }

   void MainWindow::ClearUndoStack()
   {
      _undoStack.Clear();
      // Note: allow undoing back to an empty filter list. To enable this, there must be an empty initial commit.
      _undoStack.Commit({ 0, nullptr, [self = this](const any&) { self->AwakenToEmptyFilters(); } });
   }

   void MainWindow::CommitToUndo()
   {
      _undoStack.Commit(
      {
         ConvertFiltersToText(_data.Filter),
         [self=this](any& data) { self->DeadedFilters(data); },
         [self=this](const any& data) { self->AwakenFilters(data); }
      });
      UpdateUndoRedoActions();
   }

   void MainWindow::UpdateUndoRedoActions()
   {
      _undoAction->setEnabled(_undoStack.HasUndo());
      _redoAction->setEnabled(_undoStack.HasRedo());
   }

   /////////////////////////////////////////////////////////////////////////
   //
   // Layer manipulations.

   void MainWindow::RequestNewFilter()
   {
      // TODO insert a new filter in filter tree...
      //_data.Filter = newFilter;
      _data.ApplyFilterToTree();
      FillFiltersUI();
      FillTextTreeUI();
      CommitToUndo();
   }

}