#ifndef STATISTICS_H
#define STATISTICS_H

class Statistics
{
private:
	static size_t _buildingCount;

public:
	static inline void incBuildingCount() { _buildingCount++; }
	static size_t getBuildingCount() { return _buildingCount; }
	static void resetBuildingCount() { _buildingCount = 0; }
};

#endif
