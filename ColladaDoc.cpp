#include "stdafx.h"
#include "ColladaDoc.h"
#include <boost/filesystem.hpp>

using namespace std;
using namespace Ogre;

ColladaDoc::ColladaDoc(const string &file)
 : _file(file)
{
	// set texture folder
	_texFolder = getFileNameFromPath(_file) + "_textures";
	replace(_texFolder.begin(), _texFolder.end(), ' ', '_');

	TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "utf-8", ""); 
	_doc.LinkEndChild(decl);
	_root = new TiXmlElement("COLLADA");
	_root->SetAttribute("xmlns", "http://www.collada.org/2005/11/COLLADASchema");
	_root->SetAttribute("version", "1.4.1");

	// <contributor>
	TiXmlElement* contributor = new TiXmlElement("contributor");
	TiXmlElement* author = new TiXmlElement("author");
	author->InsertEndChild(TiXmlText("Citygen Collada exporter - http://www.citygen.net"));
	TiXmlElement* authoring_tool = new TiXmlElement("authoring_tool");
	authoring_tool->InsertEndChild(TiXmlText("Citygen - Interactive Procedural City Generation - http://www.citygen.net"));
	TiXmlElement* comments = new TiXmlElement("comments");
	comments->InsertEndChild(TiXmlText("Citygen may only be used commercially if licensed to do so. Contact: http://www.citygen.net/contact"));
	contributor->LinkEndChild(author);
	contributor->LinkEndChild(authoring_tool);
	contributor->LinkEndChild(comments);
	
	// <asset>
	TiXmlElement* asset = new TiXmlElement("asset");
	TiXmlElement* created = new TiXmlElement("created");
	wxString datewxString = wxDateTime::Now().Format(_("%Y-%m-%dT%H:%M:%SZ"));
	string dateString(datewxString.mb_str(wxCSConv(wxLocale::GetSystemEncoding())));
	created->InsertEndChild(TiXmlText(dateString.c_str()));
	TiXmlElement* modified = new TiXmlElement("modified");
	modified->InsertEndChild(TiXmlText(dateString.c_str()));
	TiXmlElement* up_axis = new TiXmlElement("up_axis");
	up_axis->InsertEndChild(TiXmlText("Y_UP"));
	asset->LinkEndChild(contributor);
	asset->LinkEndChild(created);
	asset->LinkEndChild(modified);
	asset->LinkEndChild(up_axis);
	_root->LinkEndChild(asset);  

	// doc setup
	_libCamera = new TiXmlElement("library_cameras");
	_libEffect = new TiXmlElement("library_effects");
	_libImage = new TiXmlElement("library_images");
	_libLight = new TiXmlElement("library_lights");
	_libMaterial = new TiXmlElement("library_materials");
	_libGeometry = new TiXmlElement("library_geometries");
	_libVisScene = new TiXmlElement("library_visual_scenes");
	_root->LinkEndChild(_libCamera);
	_root->LinkEndChild(_libEffect);
	_root->LinkEndChild(_libImage);
	_root->LinkEndChild(_libLight);
	_root->LinkEndChild(_libMaterial);
	_root->LinkEndChild(_libGeometry);
	_root->LinkEndChild(_libVisScene);
	_doc.LinkEndChild(_root);

	addDefaultScene();
}

bool ColladaDoc::save()
{
	// copy textures to 'filename_textures' folder
	string defaultSrcFolder = "./Media/materials/textures/";
	string destFolder = getFolderPathFromPath(_file) + _texFolder + "/";
	using namespace boost::filesystem;
	try
	{
		create_directory(destFolder);
		BOOST_FOREACH(string texFile, _texFiles)
		{
			if(exists(destFolder+texFile)) remove(destFolder+texFile);
			copy_file(defaultSrcFolder+texFile, destFolder+texFile);
		}
	}
	catch(...)
	{
		return false;
	}
	return _doc.SaveFile(_file.c_str());
}

