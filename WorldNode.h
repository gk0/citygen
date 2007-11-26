#ifndef WORLDNODE_H
#define WORLDNODE_H

#include "stdafx.h"
#include "WorldObject.h"
#include "NodeInterface.h"
#include "RoadGraph.h"
#include "WorldRoad.h"
#include "ExportDoc.h"

class Ogre::MovableText;

#define GROUNDCLEARANCE Ogre::Vector3(0,0.3,0)

class WorldNode : public WorldObject, public NodeInterface, public Ogre::ManualResourceLoader
{

private:
	std::vector<Ogre::Real> _vertexData;
	std::vector<Ogre::uint16> _indexData;

	size_t				_degree;
	RoadGraph&			_simpleRoadGraph;
	static int			_instanceCount;
	Ogre::Entity*		_junctionEntity;
	Ogre::Entity*		_mesh;
	Ogre::Entity*		_highlight;
	Ogre::Entity*		_selected;
	Ogre::MovableText*	_label;
	Ogre::SceneManager* _creator;
	Ogre::String		_name;
	//std::vector<WorldRoad*> mRoads;
	std::map<RoadId, std::pair<Ogre::Vector3, Ogre::Vector3>, road_less_than > _roadJunction;

	Ogre::Vector2 getRoadBounaryIntersection(const RoadId leftR, const RoadId rightR);
	void onMove();

	std::vector<RoadId> getClockwiseVecOfRoads();
	std::vector<NodeId> getClockwiseVecOfNodes(const std::vector<RoadId>& roads);

public:
	NodeId mSimpleNodeId;
	
	WorldNode(RoadGraph& g, RoadGraph& s, Ogre::SceneManager *creator);
	~WorldNode();

	void setLabel(const Ogre::String& label);
	const Ogre::String& getLabel() const;
	void showHighlighted(bool highlighted);
	void showSelected(bool selected);
	Ogre::Vector3 getPosition3D() const;
	void setPosition(const Ogre::Vector3 &pos);
	bool setPosition2D(Ogre::Real x, Ogre::Real z);
	bool setPosition2D(const Ogre::Vector2& pos);
	void setPosition3D(Ogre::Real x, Ogre::Real y, Ogre::Real z);
	void setPosition3D(const Ogre::Vector3& pos);
	void setPosition(Ogre::Real x, Ogre::Real y, Ogre::Real z);

	Ogre::Vector2 getPosition2D() const;
	const Ogre::Vector3& getPosition() const { return WorldObject::getPosition(); }

	//void attach(WorldObject* wo);
	//void detach(WorldObject* wo);
	void invalidate();

	bool move(Ogre::Vector2 pos);

	void notify();
	
	bool loadXML(const TiXmlHandle& nodeRoot);
	bool hasRoadIntersection();

	void build();

	std::pair<Ogre::Vector3, Ogre::Vector3> getRoadJunction(RoadId rd);

	void createTerminus();

	void onAddRoad();
	void onRemoveRoad();

	bool createTJunction();

	inline static Ogre::Vector2 madnessCheck(const Ogre::Vector2& nodePos, const Ogre::Vector2& pos, 
		const Ogre::Real limitSq, const Ogre::Real limit)
	{
		Ogre::Vector2 dir = pos - nodePos;
		Ogre::Real len = dir.squaredLength();
		if(len < limitSq) 
		{	
			return pos;
		}
		else
		{
			dir.normalise();
			return (nodePos + (dir * limit));
		}
	}

	int snapInfo(const Ogre::Real snapSz, Ogre::Vector2& pos, WorldNode*& wn, WorldRoad*& wr) const;

	friend WorldRoad* getWorldRoad(const WorldNode* wn1, const WorldNode* wn2)
	{
		RoadGraph& g(wn1->_simpleRoadGraph);
		RoadInterface* ri = g.getRoad(g.getRoad(wn1->mSimpleNodeId, wn2->mSimpleNodeId));
		assert(typeid(*ri) == typeid(WorldRoad));
		return static_cast<WorldRoad*>(ri);
	}

	std::vector<WorldRoad*> getWorldRoads() const;

	static void resetInstanceCount() { _instanceCount = 0; }

	void exportObject(ExportDoc &doc);

	void loadResource(Ogre::Resource* r) {}

}; 

#endif
