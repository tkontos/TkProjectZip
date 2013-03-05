#include "StdAfx.h"
#include "SolutionHelper.h"
#include "VcprojFileHelper.h"
#include "ZipUnzip/Zipper.h"

CSolutionHelper::CSolutionHelper(const CProjZipSettings &pzs)
   : m_pzs(pzs)
{
}

CSolutionHelper::~CSolutionHelper(void)
{
}

bool CSolutionHelper::LoadSolution( const TCHAR *pszFileName_)
{
   m_files.clear();
   m_exclFiles.clear();

   USES_CONVERSION;
   const char *pszFileName = T2CA(pszFileName_);

   if (!LoadSolutionImpl(pszFileName))
      return false;

   m_files.push_back(pszFileName);
   m_slnPath = bfs::path(pszFileName);
   FixFilePaths();
   for( StrList::const_iterator iter = m_solFiles.begin(); iter != m_solFiles.end(); ++iter)
   {
      m_files.push_back(*iter);
      if (boost::iends_with(*iter, ".xml"))
         LoadAdditionalFiles( iter->c_str());
   }

   if (!LoadProjectFiles())
      return false;

   std::for_each(m_files.begin(), m_files.end(),
         []( std::string &s)
         {
            char sz[_MAX_PATH];
            PathCanonicalizeA(sz, s.c_str());
            s = sz;
         }
   );

   RemoveDuplicateFiles();
   RemoveExcludedFiles();
   return true;
}

bool CSolutionHelper::LoadSolutionImpl( const char *pszFileName)
{
   m_projFiles.clear();
   m_solFiles.clear();

   std::ifstream ifs(pszFileName);
   char sz[1024] = {0};

   bool bInProject = false;
   bool bInSolItems = false;
   while( ifs.getline(sz, _countof(sz)))
   {
      static char szStart[] = "Project(";
      static char szEnd  [] = "EndProject";

      if (strstr(sz, szStart) == sz)
      {
         if (bInProject)
            return false;
         bInProject = true;
         if (!HandleProjectLine(sz))
            return false;
      }
      else if (strstr(sz, szEnd) == sz)
      {
         if (!bInProject)
            return false;
         bInProject = false;
      }
      else if (bInProject)
      {
         static char szStart[] = "ProjectSection(";
         static char szEnd  [] = "EndProjectSection";

         if (strstr(sz, szStart) != NULL)
         {
            if (bInSolItems)
               return false;
            bInSolItems = true;
         }
         else if (strstr(sz, szEnd) != NULL)
         {
            if (!bInSolItems)
               return false;
            bInSolItems = false;
         }
         else if (bInSolItems)
         {
            HandleSolutionItem(sz);
         }
      }
   }

   return !bInProject && !bInSolItems;
}

bool CSolutionHelper::HandleProjectLine( char *pszLine)
{
   std::vector<std::string> sl;
   char *pc = 0;
   for( int i=0; i<10; ++i)
   {
      const char szSep[] = "=,";
      char *p = strtok_s(i == 0 ? pszLine : NULL, szSep, &pc);
      if (p == 0)
         break;
      std::string s(p);
      boost::trim_if(s, [](char ch){ return ch == ' ' || ch == '"'; });
      sl.push_back(s);
   }

   if (sl.size() < 3)
      return false;

   if (sl[1] == sl[2])
      return true;

   m_projFiles.push_back(sl[2]);
   return true;
}

bool CSolutionHelper::HandleSolutionItem( char *pszLine)
{
   std::vector<std::string> sl;
   char *pc = 0;
   for( int i=0; i<10; ++i)
   {
      const char szSep[] = "=";
      char *p = strtok_s(i == 0 ? pszLine : NULL, szSep, &pc);
      if (p == 0)
         break;
      std::string s(p);
      boost::trim_if(s, [](char ch){ return ch == ' ' || ch == '"'; });
      sl.push_back(s);
   }

   if (sl.size() != 2 || sl[0] == sl[1])
      return false;

   m_solFiles.push_back(sl[1]);
   return true;
}

void CSolutionHelper::FixFilePaths()
{
   auto fpf = [this]( std::string &s) {
      bfs::path projPath = this->m_slnPath.parent_path() / s;
      s = projPath.string();
   };

   std::for_each(m_solFiles.begin() , m_solFiles.end()   , fpf);
   std::for_each(m_projFiles.begin(), m_projFiles.end()  , fpf);
}

bool CSolutionHelper::LoadProjectFiles()
{
   bfs::path slnDirPath = m_slnPath.parent_path();
   for( StrList::const_iterator iter = m_projFiles.begin(); iter != m_projFiles.end(); ++iter)
   {
      std::string sfn = *iter;

      bfs::path p = bfs::path(sfn);
      if (!boost::istarts_with(p.string(), slnDirPath.string()))
      {
         m_exclFiles.push_back(ExclData(p.string(), ExclData::ExR_NotInTree));
         continue;
      }

      CVcprojFileHelper vcp;
      if (FAILED(vcp.LoadProjFile(iter->c_str())))
         return false;

      m_files.push_back(sfn);

      std::string sfnf = sfn + ".filters";
      if (FileExists(sfnf.c_str()))
      {
         m_files.push_back(sfnf);
      }

      StrList sl;
      vcp.GetFileList(&sl);
      std::copy(sl.begin(), sl.end(), std::back_inserter(m_files));
   }
   return true;
}

