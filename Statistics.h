#ifndef STATISTICS_H
#define STATISTICS_H

class Statistics
{
private:
	static size_t _buildingCount;
	static size_t _roadCount;

public:
	static inline void incBuildingCount() { _buildingCount++; }
	static size_t getBuildingCount() { return _buildingCount; }
	static void resetBuildingCount() { _buildingCount = 0; }

	static inline void incRoadCount() { _roadCount++; }
	static size_t getRoadCount() { return _roadCount; }
	static void resetRoadCount() { _roadCount = 0; }
};

#endif
