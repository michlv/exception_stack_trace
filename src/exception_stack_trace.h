/*
    Copyright (c) 2016 Vladimir Michl

    This file is part of Exception Stack Trace library.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <string>
#include <stdexcept>

namespace info {
  class StackTrace {
  public:
    static const int stack_size_max = 64;
    struct WalkSymbols {
      virtual void operator()(const char *sname, void *saddr, void *addr, const char *fname) = 0;
    };
    
    StackTrace(const int suppressTopXSymbols=1, const char *aName=NULL);
    int getRaw(void * const *&stacktrace) const;
    const char *getName() const ;
    void walkSymbols(WalkSymbols &callBack) const;
    std::string getSymbols() const;
    std::string getSimpleStackTrace() const;

  private:
    const int suppress_top_x_symbols;
    const char *name;
    int size;
    void *stack[stack_size_max];
  };
}

namespace exceptionstacktrace {
  const info::StackTrace *getStackTrace(const void *exception);
  const info::StackTrace *getStackTrace(const std::exception &exception);
  std::string get_stack_trace_names(const void *exception);
  std::string get_stack_trace_names(const std::exception &exception);
}
