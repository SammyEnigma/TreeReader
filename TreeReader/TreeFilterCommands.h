#pragma once

#include "TextTree.h"
#include "TreeFilter.h"
#include "SimpleTreeReader.h"
#include "NamedFilters.h"
#include "UndoStack.h"

#include <memory>
#include <string>
#include <filesystem>

namespace TreeReader
{
   struct CommandsOptions
   {
      std::wstring OutputLineIndent = L"  ";

      ReadSimpleTextTreeOptions ReadOptions;

      bool operator!=(const CommandsOptions& other) const
      {
         return OutputLineIndent != other.OutputLineIndent
             || ReadOptions      != other.ReadOptions;
      }
   };

   struct CommandsContext
   {
      // Options.

      CommandsOptions Options;

      void SaveOptions(const std::filesystem::path& filename);
      void SaveOptions(std::wostream& stream);
      void LoadOptions(const std::filesystem::path& filename);
      void LoadOptions(std::wistream& stream);

      // Tree loading and saving.

      void SetInputFilter(const std::wstring& filterRegex);
      void SetInputIndent(const std::wstring& indentText);
      void SetOutputIndent(const std::wstring& indentText);

      std::wstring LoadTree(const std::filesystem::path& filename);
      void SaveFilteredTree(const std::filesystem::path& filename);
      bool IsFilteredTreeSaved() const { return _filteredWasSaved; }

      // Current filter.

      void SetFilter(const TreeFilterPtr& filter);
      const TreeFilterPtr& GetFilter() const { return _filter; }

      // Filtering.

      void ApplyFilterToTree(bool async = false);
      void ApplyFilterToTreeAsync() { ApplyFilterToTree(true); }
      void AbortAsyncFilter();
      bool IsAsyncFilterReady();

      void SearchInTree(const std::wstring& text);
      void SearchInTreeAsync(const std::wstring& text);
      void AbortAsyncSearch();
      bool IsAsyncSearchReady();

      // Named filters management.

      NamedFilterPtr NameFilter(const std::wstring& filterName);
      NamedFilterPtr NameFilter(const std::wstring& filterName, const TreeFilterPtr& filter);
      bool RemoveNamedFilter(const std::wstring& filterName);
      const NamedFilters& GetNamedFilters() const { return *_knownFilters; }

      void SaveNamedFilters(const std::filesystem::path& filename);
      void LoadNamedFilters(const std::filesystem::path& filename);

      // Current text tree and filtered tree.

      std::shared_ptr<TextTree> GetCurrentTree() const;
      std::shared_ptr<TextTree> GetFilteredTree() const;
      void PushFilteredAsTree();
      void PopTree();

      // Undo / redo.

      void ClearUndoStack();
      UndoStack& UndoRedo() { return _undoRedo; }

   protected:
      void DeadedFilters(std::any& data);
      void AwakenFilters(const std::any& data);
      void CommitFilterToUndo();
      void ApplySearchInTree(bool async);

      std::wstring _treeFileName;
      std::vector<std::shared_ptr<TextTree>> _trees;

      TreeFilterPtr _filter;

      std::wstring _filteredFileName;
      std::shared_ptr<TextTree> _filtered;
      bool _filteredWasSaved = false;
      AsyncFilterTreeResult _asyncFiltering;

      std::wstring _searchedText;
      std::shared_ptr<TextTree> _searched;
      AsyncFilterTreeResult _asyncSearching;

      std::shared_ptr<NamedFilters> _knownFilters = std::make_shared<NamedFilters>();

      UndoStack _undoRedo;

   };

}