void ColladaDoc::addDefaultScene()
{
	//<visual_scene id="Scene0"></visual_scene>
	_defaultScene = new TiXmlElement("visual_scene");
	_defaultScene->SetAttribute("id", "Scene0");
	_libVisScene->LinkEndChild(_defaultScene);
	//<scene>
	//	<instance_visual_scene url="#Scene0"/>
	//</scene>
	TiXmlElement* scene = new TiXmlElement("scene");
	TiXmlElement* ivs = new TiXmlElement("instance_visual_scene");
	ivs->SetAttribute("url", "#Scene0");
	scene->LinkEndChild(ivs);
	_root->LinkEndChild(scene);
}


TiXmlElement* createSource(const string &id, const size_t count, const string &params, const stringstream &os)
{
	TiXmlElement* source = new TiXmlElement("source");
	source->SetAttribute("id", id.c_str());

	TiXmlElement* float_array = new TiXmlElement("float_array");
	float_array->SetAttribute("id", (id+"-array").c_str());
	float_array->SetAttribute("count", static_cast<int>(count*params.size()));
	float_array->InsertEndChild(TiXmlText(os.str().c_str()));

	TiXmlElement* technique_common = new TiXmlElement("technique_common");
	TiXmlElement* accessor = new TiXmlElement("accessor");
	accessor->SetAttribute("source", ("#"+id+"-array").c_str());
	accessor->SetAttribute("count", static_cast<int>(count));
	accessor->SetAttribute("stride", static_cast<int>(params.size()));
	technique_common->LinkEndChild(accessor);
	char paramName[2] = {'X',0};
	BOOST_FOREACH(char c, params)
	{
		paramName[0] = c;
		TiXmlElement* param = new TiXmlElement("param");
		param->SetAttribute("name", paramName);
		param->SetAttribute("type", "float");
		accessor->LinkEndChild(param);
	}

	source->LinkEndChild(float_array);
	source->LinkEndChild(technique_common);

	return source;
}

