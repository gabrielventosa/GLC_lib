/****************************************************************************

 This file is part of the GLC-lib library.
 Copyright (C) 2005-2008 Laurent Ribon (laumaya@users.sourceforge.net)
 http://glc-lib.sourceforge.net

 GLC-lib is free software; you can redistribute it and/or modify
 it under the terms of the GNU Lesser General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.

 GLC-lib is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with GLC-lib; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

 *****************************************************************************/
//! \file glc_worldto3ds.cpp implementation of the GLC_WorldTo3ds class.

#include <QFile>
#include <QtDebug>

#include "../geometry/glc_3drep.h"
#include "../geometry/glc_geometry.h"
#include "../geometry/glc_mesh.h"

#include "../shading/glc_material.h"

// Lib3ds Header
#include "3rdparty/lib3ds2/lib3ds.h"

#include "glc_worldto3ds.h"

GLC_WorldTo3ds::GLC_WorldTo3ds(const GLC_World& world)
: QObject()
, m_World(world)
, m_pLib3dsFile(NULL)
, m_FileName()
, m_ReferenceToMesh()
, m_NameToMaterial()
, m_pRootLib3dsNode(NULL)
, m_CurrentNodeId(0)
, m_OccIdToNode()
, m_CurrentMeshIndex(0)
, m_UseAbsolutePosition(false)
, m_TextureToFileName()
, m_CurrentMeshCount(0)
{


}

GLC_WorldTo3ds::~GLC_WorldTo3ds()
{

}

//////////////////////////////////////////////////////////////////////
// Set Functions
//////////////////////////////////////////////////////////////////////
bool GLC_WorldTo3ds::exportToFile(const QString& fileName, bool useAbsolutePosition)
{
	m_ReferenceToMesh.clear();
	m_NameToMaterial.clear();
	m_pRootLib3dsNode= NULL;
	m_CurrentNodeId= 0;
	m_OccIdToNode.clear();
	m_CurrentMeshIndex= 0;
	m_UseAbsolutePosition= useAbsolutePosition;
	m_TextureToFileName.clear();
	m_CurrentMeshCount= 0;

	m_FileName= fileName;
	bool subject= false;
	{
		QFile exportFile(m_FileName);
		subject= exportFile.open(QIODevice::WriteOnly);
		exportFile.close();
	}
	if (subject)
	{
		m_pLib3dsFile= lib3ds_file_new();
		saveWorld();
		subject= lib3ds_file_save(m_pLib3dsFile, fileName.toLocal8Bit().data());
	}

	return subject;
}


//////////////////////////////////////////////////////////////////////
// Private services functions
//////////////////////////////////////////////////////////////////////

void GLC_WorldTo3ds::saveWorld()
{
	if (!m_UseAbsolutePosition)
	{
		saveMeshes();
	}

	// Save node structure
	GLC_StructOccurence* pRoot= m_World.rootOccurence();
	const int childCount= pRoot->childCount();
	for (int i= 0; i < childCount; ++i)
	{
		saveBranch(pRoot->child(i));
	}
}

void GLC_WorldTo3ds::saveMeshes()
{
	// Retrieve the list of mesh and save them into the 3ds
	QList<GLC_StructReference*> refList= m_World.references();
	const int refCount= refList.count();
	for (int i= 0; i < refCount; ++i)
	{
		GLC_StructReference* pRef= refList.at(i);
		if (pRef->hasRepresentation())
		{
			GLC_3DRep* pRep= dynamic_cast<GLC_3DRep*>(pRef->representationHandle());
			if (NULL != pRep)
			{
				// This reference has a mesh
				const QString meshName= pRef->name() + '_' + QString::number(++m_CurrentMeshIndex);
				QList<Lib3dsMesh*> meshes= createMeshsFrom3DRep(pRep, meshName);
				{
					const int count= meshes.count();
					int meshIndex= m_CurrentMeshCount;
					m_CurrentMeshCount+= count;
					lib3ds_file_reserve_meshes(m_pLib3dsFile, m_CurrentMeshCount, false);
					for (int i= 0; i < count; ++i, ++meshIndex)
					{
						lib3ds_file_insert_mesh(m_pLib3dsFile, meshes.at(i), meshIndex);
						m_ReferenceToMesh.insertMulti(pRef, meshes.at(i));
					}
				}
			}
		}
	}
}

void GLC_WorldTo3ds::saveBranch(GLC_StructOccurence* pOcc)
{
	createNodeFromOccurrence(pOcc);

	const int childCount= pOcc->childCount();
	for (int i= 0; i < childCount; ++i)
	{
		saveBranch(pOcc->child(i));
	}
}

