#include "glc_worldtostl.h"

#include "../glc_exception.h"
#include "../geometry/glc_mesh.h"

GLC_WorldToSTL::GLC_WorldToSTL(const GLC_World& world, bool threaded) :
    QObject()
  , m_World(world)
  , m_FileName()
//  , m_pOutStream(NULL)
//  , m_CurrentId(0)
  , m_pCurrentFile(NULL)
  , m_AbsolutePath()
//  , m_ReferenceToIdHash()
//  , m_InstanceToIdHash()
//  , m_ReferenceRepToIdHash()
//  , m_InstanceRep()
  , m_FileNameIncrement(0)
 // , m_ListOfOverLoadedOccurence()
  , m_pReadWriteLock(NULL)
  , m_pIsInterupted(NULL)
  , m_IsThreaded(threaded)
{
    m_World.rootOccurence()->updateOccurenceNumber(1);
}

GLC_WorldToSTL::~GLC_WorldToSTL()
{
    delete m_pCurrentFile;
}

bool GLC_WorldToSTL::exportToSTL(const QString& filename)
{
    m_FileNameIncrement= 0;
    m_FileName= filename;

    //int previousQuantumValue= 0;
    //int currentQuantumValue= 0;
    //emit currentQuantum(currentQuantumValue);

    QFile file (m_FileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
             return false;

    QTextStream outStream(&file);

    exportAssemblyFromOccurence(m_World.rootOccurence(), outStream);

    file.close();

    return true;

}

void GLC_WorldToSTL::exportAssemblyFromOccurence(const GLC_StructOccurence* pOccurence, QTextStream &outStream)
{

    const int childCount = pOccurence->childCount();
    for (int i=0;i < childCount; i++)
    {
        exportAssemblyFromOccurence(pOccurence->child(i), outStream);
        qDebug() << "Processing child: " << pOccurence->child(i)->structReference()->name();
        QString ChildName = pOccurence->child(i)->structReference()->name();
        GLC_StructReference* pCurrentRef= pOccurence->child(i)->structReference();
        if (pCurrentRef->hasRepresentation())
        {
            GLC_3DRep* pCurrentRep= dynamic_cast<GLC_3DRep*>(pCurrentRef->representationHandle());
            for (int j=0; j<pCurrentRep->numberOfBody();j++)
            {

                QFile dfile ("/Users/gabriel/stl2/"+ChildName+"-"+"MESH-"+QString::number(j)+".stl");
                if (!dfile.open(QIODevice::WriteOnly | QIODevice::Text))
                         return;
                QTextStream doutStream(&dfile);
                doutStream << "solid " << pOccurence->child(i)->structReference()->name() << "\n";


                GLC_Mesh* pMesh = dynamic_cast <GLC_Mesh*> (pCurrentRep->geomAt(j));
                QSet<GLC_Material*> materialSet = pMesh->materialSet();
                QSet<GLC_Material*>::iterator iMat = materialSet.begin();
                while(iMat !=materialSet.end())

                {
                        GLC_Material* pCurrentGLCMat =*iMat;
                        QString MaterialName = pCurrentGLCMat->name();

                        GLfloatVector positionVector = pMesh->positionVector();

                        IndexList currentTriangleIndex= pMesh->getEquivalentTrianglesStripsFansIndex(0, pCurrentGLCMat->id());
                        const int faceCount= currentTriangleIndex.count() / 3;
                        outStream << "solid " << pOccurence->child(i)->structReference()->name() << "\n";


                        for (int k=0; k < faceCount; k++)
                        {

                           GLuint vertex1 = currentTriangleIndex.at(k*3);
                           GLuint vertex2 = currentTriangleIndex.at((k*3)+1);
                           GLuint vertex3 = currentTriangleIndex.at((k*3)+2);

                           GLfloatVector faceVertexCoords;
                           faceVertexCoords.append(positionVector.at(vertex1*3));
                           faceVertexCoords.append(positionVector.at((vertex1*3)+1));
                           faceVertexCoords.append(positionVector.at((vertex1*3)+2));

                           faceVertexCoords.append(positionVector.at(vertex2*3));
                           faceVertexCoords.append(positionVector.at((vertex2*3)+1));
                           faceVertexCoords.append(positionVector.at((vertex2*3)+2));

                           faceVertexCoords.append(positionVector.at(vertex3*3));
                           faceVertexCoords.append(positionVector.at((vertex3*3)+1));
                           faceVertexCoords.append(positionVector.at((vertex3*3)+2));


                           GLfloatVector normalVector = calculateNormals(faceVertexCoords);


                            outStream<< "    facet normal " << QString::number(normalVector.at(0),'E',6) << " " << QString::number(normalVector.at(1),'E',6) << " " << QString::number(normalVector.at(1),'E',6) << "\n";
                            outStream<< "    outer loop\n";
                            outStream<< "    vertex " << QString::number(faceVertexCoords.at(0),'E',6) << " " << QString::number(faceVertexCoords.at(1),'E',6) << " " << QString::number(faceVertexCoords.at(2),'E',6)<<"\n";
                            outStream<< "    vertex " << QString::number(faceVertexCoords.at(3),'E',6) << " " << QString::number(faceVertexCoords.at(4),'E',6) << " " << QString::number(faceVertexCoords.at(5),'E',6)<<"\n";
                            outStream<< "    vertex " << QString::number(faceVertexCoords.at(6),'E',6) << " " << QString::number(faceVertexCoords.at(7),'E',6) << " " << QString::number(faceVertexCoords.at(8),'E',6)<<"\n";
                            outStream<< "    endloop\n";
                            outStream<< "    endfacet\n";

                            doutStream<< "    facet normal " << QString::number(normalVector.at(0),'E',6) << " " << QString::number(normalVector.at(1),'E',6) << " " << QString::number(normalVector.at(1),'E',6) << "\n";
                            doutStream<< "    outer loop\n";
                            doutStream<< "    vertex " << QString::number(faceVertexCoords.at(0),'E',6) << " " << QString::number(faceVertexCoords.at(1),'E',6) << " " << QString::number(faceVertexCoords.at(2),'E',6)<<"\n";
                            doutStream<< "    vertex " << QString::number(faceVertexCoords.at(3),'E',6) << " " << QString::number(faceVertexCoords.at(4),'E',6) << " " << QString::number(faceVertexCoords.at(5),'E',6)<<"\n";
                            doutStream<< "    vertex " << QString::number(faceVertexCoords.at(6),'E',6) << " " << QString::number(faceVertexCoords.at(7),'E',6) << " " << QString::number(faceVertexCoords.at(8),'E',6)<<"\n";
                            doutStream<< "    endloop\n";
                            doutStream<< "    endfacet\n";


                        }
                        outStream << "endsolid " << pOccurence->child(i)->structReference()->name()<<"\n";

                        ++iMat;


                }
                doutStream << "endsolid " << pOccurence->child(i)->structReference()->name()<<"\n";
                dfile.close();
            }
        }
    }

    return;
}

GLfloatVector GLC_WorldToSTL::calculateNormals(GLfloatVector NormalsVector){

    GLfloatVector normals;

        float orig1 = NormalsVector.at(0);
        float orig2 = NormalsVector.at(1);
        float orig3 = NormalsVector.at(2);

        float a1 = NormalsVector.at(3)-orig1;
        float a2 = NormalsVector.at(4)-orig2;
        float a3 = NormalsVector.at(5)-orig3;

        float b1 = NormalsVector.at(6)-orig1;
        float b2 = NormalsVector.at(7)-orig2;
        float b3 = NormalsVector.at(8)-orig3;

        float n1 = a2*b3-a3*b2;
        float n2 = a3*b1-a1*b3;
        float n3 = a1*b2-a2*b1;

        normals.append(n1);
        normals.append(n2);
        normals.append(n3);

    return normals;

}
