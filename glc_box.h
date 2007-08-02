/****************************************************************************

 This file is part of the GLC-lib library.
 Copyright (C) 2005-2006 Laurent Ribon (laumaya@users.sourceforge.net)
 Version 0.9.6, packaged on June, 2006.

 http://glc-lib.sourceforge.net

 GLC-lib is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 GLC-lib is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with GLC-lib; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

*****************************************************************************/

//! \file glc_box.h interface for the GLC_Box class.

#ifndef GLC_BOX_H_
#define GLC_BOX_H_


#include "glc_geometry.h"

//////////////////////////////////////////////////////////////////////
//! \class GLC_Box
/*! \brief GLC_Box : OpenGL Box*/

/*! An GLC_Box is a polygonal geometry\n
 *  It's a rectangular parallelepiped box centred at (0, 0, 0)*/
 

//////////////////////////////////////////////////////////////////////

class GLC_Box : public GLC_Geometry  
{
//////////////////////////////////////////////////////////////////////
/*! @name Constructor */
//@{
//////////////////////////////////////////////////////////////////////
public:	
	//! Construct an GLC_Box
	/*! By default, discretion is set to #GLC_POLYDISCRET*/
	GLC_Box(double, double, double, const QColor&);
//@}

//////////////////////////////////////////////////////////////////////
/*! \name Get Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Get X length
	double getLgX(void) const;

	//! Get Y length
	double getLgY(void) const;

	//! Get Z length
	double getLgZ(void) const;
	
	//! return the box bounding box
	virtual GLC_BoundingBox* getBoundingBox(void) const;

//@}

//////////////////////////////////////////////////////////////////////
/*! \name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:

// Display List modifying functions
	//! Set X length
	/*! This Function invalid OpenGL display list
	 * LgX must be > 0*/
	void setLgX(double LgX);

	//! Set Y length
	/*! This Function invalid OpenGL display list
	 * LgY must be > 0*/
	void setLgY(double LgY);

	//! Set Z length
	/*! This Function invalid OpenGL display list
	 * LgZ must be > 0*/
	void setLgZ(double LgZ);
	
// End of Display List modifying functions

//@}

//////////////////////////////////////////////////////////////////////
/*! \name OpenGL Functions*/
//@{
//////////////////////////////////////////////////////////////////////
private:
	//! Virtual interface for OpenGL Geometry set up.
	/*! This Virtual function is implemented here.\n
	 *  Throw GLC_OpenGlException*/
	virtual void glDraw(void);
//@}

//////////////////////////////////////////////////////////////////////
// Private members
//////////////////////////////////////////////////////////////////////

private:

	//! X Length
	double m_dLgX;

	//! Y Length
	double m_dLgY;

	//! Z Length
	double m_dLgZ;

	
};
#endif //GLC_BOX_H_