void ColladaDoc::addMesh(MeshPtr mesh)
{
	addMeshToLibrary(mesh);

	// add mesh instance
	for(unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
	{
		string name = mesh.getPointer()->getName()+StringConverter::toString(i);

		// add an instance of the geometry to the scene
		TiXmlElement* node = new TiXmlElement("node");
		node->SetAttribute("id", (name+"node").c_str());
		node->SetAttribute("type", "NODE");
		TiXmlElement* geom_instance = new TiXmlElement("instance_geometry");
		geom_instance->SetAttribute("url", ("#"+name).c_str());
		node->LinkEndChild(geom_instance);
		_defaultScene->LinkEndChild(node);
	}
}

void ColladaDoc::addMesh(SceneNode* sn, MeshPtr mesh)
{
	addMeshToLibrary(mesh);

	// add mesh instance
	for(unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
	{
		string name = mesh.getPointer()->getName()+StringConverter::toString(i);

		// add an instance of the geometry to the scene
		TiXmlElement* node = createNode(sn);
		TiXmlElement* geom_instance = new TiXmlElement("instance_geometry");
		geom_instance->SetAttribute("url", ("#"+name).c_str());
		node->LinkEndChild(geom_instance);
		_defaultScene->LinkEndChild(node);
	}
}

void ColladaDoc::addMeshToLibrary(MeshPtr mesh)
{
	//string name = mesh.getPointer()->getName();

	// check if mesh uses shared vertex data
	for(unsigned short i = 0; i < mesh->getNumSubMeshes(); ++i)
	{
		Ogre::SubMesh* submesh = mesh->getSubMesh(i);
		string name = mesh.getPointer()->getName()+StringConverter::toString(i);

		// we only need to add the shared vertices once
		if(submesh->useSharedVertices)
			Ogre::LogManager::getSingleton().logMessage("down with shared vertices");

		string matName = submesh->getMaterialName();
		size_t vertexCount = submesh->vertexData->vertexCount;

		// begin geom
		TiXmlElement* geom = new TiXmlElement("geometry");
		geom->SetAttribute("id", name.c_str());
		TiXmlElement* geomMesh = new TiXmlElement("mesh");


		Ogre::VertexData* vertex_data = submesh->useSharedVertices ? mesh->sharedVertexData : submesh->vertexData;

		if(!submesh->useSharedVertices)
		{
			// vertex data
			//////////////////////////////////////////////////////////////////////////

			const Ogre::VertexElement* posElem =
				vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);

			const Ogre::VertexElement* normElem =
				vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_NORMAL);

			const Ogre::VertexElement* texElem =
				vertex_data->vertexDeclaration->findElementBySemantic(Ogre::VES_TEXTURE_COORDINATES);

			Ogre::HardwareVertexBufferSharedPtr vbuf =
				vertex_data->vertexBufferBinding->getBuffer(posElem->getSource());

			unsigned char* vertex =
				static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));

			// There is _no_ baseVertexPointerToElement() which takes an Ogre::Real or a double
			//  as second argument. So make it float, to avoid trouble when Ogre::Real will
			//  be compiled/type defined as double:
			//      Ogre::Real* pReal;
			float* pReal;
			stringstream posSS, normSS, texSS;

			for( size_t j = 0; j < vertex_data->vertexCount; ++j, vertex += vbuf->getVertexSize())
			{
				posElem->baseVertexPointerToElement(vertex, &pReal);
				posSS << pReal[0] << " " << pReal[1 ] << " " << pReal[2] << " ";

				normElem->baseVertexPointerToElement(vertex, &pReal);
				normSS << pReal[0] << " " << pReal[1] << " " << pReal[2] << " ";

				texElem->baseVertexPointerToElement(vertex, &pReal);
				texSS << pReal[0] << " " << pReal[1] << " ";
			}
			vbuf->unlock();

			// add vertex data to XML
			//////////////////////////////////////////////////////////////////////////

			// Source Position
			TiXmlElement* source1 = createSource(name+"Pos", vertexCount, "XYZ", posSS);
			TiXmlElement* source2 = createSource(name+"Norm", vertexCount, "XYZ", normSS);
			TiXmlElement* source3 = createSource(name+"Tex", vertexCount, "ST", texSS);

			// Vertices's
			TiXmlElement* vertices = new TiXmlElement("vertices");
			vertices->SetAttribute("id", (name+"-vertices").c_str());
			String x("#"+name+"Pos");
			TiXmlElement* inputPos = createInput("POSITION", x);
			vertices->LinkEndChild(inputPos);

			// index data
			//////////////////////////////////////////////////////////////////////////

			Ogre::IndexData* index_data = submesh->indexData;
			size_t triangleCount = index_data->indexCount / 3;
			Ogre::HardwareIndexBufferSharedPtr ibuf = index_data->indexBuffer;

			bool use32bitindexes = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);

			unsigned long*  pLong = static_cast<unsigned long*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
			unsigned short* pShort = reinterpret_cast<unsigned short*>(pLong);

			stringstream indexSS;

			if(use32bitindexes)
				for(size_t k=0; k<index_data->indexCount; k = k+3)
					indexSS << pLong[k] << " " << pLong[k+1] << " " << pLong[k+2] << " ";
			else
				for(size_t k=0; k<index_data->indexCount; k = k+3)
					indexSS << pShort[k] << " " << pShort[k+1] << " " << pShort[k+2] << " ";

			ibuf->unlock();


			// add index data to XML
			//////////////////////////////////////////////////////////////////////////
			TiXmlElement* triangles = new TiXmlElement("triangles");
			string tmp(matName);
			replace(tmp.begin(), tmp.end(), '/', '_');
			triangles->SetAttribute("material", (tmp+"-material").c_str());
			triangles->SetAttribute("count", static_cast<int>(triangleCount));
			String verSource("#"+name+"-vertices"), normSource("#"+name+"Norm"), texSource("#"+name+"Tex");
			TiXmlElement* inputVer = createInput("VERTEX", verSource);
			inputVer->SetAttribute("offset","0");
			TiXmlElement* inputNorm = createInput("NORMAL", normSource);
			inputNorm->SetAttribute("offset","0");
			TiXmlElement* inputTex = createInput("TEXCOORD", texSource);
			inputTex->SetAttribute("set","0");
			inputTex->SetAttribute("offset","0");
			TiXmlElement* index_array = new TiXmlElement("p");
			index_array->InsertEndChild(TiXmlText(indexSS.str().c_str()));
			triangles->LinkEndChild(inputVer);
			triangles->LinkEndChild(inputNorm);
			triangles->LinkEndChild(inputTex);
			triangles->LinkEndChild(index_array);

			// add geom
			geomMesh->LinkEndChild(source1);
			geomMesh->LinkEndChild(source2);
			geomMesh->LinkEndChild(source3);
			geomMesh->LinkEndChild(vertices);
			geomMesh->LinkEndChild(triangles);
			geom->LinkEndChild(geomMesh);

			// add Geometry Object to Library
			_libGeometry->LinkEndChild(geom);

			// if material has not already been added
			if(_materialNames.find(matName) == _materialNames.end())
			{
				MaterialPtr matPtr = static_cast<MaterialPtr>(MaterialManager::getSingleton().getByName(matName));
				addMaterial(matPtr.get());
				
				// add material to set
				_materialNames.insert(matName);
			}
		}
	}
}

