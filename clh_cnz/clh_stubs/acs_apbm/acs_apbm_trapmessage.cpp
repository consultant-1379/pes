#include "acs_apbm_trapmessage.h"
#include <boost/lexical_cast.hpp>
#include <string>
#include <iostream>
#include <sstream>

using namespace std;

//----------------------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------------------
acs_apbm_trapmessage::acs_apbm_trapmessage():
m_vec()
{
}

//----------------------------------------------------------------------------------------
// Destructor
//----------------------------------------------------------------------------------------
acs_apbm_trapmessage::~acs_apbm_trapmessage()
{
}

//----------------------------------------------------------------------------------------
// Get OID
//----------------------------------------------------------------------------------------
int acs_apbm_trapmessage::OID() const
{
   return (int&)m_data[0];
}

//----------------------------------------------------------------------------------------
// Get vector
//----------------------------------------------------------------------------------------
const vector<int>& acs_apbm_trapmessage::values() const
{
   m_vec.clear(); // ***
   size_t size = sizeof(int);
   m_vec.push_back(reinterpret_cast<int&>(m_data[size]));        // Subrack
   size += sizeof(int);
   m_vec.push_back(reinterpret_cast<int&>(m_data[size]));        // Slot
   return m_vec;
}

//----------------------------------------------------------------------------------------
// Get message
//----------------------------------------------------------------------------------------
const char* acs_apbm_trapmessage::message() const
{
   size_t size = 3 * sizeof(int) + sizeof(uint32_t);
   return &m_data[size];
}

//----------------------------------------------------------------------------------------
// Get message length
//----------------------------------------------------------------------------------------
unsigned acs_apbm_trapmessage::message_length() const
{
   size_t size = 3 * sizeof(int);
   return reinterpret_cast<uint32_t&>(m_data[size]);   // Size
}


