// Zipper.cpp: implementation of the CZipper class.
//
//////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <tchar.h>
#include "Zipper.h"

#include "zlib\zip.h"
#include "zlib\iowin32.h"

#include <stdio.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

const UINT BUFFERSIZE = 2048;

CZipper::CZipper(LPCSTR szFilePath, LPCSTR szRootFolder, bool bAppend, LPCSTR szPassword)
   : m_uzFile(0)
{
   CloseZip();
   ZeroMemory(&m_info, sizeof(m_info));

   if (szFilePath)
      OpenZip(szFilePath, szRootFolder, bAppend, szPassword);
}

CZipper::~CZipper()
{
   CloseZip();
}

bool CZipper::CloseZip()
{
   int nRet = m_uzFile ? zipClose(m_uzFile, NULL) : ZIP_OK;

   m_uzFile = NULL;
   m_szRootFolder[0] = 0;
   m_szPassword[0] = 0;
   // note: we don't clear m_info until the object is re-used or deleted

   return (nRet == ZIP_OK);
}

void CZipper::GetFileInfo(Z_FileInfo& info)
{
   info = m_info;
}

// simple interface
bool CZipper::ZipFile( LPCSTR szFilePath )
{
   // make zip path
   char szDrive[_MAX_DRIVE], szFolder[MAX_PATH], szName[_MAX_FNAME];
   _splitpath(szFilePath, szDrive, szFolder, szName, NULL);

   char szZipPath[MAX_PATH];
   _makepath(szZipPath, szDrive, szFolder, szName, "zip");

   CZipper zip;

   if (zip.OpenZip(szZipPath, false))
      return zip.AddFileToZip(szFilePath, false);

   return false;
}
   
bool CZipper::ZipFolder(LPCSTR szFilePath, bool bIgnoreFilePath)
{
   // make zip path
   char szDrive[_MAX_DRIVE], szFolder[MAX_PATH], szName[_MAX_FNAME], szExt[_MAX_EXT];
   _splitpath(szFilePath, szDrive, szFolder, szName, szExt);

   strcat_s(szName, szExt); // save extension if any

   // set root path to include the folder name
   char szRootPath[MAX_PATH];
   _makepath(szRootPath, szDrive, szFolder, szName, NULL);

   char szZipPath[MAX_PATH];
   _makepath(szZipPath, szDrive, szFolder, szName, "zip");

   CZipper zip;

   if (zip.OpenZip(szZipPath, szRootPath, false))
      return zip.AddFolderToZip(szFilePath, bIgnoreFilePath);

   return false;
}
   
