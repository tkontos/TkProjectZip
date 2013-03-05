// Zipper.h: interface for the CZipper class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ZIPPER_H__4249275D_B50B_4AAE_8715_B706D1CA0F2F__INCLUDED_)
#define AFX_ZIPPER_H__4249275D_B50B_4AAE_8715_B706D1CA0F2F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

struct Z_FileInfo
{
   int nFileCount;
   int nFolderCount;
   DWORD dwUncompressedSize;
};

class CZipper  
{
public:
   CZipper(LPCSTR szFilePath = NULL, LPCSTR szRootFolder = NULL, bool bAppend = FALSE, LPCSTR szPassword = 0);
   virtual ~CZipper();

   // simple interface
   static bool ZipFile(LPCSTR szFilePath); // saves as same name with .zip
   static bool ZipFolder(LPCSTR szFilePath, bool bIgnoreFilePath); // saves as same name with .zip
   
   // works with prior opened zip
   bool AddFileToZip(LPCSTR szFilePath, bool bIgnoreFilePath = FALSE);
   bool AddFileToZip(LPCSTR szFilePath, LPCSTR szRelFolderPath); // replaces path info from szFilePath with szFolder
   bool AddFileToZip(const BYTE* pFileContents, int nSize, LPCSTR szRelFilePath);
   bool AddFolderToZip(LPCSTR szFolderPath, bool bIgnoreFilePath = FALSE);

   // extended interface
   bool OpenZip(LPCSTR szFilePath, LPCSTR szRootFolder = NULL, bool bAppend = FALSE, LPCSTR szPassword = 0);
   bool CloseZip(); // for multiple reuse
   void GetFileInfo(Z_FileInfo& info);
   
   static bool GetLastModified(const char* szPath, SYSTEMTIME& sysTime);

protected:
   void       *m_uzFile;
   char        m_szRootFolder[MAX_PATH + 1];
   char        m_szPassword[100];
   Z_FileInfo  m_info;

protected:
   void PrepareSourcePath(char * szPath);

   void UnterminatePath(char * szPath);
   void PrepareZipPath(char * szPath);
   int GetFileCrc(const char* filenameinzip, unsigned long *result_crc);
   int GetDataCrc(const void *buf, size_t size, unsigned long *result_crc);
};

#endif // !defined(AFX_ZIPPER_H__4249275D_B50B_4AAE_8715_B706D1CA0F2F__INCLUDED_)
