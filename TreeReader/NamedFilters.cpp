#include "NamedFilters.h"
#include "TreeFilterMaker.h"

#include <fstream>
#include <iomanip>

namespace TreeReader
{
   using namespace std;

   TreeFilterPtr NamedFilters::Get(const wstring& name) const
   {
      const auto pos = Filters.find(name);
      if (pos == Filters.end())
         return {};

      return pos->second;
   }

   void WriteNamedFilters(const filesystem::path& path, const NamedFilters& filters)
   {
      wofstream stream(path);
      WriteNamedFilters(stream, filters);
   }

   void WriteNamedFilters(wostream& stream, const NamedFilters& filters)
   {
      for (const auto& [name, filter] : filters.Filters)
         if (filter)
            stream << quoted(name) << L" : " << quoted(ConvertFiltersToText(filter)) << endl;
   }

   NamedFilters ReadNamedFilters(const filesystem::path& path)
   {
      wifstream stream(path);
      return ReadNamedFilters(stream);
   }

   NamedFilters ReadNamedFilters(wistream& stream)
   {
      NamedFilters filters;

      while (stream)
      {
         wstring name;
         wstring filterText;
         wchar_t column;
         stream >> skipws >> quoted(name) >> skipws >> column >> skipws >> quoted(filterText);
         if (name.empty())
            continue;

         TreeFilterPtr filter = ConvertTextToFilters(filterText);
         if (!filter)
            continue;

         filters.Filters[name] = filter;
      }

      return filters;
   }
}
