/****************************************************************************

 This file is part of the GLC-lib library.
 Copyright (C) 2005-2008 Laurent Ribon (laumaya@users.sourceforge.net)
 Version 1.1.0, packaged on March, 2009.

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

//! \file glc_offtoworld.h interface for the GLC_OffToWorld class.

#ifndef GLC_OFFTOWORLD_H_
#define GLC_OFFTOWORLD_H_

#include <QString>
#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QColor>
#include <QList>

#include "glc_vector3d.h"
#include "glc_vector2df.h"
#include "glc_vector3df.h"
#include "glc_mesh2.h"

class GLC_World;
class QGLContext;

//////////////////////////////////////////////////////////////////////
//! \class GLC_OffToWorld
/*! \brief GLC_OffToWorld : Create an GLC_World from off file */

/*! An GLC_OffToWorld extract the only mesh from an .off file \n
 * 	List of elements extracted from the off
 * 		- Vertex
 * 		- Face
 * 		- Normal coordinate
 * 		- Face Color
 * 		- Vertex Color
  */
//////////////////////////////////////////////////////////////////////

class GLC_OffToWorld : public QObject
{
	Q_OBJECT
//////////////////////////////////////////////////////////////////////
/*! @name Constructor / Destructor */
//@{
//////////////////////////////////////////////////////////////////////

public:
	GLC_OffToWorld();
	virtual ~GLC_OffToWorld();
//@}
//////////////////////////////////////////////////////////////////////
/*! @name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
	//! Create an GLC_World from an input OFF File
	GLC_World* CreateWorldFromOff(QFile &file);
//@}

//////////////////////////////////////////////////////////////////////
/*! @name Private services functions */
//@{
//////////////////////////////////////////////////////////////////////
private:
	//! clear stlToWorld allocate memmory
	void clear();
	//! Extract a vertex from a string and add color component if needed
	void extractVertex(QString &);
	//! Extract Number off Vertex and faces
	void extractNbrVertexsAndNbrFaces(QString &);
	//! Extract a face from a string
	void extractFaceIndex(QString &);
	//! compute face normal
	GLC_Vector3df computeNormal(QVector<int> &);


//@}

//////////////////////////////////////////////////////////////////////
// Qt Signals
//////////////////////////////////////////////////////////////////////
	signals:
	void currentQuantum(int);

//////////////////////////////////////////////////////////////////////
	/* Private members */
//////////////////////////////////////////////////////////////////////
private:
	//! pointer to a GLC_World
	GLC_World* m_pWorld;
	//! The Stl File name
	QString m_FileName;
	//! The current line number
	int m_CurrentLineNumber;
	//! The current mesh
	GLC_Mesh2* m_pCurrentMesh;
	//! Index of the current vertex
	int m_CurVertexIndex;
	//! The number of vertexs
	int m_NbrOfVertexs;
	//! The number of faces
	int m_NbrOfFaces;
	//! The OFF is a COFF
	bool m_IsCoff;
	//! The OFF is 4OFF
	bool m_Is4off;
	// The list of GLC_Vertex of the current face
	VertexList m_CurrentListOfVertex;


};

#endif /*GLC_OFFTOWORLD_H_*/
