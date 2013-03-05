#pragma once

#include <iostream>
#include <sstream>
#include <memory>

class StreamingException : public std::runtime_error
{
public:
   StreamingException()
      : std::runtime_error("")
      , ss_(new std::stringstream())
   {
   }
   ~StreamingException() throw()
   {
   }

   template<typename T>
   StreamingException &operator << (const T &t)
   {
      (*ss_) << t;
      return *this;
   }

   virtual const char *what() const throw()
   {
      if (s_.get() == 0)
         s_.reset(new std::string(ss_->str()));
      return s_->c_str();
   }
private:
   mutable std::auto_ptr<std::stringstream>  ss_;
   mutable std::auto_ptr<std::string>        s_;
};