// <input semantic="TEXCOORD" source="#texture2" offset=”0”/>
TiXmlElement* ColladaDoc::createInput(const char* semantic, string& source)
{
	TiXmlElement* input = new TiXmlElement("input");
	input->SetAttribute("semantic", semantic);
	input->SetAttribute("source", source.c_str());
	return input;
}

void ColladaDoc::addCamera(Camera *cam, SceneNode *camNode)
{
	string id = cam->getName();
	TiXmlElement* camera = new TiXmlElement("camera");
	camera->SetAttribute("id", (id+"-obj").c_str());
	camera->SetAttribute("name", id.c_str());

	TiXmlElement* optics = new TiXmlElement("optics");
	TiXmlElement* technique_common = new TiXmlElement("technique_common");
	TiXmlElement* perspective = new TiXmlElement("perspective");
	TiXmlElement* yfov = new TiXmlElement("yfov");
	yfov->InsertEndChild(TiXmlText(StringConverter::toString(cam->getFOVy()).c_str()));
	TiXmlElement* aspect_ratio = new TiXmlElement("aspect_ratio");
	aspect_ratio->InsertEndChild(TiXmlText(StringConverter::toString(cam->getAspectRatio()).c_str()));
	TiXmlElement* znear = new TiXmlElement("znear");
	znear->InsertEndChild(TiXmlText(StringConverter::toString(cam->getNearClipDistance()).c_str()));
	TiXmlElement* zfar = new TiXmlElement("zfar");
	// if far clip is 0, infinite was intended so set to big no. 10000
	Real zfarClip = (cam->getFarClipDistance() > 0) ? cam->getFarClipDistance() : 10000;
	zfar->InsertEndChild(TiXmlText(StringConverter::toString(zfarClip).c_str()));
	perspective->LinkEndChild(yfov);
	perspective->LinkEndChild(aspect_ratio);
	perspective->LinkEndChild(znear);
	perspective->LinkEndChild(zfar);
	technique_common->LinkEndChild(perspective);
	optics->LinkEndChild(technique_common);
	
	camera->LinkEndChild(optics);
	_libCamera->LinkEndChild(camera);


	//TODO instance camera
	TiXmlElement* cameraNode = createNode(camNode);
	TiXmlElement* cameraNode2 = createNode(id+"Node2", cam->getPosition(), cam->getOrientation());
	TiXmlElement* cameraInstance = new TiXmlElement("instance_camera");
	cameraInstance->SetAttribute("url", ("#"+id+"-obj").c_str());

	cameraNode2->LinkEndChild(cameraInstance);
	cameraNode->LinkEndChild(cameraNode2);
	_defaultScene->LinkEndChild(cameraNode);
}

