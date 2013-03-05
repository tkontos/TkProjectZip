#pragma once

#include "resource.h"
#include "ProjZipSettings.h"

class CProjZipSettingsDlg
   : public CDialogImpl<CProjZipSettingsDlg>
   , public CWinDataExchange<CProjZipSettingsDlg>
{
public:
   CProjZipSettingsDlg(CProjZipSettings &pzs, CString &slnFileName)
      : m_pzs(pzs), m_slnFileName(slnFileName) { } 

   enum { IDD = IDD_SETTINGS};

   BEGIN_MSG_MAP(CProjZipSettingsDlg)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      COMMAND_ID_HANDLER(IDC_FILE_BROWSE, OnFileBrowse)
      COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
      COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
      COMMAND_ID_HANDLER(IDC_RADIO1, OnRadio)
      COMMAND_ID_HANDLER(IDC_RADIO2, OnRadio)
      CHAIN_MSG_MAP_MEMBER( m_spinHelper)
   END_MSG_MAP()

   LRESULT OnInitDialog ( UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
   LRESULT OnFileBrowse ( WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
   LRESULT OnCloseCmd   ( WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
   LRESULT OnRadio      ( WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
   {
      RadioClicked();
      return 0;
   }

private:
   BEGIN_DDX_MAP(CProjZipSettingsDlg)
      DDX_TEXT ( IDC_COMBO1, m_slnFileName);
      DDX_RADIO( IDC_RADIO1, m_pzs.m_bRecentFiles)
      DDX_INT  ( IDC_DAYS  , m_pzs.m_iRecentFileDays)
      DDX_CHECK( IDC_CHECK1, m_pzs.m_bExcludeExeFiles)
      DDX_CHECK( IDC_CHECK4, m_pzs.m_bExcludeOtherFiles)
      DDX_TEXT ( IDC_EDIT1 , m_pzs.m_sExcludedFiles)
      DDX_CHECK( IDC_CHECK2, m_pzs.m_bAddDateToZipFilename)
      DDX_CHECK( IDC_CHECK3, m_pzs.m_bOnlyShowOnShiftKey)
      DDX_CHECK( IDC_CHECK5, m_pzs.m_bOpenZipDirectory)
   END_DDX_MAP()

   CProjZipSettings     &m_pzs;
   CString              &m_slnFileName;
   CMruComboCtrl         m_cbFile;
   CTkSpinHelper         m_spinHelper;

   void  RadioClicked();
};
