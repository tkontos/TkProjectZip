// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//   are changed infrequently
//

#pragma once

// Change these values to use different versions
#define WINVER    0x0500
#define _WIN32_WINNT 0x0501
#define _WIN32_IE 0x0501
#define _RICHEDIT_VER   0x0200

#define  _WTL_NO_UNION_CLASSES
#define  _WTL_NO_CSTRING
#define  _WTL_NO_WTYPES
#define  _ATL_USE_DDX_FLOAT

#include <atlbase.h>
#include <atlwin.h>
#include <atltypes.h>
#include <atlstr.h>
#include <atlapp.h>

extern CAppModule _Module;

#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>
#include <atlctrlx.h>
#include <atlmisc.h>
#include <atlddx.h>
#include <atlcomtime.h>

#include <xmllite.h>

#include <string>
#include <map>
#include <set>
#include <list>
#include <vector>
#include <queue>
#include <stack>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>

typedef std::list<std::string>  StrList;

namespace std
{
   typedef basic_string<TCHAR>   tstring;
}

// PPL
#include <ppl.h>
#include <concurrent_vector.h>
#include <concurrent_queue.h>
#include <agents.h>
namespace ConcRT = Concurrency;

#include "StreamingException.h"
#include "MruCombo.h"
#include "TkSpinHelper.h"

//#define BOOST_FILESYSTEM_VERSION    2
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/Tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>

namespace bfs = boost::filesystem;     // namespace alias

#include "../UtilCommon/ModuleVer.h"
#include "rsettings.h"

inline void COM_VERIFY(HRESULT hr)
{
   if (FAILED(hr))
      throw hr;
}

inline bool FileExists( const char *pszFileName)
{
   return GetFileAttributesA(pszFileName) != INVALID_FILE_ATTRIBUTES;
}
inline bool FileExists( const wchar_t *pszFileName)
{
   return GetFileAttributesW(pszFileName) != INVALID_FILE_ATTRIBUTES;
}

inline BOOL IsShiftKeyPressed()
{
   return ::GetAsyncKeyState ( VK_SHIFT ) < 0;
}

