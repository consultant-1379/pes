/*
 * OmHandler.h
 *
 *  Created on: May 26, 2010
 *      Author: designer
 */

#ifndef OMHANDLER_H_
#define OMHANDLER_H_

#include "ACS_CC_Types.h"
#include <string>
#include <vector>

class OmHandler
{
public:

   //==============//
   // Constructors //
   //==============//

   /** @brief OmHandler Default constructor
    *
    *	OmHandler Constructor of class
    *
    *	@remarks Remarks
    */
   OmHandler();

   /** @brief OmHandler Destructor
    *
    *	OmHandler Destructor of class
    *
    *	@remarks Remarks
    */
   ~OmHandler();

   /**
    *
    * Init method: used to Initialize Interaction between the calling process and IMM.
    * Each application that intends to use OmHandler class must call this method to perform any action and
    * call anyone of the other methods.
    * @return ACS_CC_ReturnType. On success ACS_CC_SUCCESS on Failure ACS_CC_FAILURE
    */
   ACS_CC_ReturnType Init();

   /**
    * Finalize method: used to finalize interaction with IMM and release all handler to it.
    * After calling this method no other operation on IMM could be performed.
    * No parameter required
    */
   ACS_CC_ReturnType Finalize();

   /**
    * getClassInstances method: this method is used to retrieve the dn of all object that are instances of the class having the name
    * to which p_className point.
    * Required parameter is:
    *  @param  p_className :		pointer to the name of the class to which retrieve the instances
    *	@param p_dnList :			a vector of std::string elements containing the dn of the object that maching the search criteria
    *
    *	@return ACS_CC_ReturnType. On success ACS_CC_SUCCESS on Failure ACS_CC_FAILURE
    *
    */
   ACS_CC_ReturnType getClassInstances(const char* p_className,
         std::vector<std::string> &p_dnList);

private:

   OmHandler& operator=(const OmHandler &omHandler);

   bool m_initialized;
};

#endif /* OMHANDLER_H_ */
