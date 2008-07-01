#include "stdafx.h"
#include "MeshBuilder.h"

#include <OgreHardwareBufferManager.h>
#include <OgreMeshManager.h>
#include <OgreSubMesh.h>
#include <OgreLogManager.h>

using namespace Ogre;
using namespace std;

MeshBuilder::MeshBuilder(const String &name, const String &groupName, 
			Ogre::ManualResourceLoader *loader)
			: _name(name),
			  _groupName(groupName),
			  _loader(loader)
{
}

void MeshBuilder::registerData(Material* material,
							   const vector<Real> &vdata, 
							   const vector<uint16> &idata)
{
	// check material is already present
	PolyDataMap::iterator pdIt = _polyDataMap.find(material);
	if(pdIt == _polyDataMap.end())
	{
		vector<PtrPolyData> vtmp;
		pdIt = _polyDataMap.insert(pdIt, make_pair<Material*, vector<PtrPolyData> >(material, vtmp));
	
	}
	pdIt->second.push_back(make_pair<const vector<Real>*, const vector<uint16>* >(&vdata, &idata));
}

void MeshBuilder::build()
{
	// create a mesh
	MeshPtr mesh = MeshManager::getSingleton().createManual(_name, _groupName, _loader);

	// build edge lists if required for shadows
	mesh->setAutoBuildEdgeLists(true);

	AxisAlignedBox aabox;		// record the bounding box

	PolyDataMap::iterator pdIt = _polyDataMap.begin();
	for(; pdIt != _polyDataMap.end(); pdIt++)
	{
		size_t numVertices = 0;
		size_t numFaces = 0;
		size_t numVerticesPerFaces = 3;

		BOOST_FOREACH(PtrPolyData& pd, pdIt->second)
		{
			numVertices += (pd.first->size() >> 3); // divide by 8
			numFaces += (pd.second->size() / 3);
		}

		if(numVertices == 0) continue; 

		// We create a submesh per material
		SubMesh* submesh = mesh->createSubMesh();
		// We must create the vertex data, indicating how many vertices there will be
		submesh->useSharedVertices = false;
		submesh->vertexData = new VertexData();
		submesh->vertexData->vertexStart = 0;
		submesh->vertexData->vertexCount = numVertices;
		// We must now declare what the vertex data contains
		VertexDeclaration* declaration = submesh->vertexData->vertexDeclaration;
		static const unsigned short source = 0;
		size_t offset = 0;
		offset += declaration->addElement(source,offset,VET_FLOAT3, VES_POSITION).getSize();
		offset += declaration->addElement(source,offset,VET_FLOAT3, VES_NORMAL).getSize();
		offset += declaration->addElement(source,offset,VET_FLOAT2, VES_TEXTURE_COORDINATES).getSize();
		offset += declaration->addElement(source,offset,VET_FLOAT3, VES_TANGENT).getSize();
		// We create the hardware vertex buffer
		HardwareVertexBufferSharedPtr vbuffer =
			HardwareBufferManager::getSingleton().createVertexBuffer(declaration->getVertexSize(source), // == offset
			submesh->vertexData->vertexCount,   // == nbVertices
			HardwareBuffer::HBU_STATIC_WRITE_ONLY);

		// Install vertex data
		float* vdata2;
		float* vdata = vdata2 = static_cast<float*>(vbuffer->lock(HardwareBuffer::HBL_DISCARD));
		
		BOOST_FOREACH(PtrPolyData& pd, pdIt->second)
		{
			const vector<Real> &vd(*(pd.first));
			vector<Real>::const_iterator vdIt = vd.begin();
			while(vdIt != vd.end())
			{
				// pos
				*vdata++ = *vdIt++;
				*vdata++ = *vdIt++;
				*vdata++ = *vdIt++;
				// stupid access code
				float* vdatatmp = vdata - 3;
				aabox.merge(Vector3(vdatatmp[0], vdatatmp[1], vdatatmp[2]));
				// normal
				*vdata++ = *vdIt++;
				*vdata++ = *vdIt++;
				*vdata++ = *vdIt++;
				// texture coordinate
				*vdata++ = *vdIt++;
				*vdata++ = *vdIt++;

				// skip tangent vector, warning!: this leaves crap in its place
				vdata += 3;
			}
		}
		// lets not lock the buffer yet, since i will be writing the tangent vectors into it
		//vbuffer->unlock();
		//submesh->vertexData->vertexBufferBinding->setBinding(source,vbuffer);

		// Creates the index data
		submesh->indexData->indexStart = 0;
		submesh->indexData->indexCount = numFaces*numVerticesPerFaces;
		submesh->indexData->indexBuffer =
			HardwareBufferManager::getSingleton().createIndexBuffer(HardwareIndexBuffer::IT_16BIT,
			submesh->indexData->indexCount,
			HardwareBuffer::HBU_STATIC_WRITE_ONLY);
		uint16* idata = static_cast<uint16*>(submesh->indexData->indexBuffer->lock(HardwareBuffer::HBL_DISCARD));

		uint16 voffset = 0;
		Vector3 *p1,*p2,*p3, *tv1, *tv2, *tv3;
		Vector2 *uv1, *uv2, *uv3;
		BOOST_FOREACH(PtrPolyData& pd, pdIt->second)
		{
			const vector<uint16> &id(*(pd.second));
			vector<uint16>::const_iterator idIt = id.begin();
			while(idIt != id.end())
			{
				// polygon
				idata[0] = voffset + *idIt++;
				idata[1] = voffset + *idIt++;
				idata[2] = voffset + *idIt++;

				// tangent vectors generated per face
				// TODO: tidy this up: do it in a safe way? 
				p1 = reinterpret_cast<Vector3*>(vdata2 + (idata[0] * 11));
				p2 = reinterpret_cast<Vector3*>(vdata2 + (idata[1] * 11));
				p3 = reinterpret_cast<Vector3*>(vdata2 + (idata[2] * 11));
				uv1 = reinterpret_cast<Vector2*>(p1 + 2);
				uv2 = reinterpret_cast<Vector2*>(p2 + 2);
				uv3 = reinterpret_cast<Vector2*>(p3 + 2);
				tv1 = reinterpret_cast<Vector3*>(uv1 + 1);
				tv2 = reinterpret_cast<Vector3*>(uv2 + 1);
				tv3 = reinterpret_cast<Vector3*>(uv3 + 1);
				Vector3 p21 = *p2 - *p1;
				Vector3 p31 = *p3 - *p1;
				Vector2 uv21 = *uv2 - *uv1;
				Vector2 uv31 = *uv3 - *uv1;

				float f = uv21.x*uv31.y - uv21.y*uv31.x;
				f = (f == 0) ? 1 : 1 / f;

				// use normalisedCopy NOT normalise
				*tv1 = *tv2 = *tv3 = (((p21 * uv31.y) - (p31 * uv21.y)) * f).normalisedCopy();

				idata += 3;
			}
			voffset += static_cast<uint16>(pd.first->size() >> 3);// / 8);
		}
		submesh->indexData->indexBuffer->unlock();

		vbuffer->unlock();
		submesh->vertexData->vertexBufferBinding->setBinding(source,vbuffer);

		// Finally we set a material to the submesh
		try {
			submesh->setMaterialName(string(pdIt->first->getName()));
		}
		catch(...)
		{
			LogManager::getSingleton().logMessage(LML_CRITICAL, "set material exception"+pdIt->first->getName());
		}
	}

	// We must indicate the bounding box
	mesh->_setBounds(aabox);  
	mesh->_setBoundingSphereRadius((aabox.getMaximum()-aabox.getMinimum()).length()/2.0); 

	// build tangent vectors
	//unsigned short src, dest;
	//mesh->suggestTangentVectorBuildParams(VES_TANGENT, src, dest);
	//mesh->buildTangentVectors(VES_TANGENT, src, dest);

	// And we load the mesh
	mesh->load();
}