// works with prior opened zip
bool CZipper::AddFileToZip(LPCSTR szFilePath, bool bIgnoreFilePath)
{
   if (!m_uzFile)
      return false;

   // we don't allow paths beginning with '..\' because this would be outside
   // the root folder
   if (!bIgnoreFilePath && strstr(szFilePath, "..\\") == szFilePath)
      return false;

   bool bFullPath = (strchr(szFilePath, ':') != NULL);

   // if the file is relative then we need to append the root before opening
   char szFullFilePath[MAX_PATH];
   
   strcpy_s(szFullFilePath, szFilePath);
   PrepareSourcePath(szFullFilePath);

   // if the file is a fullpath then remove the root path bit
   char szFileName[MAX_PATH] = "";

   if (bIgnoreFilePath)
   {
      char szName[_MAX_FNAME], szExt[_MAX_EXT];
      _splitpath(szFilePath, NULL, NULL, szName, szExt);

      _makepath(szFileName, NULL, NULL, szName, szExt);
   }
   else if (bFullPath)
   {
      // check the root can be found
      if (0 != _strnicmp(szFilePath, m_szRootFolder, strlen(m_szRootFolder)))
         return false;

      // else
      strcpy_s(szFileName, szFilePath + strlen(m_szRootFolder)+1);
   }
   else // relative path
   {
      // if the path begins with '.\' then remove it
      if (strstr(szFilePath, ".\\") == szFilePath)
         strcpy_s(szFileName, szFilePath + 2);
      else
         strcpy_s(szFileName, szFilePath);
   }

   PrepareZipPath(szFileName);

   // save file attributes
   zip_fileinfo zfi;

   zfi.internal_fa = 0;
   zfi.external_fa = (::GetFileAttributesA(szFilePath) & ~FILE_ATTRIBUTE_READONLY);
   
   // save file time
   SYSTEMTIME st;

   GetLastModified(szFullFilePath, st);

   zfi.dosDate = 0;
   zfi.tmz_date.tm_year = st.wYear;
   zfi.tmz_date.tm_mon = st.wMonth - 1;
   zfi.tmz_date.tm_mday = st.wDay;
   zfi.tmz_date.tm_hour = st.wHour;
   zfi.tmz_date.tm_min = st.wMinute;
   zfi.tmz_date.tm_sec = st.wSecond;
   
   // load input file
   HANDLE hInputFile = ::CreateFileA(szFullFilePath, 
                           GENERIC_READ,
                           0,
                           NULL,
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_READONLY,
                           NULL);

   if (hInputFile == INVALID_HANDLE_VALUE)
      return false;

   const char *pszPassword = 0;
   unsigned long fileCrc = 0;
   if (m_szPassword[0] != 0)
   {
      pszPassword = m_szPassword;
      GetFileCrc(szFullFilePath, &fileCrc);
   }

   //int nRet = zipOpenNewFileInZip(m_uzFile, 
   //                      szFileName,
   //                      &zfi, 
   //                      NULL, 
   //                      0,
   //                      NULL,
   //                      0, 
   //                      NULL,
   //                      Z_DEFLATED,
   //                      Z_DEFAULT_COMPRESSION);

   int nRet = zipOpenNewFileInZip3(m_uzFile, 
                           szFileName,
                           &zfi, 
                           NULL, 
                           0,
                           NULL,
                           0, 
                           NULL,
                           Z_DEFLATED,
                           Z_DEFAULT_COMPRESSION,
                           0,
                           -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                           pszPassword, fileCrc);

   if (nRet == ZIP_OK)
   {
      m_info.nFileCount++;

      // read the file and output to zip
      char pBuffer[BUFFERSIZE];
      DWORD dwBytesRead = 0, dwFileSize = 0;

      while (nRet == ZIP_OK && ::ReadFile(hInputFile, pBuffer, BUFFERSIZE, &dwBytesRead, NULL))
      {
         dwFileSize += dwBytesRead;

         if (dwBytesRead)
            nRet = zipWriteInFileInZip(m_uzFile, pBuffer, dwBytesRead);
         else
            break;
      }

      m_info.dwUncompressedSize += dwFileSize;
   }

   zipCloseFileInZip(m_uzFile);
   ::CloseHandle(hInputFile);

   return (nRet == ZIP_OK);
}

bool CZipper::AddFileToZip(LPCSTR szFilePath, LPCSTR szRelFolderPath)
{
   if (!m_uzFile)
      return false;

   // szRelFolderPath cannot contain drive info
   if (szRelFolderPath && strchr(szRelFolderPath, ':'))
      return false;

   // if the file is relative then we need to append the root before opening
   char szFullFilePath[MAX_PATH];
   
   strcpy_s(szFullFilePath, szFilePath);
   PrepareSourcePath(szFullFilePath);

   // save file attributes and time
   zip_fileinfo zfi;

   zfi.internal_fa = 0;
   zfi.external_fa = ::GetFileAttributesA(szFilePath);
   
   // save file time
   SYSTEMTIME st;

   GetLastModified(szFullFilePath, st);

   zfi.dosDate = 0;
   zfi.tmz_date.tm_year = st.wYear;
   zfi.tmz_date.tm_mon = st.wMonth - 1;
   zfi.tmz_date.tm_mday = st.wDay;
   zfi.tmz_date.tm_hour = st.wHour;
   zfi.tmz_date.tm_min = st.wMinute;
   zfi.tmz_date.tm_sec = st.wSecond;

   // load input file
   HANDLE hInputFile = ::CreateFileA(szFullFilePath, 
                           GENERIC_READ,
                           0,
                           NULL,
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_READONLY,
                           NULL);

   if (hInputFile == INVALID_HANDLE_VALUE)
      return false;

   // strip drive info off filepath
   char szName[_MAX_FNAME], szExt[_MAX_EXT];
   _splitpath(szFilePath, NULL, NULL, szName, szExt);

   // prepend new folder path 
   char szFileName[MAX_PATH];
   _makepath(szFileName, NULL, szRelFolderPath, szName, szExt);

   PrepareZipPath(szFileName);

   // open the file in the zip making sure we remove any leading '\'
   const char *pszPassword = 0;
   unsigned long fileCrc = 0;
   if (m_szPassword[0] != 0)
   {
      pszPassword = m_szPassword;
      GetFileCrc(szFullFilePath, &fileCrc);
   }

   //int nRet = zipOpenNewFileInZip(m_uzFile, 
   //                      szFileName,
   //                      &zfi, 
   //                      NULL, 
   //                      0,
   //                      NULL,
   //                      0, 
   //                      NULL,
   //                      Z_DEFLATED,
   //                      Z_DEFAULT_COMPRESSION);

   int nRet = zipOpenNewFileInZip3(m_uzFile, 
                           szFileName,
                           &zfi, 
                           NULL, 
                           0,
                           NULL,
                           0, 
                           NULL,
                           Z_DEFLATED,
                           Z_DEFAULT_COMPRESSION,
                           0,
                           -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                           pszPassword, fileCrc);

   if (nRet == ZIP_OK)
   {
      m_info.nFileCount++;

      // read the file and output to zip
      char pBuffer[BUFFERSIZE];
      DWORD dwBytesRead = 0, dwFileSize = 0;

      while (nRet == ZIP_OK && ::ReadFile(hInputFile, pBuffer, BUFFERSIZE, &dwBytesRead, NULL))
      {
         dwFileSize += dwBytesRead;

         if (dwBytesRead)
            nRet = zipWriteInFileInZip(m_uzFile, pBuffer, dwBytesRead);
         else
            break;
      }

      m_info.dwUncompressedSize += dwFileSize;
   }

   zipCloseFileInZip(m_uzFile);
   ::CloseHandle(hInputFile);

   return (nRet == ZIP_OK);
}

