//========================================================================================
/** @file acs_aeh_error.h
 *	@brief
 *	@author xfabron (Fabio Ronca)
 *	@date 2011-05-04
 *	@version 0.9.1
 *
 *	COPYRIGHT Ericsson AB, 2011
 *	All rights reserved.
 *
 *	The information in this document is the property of Ericsson.
 *	Except as specifically authorized in writing by Ericsson, the receiver of
 *	this document shall keep the information contained herein confidential and
 *	shall protect the same in whole or in part from disclosure and dissemination
 *	to third parties. Disclosure and dissemination to the receivers employees
 *	shall only be made on a strict need to know basis.
 *
 *
 *	DESCRIPTION
 *	This class serves as a base class to the AEH object, but could
 *	be used by any object.
 *	The class provides the public methods getError, getErrorText and the
 *	protected methods setError and setErrorText (which is overloaded).
 *	getError returns the type of error as set by a previous	setError.
 *	getErrorText returns a text error string as set by a previous
 *	setErrorText.
 *
 *	An example string provided by getErrorText could be as follows:
 *
 *	"ACS_AEH_EvReport : ThisIsASeriousError string - AH4"
 *
 *  ERROR HANDLING
 *	This is class to be used when an error occurs and no specific
 *	error handling is necessary when using this class.
 *
 *
 *	SEE ALSO
 *	Derived classes using this base class, i.e ACS_AEH_EvReport.
 *
 *
 *	REVISION INFO
 *	+========+============+==============+=====================================+
 *	| REV    | DATE       | AUTHOR       | DESCRIPTION                         |
 *	+========+============+==============+=====================================+
 *	| P0.9.1 | 2011-05-04 | xfabron      | File created.                       |
 *	+--------+------------+--------------+-------------------------------------+
 *	| P0.9.1 | 2011-05-13 | xfabron      | Released for ITSTEAM2 sprint 11     |
 *	+--------+------------+--------------+-------------------------------------+
 *	|        |            |              |                                     |
 *	+========+============+==============+=====================================+
 */
//========================================================================================


#ifndef ACS_AEH_ERROR_H 
#define ACS_AEH_ERROR_H

#include "acs_aeh_types.h"

class acs_aeh_error
{
public:

	//==============//
	// Constructors //
	//==============//

	/** @brief Default constructor
	 *
	 *	Constructor of class
	 *
	 *	@remarks Is used by derived objects.
	 */
	acs_aeh_error();


	/** @brief  Copy constructor
	 *
	 *	Copy constructor
	 *
	 *	@param[in] 	error ACS_AEH_Error object
	 *	@remarks 	-
	 */
	acs_aeh_error(const acs_aeh_error& error);


	/** @brief  destructor.
	 *
	 *	The destructor of the class.
	 *
	 *	@remarks -
	 */
	virtual ~acs_aeh_error();


	/** @brief  Assignment operator.
	 *	Assignment operator.
	 *
	 *	@param[in] 	error: an ACS_AEH_Error object
	 *	@return 	Reference to self
	 *	@remarks 	-
	 */
	acs_aeh_error& operator=(const acs_aeh_error& error);


	//===========//
	// Functions //
	//===========//

	/** @brief
	 *	Gets the error type value set by a previous setError.<br>
	 *  The user of this base class should provide descriptions and definitions of these values.<br>
	 *
	 *	@return 	ACS_AEH_ErrorType.
	 *	@remarks	-
	 */
	ACS_AEH_ErrorType getError() const;


	/** @brief
	 *	Gets an error text string set by a previous setErrorText.<br>
	 *  The user of the Error base class should provide descriptions and definitions of these strings.<br>
	 *
	 *	@return 	A pointer to an error text string.
	 *	@remarks 	-
	 */
	const char* getErrorText() const;

protected:
	ACS_AEH_ErrorType m_error;
	const char* m_errorText;
};

#endif