void GLC_WorldTo3ds::createNodeFromOccurrence(GLC_StructOccurence* pOcc)
{
	Lib3dsNode* p3dsNode =lib3ds_node_new(LIB3DS_NODE_MESH_INSTANCE);
	p3dsNode->node_id= m_CurrentNodeId++;
	m_OccIdToNode.insert(pOcc->id(), p3dsNode);

	if (pOcc->parent() == m_World.rootOccurence())
	{
		p3dsNode->parent= NULL;
	}
	else
	{
		Q_ASSERT(m_OccIdToNode.contains(pOcc->parent()->id()));
		p3dsNode->parent= m_OccIdToNode.value(pOcc->parent()->id());
	}

	lib3ds_file_append_node(m_pLib3dsFile, p3dsNode, p3dsNode->parent);

	GLC_StructReference* pRef= pOcc->structReference();
	if (m_UseAbsolutePosition)
	{
		if (pOcc->structReference()->hasRepresentation())
		{
			GLC_3DRep* pRep= dynamic_cast<GLC_3DRep*>(pOcc->structReference()->representationHandle());
			if (NULL != pRep)
			{
				// This reference has a mesh
				const GLC_Matrix4x4 matrix= pOcc->absoluteMatrix();
				const QString meshName= pRef->name() + '_' + QString::number(++m_CurrentMeshIndex);
				QList<Lib3dsMesh*> meshes= createMeshsFrom3DRep(pRep, meshName, matrix);

				const int meshCount= meshes.count();
				int meshIndex= m_CurrentMeshCount;
				m_CurrentMeshCount+= meshCount;
				lib3ds_file_reserve_meshes(m_pLib3dsFile, m_CurrentMeshCount, false);

				for (int i= 0; i < meshCount; ++i, ++meshIndex)
				{
					lib3ds_file_insert_mesh(m_pLib3dsFile, meshes.at(i), meshIndex);
				}

				if (meshCount > 1)
				{
					for (int i= 0; i < meshCount; ++i)
					{

						Lib3dsNode* pCurrent3dsNode = lib3ds_node_new(LIB3DS_NODE_MESH_INSTANCE);
						pCurrent3dsNode->node_id= m_CurrentNodeId++;
						pCurrent3dsNode->parent= p3dsNode;

						strcpy(pCurrent3dsNode->name, meshes.at(i)->name);

						lib3ds_file_append_node(m_pLib3dsFile, pCurrent3dsNode, pCurrent3dsNode->parent);
					}
				}
				else if (!meshes.isEmpty())
				{
					strcpy(p3dsNode->name, meshes.first()->name);
				}
			}
		}
	}
	else
	{
		// Node matrix
		const GLC_Matrix4x4 matrix= pOcc->structInstance()->relativeMatrix();
		setNodePosition(p3dsNode, matrix);

		// Set mesh name if necessary
		if (m_ReferenceToMesh.contains(pRef))
		{

			QList<Lib3dsMesh*> meshes= m_ReferenceToMesh.values(pRef);
			const int meshCount= meshes.count();
			if (meshCount > 1)
			{
				for (int i= 0; i < meshCount; ++i)
				{

					Lib3dsNode* pCurrent3dsNode = lib3ds_node_new(LIB3DS_NODE_MESH_INSTANCE);
					pCurrent3dsNode->node_id= m_CurrentNodeId++;
					pCurrent3dsNode->parent= p3dsNode;

					strcpy(pCurrent3dsNode->name, meshes.at(i)->name);
					lib3ds_file_append_node(m_pLib3dsFile, pCurrent3dsNode, pCurrent3dsNode->parent);
				}
			}
			else
			{
				strcpy(p3dsNode->name, m_ReferenceToMesh.value(pRef)->name);
			}

		}
	}
}

QList<Lib3dsMesh*> GLC_WorldTo3ds::createMeshsFrom3DRep(GLC_3DRep* pRep, const QString& name, const GLC_Matrix4x4& matrix)
{
	QList<Lib3dsMesh*> subject;
	int bodyIndex= 0;

	const int bodyCount= pRep->numberOfBody();
	for (int i= 0; i < bodyCount; ++i)
	{
		GLC_Mesh* pCurrentMesh= dynamic_cast<GLC_Mesh*>(pRep->geomAt(i));
		if ((NULL != pCurrentMesh) && !pCurrentMesh->isEmpty())
		{
			bool deleteCurrentMesh= false;
			if (pCurrentMesh->lodCount() > 1)
			{
				// Keep only the first level of detail
				pCurrentMesh= pCurrentMesh->createMeshOfGivenLod(0);
				deleteCurrentMesh= true;
			}
			const QString bodyMeshName= name + '_' + QString::number(bodyIndex++);
			if (matrix.type() != GLC_Matrix4x4::Identity)
			{
				if (!deleteCurrentMesh)
				{
					pCurrentMesh= new GLC_Mesh(*pCurrentMesh);
					deleteCurrentMesh= true;
				}
				pCurrentMesh->transformVertice(matrix);
				Q_ASSERT(!pCurrentMesh->isEmpty());
			}
			Lib3dsMesh* p3dsMesh= create3dsMeshFromGLC_Mesh(pCurrentMesh, bodyMeshName);

			if (deleteCurrentMesh) delete pCurrentMesh;
			subject.append(p3dsMesh);
		}
	}

	return subject;
}