bool CZipper::AddFileToZip(const BYTE* pFileContents, int nSize, LPCSTR szRelFilePath)
{
   if (!m_uzFile)
      return false;

   // szRelFilePath cannot contain drive info
   if (szRelFilePath && strchr(szRelFilePath, ':'))
      return false;

   // save file attributes and time
   zip_fileinfo zfi;

   zfi.internal_fa = 0;
   zfi.external_fa = FILE_ATTRIBUTE_NORMAL;
   
   // use time now
   SYSTEMTIME st;

   GetSystemTime(&st);

   zfi.dosDate = 0;
   zfi.tmz_date.tm_year = st.wYear;
   zfi.tmz_date.tm_mon = st.wMonth - 1;
   zfi.tmz_date.tm_mday = st.wDay;
   zfi.tmz_date.tm_hour = st.wHour;
   zfi.tmz_date.tm_min = st.wMinute;
   zfi.tmz_date.tm_sec = st.wSecond;

   // open the file in the zip making sure we remove any leading '\'
   char szFilePath[MAX_PATH];

   strcpy_s(szFilePath, szRelFilePath);
   PrepareZipPath(szFilePath);

   const char *pszPassword = 0;
   unsigned long fileCrc = 0;
   if (m_szPassword[0] != 0)
   {
      pszPassword = m_szPassword;
      GetDataCrc(pFileContents, (size_t)nSize, &fileCrc);
   }

   //int nRet = zipOpenNewFileInZip(m_uzFile, 
   //                      szFilePath,
   //                      &zfi, 
   //                      NULL, 
   //                      0,
   //                      NULL,
   //                      0, 
   //                      NULL,
   //                      Z_DEFLATED,
   //                      Z_DEFAULT_COMPRESSION);

   int nRet = zipOpenNewFileInZip3(m_uzFile, 
                           szFilePath,
                           &zfi, 
                           NULL, 
                           0,
                           NULL,
                           0, 
                           NULL,
                           Z_DEFLATED,
                           Z_DEFAULT_COMPRESSION,
                           0,
                           -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                           pszPassword, fileCrc);

   if (nRet == ZIP_OK)
   {
      m_info.nFileCount++;

      // read the file and output to zip
      const BYTE* pBuf = pFileContents;

      do
      {
         DWORD dwBytesRead = min(BUFFERSIZE, pFileContents + nSize - pBuf);
         nRet = zipWriteInFileInZip(m_uzFile, pBuf, dwBytesRead);

         pBuf += dwBytesRead;
      }
      while (nRet == ZIP_OK && pBuf < pFileContents + nSize);

      m_info.dwUncompressedSize += nSize;
   }

   zipCloseFileInZip(m_uzFile);

   return (nRet == ZIP_OK);
}

