#include "stdafx.h"
#include "MeshBuilder.h"


// THIS CLASS SEEMS TOO GOOD TO BE TRUE, UNLIKE A LOT OF OTHER CODE I'VE WRITTEN

// OMG, I AM SO HAPPY WITH THIS CLASS I'M FINDING IT HARD TO CONCENTRATE, ITS LIKE
// SOME SORT OF MAGIC CLASS THAT MAKES LIFE WORTH LIVING

// AND 2 HOURS IS ALL IT TOOK, PLUS ABOUT 10 WORKING OUT WHAT TO DO

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
		offset += declaration->addElement(source,offset,VET_FLOAT3,VES_POSITION).getSize();
		offset += declaration->addElement(source,offset,VET_FLOAT3,VES_NORMAL).getSize();
		offset += declaration->addElement(source,offset,VET_FLOAT2,VES_TEXTURE_COORDINATES).getSize();
		// We create the hardware vertex buffer
		HardwareVertexBufferSharedPtr vbuffer =
			HardwareBufferManager::getSingleton().createVertexBuffer(declaration->getVertexSize(source), // == offset
			submesh->vertexData->vertexCount,   // == nbVertices
			HardwareBuffer::HBU_STATIC_WRITE_ONLY);


		// Install vertex data
		float* vdata = static_cast<float*>(vbuffer->lock(HardwareBuffer::HBL_DISCARD));
		
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
			}
		}
		vbuffer->unlock();
		submesh->vertexData->vertexBufferBinding->setBinding(source,vbuffer);


		// Creates the index data
		submesh->indexData->indexStart = 0;
		submesh->indexData->indexCount = numFaces*numVerticesPerFaces;
		submesh->indexData->indexBuffer =
			HardwareBufferManager::getSingleton().createIndexBuffer(HardwareIndexBuffer::IT_16BIT,
			submesh->indexData->indexCount,
			HardwareBuffer::HBU_STATIC_WRITE_ONLY);
		uint16* idata = static_cast<uint16*>(submesh->indexData->indexBuffer->lock(HardwareBuffer::HBL_DISCARD));

		uint16 voffset = 0;
		BOOST_FOREACH(PtrPolyData& pd, pdIt->second)
		{
			const vector<uint16> &id(*(pd.second));
			vector<uint16>::const_iterator idIt = id.begin();
			while(idIt != id.end())
			{
				// polygon
				*idata++ = voffset + *idIt++;
				*idata++ = voffset + *idIt++;
				*idata++ = voffset + *idIt++;
			}
			voffset += static_cast<uint16>(pd.first->size() >> 3); // / 8);
		}
		submesh->indexData->indexBuffer->unlock(); 

		// Finally we set a material to the submesh
		submesh->setMaterialName(pdIt->first->getName());
	}

	// We must indicate the bounding box
	mesh->_setBounds(aabox);  
	mesh->_setBoundingSphereRadius((aabox.getMaximum()-aabox.getMinimum()).length()/2.0); 

	// build tangent vectors
	//unsigned short src, dest;
	//if (!mesh->suggestTangentVectorBuildParams(VES_TANGENT, src, dest))
	//	mesh->buildTangentVectors(VES_TANGENT, src, dest);

	// And we load the mesh
	mesh->load();
}

