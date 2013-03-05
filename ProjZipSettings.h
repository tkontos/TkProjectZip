#pragma once

class CProjZipSettings : public CRegSettings
{
public:
   static const TCHAR m_szRegKey[];

   CProjZipSettings()
   {
      Init(HKEY_CURRENT_USER, m_szRegKey);
      Load();
   }

   BOOL                 m_bOpenZipDirectory;
   BOOL                 m_bOnlyShowOnShiftKey;
   BOOL                 m_bRecentFiles;
   int                  m_iRecentFileDays;
   BOOL                 m_bExcludeExeFiles;
   BOOL                 m_bExcludeOtherFiles;
   CString              m_sExcludedFiles;
   BOOL                 m_bAddDateToZipFilename;
   CString              m_sInitialDirectory;
   CString              m_strExeDirectory;

   int   GetDaysChanged() const
   {
      return m_bRecentFiles ? m_iRecentFileDays : 0;
   }

private:
   BEGIN_REG_MAP(CProjZipSettings)
      REG_ITEM_EX    ("OpenZipDirectory"     , m_bOpenZipDirectory      , 0)
      REG_ITEM_EX    ("OnlyShowOnShiftKey"   , m_bOnlyShowOnShiftKey    , 0)
      REG_ITEM_EX    ("RecentFiles"          , m_bRecentFiles           , 0)
      REG_ITEM_EX    ("RecentFileDays"       , m_iRecentFileDays        , 7)
      REG_ITEM_EX    ("ExcludeExeFiles"      , m_bExcludeExeFiles       , 1)
      REG_ITEM_EX    ("ExcludeOtherFiles"    , m_bExcludeOtherFiles     , 0)
      REG_ITEM_EX    ("ExcludedFiles"        , m_sExcludedFiles         , _T(""))
      REG_ITEM_EX    ("AddDateToZipFilename" , m_bAddDateToZipFilename  , 1)
      REG_ITEM_EX    ("InitialDirectory"     , m_sInitialDirectory      , _T(""))
      REG_ITEM_EX    ("ExeDirectory"         , m_strExeDirectory        , _T("Out/Release"))
//      REG_ITEM_LIST  ("FileList"             , m_fileList               )
   END_REG_MAP()


};