bool CZipper::AddFolderToZip(LPCSTR szFolderPath, bool bIgnoreFilePath)
{
   if (!m_uzFile)
      return false;

   m_info.nFolderCount++;

   // if the path is relative then we need to append the root before opening
   char szFullPath[MAX_PATH];
   
   strcpy_s(szFullPath, szFolderPath);
   PrepareSourcePath(szFullPath);

   // always add folder first
   // save file attributes
   zip_fileinfo zfi;
   
   zfi.internal_fa = 0;
   zfi.external_fa = ::GetFileAttributesA(szFullPath);
   
   SYSTEMTIME st;
   
   GetLastModified(szFullPath, st);
   
   zfi.dosDate = 0;
   zfi.tmz_date.tm_year = st.wYear;
   zfi.tmz_date.tm_mon = st.wMonth - 1;
   zfi.tmz_date.tm_mday = st.wDay;
   zfi.tmz_date.tm_hour = st.wHour;
   zfi.tmz_date.tm_min = st.wMinute;
   zfi.tmz_date.tm_sec = st.wSecond;
   
   // if the folder is a fullpath then remove the root path bit
   char szFolderName[MAX_PATH] = "";
   
   if (bIgnoreFilePath)
   {
      char szExt[_MAX_EXT];
      _splitpath(szFullPath, NULL, NULL, szFolderName, szExt);

      strcat_s(szFolderName, szExt);
   }
   else
   {
      // check the root can be found
      if (0 != _strnicmp(szFullPath, m_szRootFolder, strlen(m_szRootFolder)))
         return false;
      
      // else
      strcpy_s(szFolderName, szFullPath + strlen(m_szRootFolder));
   }
   
   // open the file in the zip making sure we remove any leading '\'
   // provided that the folder name is not empty.
   // note: its ok for it to be empty if this folder coincides with the root folder
   PrepareZipPath(szFolderName);

   if (strlen(szFolderName))
   {
      int nRet = zipOpenNewFileInZip(m_uzFile, 
                              szFolderName,
                              &zfi, 
                              NULL, 
                              0,
                              NULL,
                              0, 
                              NULL,
                              Z_DEFLATED,
                              Z_DEFAULT_COMPRESSION);
      
      zipCloseFileInZip(m_uzFile);
   }

   // build searchspec
   char szDrive[_MAX_DRIVE], szFolder[MAX_PATH], szName[_MAX_FNAME], szExt[_MAX_EXT];
   _splitpath(szFullPath, szDrive, szFolder, szName, szExt);

   strcat_s(szFolder, szName);
   strcat_s(szFolder, szExt);

   char szSearchSpec[MAX_PATH];
   _makepath(szSearchSpec, szDrive, szFolder, "*", "*");

   WIN32_FIND_DATAA finfo;
   HANDLE hSearch = FindFirstFileA(szSearchSpec, &finfo);

   if (hSearch != INVALID_HANDLE_VALUE) 
   {
      do 
      {
         if (finfo.cFileName[0] != '.') 
         {
            char szItem[MAX_PATH];
            _makepath(szItem, szDrive, szFolder, finfo.cFileName, NULL);
            
            if (finfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
               AddFolderToZip(szItem, bIgnoreFilePath);
            }
            else 
               AddFileToZip(szItem, bIgnoreFilePath);
         }
      } 
      while (FindNextFileA(hSearch, &finfo));
      
      FindClose(hSearch);
   }

   return TRUE;
}

