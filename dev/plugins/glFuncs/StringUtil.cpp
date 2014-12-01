#include "StringUtil.h"          
#include <vector>


using namespace yapt;

// everything else is inline
std::string StringUtil::whiteSpaces( " \f\n\r\t\v" );

void StringUtil::Split(std::vector<std::string> &strings, const char *strInput, int splitChar)
{
  std::string input(strInput);
  size_t iPos = 0;
  while(iPos != -1)
  {
      size_t iStart = iPos;
      iPos = input.find(splitChar,iPos);
      if (iPos != -1)
      {
          std::string str = input.substr(iStart, iPos-iStart);
          StringUtil::Trim(str);
          strings.push_back(str);
          iPos++;
      } else
      {
          std::string str = input.substr(iStart, input.length()-iStart);
          StringUtil::Trim(str);
          strings.push_back(str);
      }
  }
}      
