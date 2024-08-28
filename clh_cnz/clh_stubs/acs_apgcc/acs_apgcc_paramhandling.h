#ifndef ACS_APGCC_PARAMHANDLING_H_
#define ACS_APGCC_PARAMHANDLING_H_

#include "ACS_CC_Types.h"
#include <boost/property_tree/ini_parser.hpp>
#include <string>
#include <typeinfo>

using namespace std;

class acs_apgcc_paramhandling
{
public:

   acs_apgcc_paramhandling() :
   m_returntype(), m_errortext(), m_pt()
   {
      const char parameterfile[] = "/opt/ap/pes/conf/pes_clh.ini";
      try
      {
         boost::property_tree::ini_parser::read_ini(parameterfile, m_pt);
      }
      catch (exception& e)
      {
         m_returntype = ACS_CC_FAILURE;
         m_errortext = e.what();
      }
   }

   ~acs_apgcc_paramhandling()
   {
   }

   int getInternalLastError()
   {
      return m_returntype;
   }

   char* getInternalLastErrorText()
   {
      return const_cast<char*>(m_errortext.c_str());
   }

   template<size_t size>
   ACS_CC_ReturnType getParameter(string objectName, string attributeName, char (&value)[size])
   {
      if (m_returntype == ACS_CC_SUCCESS)
      {
         try
         {
            strncpy(
                  value,
                  m_pt.get<string>(objectName + "." + attributeName).c_str(),
                  size
                  );
         }
         catch (exception& e)
         {
            m_returntype = ACS_CC_FAILURE;
            m_errortext = e.what();
         }
      }
      return m_returntype;
   }

   template<typename value_t>
   ACS_CC_ReturnType getParameter(string objectName, string attributeName, value_t* value)
   {
      if (m_returntype == ACS_CC_SUCCESS)
      {
         try
         {
            *value = m_pt.get<value_t>(objectName + "." + attributeName);
         }
         catch (exception& e)
         {
            m_returntype = ACS_CC_FAILURE;
            m_errortext = e.what();
         }
      }
      return m_returntype;
   }

private:
   ACS_CC_ReturnType m_returntype;
   string m_errortext;
   boost::property_tree::ptree m_pt;
};

#endif // ACS_APGCC_PARAMHANDLING_H_