// extended interface
bool CZipper::OpenZip(LPCSTR szFilePath, LPCSTR szRootFolder, bool bAppend, LPCSTR szPassword)
{
   CloseZip();
   ZeroMemory(&m_info, sizeof(m_info));

   if (!szFilePath || !strlen(szFilePath))
      return false;

   if (szPassword != 0 && szPassword[0] != 0)
      strcpy_s( m_szPassword, szPassword);

   // convert szFilePath to fully qualified path 
   char szFullPath[MAX_PATH];

   if (!GetFullPathNameA(szFilePath, MAX_PATH, szFullPath, NULL))
      return false;

   // zipOpen will fail if bAppend is TRUE and zip does not exist
   if (bAppend && ::GetFileAttributesA(szFullPath) == 0xffffffff)
      bAppend = false;

   m_uzFile = zipOpen(szFullPath, bAppend ? APPEND_STATUS_ADDINZIP : APPEND_STATUS_CREATE);

   if (m_uzFile)
   {
      if (!szRootFolder)
      {
         char szDrive[_MAX_DRIVE], szFolder[MAX_PATH];
         _splitpath(szFullPath, szDrive, szFolder, NULL, NULL);

         _makepath(m_szRootFolder, szDrive, szFolder, NULL, NULL);
      }
      else if (strlen(szRootFolder))
      {
         _makepath(m_szRootFolder, NULL, szRootFolder, NULL, NULL);
      }

      // remove any trailing whitespace and '\'
      UnterminatePath(m_szRootFolder);
   }

   return (m_uzFile != NULL);
}

void CZipper::UnterminatePath(char * szPath)
{
   int nEnd = strlen(szPath) - 1;

   while (isspace(szPath[nEnd]))
      nEnd--;

   while (szPath[nEnd] == '\\')
      nEnd--;

   szPath[nEnd + 1] = 0;
}

void CZipper::PrepareZipPath(char * szPath)
{
   UnterminatePath(szPath);

   // remove leading whitespace and '\'
   char szTemp[MAX_PATH];
   strcpy_s(szTemp, szPath);

   int nStart = 0;

   while (isspace(szTemp[nStart]))
      nStart++;

   while (szTemp[nStart] == '\\')
      nStart++;

   if (nStart)
      strcpy(szPath, szTemp + nStart);
}

void CZipper::PrepareSourcePath(char * szPath)
{
   bool bFullPath = (strchr(szPath, ':') != NULL);

   // if the file is relative then we need to append the root before opening
   if (!bFullPath)
   {
      char szTemp[MAX_PATH];
      strcpy_s(szTemp, szPath);

      _makepath(szPath, NULL, m_szRootFolder, szTemp, NULL);
   }
}

bool CZipper::GetLastModified(const char* szPath, SYSTEMTIME& sysTime)
{
   ZeroMemory(&sysTime, sizeof(SYSTEMTIME));

   DWORD dwAttr = ::GetFileAttributesA(szPath);

   // files only
   if (dwAttr == 0xFFFFFFFF)
      return false;

   WIN32_FIND_DATAA findFileData;
   HANDLE hFind = FindFirstFileA((char *)szPath, &findFileData);

   if (hFind == INVALID_HANDLE_VALUE)
      return false;

   FindClose(hFind);

   FILETIME ft = findFileData.ftLastWriteTime;

   FileTimeToLocalFileTime(&findFileData.ftLastWriteTime, &ft);
   FileTimeToSystemTime(&ft, &sysTime);

   return true;
}

/* calculate the CRC32 of a file,
   because to encrypt a file, we need known the CRC32 of the file before */
int CZipper::GetFileCrc(const char* filenameinzip, unsigned long *result_crc)
{
   unsigned long calculate_crc=0;
   int err=ZIP_OK;
   FILE * fin = fopen(filenameinzip,"rb");
   unsigned long size_read = 0;
   unsigned long total_read = 0;
   if (fin==NULL)
   {
       err = ZIP_ERRNO;
   }

   if (err == ZIP_OK)
   {
      do
      {
         const int size_buf = 4096;
         BYTE buf[size_buf];

         err = ZIP_OK;
         size_read = (int)fread(buf,1,size_buf,fin);
         if (size_read < size_buf)
         {
            if (feof(fin)==0)
            {
               //printf("error in reading %s\n",filenameinzip);
               err = ZIP_ERRNO;
            }
         }

         if (size_read>0)
            calculate_crc = crc32(calculate_crc,buf,size_read);
         total_read += size_read;

      } while ((err == ZIP_OK) && (size_read>0));
   }

   if (fin)
      fclose(fin);

   *result_crc=calculate_crc;
   //printf("file %s crc %x\n",filenameinzip,calculate_crc);
   return err;
}

int CZipper::GetDataCrc(const void *buf, size_t size, unsigned long *result_crc)
{
   unsigned long calculate_crc=0;
   calculate_crc = crc32(calculate_crc,(const BYTE *)buf,size);

   *result_crc=calculate_crc;
   return ZIP_OK;
}
