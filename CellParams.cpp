#include "stdafx.h"
#include "CellParams.h"

const CellParams CellParams::MANHATTAN(
	0,		// type
	1,		// seed
	100,		// segmentSize
	0.2,	// segmentDeviance
	4,		// degree
	0.02,	// degreeDeviance
	30.0,	// snapSize
	0.1,	// snapDeviance
	18,	// buildingHeight
	0.6,	// buildingDeviance
	4.6,	// roadWidth
	0,		// roadLimit
	1.0,	// connectivity
	3.2,	// footpathWidth;
	0.28,	// footpathHeight;
	10.5,	// lotWidth
	18.0,	// lotDepth
	0.5,	// lotDeviance
	false	// debug
);

const CellParams CellParams::INDUSTRIAL(
	1,		// type
	1,		// seed
	28,		// segmentSize
	0.2,	// segmentDeviance
	4,		// degree
	0.01,	// degreeDeviance
	16.8,	// snapSize
	0.1,	// snapDeviance
	0.7,	// buildingHeight
	0.3,	// buildingDeviance
	3.5,	// roadWidth
	0,		// roadLimit
	0.3,	// connectivity
	1.5,	// footpathWidth;
	0.28,	// footpathHeight;
	16.0,	// lotWidth
	16.0,	// lotDepth
	0.7,	// lotDeviance
	false	// debug
);	

const CellParams CellParams::SUBURBIA(
	2,		// type
	1,		// seed
	26.6,	// segmentSize
	0.6,	// segmentDeviance
	9,		// degree
	0.6,	// degreeDeviance
	19.6,	// snapSize
	0.1,	// snapDeviance
	0.4,	// buildingHeight
	0.1,	// buildingDeviance
	3.0,	// roadWidth
	0,		// roadLimit
	0.0,	// connectivity
	1.5,	// footpathWidth;
	0.28,	// footpathHeight;
	5.0,	// lotWidth
	10.0,	// lotDepth
	0.2,	// lotDeviance
	false	// debug
);