void  CSolutionHelper::LoadAdditionalFiles( const char *pszXmlFile)
{
   USES_CONVERSION;

   CComPtr<IXmlReader> reader;
   COM_VERIFY(CreateXmlReader(__uuidof(IXmlReader), reinterpret_cast<void**>(&reader), 0));

   CComPtr<IStream> stream;
   COM_VERIFY(SHCreateStreamOnFileA(pszXmlFile, STGM_READ | STGM_SHARE_DENY_NONE, &stream));
   COM_VERIFY(reader->SetInput(stream));

   HRESULT hr = S_OK;
   XmlNodeType nodeType = XmlNodeType_None;
   for( bool bFoundHeader = false; S_OK == (hr = reader->Read(&nodeType)); )
   {
      if (nodeType != XmlNodeType_Element)
         continue;

      const wchar_t *pszName, *pszValue;
      HRESULT hr = reader->GetLocalName(&pszName, NULL);
      COM_VERIFY(hr);

      if (bFoundHeader)
      {
         if (_wcsicmp(pszName, L"File") == 0)
         {
            bfs::path p;
            if (reader->MoveToFirstAttribute() == S_OK)
            {
               hr = reader->GetLocalName(&pszName, NULL);
               COM_VERIFY(hr);
               hr = reader->GetValue(&pszValue, NULL);
               COM_VERIFY(hr);

               p = m_slnPath.parent_path() / W2CA(pszValue);
            }

            if (bfs::exists(p))
               m_files.push_back(p.string());
         }
      }
      else if (_wcsicmp(pszName, L"ProjectZip") == 0)
      {
         bFoundHeader = true;
      }
   }
}

void CSolutionHelper::RemoveDuplicateFiles()
{
   std::set<std::string>   ss;
   StrList                 sl;

   bfs::path slnDirPath = m_slnPath.parent_path();
   for( StrList::const_iterator iter = m_files.begin(); iter != m_files.end(); ++iter)
   {
      bfs::path p = bfs::path(*iter);
      if (!boost::istarts_with(p.string(), slnDirPath.string()))
      {
         m_exclFiles.push_back(ExclData(p.string(), ExclData::ExR_NotInTree));
         continue;
      }

      std::string s = boost::algorithm::to_lower_copy(p.string());
      if (ss.find(s) == ss.end())
      {
         ss.insert(s);
         sl.push_back(p.string());
      }
   }

   m_files.swap(sl);
}

void CSolutionHelper::RemoveExcludedFiles()
{
   std::set<std::string> exclExtSet;
   if (m_pzs.m_bExcludeExeFiles)
   {
      static char *ppsz[] = {".exe", ".dll", ".pdb"};
      exclExtSet.insert(ppsz, ppsz + _countof(ppsz));
   }
   if (m_pzs.m_bExcludeOtherFiles)
   {
      USES_CONVERSION;
      char sz[1024];
      strcpy_s(sz, T2CA(m_pzs.m_sExcludedFiles));
      char *p = 0, *q = 0;
      const char szSep[] = "; \t";
      p = strtok_s(sz, szSep, &q);
      while( p != 0)
      {
         std::string s(p);
         boost::trim(s);
         if (s.size() > 0)
         {
            boost::to_lower(s);
            exclExtSet.insert(s);
         }

         p = strtok_s(NULL, szSep, &q);
      }
   }

   for( StrList::const_iterator iter = m_files.begin(); iter != m_files.end(); )
   {
      bfs::path p = bfs::path(*iter).normalize();
      if (exclExtSet.find(boost::to_lower_copy(p.extension().string())) != exclExtSet.end())
      {
         iter = m_files.erase(iter);
      }
      else
      {
         ++iter;
      }
   }
}

COleDateTime GetMidnight()
{
   COleDateTime now = COleDateTime::GetCurrentTime();
   return COleDateTime(now.GetYear(), now.GetMonth(), now.GetDay(), 0, 0, 0);
}

bool CSolutionHelper::ZipAllFiles( const char *pszZipFile, ISolutionHelperNotifier *pNotifier)
{
   int nDaysChanged = m_pzs.GetDaysChanged();

   std::string sSlnPath = m_slnPath.parent_path().string();
   CZipper z(pszZipFile, sSlnPath.c_str(), false); // , "XXXX");
   size_t ndx = 0;
   for( StrList::const_iterator iter = m_files.begin(); iter != m_files.end(); ++iter, ++ndx)
   {
      if (pNotifier != 0)
      {
         if (!pNotifier->OnNotify(ndx, m_files.size(), *iter))
            return false;
      }
      if (nDaysChanged > 0)
      {
         SYSTEMTIME st;
         if (!z.GetLastModified(iter->c_str(), st))
         {
            m_exclFiles.push_back(ExclData(*iter, ExclData::ExR_NotZip));
            continue;
         }

         COleDateTimeSpan span = GetMidnight() - COleDateTime(st);
         if (span.GetTotalDays() >= nDaysChanged)
            continue;
      }

      if (!z.AddFileToZip(iter->c_str()))
         m_exclFiles.push_back(ExclData(*iter, ExclData::ExR_NotZip));
   }
  
   return true;
}

void Test()
{
   //static char sz[] = "P:/Proj/Common/Util/ProjectZip/ProjectZip.sln";
   //static char sz[] = "P:/Proj/Util/TkProjectZip.sln";

   //CSolutionHelper sh;
   //if (!sh.LoadSolution(sz))
   //   return;

   //sh.ZipAllFiles("D:/xxxxx.zip");
}
