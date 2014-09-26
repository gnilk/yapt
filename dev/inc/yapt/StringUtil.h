#pragma once
#include <string>
#include <list>
#include <stack>


namespace yapt {
  class StringUtil
  {
    static std::string whiteSpaces;

    public:
      __inline static void TrimRight( std::string& str, const std::string& trimChars = whiteSpaces )
      {
        std::string::size_type pos = str.find_last_not_of( trimChars );
        str.erase( pos + 1 );    
      }

      __inline static void TrimLeft( std::string& str, const std::string& trimChars = whiteSpaces )
      {
        std::string::size_type pos = str.find_first_not_of( trimChars );
        str.erase( 0, pos );
      }

      __inline static std::string &Trim( std::string& str, const std::string& trimChars = whiteSpaces )
      {
        TrimRight( str, trimChars );
        TrimLeft( str, trimChars );
        return str;
      }

      __inline static std::string ToLower(std::string s) {
        std::string res = "";
        for(size_t i=0;i<s.length();i++) {
          res+=((char)tolower(s.at(i)));
        }
        return res;
      }
      __inline static bool EqualsIgnoreCase(std::string a, std::string b) {
        std::string sa = ToLower(a);
        std::string sb = ToLower(b);
        return (sa==sb);
      }

  };
}