Lib3dsMesh* GLC_WorldTo3ds::create3dsMeshFromGLC_Mesh(GLC_Mesh* pMesh, const QString& meshName)
{
	// Create empty 3ds mesh with the given name
	Lib3dsMesh* p3dsMesh= lib3ds_mesh_new(meshName.toLocal8Bit().data());

	const int stride= 3;

	GLfloatVector vertice= pMesh->positionVector();
	GLfloatVector texelVector= pMesh->texelVector();

	const uint pointsCount= vertice.count() / stride;

	// Add points to the 3DS mesh
	lib3ds_mesh_resize_vertices(p3dsMesh, pointsCount, !texelVector.isEmpty(), true);

	for (uint i= 0; i < pointsCount; ++i)
	{
		p3dsMesh->vertices[i][0]= vertice[i * 3];
		p3dsMesh->vertices[i][1]= vertice[i * 3 + 1];
		p3dsMesh->vertices[i][2]= vertice[i * 3 + 2];
	}

	// Add texel to the 3DS mesh
	if(!texelVector.isEmpty())
	{
		for (uint i= 0; i < pointsCount; ++i)
		{
			p3dsMesh->texcos[i][0]= texelVector[i * 2];
			p3dsMesh->texcos[i][1]= texelVector[i * 2 + 1];
		}
	}

	// Add faces to the 3ds mesh
	const uint totalFaceCount= pMesh->faceCount(0);
	lib3ds_mesh_resize_faces(p3dsMesh, totalFaceCount);

	QSet<GLC_Material*> materialSet= pMesh->materialSet();
	QSet<GLC_Material*>::iterator iMat= materialSet.begin();
	uint currentFaceIndex= 0;
	while(iMat != materialSet.end())
	{
		GLC_Material* pCurrentGLCMat= *iMat;
		Lib3dsMaterial* pMaterial= get3dsMaterialFromGLC_Material(pCurrentGLCMat);
		IndexList currentTriangleIndex= pMesh->getEquivalentTrianglesStripsFansIndex(0, pCurrentGLCMat->id());
		const int faceCount= currentTriangleIndex.count() / 3;
		for (int i= 0; i < faceCount; ++i)
		{
			Lib3dsFace face;
			face.material= lib3ds_file_material_by_name(m_pLib3dsFile, pMaterial->name);

			face.index[0]= currentTriangleIndex.at(i * 3);
			face.index[1]= currentTriangleIndex.at(i * 3 + 1);
			face.index[2]= currentTriangleIndex.at(i * 3 + 2);

			p3dsMesh->faces[currentFaceIndex++]= face;
			Q_ASSERT(currentFaceIndex <= totalFaceCount);
		}
		++iMat;
	}

	return p3dsMesh;
}

Lib3dsMaterial* GLC_WorldTo3ds::get3dsMaterialFromGLC_Material(GLC_Material* pMat)
{
	Lib3dsMaterial* pSubject= NULL;
	const QString matName= materialName(pMat);
	if (m_NameToMaterial.contains(matName))
	{
		pSubject= m_NameToMaterial.value(matName);
	}
	else
	{
		pSubject= create3dsMaterialFromGLC_Material(pMat, matName);
	}

	return pSubject;
}

