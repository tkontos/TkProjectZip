#pragma once

#include "ProjZipSettings.h"

struct ISolutionHelperNotifier
{
   virtual bool OnNotify( size_t ndx, size_t size, const std::string &sfile) = 0;
};

class CSolutionHelper
{
public:
   typedef std::list<std::string>   StrList;

   CSolutionHelper(const CProjZipSettings &pzs);
   ~CSolutionHelper(void);

   bool  LoadSolution( const TCHAR *pszFileName);

   bool  ZipAllFiles ( const wchar_t *pszZipFile, ISolutionHelperNotifier *pNotifier = 0)
   {
      USES_CONVERSION;
      return ZipAllFiles(W2CA(pszZipFile), pNotifier);
   }
   bool  ZipAllFiles ( const char *pszZipFile, ISolutionHelperNotifier *pNotifier = 0);

   const StrList  &GetFileList() const
   {
      return m_files;
   }

private:
   bool LoadProjectFiles ( );
   void FixFilePaths ( );
   bool LoadSolutionImpl      ( const char *pszFileName);
   bool HandleProjectLine     ( char *pszLine);
   bool HandleSolutionItem    ( char *pszLine);
   void RemoveDuplicateFiles  ( );
   void LoadAdditionalFiles   ( const char *pszXmlFile);
   void RemoveExcludedFiles();
   bfs::wpath  m_slnPath;
   StrList     m_projFiles, m_solFiles, m_files;

   struct ExclData
   {
      enum Reason { ExR_None, ExR_NotInTree, ExR_NotZip};

      ExclData()
         : reason( ExR_None) { }
      ExclData( const std::string &s, Reason r)
         : fileName(s), reason(r) { }

      friend bool operator<(const ExclData &a, const ExclData &b)
      {
         return std::make_pair(a.fileName, a.reason) < std::make_pair(b.fileName, b.reason);
      }

      std::string fileName;
      Reason      reason;
   };

   const CProjZipSettings    &m_pzs;
   std::list<ExclData>        m_exclFiles;
};