void ColladaDoc::addLight(Light *l)
{
	string id = l->getName();
	TiXmlElement* light = new TiXmlElement("light");
	light->SetAttribute("id", (id+"-obj").c_str());
	light->SetAttribute("name", id.c_str());

	TiXmlElement* technique_common = new TiXmlElement("technique_common");
	TiXmlElement* point = new TiXmlElement("point");
	TiXmlElement* color = new TiXmlElement("color");
	ColourValue lc = l->getDiffuseColour();
	string lightColourStr = StringConverter::toString(lc.r) + " " + StringConverter::toString(lc.g) + " " + StringConverter::toString(lc.b);
	color->InsertEndChild(TiXmlText(lightColourStr.c_str()));
	//TiXmlElement* constant_attenuation = new TiXmlElement("constant_attenuation");
	//constant_attenuation->InsertEndChild(TiXmlText(StringConverter::toString(l->getAttenuationConstant()).c_str()));
	//TiXmlElement* linear_attenuation = new TiXmlElement("linear_attenuation");
	//linear_attenuation->InsertEndChild(TiXmlText(StringConverter::toString(l->getAttenuationLinear()).c_str()));
	//TiXmlElement* quadratic_attenuation = new TiXmlElement("quadratic_attenuation");
	//quadratic_attenuation->InsertEndChild(TiXmlText(StringConverter::toString(l->getAttenuationQuadric()).c_str()));

	point->LinkEndChild(color);
	//point->LinkEndChild(constant_attenuation);
	//point->LinkEndChild(linear_attenuation);
	//point->LinkEndChild(quadratic_attenuation);
	technique_common->LinkEndChild(point);
	light->LinkEndChild(technique_common);
	_libLight->LinkEndChild(light);

	// instance light
	TiXmlElement* lightNode = createNode(id+"-obj-node", l->getPosition(), Quaternion());
	TiXmlElement* lightInstance = new TiXmlElement("instance_light");
	lightInstance->SetAttribute("url", ("#"+id+"-obj").c_str());
	lightNode->LinkEndChild(lightInstance);
	_defaultScene->LinkEndChild(lightNode);
}


TiXmlElement* ColladaDoc::createNode(const SceneNode *nd)
{
	string name = nd->getName();
	Vector3 pos = nd->getPosition();
	Quaternion orient = nd->getOrientation();
	return createNode(name, pos, orient);
}

TiXmlElement* ColladaDoc::createNode(const string &id, const Vector3 &pos, const Quaternion &orient)
{
	TiXmlElement* node = new TiXmlElement("node");
	node->SetAttribute("id", id.c_str());
	node->SetAttribute("type", "NODE");
	TiXmlElement* translate = new TiXmlElement("translate");
	translate->InsertEndChild(TiXmlText(StringConverter::toString(pos).c_str()));
	TiXmlElement* rotate = new TiXmlElement("rotate");
	rotate->InsertEndChild(TiXmlText(StringConverter::toString(orient).c_str()));
/*
<rotate sid="rotateZ">0 0 1 0.141706</rotate>
<rotate sid="rotateY">0 1 0 0.031736</rotate>
<rotate sid="rotateX">1 0 0 -0.122091</rotate>
	Vector3 rotateVector;
	orient.ToAxes(&rotateVector);
	TiXmlElement* rotateZ = new TiXmlElement("rotate");
	rotateZ->SetAttribute("sid", "rotateZ");
	rotateZ->InsertEndChild(TiXmlText(("0 0 1 "+StringConverter::toString(rotateVector.z)).c_str()));
	TiXmlElement* rotateY = new TiXmlElement("rotate");
	rotateY->SetAttribute("sid", "rotateY");
	rotateY->InsertEndChild(TiXmlText(("0 1 0 "+StringConverter::toString(rotateVector.y)).c_str()));
	TiXmlElement* rotateX = new TiXmlElement("rotate");
	rotateX->SetAttribute("sid", "rotateX");
	rotateX->InsertEndChild(TiXmlText(("1 0 0 "+StringConverter::toString(rotateVector.x)).c_str()));*/

	node->LinkEndChild(translate);
	node->LinkEndChild(rotate);

	return node;
}

TiXmlElement* createColourElement(const string &name, const Ogre::ColourValue &c)
{
	TiXmlElement* elem = new TiXmlElement(name.c_str());
	TiXmlElement* colour = new TiXmlElement("color");
	colour->InsertEndChild(TiXmlText(StringConverter::toString(c).c_str()));
	elem->LinkEndChild(colour);
	return elem;
}

TiXmlElement* createFloatElement(const string &name, const Real &r)
{
	TiXmlElement* elem = new TiXmlElement(name.c_str());
	TiXmlElement* floatElem = new TiXmlElement("float");
	floatElem->InsertEndChild(TiXmlText(StringConverter::toString(r).c_str()));
	elem->LinkEndChild(floatElem);
	return elem;
}


