#include "stdafx.h"
#include "VcprojFileHelper.h"

//////////////////////////////////////////////////////////////////////////

class XFileExtension
{
public:
   XFileExtension( const char *pszFilter = 0);
   bool  IsFileIncluded( const bfs::path &fp) const;

private:
   std::string                m_root;
   std::vector<std::string>   m_filter;
};

//////////////////////////////////////////////////////////////////////////

HRESULT CVcprojFileHelper::LoadProjFile( const char *pszFileName )
{
   m_fileList.clear();
   try
   {
      PathList sl;
      LoadProjFileImpl(pszFileName, sl);
      sl.swap(m_fileList);
      return S_OK;
   }
   catch (HRESULT hr)
   {
      return hr;
   }
}

void CVcprojFileHelper::GetFileList( StrList *psl, const char *pszFileFilter /*= 0*/ ) const
{
   XFileExtension fe(pszFileFilter);
   for(PathList::const_iterator iter = m_fileList.begin(); iter != m_fileList.end(); ++iter)
   {
      const bfs::path &p = *iter;
      if (fe.IsFileIncluded(p))
         psl->push_back(p.string());
   }
}

//////////////////////////////////////////////////////////////////////////

void CVcprojFileHelper::LoadProjFileImpl( const char *pszFileName, PathList &pathList)
{
   USES_CONVERSION;

   CComPtr<IXmlReader> reader;
   COM_VERIFY(CreateXmlReader(__uuidof(IXmlReader), reinterpret_cast<void**>(&reader), 0));

   CComPtr<IStream> stream;
   COM_VERIFY(SHCreateStreamOnFileA(pszFileName, STGM_READ | STGM_SHARE_DENY_NONE, &stream));
   COM_VERIFY(reader->SetInput(stream));

   bfs::path projFile(pszFileName);

   HRESULT hr = S_OK;
   XmlNodeType nodeType = XmlNodeType_None;
   while (S_OK == (hr = reader->Read(&nodeType)))
   {
      if (nodeType != XmlNodeType_Element)
         continue;

      const wchar_t *pszName, *pszValue;
      reader->GetLocalName(&pszName, NULL);

      static struct  
      {
         const wchar_t *pszName;
         const wchar_t *pszAttr;
      } nameAttrPair[] = 
      {
         L"File"              , L"RelativePath",      // VC8, VC9
         L"ClCompile"         , L"Include",           // VC10
         L"ClInclude"         , L"Include",           // VC10
         L"ResourceCompile"   , L"Include",           // VC10
         L"None"              , L"Include",           // VC10

         L"Compile"           , L"Include",           // C#
         L"EmbeddedResource"  , L"Include",

         L"ApplicationDefinition", L"Include",        // WPF
         L"Page"                 , L"Include",        // WPF
         L"Resource"             , L"Include",        // WPF
         L"Content"              , L"Include",        // WPF
      };

      const wchar_t *pszAttrName = 0;
      for(int i=0; i<_countof(nameAttrPair); ++i)
      {
         if (_wcsicmp(pszName, nameAttrPair[i].pszName) == 0)
         {
            pszAttrName = nameAttrPair[i].pszAttr;
            break;
         }
      }

      if (pszAttrName == 0)
      {
         // No format we recognize, skip
         continue;
      }

      for (hr = reader->MoveToFirstAttribute(); S_OK == hr; hr = reader->MoveToNextAttribute())
      {
         reader->GetLocalName(&pszName, NULL);
         reader->GetValue(&pszValue, NULL);
         if (_wcsicmp(pszName, pszAttrName) != 0)
            continue;

         if (pszValue[0] == '.' && pszValue[1] == '\\')
            pszValue += 2;

         bfs::path file = projFile.parent_path() / W2CA(pszValue);
         pathList.push_back(file);
      }

      COM_VERIFY(hr);
   }

}

//////////////////////////////////////////////////////////////////////////

XFileExtension::XFileExtension( const char *pszFilter /*= 0*/ )
{
   if (pszFilter != 0)
   {
      std::auto_ptr<char> q(_strdup(pszFilter));
      char *p;
      char *psz = strtok_s(q.get(), ",;", &p);
      while(psz != 0)
      {
         m_filter.push_back(psz);
         psz = strtok_s(NULL, ",;", &p);
      }
   }
}

bool XFileExtension::IsFileIncluded( const bfs::path &fp ) const
{
   if (m_filter.size() == 0)
      return true;

   std::string ext = boost::algorithm::to_lower_copy(fp.extension().string());
   for( size_t i=0; i<m_filter.size(); ++i)
   {
      if (m_filter[i] == ext)
         return true;
   }
   return false;
}

//////////////////////////////////////////////////////////////////////////

void  TestVcprojFileHelper()
{
   CVcprojFileHelper vcp;
   HRESULT hr = vcp.LoadProjFile("P:/Proj/Visx/WaveScan/Ws500/WaveScan/WaveScan.vcproj");
   if (FAILED(hr))
   {
      ATLTRACE( _T("Error = 0x%08X\n"), hr);
      return;
   }

   CVcprojFileHelper::StrList sl;
   vcp.GetFileList(&sl, ".rc");
   ATLTRACE("TestVcprojFileHelper Done\n");

}
