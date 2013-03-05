#include "stdafx.h"

void  RemoveDuplicateFiles(std::vector<std::string> &fileList)
{
   std::set<std::string>         ss;
   std::vector<std::string>      sl;

   std::for_each( fileList.begin(), fileList.end(), [&ss,&sl](const std::string &s) {
      std::string s1 = boost::algorithm::to_lower_copy(s);
      if (ss.find(s1) == ss.end())
      {
         ss.insert(s1);
         sl.push_back(s);
      }
   }
   );

   fileList.swap(sl);
}
