#pragma once

class CVcprojFileHelper
{
public:
   typedef std::list<std::string> StrList;

   HRESULT  LoadProjFile ( const char *pszFileName);
   void     GetFileList  ( StrList *psl, const char *pszFileFilter = 0) const;

private:
   typedef std::list<bfs::path>  PathList;
   PathList m_fileList;

   static void LoadProjFileImpl( const char *pszFileName, PathList &pathList);
};