/*
<technique sid="blender">
<phong>
<emission>
<color>0.00000 0.00000 0.00000 1</color>
</emission>
<ambient>
<color>0.40000 0.40000 0.40000 1</color>
</ambient>
<diffuse>
<texture texcoord="CHANNEL1" texture="Tex"/>
</diffuse>
<specular>
<color>0.50000 0.50000 0.50000 1</color>
</specular>
<shininess>
<float>12.5</float>
</shininess>
<reflective>
<color>1.00000 1.00000 1.00000 1</color>
</reflective>
<reflectivity>
<float>0.8</float>
</reflectivity>
<transparent>
<color>1 1 1 1</color>
</transparent>
<transparency>
<float>0.0</float>
</transparency>
</phong>
</technique>
</profile_COMMON>
</effect>
*/
void ColladaDoc::addMaterial(Material *m)
{
	string id = m->getName();
	replace(id.begin(), id.end(), '/', '_');
	Ogre::Technique* t = m->getBestTechnique();
	string tex1, tex2;
	Ogre::Pass* p;

	unsigned short numOfPasses = static_cast<unsigned short>(t->getNumPasses());
	if(numOfPasses != 0) 
	{
		for(short i=(numOfPasses-1); i>=0; i--)
		{
			p = t->getPass(i);
			for(unsigned short j=0; j<p->getNumTextureUnitStates(); j++)
			{
				TextureUnitState* tu = p->getTextureUnitState(j);
				//TextureType ty = tu->getTextureType();
				tex1 = tu->getTextureName();
				tex2 = tu->getTextureNameAlias();
				break;
			}
			if(tex1 != string("")) break;
		}
	}
	
	// add the image
	//////////////////////////////////////////////////////////////////////////
	//<image id="C__Documents_and_Settings_George_Desktop_buildingtex2_png-img">
	//	<init_from>./buildingtex2.png.jpg</init_from>
	//</image>
	_texFiles.insert(tex1);
	string texUrl(tex1);
	replace(texUrl.begin(), texUrl.end(), '.', '_'); 
	TiXmlElement* image = new TiXmlElement("image");
	image->SetAttribute("id", (texUrl+"-img").c_str());
	image->SetAttribute("name", texUrl.c_str());
	TiXmlElement* init_from = new TiXmlElement("init_from");
	init_from->InsertEndChild(TiXmlText(("./"+_texFolder+"/"+tex1).c_str()));
	image->LinkEndChild(init_from);
	_libImage->LinkEndChild(image);

	// add the effect
	//////////////////////////////////////////////////////////////////////////
	TiXmlElement* effect = new TiXmlElement("effect");
	effect->SetAttribute("id", (id+"-effect").c_str());
	effect->SetAttribute("name", (id+"-effect").c_str());

	// profie
	TiXmlElement* profile_COMMON = new TiXmlElement("profile_COMMON");

	if(tex1 != string(""))
	{
	/*
		// texture param
		TiXmlElement* newparam = new TiXmlElement("newparam");
		newparam->SetAttribute("sid",(texUrl+"-img-surface").c_str());
		TiXmlElement* surface = new TiXmlElement("surface");
		surface->SetAttribute("type","2D");
		TiXmlElement* init_from2 = new TiXmlElement("init_from");
		init_from2->InsertEndChild(TiXmlText((texUrl+"-img").c_str()));
		TiXmlElement* format = new TiXmlElement("format");
		format->InsertEndChild(TiXmlText("A8R8G8B8"));
		surface->LinkEndChild(init_from2);
		surface->LinkEndChild(format);
		newparam->LinkEndChild(surface);
		profile_COMMON->LinkEndChild(newparam);

		TiXmlElement* newparam2 = new TiXmlElement("newparam");
		newparam2->SetAttribute("sid",(texUrl+"-img-sampler").c_str());
		TiXmlElement* sampler2D = new TiXmlElement("sampler2D");
		TiXmlElement* source = new TiXmlElement("source");
		source->InsertEndChild(TiXmlText((texUrl+"-img-surface").c_str()));
		TiXmlElement* minfilter = new TiXmlElement("minfilter");
		minfilter->InsertEndChild(TiXmlText("LINEAR_MIPMAP_LINEAR"));
		TiXmlElement* magfilter = new TiXmlElement("magfilter");
		magfilter->InsertEndChild(TiXmlText("LINEAR"));
		sampler2D->LinkEndChild(source);
		sampler2D->LinkEndChild(minfilter);
		sampler2D->LinkEndChild(magfilter);
		newparam2->LinkEndChild(sampler2D);
		profile_COMMON->LinkEndChild(newparam2);
		*/
	}

	// technique
	TiXmlElement* technique = new TiXmlElement("technique");
	technique->SetAttribute("sid", "standard");

	// phong
	TiXmlElement* phong = new TiXmlElement("phong");

	TiXmlElement* emission = createColourElement("emission", p->getSelfIllumination());
	TiXmlElement* ambient = createColourElement("ambient", p->getAmbient());
	TiXmlElement* diffuse;
	if(tex1 != string(""))
	{
		diffuse = new TiXmlElement("diffuse");
		TiXmlElement* texture = new TiXmlElement("texture");
		texture->SetAttribute("texture", (texUrl+"-img").c_str());
		texture->SetAttribute("texcoord", "CHANNEL0");
		/*<extra>
			<technique profile="MAYA">
			<wrapU sid="wrapU0">TRUE</wrapU>
			<wrapV sid="wrapV0">TRUE</wrapV>
			<blend_mode>ADD</blend_mode>
			</technique>
		</extra>*/


		diffuse->LinkEndChild(texture);
		//diffuse->LinkEndChild(extra);
	}
	else
		diffuse = createColourElement("diffuse", p->getDiffuse());
	TiXmlElement* specular = createColourElement("specular", p->getSpecular());
	TiXmlElement* shininess = createFloatElement("shininess", p->getShininess());
	TiXmlElement* reflective = createColourElement("reflective", ColourValue::Black);
	TiXmlElement* reflectivity = createFloatElement("reflectivity", 0.0f);
	TiXmlElement* transparent = createColourElement("transparent", ColourValue::Black);
	TiXmlElement* transparency = createFloatElement("transparency", 0.0f);

	phong->LinkEndChild(emission);
	phong->LinkEndChild(ambient);
	phong->LinkEndChild(diffuse);
	phong->LinkEndChild(specular);
	phong->LinkEndChild(shininess);
	phong->LinkEndChild(reflective);
	phong->LinkEndChild(reflectivity);
	phong->LinkEndChild(transparent);
	phong->LinkEndChild(transparency);
/*
	TiXmlElement* extra = new TiXmlElement("extra");
	TiXmlElement* technique2 = new TiXmlElement("technique");
	technique2->SetAttribute("profile", "MAYA");
	extra->LinkEndChild(technique2);
*/
	technique->LinkEndChild(phong);
//	technique->LinkEndChild(extra);
	profile_COMMON->LinkEndChild(technique);
	effect->LinkEndChild(profile_COMMON);

	_libEffect->LinkEndChild(effect);


	// add material
	//////////////////////////////////////////////////////////////////////////
	TiXmlElement* material = new TiXmlElement("material");
	material->SetAttribute("id", (id+"-material").c_str());
	material->SetAttribute("name", (id+"-material").c_str());
	TiXmlElement* instance_effect = new TiXmlElement("instance_effect");
	instance_effect->SetAttribute("url", ("#"+id+"-effect").c_str());

	material->LinkEndChild(instance_effect);
	_libMaterial->LinkEndChild(material);

}

string ColladaDoc::getFileNameFromPath(const string &path)
{
	///string fileName;
	size_t beginPos,endPos;
	beginPos = _file.find_last_of('/') + 1;
	endPos = _file.find_last_of('.');
	return _file.substr(beginPos, endPos - beginPos);
}

string ColladaDoc::getFolderPathFromPath(const string &path)
{
	///string fileName;
	size_t endPos = _file.find_last_of('/') + 1;
	return _file.substr(0, endPos);
}
