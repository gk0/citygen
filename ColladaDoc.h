#ifndef COLLADADOC_H
#define COLLADADOC_H

#include "stdafx.h"
#include "ExportDoc.h"

#include <tinyxml.h>

class ColladaDoc : public ExportDoc
{
private:
	TiXmlDocument _doc;
	TiXmlElement* _root;
	TiXmlElement* _libCamera;
	TiXmlElement* _libEffect;
	TiXmlElement* _libImage;
	TiXmlElement* _libLight;
	TiXmlElement* _libMaterial;
	TiXmlElement* _libGeometry;
	TiXmlElement* _libVisScene;
	TiXmlElement* _defaultScene;
	std::set<Ogre::String> _meshNames;
	std::set<Ogre::String> _materialNames;
	std::set<std::string> _texFiles;
	std::string _file;
	std::string _texFolder;

public:
	ColladaDoc(const std::string &file);
	bool save();
	void addMesh(Ogre::MeshPtr mesh);
	void addMesh(Ogre::SceneNode* sn, Ogre::MeshPtr mesh);
	void addCamera(Ogre::Camera *cam, Ogre::SceneNode *camNode);
	void addLight(Ogre::Light *l);
	std::string getFileNameFromPath(const std::string &path);
	std::string getFolderPathFromPath(const std::string &path);

private:
	void addDefaultScene();

	void addMaterial(Ogre::Material *m);
	void addMeshInstance(Ogre::MeshPtr mesh);
	void addMeshToLibrary(Ogre::MeshPtr mesh);


	// xml helpers
	static TiXmlElement* createInput(const char* semantic, std::string& source);
	static TiXmlElement* createNode(const Ogre::SceneNode *nd);
	static TiXmlElement* createNode(const std::string &id, const Ogre::Vector3 &pos, const Ogre::Quaternion &orient);

};

#endif
