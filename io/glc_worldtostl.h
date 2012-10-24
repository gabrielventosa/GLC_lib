#ifndef GLC_WORLDTOSTL_H
#define GLC_WORLDTOSTL_H

#include <QObject>

#include "../sceneGraph/glc_world.h"
#include "../glc_config.h"
#include <QReadWriteLock>

class QFile;
class GLC_Mesh;

class GLC_LIB_EXPORT GLC_WorldToSTL : public QObject
{
    Q_OBJECT
public:
    GLC_WorldToSTL(const GLC_World& world, bool threaded= true);
    virtual ~GLC_WorldToSTL();

//////////////////////////////////////////////////////////////////////
/*! @name Set Functions*/
//@{
//////////////////////////////////////////////////////////////////////
public:
    //! Save the world to the specified file name
    bool exportToSTL(const QString& filename);

//@}

private:
    //! Export the assembly structure from the list of reference
    void exportAssemblyStructure();
    void exportAssemblyFromOccurence(const GLC_StructOccurence* pOccurence, QTextStream &outStream);
    GLfloatVector calculateNormals(GLfloatVector NormalsVector);


signals:
    
public slots:

////////////////////////////////////////////////////////
///* Private members
///////////////////////////////////////////////////////
private:

    //! The world to export
    GLC_World m_World;

    //! The file name in which the world is exported
    QString m_FileName;

    //! The current file
    QFile* m_pCurrentFile;

    //! the 3dxml absolute path
    QString m_AbsolutePath;

    //! file name increment
    unsigned int m_FileNameIncrement;

    //! Mutex
    QReadWriteLock* m_pReadWriteLock;

    //! Flag to know if export must be interupted
    bool* m_pIsInterupted;

    //! Flag to know if export is threaded (the default)
    bool m_IsThreaded;

    
};

#endif // GLC_WORLDTOSTL_H
