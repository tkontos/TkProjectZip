#include "StdAfx.h"
#include "TkSpinHelper.h"

bool GetCtrlValue(HWND hWnd, double &val)
{
   TCHAR sz[101];
   if (::GetWindowText(hWnd, sz, 100) <= 0)
      return false;

   TCHAR *p;
   val = _tcstod(sz, &p);
   return *p == 0;
}

template<typename T>
bool  GetCtrlValue( HWND hWnd, int idc, T &val, T defVal = T())
{
   double d;
   if(GetCtrlValue( ::GetDlgItem(hWnd, idc), d))
   {
      val = T(d);
      return true;
   }
   else
   {
      val = defVal;
      return false;
   }
}

CTkSpinHelper::~CTkSpinHelper(void)
{
}

void  CTkSpinHelper::InitCtrlID( int idc)
{
   CtrlData cd;
   memset(&cd, 0, sizeof(cd));
   cd.bUserHandled = true;

   InitCtrlImpl( idc, cd);
}

void  CTkSpinHelper::InitCtrlID( int idc, double dMin, double dMax, double dIncr, int iDecPlaces)
{
   CtrlData cd;
   memset(&cd, 0, sizeof(cd));
   cd.dMin = dMin;
   cd.dMax = dMax;
   cd.dIncr = dIncr;
   cd.iDecPlaces = iDecPlaces;
   InitCtrlImpl( idc, cd);
}

void  CTkSpinHelper::InitCtrlImpl( int idc, CtrlData &cd)
{
   cd.hWndEdit = GetDlgItem(m_hWndParent, idc);
   ATLASSERT( ::IsWindow(cd.hWndEdit));
   cd.hWndSpin = GetWindow(cd.hWndEdit, GW_HWNDNEXT);

   ATLASSERT( (GetWindowLong(cd.hWndSpin, GWL_STYLE) & UDS_SETBUDDYINT) == 0);

   CtrlDataMap::iterator it = m_map.find(idc);
   if (it == m_map.end())
   {
      m_map.insert(std::make_pair(idc, cd));
   }
   else
   {
      it->second = cd;
   }
}

LRESULT CTkSpinHelper::OnDeltaposSpin(int wID, NMHDR* pNMHDR, BOOL &bHandled)
{
   if (m_hWndParent == 0 || !::IsWindow(m_hWndParent))
      return 0;

   int iCtrlId = GetDataCtrlID(wID);
   CtrlDataMap::const_iterator it = m_map.find(iCtrlId);
   if (it == m_map.end())
   {
      bHandled = FALSE;
      return 0;
   }

   NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
   int iDelta = -pNMUpDown->iDelta;

   const CtrlData &cd = it->second;

   double dVal;
   if(!GetCtrlValue(m_hWndParent, iCtrlId, dVal))
      SetCtrlData( cd, iCtrlId, cd.dMin, 0);
   else
      SetCtrlData( cd, iCtrlId, dVal, iDelta);

   return 0;
}

int  CTkSpinHelper::GetDataCtrlID( int iSpinId)
{
   CWindow w = ::GetDlgItem(m_hWndParent, iSpinId);
   if (w != 0)
      w = w.GetWindow(GW_HWNDPREV);
   return w != 0 ? w.GetDlgCtrlID() : 0;
}

void CTkSpinHelper::OnSpinHandler( int iCtrlId, double dVal, int iDelta)
{
   ATLASSERT( FALSE);
}

void  CTkSpinHelper::SetCtrlData( const CtrlData &cd, int iCtrlId, double dVal, int iDelta)
{
   if (cd.bUserHandled)
   {
      OnSpinHandler( iCtrlId, dVal, iDelta);
      return;
   }

   dVal += iDelta * cd.dIncr;
   if (cd.dMax > cd.dMin)
   {
      if (dVal > cd.dMax)
         dVal = cd.dMax;
      else if (dVal < cd.dMin)
         dVal = cd.dMin;
   }

   std::stringstream ss;
   ss << std::fixed << std::setprecision(cd.iDecPlaces) << dVal;

   SetDlgItemTextA(m_hWndParent, iCtrlId, ss.str().c_str());
}

void CTkSpinHelper::ShowSpinner( int idc, bool bShow )
{
   CtrlDataMap::iterator it = m_map.find(idc);
   if (it == m_map.end())
      return;

   const CtrlData &cd = it->second;
   ::ShowWindow( cd.hWndSpin, bShow ? SW_SHOW : SW_HIDE);

}

void CTkSpinHelper::EnableSpinner( int idc, bool bEnable )
{
   CtrlDataMap::iterator it = m_map.find(idc);
   if (it == m_map.end())
      return;

   const CtrlData &cd = it->second;
   ::EnableWindow( cd.hWndSpin, bEnable);
}
