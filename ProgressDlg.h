// aboutdlg.h : interface of the CProgressDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "SolutionHelper.h"

class CProgressDlg
   : public CDialogImpl<CProgressDlg>
   , public ISolutionHelperNotifier
{
public:
   CProgressDlg( CSolutionHelper &psh, const TCHAR *pszZipName)
      : m_solHelper( psh), m_sFileName(pszZipName) { }

   enum { IDD = IDD_PROGRESS};

   BEGIN_MSG_MAP(CProgressDlg)
      MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
      COMMAND_ID_HANDLER(IDOK, OnCloseCmd)
      COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
   END_MSG_MAP()

   LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/);
   LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/);

protected:
   virtual bool OnNotify( size_t ndx, size_t size, const std::string &sfile)
   {
      ::SetDlgItemTextA(m_hWnd, IDC_MESSAGE, sfile.c_str());
      ::SendMessage(m_hWndProgCtrl, PBM_SETPOS, (WPARAM)ndx, 0L);
      return m_taskGroup.is_canceling() ? false : true;
   }

private:
   CString                 m_sFileName;
   CSolutionHelper        &m_solHelper;
   HWND                    m_hWndProgCtrl;
   ConcRT::task_group      m_taskGroup;
};