Lib3dsMaterial* GLC_WorldTo3ds::create3dsMaterialFromGLC_Material(GLC_Material* pMat, const QString& matName)
{
	Lib3dsMaterial* pSubject= new Lib3dsMaterial;
	//lib3ds_file_reserve_materials(m_pLib3dsFile, m_pLib3dsFile->materials_size + 1, false);

	lib3ds_file_insert_material(m_pLib3dsFile, pSubject, m_pLib3dsFile->materials_size - 1);

	strcpy(pSubject->name, matName.toLocal8Bit().data());


	// Ambient Color
	QColor ambient= pMat->ambientColor();
	pSubject->ambient[0]= static_cast<float>(ambient.redF());
	pSubject->ambient[1]= static_cast<float>(ambient.greenF());
	pSubject->ambient[2]= static_cast<float>(ambient.blueF());
	pSubject->ambient[3]= static_cast<float>(ambient.alphaF());

	// Diffuse Color
	QColor diffuse= pMat->diffuseColor();
	pSubject->diffuse[0]= static_cast<float>(diffuse.redF());
	pSubject->diffuse[1]= static_cast<float>(diffuse.greenF());
	pSubject->diffuse[2]= static_cast<float>(diffuse.blueF());
	pSubject->diffuse[3]= static_cast<float>(diffuse.alphaF());

	// Specular Color
	QColor specular= pMat->specularColor();
	pSubject->specular[0]= static_cast<float>(specular.redF());
	pSubject->specular[1]= static_cast<float>(specular.greenF());
	pSubject->specular[2]= static_cast<float>(specular.blueF());
	pSubject->specular[3]= static_cast<float>(specular.alphaF());


	// Shininess
	pSubject->shininess= pMat->shininess();

	// Transparency

	pSubject->transparency= 1.0f - static_cast<float>(pMat->opacity());

	// Texture
	if (pMat->hasTexture())
	{
		if (!m_TextureToFileName.contains(pMat->textureHandle()))
		{
			QString filePath= QFileInfo(m_FileName).absolutePath();
			QString textureName= matName;
			QImage textureImage= pMat->textureHandle()->imageOfTexture();
			if (!pMat->textureFileName().isEmpty())
			{
				textureName= matName + '-' + QFileInfo(pMat->textureFileName()).fileName();
				textureImage.load(pMat->textureFileName());
			}
			else
			{
				textureName= textureName + ".jpg";
			}
			textureName= textureName.right(63);

			if (!textureImage.isNull())
			{
				const QString type(QFileInfo(textureName).suffix());
				QString newTextureFile= filePath + QDir::separator() + textureName;
				textureImage.save(newTextureFile, type.toUpper().toAscii().data());
				strcpy(pSubject->texture1_map.name, textureName.toLocal8Bit().data());
				m_TextureToFileName.insert(pMat->textureHandle(), textureName);
			}
		}
		else
		{
			QString textureName= m_TextureToFileName.value(pMat->textureHandle());
			strcpy(pSubject->texture1_map.name, textureName.toLocal8Bit().data());
		}

	}

	m_NameToMaterial.insert(matName, pSubject);

	return pSubject;
}

QString GLC_WorldTo3ds::materialName(GLC_Material* pMat) const
{
	QString subject= pMat->name() + '_' + QString::number(pMat->id());
	subject= subject.right(63);

	return subject;
}

void GLC_WorldTo3ds::setNodePosition(Lib3dsNode* pNode, const GLC_Matrix4x4& matrix)
{
	Lib3dsMeshInstanceNode* pMeshNode= (Lib3dsMeshInstanceNode*)pNode;

	// Translation

	Lib3dsKey* pTranslationKey= new Lib3dsKey;
	pTranslationKey->value[0]= matrix.getData()[12];
	pTranslationKey->value[1]= matrix.getData()[13];
	pTranslationKey->value[2]= matrix.getData()[14];

	pTranslationKey->frame= 1;
	pMeshNode->pos_track.nkeys= 1;
	pMeshNode->pos_track.keys= pTranslationKey;

	// Scaling
	Lib3dsKey* pScalingKey= new Lib3dsKey;
	pScalingKey->value[0]= static_cast<float>(matrix.scalingX());
	pScalingKey->value[1]= static_cast<float>(matrix.scalingY());
	pScalingKey->value[2]= static_cast<float>(matrix.scalingZ());

	pScalingKey->frame= 1;
	pMeshNode->scl_track.nkeys= 1;
	pMeshNode->scl_track.keys= pScalingKey;

	// Rotation

	Lib3dsKey* pRotationKey= new Lib3dsKey;

	QQuaternion quaternion= matrix.quaternion();
	QPair<GLC_Vector3d, double> pair= matrix.rotationVectorAndAngle();

	pRotationKey->value[3]= static_cast<float>(pair.second);
	pRotationKey->value[0]= static_cast<float>(pair.first.x());
	pRotationKey->value[1]= static_cast<float>(pair.first.y());
	pRotationKey->value[2]= static_cast<float>(pair.first.z());

	pRotationKey->frame= 1;
	pMeshNode->rot_track.nkeys= 1;
	pMeshNode->rot_track.keys= pRotationKey;

}
