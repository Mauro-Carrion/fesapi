/*-----------------------------------------------------------------------
Licensed to the Apache Software Foundation (ASF) under one
or more contributor license agreements.  See the NOTICE file
distributed with this work for additional information
regarding copyright ownership.  The ASF licenses this file
to you under the Apache License, Version 2.0 (the
"License"; you may not use this file except in compliance
with the License.  You may obtain a copy of the License at
http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing,
software distributed under the License is distributed on an
"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
KIND, either express or implied.  See the License for the
specific language governing permissions and limitations
under the License.
-----------------------------------------------------------------------*/
// Acknowledgements: the serializeOrganizations function have been provided by Geosiris (contact: mathieu.poudret@geosiris.com)

//#define OFFICIAL

#ifdef _WIN32
// ************************
// For memory leak
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
// ************************
#endif

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "version_config.h"

#include "proxies/stdsoap2.h"

#include "common/EpcDocument.h"
#include "common/EnumStringMapper.h"
#include "resqml2_0_1/LocalDepth3dCrs.h"
#include "resqml2_0_1/LocalTime3dCrs.h"
#include "resqml2_0_1/FrontierFeature.h"
#include "resqml2_0_1/Horizon.h"
#include "resqml2_0_1/GeobodyFeature.h"
#include "resqml2_0_1/TectonicBoundaryFeature.h"
#include "resqml2_0_1/GenericFeatureInterpretation.h"
#include "resqml2_0_1/FluidBoundaryFeature.h"
#include "resqml2_0_1/HorizonInterpretation.h"
#include "resqml2_0_1/GeobodyInterpretation.h"
#include "resqml2_0_1/GeobodyBoundaryInterpretation.h"
#include "resqml2_0_1/FaultInterpretation.h"
#include "resqml2_0_1/TriangulatedSetRepresentation.h"
#include "resqml2_0_1/PolylineSetRepresentation.h"
#include "resqml2_0_1/PointSetRepresentation.h"
#include "resqml2_0_1/PlaneSetRepresentation.h"
#include "common/HdfProxy.h"
#include "resqml2_0_1/OrganizationFeature.h"
#include "resqml2_0_1/EarthModelInterpretation.h"
#include "resqml2_0_1/StructuralOrganizationInterpretation.h"
#include "resqml2_0_1/StratigraphicUnitInterpretation.h"
#include "resqml2_0_1/Grid2dRepresentation.h"
#include "resqml2_0_1/SeismicLatticeFeature.h"
#include "resqml2_0_1/SeismicLineSetFeature.h"
#include "resqml2_0_1/WellboreFeature.h"
#include "resqml2_0_1/WellboreInterpretation.h"
#include "resqml2_0_1/WellboreTrajectoryRepresentation.h"
#include "resqml2_0_1/MdDatum.h"
#include "resqml2_0_1/PolylineRepresentation.h"
#include "resqml2_0_1/PropertyKind.h"
#include "resqml2_0_1/WellboreMarker.h"
#include "resqml2_0_1/WellboreMarkerFrameRepresentation.h"
#include "resqml2_0_1/PropertySet.h"
#include "resqml2_0_1/ContinuousProperty.h"
#include "resqml2_0_1/DiscreteProperty.h"
#include "resqml2_0_1/CategoricalProperty.h"
#include "resqml2_0_1/StringTableLookup.h"
#include "resqml2_0_1/IjkGridExplicitRepresentation.h"
#include "resqml2_0_1/IjkGridParametricRepresentation.h"
#include "resqml2_0_1/UnstructuredGridRepresentation.h"
#include "resqml2_0_1/SealedSurfaceFrameworkRepresentation.h"
#include "resqml2_0_1/SubRepresentation.h"
#include "resqml2_0_1/TimeSeries.h"
#include "resqml2_0_1/ContinuousPropertySeries.h"
#include "resqml2_0_1/RockFluidOrganizationInterpretation.h"
#include "resqml2_0_1/RockFluidUnitInterpretation.h"
#include "resqml2_0_1/RockFluidUnitFeature.h"
#include "resqml2_0_1/PropertyKindMapper.h"

#include "resqml2_0_1/Activity.h"
#include "resqml2_0_1/ActivityTemplate.h"

#ifdef WITH_EXPERIMENTAL
#include "common/GraphicalInformationSet.h"
#include "resqml2_2/DiscreteColorMap.h"
#include "resqml2_2/ContinuousColorMap.h"
#endif

#include "witsml2_0/Well.h"
#include "witsml2_0/Wellbore.h"
#include "witsml2_0/Trajectory.h"
#include "witsml2_0/WellCompletion.h"
#include "witsml2_0/WellboreCompletion.h"

#include "tools/TimeTools.h"

using namespace std;
using namespace RESQML2_0_1_NS;

Horizon* horizon1 = nullptr;
Horizon* horizon2 = nullptr;
TectonicBoundaryFeature* fault1 = nullptr;
HorizonInterpretation* horizon1Interp1 = nullptr;
HorizonInterpretation* horizon2Interp1 = nullptr;
FaultInterpretation* fault1Interp1 = nullptr;
Grid2dRepresentation* h1i1SingleGrid2dRep = nullptr;
PolylineSetRepresentation* f1i1PolyLineRep = nullptr;
TriangulatedSetRepresentation* f1i1triRepSinglePatch = nullptr;
TriangulatedSetRepresentation* f1i1triRep = nullptr;
TriangulatedSetRepresentation* h1i1triRep = nullptr;
TriangulatedSetRepresentation* h2i1triRep = nullptr;
TriangulatedSetRepresentation* xMinusFrontierRep = nullptr;
TriangulatedSetRepresentation* xPlusFrontierRep = nullptr;
TriangulatedSetRepresentation* yMinusFrontierRep = nullptr;
TriangulatedSetRepresentation* yPlusFrontierRep = nullptr;
WellboreTrajectoryRepresentation* w1i1TrajRep = nullptr;
LocalDepth3dCrs* local3dCrs = nullptr;
LocalTime3dCrs* localTime3dCrs = nullptr;
WellboreFeature* wellbore1 = nullptr;
WellboreInterpretation* wellbore1Interp1 = nullptr;
StratigraphicColumnRankInterpretation* stratiColumnRank0 = nullptr;
SealedSurfaceFrameworkRepresentation* sealedSurfaceFramework = nullptr;
IjkGridExplicitRepresentation* ijkgrid = nullptr;
RESQML2_NS::PropertyKind* propType1 = nullptr;
DiscreteProperty* discreteProp1 = nullptr;
ContinuousProperty* contColMapContProp = nullptr;

WITSML2_0_NS::Well* witsmlWell = nullptr;
WITSML2_0_NS::Wellbore* witsmlWellbore = nullptr;

void serializeWells(COMMON_NS::DataObjectRepository * pck, COMMON_NS::AbstractHdfProxy* hdfProxy)
{
	////////////////////////
	// WITSML
	////////////////////////

	// WELL
	witsmlWell = pck->createWell("704a287c-5c24-4af3-a97b-bc6670f4e14f", "Well1");
	witsmlWell->setNameLegal("Legal Name");
	witsmlWell->pushBackLocation("8cd3c8b2-face-4426-8aea-ae34870bd969", 275, 75, 23031);
	witsmlWell->pushBackDatum("aa92fa8b-d6cc-459e-b456-27fec0c08b24", "well1 msl datum", gsoap_eml2_1::eml21__WellboreDatumReference__kelly_x0020bushing, "Mean Sea Level", gsoap_eml2_1::eml21__LengthUom__m, 0, 5100);
	witsmlWell->pushBackDatum("d3ac5401-d3e7-4474-b846-070673b210ae", "KB", gsoap_eml2_1::eml21__WellboreDatumReference__kelly_x0020bushing, "Mean Sea Level", gsoap_eml2_1::eml21__LengthUom__m, 15, 5100);

	// WELLBORE
	witsmlWellbore = pck->createWellbore(witsmlWell, "3bd60188-5688-43df-89bb-935fe86a813f", "Wellbore1");
	witsmlWellbore->setNumber("Wb1");
	witsmlWellbore->setStatusWellbore(gsoap_eml2_1::eml21__WellStatus__completed);
	witsmlWellbore->setIsActive(false);
	witsmlWellbore->setTypeWellbore(gsoap_eml2_1::witsml20__WellboreType__initial);
	witsmlWellbore->setShape(gsoap_eml2_1::witsml20__WellboreShape__vertical);
	witsmlWellbore->setAchievedTD(true);
	witsmlWellbore->setMd(1000, gsoap_eml2_1::eml21__LengthUom__m, "d3ac5401-d3e7-4474-b846-070673b210ae");
	witsmlWellbore->setMdPlanned(1000, gsoap_eml2_1::eml21__LengthUom__m, "d3ac5401-d3e7-4474-b846-070673b210ae");

	// TRAJECTORY
	WITSML2_0_NS::Trajectory* witsmlTrajectory = pck->createTrajectory(
		witsmlWellbore, "4e76e1de-eff1-4458-805e-a6a877fa333B", "My trajectory", gsoap_eml2_1::witsml20__ChannelStatus__closed);
	witsmlTrajectory->setMdMn(.0, gsoap_eml2_1::eml21__LengthUom__m, "d3ac5401-d3e7-4474-b846-070673b210ae");
	witsmlTrajectory->setMdMx(1000., gsoap_eml2_1::eml21__LengthUom__m, "d3ac5401-d3e7-4474-b846-070673b210ae");
	witsmlTrajectory->setDefinitive(true);
	witsmlTrajectory->setFinalTraj(true);
	witsmlTrajectory->setServiceCompany("F2I-CONSULTING");

	witsmlTrajectory->pushBackTrajectoryStation(gsoap_eml2_1::witsml20__TrajStationType__N_x0020E_x0020and_x0020TVD, 0, gsoap_eml2_1::eml21__LengthUom__m, "d3ac5401-d3e7-4474-b846-070673b210ae");
	witsmlTrajectory->setTrajectoryStationManuallyEntered(0, true);
	witsmlTrajectory->setTrajectoryStationDispNs(0, .0, gsoap_eml2_1::eml21__LengthUom__m);
	witsmlTrajectory->setTrajectoryStationDispEw(0, .0, gsoap_eml2_1::eml21__LengthUom__m);
	witsmlTrajectory->setTrajectoryStationTvd(0, .0, gsoap_eml2_1::eml21__LengthUom__m, "d3ac5401-d3e7-4474-b846-070673b210ae");
	witsmlTrajectory->pushBackTrajectoryStation(gsoap_eml2_1::witsml20__TrajStationType__N_x0020E_x0020and_x0020TVD, 325, gsoap_eml2_1::eml21__LengthUom__m, "d3ac5401-d3e7-4474-b846-070673b210ae");
	witsmlTrajectory->setTrajectoryStationManuallyEntered(1, true);
	witsmlTrajectory->setTrajectoryStationDispNs(1, 325, gsoap_eml2_1::eml21__LengthUom__m);
	witsmlTrajectory->setTrajectoryStationDispEw(1, 325, gsoap_eml2_1::eml21__LengthUom__m);
	witsmlTrajectory->setTrajectoryStationTvd(1, 325, gsoap_eml2_1::eml21__LengthUom__m, "d3ac5401-d3e7-4474-b846-070673b210ae");
	witsmlTrajectory->pushBackTrajectoryStation(gsoap_eml2_1::witsml20__TrajStationType__N_x0020E_x0020and_x0020TVD, 500, gsoap_eml2_1::eml21__LengthUom__m, "d3ac5401-d3e7-4474-b846-070673b210ae");
	witsmlTrajectory->setTrajectoryStationManuallyEntered(2, true);
	witsmlTrajectory->setTrajectoryStationDispNs(2, 500, gsoap_eml2_1::eml21__LengthUom__m);
	witsmlTrajectory->setTrajectoryStationDispEw(2, 500, gsoap_eml2_1::eml21__LengthUom__m);
	witsmlTrajectory->setTrajectoryStationTvd(2, 500, gsoap_eml2_1::eml21__LengthUom__m, "d3ac5401-d3e7-4474-b846-070673b210ae");
	witsmlTrajectory->pushBackTrajectoryStation(gsoap_eml2_1::witsml20__TrajStationType__N_x0020E_x0020and_x0020TVD, 1000, gsoap_eml2_1::eml21__LengthUom__m, "d3ac5401-d3e7-4474-b846-070673b210ae");
	witsmlTrajectory->setTrajectoryStationManuallyEntered(3, true);
	witsmlTrajectory->setTrajectoryStationDispNs(3, 1000, gsoap_eml2_1::eml21__LengthUom__m);
	witsmlTrajectory->setTrajectoryStationDispEw(3, 1000, gsoap_eml2_1::eml21__LengthUom__m);
	witsmlTrajectory->setTrajectoryStationTvd(3, 1000, gsoap_eml2_1::eml21__LengthUom__m, "d3ac5401-d3e7-4474-b846-070673b210ae");

	////////////////////////
	// RESQML
	////////////////////////

	// Features
	wellbore1 = pck->createWellboreFeature("22d5b48f-f789-46e7-a454-6d8bd05afd0b", "Wellbore1");
	if (witsmlWellbore != nullptr) {
		wellbore1->setWitsmlWellbore(witsmlWellbore);
	}

	// Interpretations
	wellbore1Interp1 = pck->createWellboreInterpretation(wellbore1, "dc7840fe-e5a3-4b53-a1df-18040bc4d0c0", "Wellbore1 Interp1", false);

	// Representation
	RESQML2_NS::MdDatum* mdInfo = pck->createMdDatum("36e91de5-7833-4b6d-90d0-1d643c0adece", "md Info", local3dCrs, gsoap_resqml2_0_1::resqml20__MdReference__mean_x0020sea_x0020level, 275, 75, 0);

	//Geometry	
	w1i1TrajRep = pck->createWellboreTrajectoryRepresentation(wellbore1Interp1, "acd2cdcf-bb5d-48da-bd0e-9aeff3e52180", "Wellbore1 Interp1 TrajRep", mdInfo);
	double controlPoints[12] = { 275, 75, 0, 275, 75, 325, 275, 75, 500, 275, 75, 1000 };
	double trajectoryTangentVectors[12] = { 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1 };
	double trajectoryMds[4] = { 0, 325, 500, 1000 };
	w1i1TrajRep->setGeometry(controlPoints, trajectoryTangentVectors, trajectoryMds, 4, 0, hdfProxy);

	// WellboreFeature frame
	WellboreFrameRepresentation* w1i1FrameRep = pck->createWellboreFrameRepresentation(wellbore1Interp1, "d873e243-d893-41ab-9a3e-d20b851c099f", "Wellbore1 Interp1 FrameRep", w1i1TrajRep);
	double logMds[5] = { 0, 250, 500, 750, 1000 };
	w1i1FrameRep->setMdValues(logMds, 5, hdfProxy);

	WellboreFrameRepresentation* w1i1RegularFrameRep = pck->createWellboreFrameRepresentation(wellbore1Interp1, "a54b8399-d3ba-4d4b-b215-8d4f8f537e66", "Wellbore1 Interp1 Regular FrameRep", w1i1TrajRep);
	w1i1RegularFrameRep->setMdValues(0, 200, 6);

	RESQML2_NS::PropertyKind * unitNumberPropType = pck->createPropertyKind("358aac23-b377-4349-9e72-bff99a6edf34", "Unit number", "urn:resqml:F2I.com:testingAPI", gsoap_resqml2_0_1::resqml20__ResqmlUom__Euc, gsoap_resqml2_0_1::resqml20__ResqmlPropertyKind__discrete);

	DiscreteProperty* discreteProp = pck->createDiscreteProperty(w1i1FrameRep, "61c2917c-2334-4205-824e-d4f4a0cf6d8e", "Wellbore1 Interp1 FrameRep IntervalIndex", 1,
		gsoap_resqml2_0_1::resqml20__IndexableElements__intervals, unitNumberPropType);
	char unitNumbers[5] = { 0, 1, 2, 3, 4 };
	discreteProp->pushBackCharHdf5Array1dOfValues(unitNumbers, 5, hdfProxy, -1);
}

void serializePerforations(COMMON_NS::DataObjectRepository * pck)
{
	// WELL COMPLETION
	WITSML2_0_NS::WellCompletion* wellCompletion = pck->createWellCompletion(witsmlWell, "6593d580-2f44-4b18-97ce-8a9cf42a0414", "WellCompletion1");
	// WELLBORE COMPLETION
	WITSML2_0_NS::WellboreCompletion* wellboreCompletion = pck->createWellboreCompletion(witsmlWellbore, wellCompletion, "7bda8ecf-2037-4dc7-8c59-db6ca09f2008", "WellboreCompletion1", "wellCompletionName");

	wellboreCompletion->pushBackPerforation("Mean Sea Level", gsoap_eml2_1::eml21__LengthUom__m, 1970, 1980, "myId");
	wellboreCompletion->pushBackPerforation("Mean Sea Level", gsoap_eml2_1::eml21__LengthUom__m, 1990, 2000);
	wellboreCompletion->pushBackPerforationHistory(0);
	wellboreCompletion->setPerforationHistoryStatus(0, 0, gsoap_eml2_1::witsml20__PerforationStatus__open);
	wellboreCompletion->setPerforationHistoryTopMd(0, 0, "Mean Sea Level", gsoap_eml2_1::eml21__LengthUom__m, 1970);
	wellboreCompletion->setPerforationHistoryBaseMd(0, 0, "Mean Sea Level", gsoap_eml2_1::eml21__LengthUom__m, 1980);
	wellboreCompletion->setPerforationHistoryStartDate(0, 0, 407568645);
	wellboreCompletion->setPerforationHistoryEndDate(0, 0, 1514764800);
	wellboreCompletion->pushBackPerforationHistory(0);
	wellboreCompletion->setPerforationHistoryStatus(1, 0, gsoap_eml2_1::witsml20__PerforationStatus__squeezed);
	wellboreCompletion->setPerforationHistoryTopMd(1, 0, "Mean Sea Level", gsoap_eml2_1::eml21__LengthUom__m, 1970);
	wellboreCompletion->setPerforationHistoryBaseMd(1, 0, "Mean Sea Level", gsoap_eml2_1::eml21__LengthUom__m, 1980);
	wellboreCompletion->setPerforationHistoryStartDate(1, 0, 1514764800);
	wellboreCompletion->pushBackPerforationHistory(1);
	wellboreCompletion->setPerforationHistoryStatus(0, 1, gsoap_eml2_1::witsml20__PerforationStatus__open);
	wellboreCompletion->setPerforationHistoryStartDate(0, 1, 410104800);
	wellboreCompletion->pushBackPerforationHistory(1);
	wellboreCompletion->setPerforationHistoryStatus(1, 1, gsoap_eml2_1::witsml20__PerforationStatus__squeezed);
	wellboreCompletion->setPerforationHistoryStartDate(1, 1, 1514764800);
}

#ifdef WITH_EXPERIMENTAL
void serializeGraphicalInformationSet(COMMON_NS::DataObjectRepository * repo, COMMON_NS::AbstractHdfProxy * hdfProxy)
{
	COMMON_NS::GraphicalInformationSet* graphicalInformationSet = repo->createGraphicalInformationSet("be17c053-9189-4bc0-9db1-75aa51a026cd", "Graphical Information Set");

	// *************
	// Default color
	// *************

	// fault1 representation is blue
	graphicalInformationSet->setDefaultHsvColor(fault1, 240., 1., 1., 1., "blue");
	graphicalInformationSet->setDefaultHsvColor(fault1Interp1, 240., 1., 1., 1., "blue");
	graphicalInformationSet->setDefaultHsvColor(f1i1triRepSinglePatch, 240., 1., 1., 1., "blue");
	graphicalInformationSet->setDefaultHsvColor(f1i1triRep, 240., 1., 1., 1., "blue");
	graphicalInformationSet->setDefaultHsvColor(f1i1PolyLineRep, 240., 1., 1., 1., "blue");

	// horizon1 representation is red
	graphicalInformationSet->setDefaultHsvColor(horizon1, 0., 1., 1., 1., "red");
	graphicalInformationSet->setDefaultHsvColor(horizon1Interp1, 0., 1., 1., 1., "red");
	graphicalInformationSet->setDefaultHsvColor(h1i1triRep, 0., 1., 1., 1., "red");

	// horizon2 representation is green
	graphicalInformationSet->setDefaultHsvColor(horizon2, 120., 1., 1., 1., "green");
	graphicalInformationSet->setDefaultHsvColor(horizon2Interp1, 120., 1., 1., 1., "green");
	graphicalInformationSet->setDefaultHsvColor(h2i1triRep, 120., 1., 1., 1., "green");
	graphicalInformationSet->setDefaultHsvColor(h1i1SingleGrid2dRep, 120., 1., 1., 1., "green");

	// ******************
	// Discrete color map
	// ******************

	// associating a discrete color map to property kind propType1
	RESQML2_2_NS::DiscreteColorMap* propKindDiscrColMap = repo->createDiscreteColorMap("d808d79c-2cad-4c4f-9712-3b3ab4aa3f4a", "Property kind discrete color map");
	unsigned int propKindDiscrColMapRgbColors[9] = { 0, 0, 255, 255, 255, 255, 255, 0, 0 };
	double propKindDiscrColMapAlphas[3] = { 1., 1., 1. };
	vector<string> propKindDiscrColMapTitles = { "blue", "white", "red" };
	propKindDiscrColMap->setRgbColors(3, propKindDiscrColMapRgbColors, propKindDiscrColMapAlphas, propKindDiscrColMapTitles);
	graphicalInformationSet->setDiscreteColorMap(propType1, propKindDiscrColMap);

	// associating a discrete color map to dicreteProp1
	RESQML2_2_NS::DiscreteColorMap* discrColMap = repo->createDiscreteColorMap("3daf4661-ae8f-4357-adee-0b0159bdd0a9", "Discrete color map");
	unsigned int discrColMapRgbColors[18] = { 255, 0, 0, 0, 255, 0, 0, 0, 255, 169, 84, 27, 0, 0, 0, 255, 255, 255 };
	double discrColMapAlphas[6] = { 1., 1., 1., 1., 1., 1. };
	vector<string> discrColMapTitles = { "red", "green", "blue", "orange", "black", "white" };
	discrColMap->setRgbColors(6, discrColMapRgbColors, discrColMapAlphas, discrColMapTitles);
	graphicalInformationSet->setDiscreteColorMap(discreteProp1, discrColMap);

	// creating a new discrete property of type propType1 without associating it to a discrete color map.
	// Thus, its associated discrete color map remains the one associated to propType1
	DiscreteProperty* discreteProp2 = repo->createDiscreteProperty(ijkgrid, "1e2822ef-b6cb-4123-bdf4-c99df84a896f", "Another two faulted sugar cubes cellIndex", 1,
		gsoap_resqml2_0_1::resqml20__IndexableElements__cells, propType1);
	unsigned short prop2Values[2] = { 0, 1 };
	discreteProp2->pushBackUShortHdf5Array3dOfValues(prop2Values, 2, 1, 1, hdfProxy, -1);

	// ********************
	// Continuous color map
	// ********************

	Horizon* contColMapHrz = repo->createHorizon("b9ec6ec9-2766-4af7-889e-5565b5fa5022", "Horizon for continuous color map");
	HorizonInterpretation* contColMapHrzInterp = repo->createHorizonInterpretation(contColMapHrz, "34b69c81-6cfa-4531-be5b-f6bd9b74802f", "Horizon interpretation for continuous color map");
	Grid2dRepresentation* contColMapGrid2dRep = repo->createGrid2dRepresentation(contColMapHrzInterp, "4e56b0e4-2cd1-4efa-97dd-95f72bcf9f80", "100x10 grid 2d for continuous color map");
	const unsigned int numPointInFastestDirection = 50;
	const unsigned int numPointsInSlowestDirection = 100;
	contColMapGrid2dRep->setGeometryAsArray2dOfLatticePoints3d(numPointInFastestDirection, numPointsInSlowestDirection,
		0., 0., 0.,
		1., 0., 0.,
		0., 1., 0.,
		1., 1.);

	contColMapContProp = repo->createContinuousProperty(contColMapGrid2dRep, "c2be50b6-08d2-461b-81a4-73dbb04ba605", "Continuous property for continuous color map", 2,
		gsoap_resqml2_0_1::resqml20__IndexableElements__nodes, "continuousColorMapIndex", gsoap_resqml2_0_1::resqml20__ResqmlPropertyKind__continuous);
	double* values = new double[numPointInFastestDirection * numPointsInSlowestDirection];
	for (size_t slowestIndex = 0; slowestIndex < numPointsInSlowestDirection; ++slowestIndex) {
		for (size_t fastestIndex = 0; fastestIndex < numPointInFastestDirection; ++fastestIndex) {
			values[fastestIndex + slowestIndex * numPointInFastestDirection] = fastestIndex * (1. / (numPointInFastestDirection - 1));
		}
	}
	contColMapContProp->pushBackDoubleHdf5Array2dOfValues(values, numPointInFastestDirection, numPointsInSlowestDirection, hdfProxy);
	delete[] values;

	RESQML2_2_NS::ContinuousColorMap* contColMap = repo->createContinuousColorMap("a207faa2-963e-48d6-b3ad-53f6c1fc4dd4", "Continuous color map", gsoap_eml2_2::resqml22__InterpolationDomain__rgb, gsoap_eml2_2::resqml22__InterpolationMethod__linear);
	unsigned int contColMapRgbColors[6] = { 0, 255, 0, 255, 0, 0 };
	vector<string> contColMapColTitles = { "green", "red" };
	double contColMapAlphas[2] = { 1., 1. };
	contColMap->setRgbColors(2, contColMapRgbColors, contColMapAlphas, contColMapColTitles);
	graphicalInformationSet->setContinuousColorMap(contColMapContProp, contColMap);
	graphicalInformationSet->setColorMapMinMax(contColMapContProp, 0., 1.);
	graphicalInformationSet->setValueVectorIndex(contColMapContProp, 1);
}
#endif

void serializeStratigraphicModel(COMMON_NS::DataObjectRepository * pck, COMMON_NS::AbstractHdfProxy* hdfProxy)
{
	// Build the Stratigraphic column
	StratigraphicColumn* stratiColumn = pck->createStratigraphicColumn("7f6666a0-fa3b-11e5-a509-0002a5d5c51b", "Stratigraphic column");
	OrganizationFeature* stratiModelFeature = pck->createStratigraphicModel("7fcde2b5-d184-4481-b31e-32aafbdc4b7f", "stratiModel");
	pck->createStratigraphicOccurrenceInterpretationInApparentDepth(stratiModelFeature, "2426e574-1ea3-4f20-9deb-669c15a17625", "stratiModel Interp");
	stratiColumnRank0 = pck->createStratigraphicColumnRankInterpretationInApparentDepth(stratiModelFeature, "ba06f220-fa3b-11e5-928c-0002a5d5c51b", "Stratigraphic column rank 0", 0);
	StratigraphicColumnRankInterpretation* stratiColumnRank1 = pck->createStratigraphicColumnRankInterpretationInApparentDepth(stratiModelFeature, "9d2d19cf-aedb-4766-9691-758d536456ba", "Stratigraphic column rank 1", 1);
	stratiColumn->pushBackStratiColumnRank(stratiColumnRank0);
	stratiColumn->pushBackStratiColumnRank(stratiColumnRank1);
	StratigraphicUnitFeature* stratiUnitAFeature = pck->createStratigraphicUnit("0426c6a0-fa3c-11e5-8b9c-0002a5d5c51b", "Unit A");
	StratigraphicUnitInterpretation* stratiUnitAInterp = pck->createStratigraphicUnitInterpretation(stratiUnitAFeature, "1a919b40-fa3c-11e5-a72c-0002a5d5c51b", "Unit A interp");
	StratigraphicUnitFeature* stratiUnitBFeature = pck->createStratigraphicUnit("273a92c0-fa3c-11e5-85f8-0002a5d5c51b", "Unit B");
	StratigraphicUnitInterpretation* stratiUnitBInterp = pck->createStratigraphicUnitInterpretation(stratiUnitBFeature, "2b9169c0-fa3c-11e5-ae2c-0002a5d5c51b", "Unit B interp");
	StratigraphicUnitFeature* stratiUnitB1Feature = pck->createStratigraphicUnit("0b7cc266-4280-4696-b9dc-5d17017797e2", "Unit B1");
	StratigraphicUnitInterpretation* stratiUnitB1Interp = pck->createStratigraphicUnitInterpretation(stratiUnitB1Feature, "7d7ab0bc-554d-48f5-ab5c-3bb7b66696e5", "Unit B1 interp");
	StratigraphicUnitFeature* stratiUnitB2Feature = pck->createStratigraphicUnit("87255cf5-033f-4fa4-941b-7947b434f4c2", "Unit B2");
	StratigraphicUnitInterpretation* stratiUnitB2Interp = pck->createStratigraphicUnitInterpretation(stratiUnitB2Feature, "34c37be0-964f-41a8-ba78-db5147744927", "Unit B2 interp");

	// Build a minimal Stratigraphic column to allow the definition of a minimal sealed volume framework
	StratigraphicColumn* minimalStratiColumn = pck->createStratigraphicColumn("90f375b7-6865-4a69-943e-f2328fdbbb7e", "Minimal Stratigraphic column");
	OrganizationFeature* minimalStratiModelFeature = pck->createStratigraphicModel("ea021981-1b79-435f-9f26-8bf03472efcf", "Minimal stratiModel");
	StratigraphicColumnRankInterpretation* minimalStratiColumnRank = pck->createStratigraphicColumnRankInterpretationInApparentDepth(minimalStratiModelFeature, "31b0b787-31df-4b87-8830-a3863eb9842c", "Minimal Stratigraphic column rank ", 0);
	minimalStratiColumn->pushBackStratiColumnRank(minimalStratiColumnRank);

	// Build the stratigraphic column ranks
	stratiColumnRank0->pushBackStratiUnitInterpretation(stratiUnitAInterp);
	stratiColumnRank1->pushBackStratiUnitInterpretation(stratiUnitAInterp);
	stratiColumnRank0->pushBackStratiUnitInterpretation(stratiUnitBInterp);
	stratiColumnRank1->pushBackStratiUnitInterpretation(stratiUnitB1Interp);
	stratiColumnRank1->pushBackStratiUnitInterpretation(stratiUnitB2Interp);
	minimalStratiColumnRank->pushBackStratiUnitInterpretation(stratiUnitB1Interp);
	stratiColumnRank0->pushBackStratigraphicBinaryContact(stratiUnitAInterp, gsoap_resqml2_0_1::resqml20__ContactMode__proportional, stratiUnitBInterp, gsoap_resqml2_0_1::resqml20__ContactMode__proportional, horizon2Interp1);

	// WellboreFeature marker frame
	if (wellbore1Interp1 != nullptr) {
		WellboreMarkerFrameRepresentation* wmf = pck->createWellboreMarkerFrameRepresentation(wellbore1Interp1, "657d5e6b-1752-425d-b3e7-237037fa11eb", "Wellbore Marker Frame", w1i1TrajRep);
		double markerMdValues[2] = { 350, 550 };
		wmf->setMdValues(markerMdValues, 2, hdfProxy);
		WellboreMarker* marker0 = new WellboreMarker(wmf, "624f9f17-6797-4d78-b3fc-9ca2c8174bcd", "", gsoap_resqml2_0_1::resqml20__GeologicBoundaryKind__horizon);
		marker0->setBoundaryFeatureInterpretation(horizon1Interp1);
		WellboreMarker* marker1 = new WellboreMarker(wmf, "3611725e-4d9b-4d3e-87e6-58fcd238f5a8", "testing Fault", gsoap_resqml2_0_1::resqml20__GeologicBoundaryKind__fault);
		marker1->setBoundaryFeatureInterpretation(fault1Interp1);
	}

	// ***********************
	// Sealed volume framework
	// ***********************
	SealedVolumeFrameworkRepresentation* svf = pck->createSealedVolumeFrameworkRepresentation(stratiColumnRank1, "c7ed87c2-9a46-4e3d-8f0f-b25d4d72892a", "Minimal Sealed volume framework", sealedSurfaceFramework);

	// Add the surfaces.
	f1i1triRep->pushBackIntoRepresentationSet(svf);
	h1i1triRep->pushBackIntoRepresentationSet(svf);
	h2i1triRep->pushBackIntoRepresentationSet(svf);
	xMinusFrontierRep->pushBackIntoRepresentationSet(svf);
	xPlusFrontierRep->pushBackIntoRepresentationSet(svf);
	yMinusFrontierRep->pushBackIntoRepresentationSet(svf);
	yPlusFrontierRep->pushBackIntoRepresentationSet(svf);

	//Region 2
	std::vector<unsigned int> region2RepIndices = { 1, 0, 0, 2, 3, 5, 6 }; // face order => top, x plus, btm, x minus, y minus, y plus
	std::vector<unsigned int> region2PatchIndices = { 0, 1, 2, 0, 0, 0, 0 };
	bool region2Sides[7] = { false, false, false, true, true, true, true }; //Top face is true, bottom face is false and Frontiers are always on true side flag in this example.
	svf->pushBackVolumeRegion(stratiUnitB1Interp, 7, region2RepIndices.data(), region2PatchIndices.data(), region2Sides);

	//Region 5
	std::vector<unsigned int> region5RepIndices = { 1, 4, 2, 0, 0, 5, 6 };
	std::vector<unsigned int> region5PatchIndices = { 1, 0, 1, 3, 2, 1, 1 };
	bool region5Sides[7] = { false, true, true, true, true, true, true };
	svf->pushBackVolumeRegion(stratiUnitB1Interp, 7, region5RepIndices.data(), region5PatchIndices.data(), region5Sides);
}

void serializeGeobody(COMMON_NS::DataObjectRepository * pck, COMMON_NS::AbstractHdfProxy* hdfProxy)
{
	// 2D
	GeneticBoundaryFeature* geobodyBoundary = pck->createGeobodyBoundaryFeature("6d3c158c-303f-4b0d-bfc0-9ce4102ea616", "Geobody boundary");
	GeobodyBoundaryInterpretation* geobodyBoundaryInterp = pck->createGeobodyBoundaryInterpretation(geobodyBoundary, "12c301a4-3e8b-401a-aca3-8d6f02d5d6d5", "Geobody boundary interp");
	PointSetRepresentation* geobodyBoundaryPointSetRep = pck->createPointSetRepresentation(geobodyBoundaryInterp, "fbc5466c-94cd-46ab-8b48-2ae2162b372f", "Geobody boundary PointSetRep");
	double geobodyBoundaryPointCoords[18] = { 10, 70, 310, 11, 21, 280, 150, 30, 310, 400, 0, 365, 450, 75, 341, 475, 100, 352 };
	geobodyBoundaryPointSetRep->pushBackGeometryPatch(6, geobodyBoundaryPointCoords, hdfProxy);

	// 3D
	GeobodyFeature* geobody = pck->createGeobodyFeature("e221f9da-ead3-4a9d-8324-fc2e6606cb01", "Geobody");
	GeobodyInterpretation* geobodyInterp = pck->createGeobodyInterpretation(geobody, "d445041f-6364-44e7-a7f8-ade5a93bfd49", "Geobody interp");
	PointSetRepresentation* geobodyGraphNode = pck->createPointSetRepresentation(geobodyInterp, "8442a6b7-a97b-431e-abda-f72cf7ef346f", "Geobody graph node");
	double geobodyPointCoords[18] = { 50, 30, 330, 10, 28, 3000, 100, 50, 350, 300, 100, 400, 400, 20, 400, 400, 300, 400 };
	geobodyGraphNode->pushBackGeometryPatch(6, geobodyPointCoords, hdfProxy);
	//RESQML2_NS::SubRepresentation* geobodyGraphEdge = pck->createSubRepresentation(geobodyBoundaryInterp, "7e0450aa-c39d-49f8-bee4-62fc42bb849d", "Geobody graph edge");
}

void serializeBoundaries(COMMON_NS::DataObjectRepository * pck, COMMON_NS::AbstractHdfProxy* hdfProxy)
{
	// Seismic Lattice
	SeismicLatticeFeature* seismicLattice = pck->createSeismicLattice("eb6a5e97-4d86-4809-b136-051f34cfcb51", "Seismic lattice", 2, 2, 150, 152, 4, 2);
	GenericFeatureInterpretation* seismicLatticeInterp = pck->createGenericFeatureInterpretation(seismicLattice, "97816427-6ef6-4776-b21c-5b93c8a6310a", "Seismic lattice Interp");
	Grid2dRepresentation* seismicLatticeRep = pck->createGrid2dRepresentation(seismicLatticeInterp, "aa5b90f1-2eab-4fa6-8720-69dd4fd51a4d", "Seismic lattice Rep");
	seismicLatticeRep->setGeometryAsArray2dOfLatticePoints3d(4, 2, 0, 0, 0, 1, 0, 2, 0, 1, 3, 250, 200);

	// Seismic Line Set
	SeismicLineSetFeature* seismicLineSet = pck->createSeismicLineSet("53c6a0be-c901-4bb6-845b-fba79745da02", "Seismic line Set");

	// Seismic Line
	SeismicLineFeature* seismicLine = pck->createSeismicLine("117f9bf6-6bb0-49f2-9cee-46912300bff6", "Seismic line", 1, 0, 5);
	seismicLine->setSeismicLineSet(seismicLineSet);
	GenericFeatureInterpretation* seismicLineInterp = pck->createGenericFeatureInterpretation(seismicLine, "", "Seismic line Interp");
	PolylineRepresentation* seismicLineRep = pck->createPolylineRepresentation(seismicLineInterp, "", "Seismic line Rep");
	double seismicLinePoints[15] = { 0, 100, 0, 150, 110, 0, 300, 120, 0, 450, 130, 0, 600, 140, 0 };
	seismicLineRep->setGeometry(seismicLinePoints, 5, hdfProxy);

	// Features
	//BoundaryFeature* bf = pck->createBoundaryFeature("", "testingBoundaryFeature");
	horizon1 = pck->createHorizon("35d7b57e-e5ff-4062-95af-ba2d7c4ce347", "Horizon1");
	tm timeStruct;
	timeStruct.tm_hour = 15;
	timeStruct.tm_min = 2;
	timeStruct.tm_sec = 35;
	timeStruct.tm_mday = 8;
	timeStruct.tm_mon = 1;
	timeStruct.tm_year = 0;
	timeStruct.tm_isdst = -1;
	mktime(&timeStruct);
	horizon1->setCreation(timeStruct);
	horizon1->setAge(300000000);
	horizon2 = pck->createHorizon("fd7950a6-f62e-4e47-96c4-048820a61c59", "Horizon2");
	horizon2->setVersion("my version");
	fault1 = pck->createFault("1424bcc2-3d9d-4f30-b1f9-69dcb897e33b", "Fault1");
	fault1->setMetadata("", "philippe", 148526020, "philippe", "", 148526100, "");

	// Interpretations
	//BoundaryFeatureInterpretation* bfi = pck->createBoundaryFeatureInterpretation(bf, "", "testingBoundaryFeatureInterp");
	horizon1Interp1 = pck->createHorizonInterpretation(horizon1, "ac12dc12-4951-459b-b585-90f48aa88a5a", "Horizon1 Interp1");
	horizon2Interp1 = pck->createHorizonInterpretation(horizon2, "06993923-6331-4fad-bceb-fc79b05adfd7", "Horizon2 Interp1");
	fault1Interp1 = pck->createFaultInterpretation(fault1, "ba224651-7dd3-4952-85b0-cff6fe37508d", "Fault1 Interp1");

	//**************
	// Horizon Representations
	//**************
	PolylineRepresentation* h1i1SinglePolylineRep = pck->createPolylineRepresentation(horizon1Interp1, "", "Horizon1 Interp1 SinglePolylineRep");
	double h1i1SinglePolylineRepPoints[12] = { 0, 100, 300, 150, 110, 300, 450, 130, 350, 600, 140, 350 };
	h1i1SinglePolylineRep->setGeometry(h1i1SinglePolylineRepPoints, 4, hdfProxy);
	double seismicLineAbscissa[4] = { 0.0, 1.0, 3.0, 4.0 };
	h1i1SinglePolylineRep->addSeismic2dCoordinatesToPatch(0, seismicLineAbscissa, seismicLineRep, hdfProxy);

#if defined(OFFICIAL)
	h1i1SingleGrid2dRep = pck->createGrid2dRepresentation(horizon1Interp1, local3dCrs, "", "Horizon1 Interp1 Grid2dRep");
	double zValues[8] = { 300, 300, 350, 350, 300, 300, 350, 350 };
	h1i1SingleGrid2dRep->setGeometryAsArray2dOfExplicitZ(zValues, 4, 2, hdfProxy, seismicLatticeRep);
#else
	h1i1SingleGrid2dRep = pck->createGrid2dRepresentation(horizon1Interp1, "", "Horizon1 Interp1 Grid2dRep");
	double zValues[8] = { 300, 300, 350, 350, 300, 300, 350, 350 };
	h1i1SingleGrid2dRep->setGeometryAsArray2dOfExplicitZ(zValues, 4, 2, hdfProxy, seismicLatticeRep, localTime3dCrs);
#endif

	PointSetRepresentation* h1i1PointSetRep = pck->createPointSetRepresentation(horizon1Interp1, "", "Horizon1 Interp1 PointSetRep");
	double pointCoords[18] = { 10, 70, 301, 11, 21, 299, 150, 30, 301, 400, 0, 351, 450, 75, 340, 475, 100, 350 };
	h1i1PointSetRep->pushBackGeometryPatch(6, pointCoords, hdfProxy);

	// Horizon 1 triangulated representation
	h1i1triRep = pck->createTriangulatedSetRepresentation(horizon1Interp1,
		"0c49b40a-632a-457a-b519-a178f40a397d",
		"Horizon1 Interp1 TriRep");
	// Patch 0
	double explicitPointsHor1Patch0[15] = { 250, 0, 300, 250, 100, 300, 250, 200, 300, 0, 0, 300, 0, 200, 300 };
	unsigned int triangleNodeIndexHorPatch0[9] = { 0, 1, 3, 1, 4, 3, 1, 2, 4 };
	h1i1triRep->pushBackTrianglePatch(5, explicitPointsHor1Patch0, 3, triangleNodeIndexHorPatch0, hdfProxy);
	// Patch 1
	double explicitPointsHor1Patch1[15] = { 300, 0, 350, 300, 100, 350, 300, 200, 350, 700, 0, 350, 700, 200, 350 };
	unsigned int triangleNodeIndexHorPatch1[9] = { 5, 6, 8, 6, 9, 8, 6, 7, 9 };
	h1i1triRep->pushBackTrianglePatch(5, explicitPointsHor1Patch1, 3, triangleNodeIndexHorPatch1, hdfProxy);

	// Horizon 2 triangulated representation
	h2i1triRep = pck->createTriangulatedSetRepresentation(horizon2Interp1,
		"ea6943d0-30b0-47fd-80ad-a3cf019a8c92",
		"Horizon2 Interp1 TriRep");
	// Patch 0 (no seismic support)
	double explicitPointsHor2Patch0[15] = { 450, 0, 500, 450, 100, 500, 450, 200, 500, 0, 0, 500, 0, 200, 500 };
	h2i1triRep->pushBackTrianglePatch(5, explicitPointsHor2Patch0, 3, triangleNodeIndexHorPatch0, hdfProxy);
	// Patch 1 (seismic support)
	double explicitPointsHor2Patch1[15] = { 500, 0, 550, 500, 100, 550, 500, 200, 550, 700, 0, 550, 700, 200, 550 };
	h2i1triRep->pushBackTrianglePatch(5, explicitPointsHor2Patch1, 3, triangleNodeIndexHorPatch1, hdfProxy);
	double inlines[5] = { 0, 1, 2, 3, 4 }; // dummy values
	double crosslines[5] = { 10, 11, 12, 13, 14 }; // dummy values
	h2i1triRep->addSeismic3dCoordinatesToPatch(1, inlines, crosslines, 5, seismicLatticeRep, hdfProxy);

	// TriRep without interp
	TriangulatedSetRepresentation* triRepWithoutInterp = pck->createTriangulatedSetRepresentation("eb0b55a5-9562-4642-946b-d2ec57172daf", "TriRep without interp");
	triRepWithoutInterp->pushBackTrianglePatch(5, explicitPointsHor2Patch0, 3, triangleNodeIndexHorPatch0, hdfProxy);

	//**************
	// Fault Representations
	//**************

	// Single patch triangulated representation
	f1i1triRepSinglePatch = pck->createTriangulatedSetRepresentation(fault1Interp1,
		"d8a03d57-8bf3-4f75-8645-ef2fbfa5d1e3",
		"Fault1 Interp1 TriRep Single Patch");
	//hsize_t dimExplicitPointsFault1 [1] = {6};
	double explicitPointsFault1[54] = { 150, 0, 200, 150, 100, 200, 150, 200, 200,
		250, 0, 300, 250, 100, 300, 250, 200, 300,
		300, 0, 350, 300, 100, 350, 300, 200, 350,
		450, 0, 500, 450, 100, 500, 450, 200, 500,
		500, 0, 550, 500, 100, 550, 500, 200, 550,
		600, 0, 650, 600, 100, 650, 600, 200, 650 };
	unsigned int triangleNodeIndexFault[60] = { 0, 4, 3, 0, 1, 4, 1, 2, 4, 2, 5, 4,
		3, 7, 6, 3, 4, 7, 4, 5, 7, 5, 8, 7,
		6, 10, 9, 6, 7, 10, 7, 8, 10, 8, 11, 10,
		9, 13, 12, 9, 10, 13, 10, 11, 13, 11, 14, 13,
		12, 16, 15, 12, 13, 16, 13, 14, 16, 14, 17, 16 };
	f1i1triRepSinglePatch->pushBackTrianglePatch(18, explicitPointsFault1, 20, triangleNodeIndexFault, hdfProxy);

	// multi patch triangulated representation
	f1i1triRep = pck->createTriangulatedSetRepresentation(fault1Interp1,
		"1a4112fa-c4ef-4c8d-aed0-47d9273bebc5",
		"Fault1 Interp1 TriRep");
	// Patch 0
	double explicitPointsFault1Patch0[18] = { 150, 0, 200, 150, 100, 200, 150, 200, 200,
		250, 0, 300, 250, 100, 300, 250, 200, 300 };
	unsigned int triangleNodeIndexFaultPatch0[12] = { 0, 4, 3, 0, 1, 4, 1, 2, 4, 2, 5, 4 };
	f1i1triRep->pushBackTrianglePatch(6, explicitPointsFault1Patch0, 4, triangleNodeIndexFaultPatch0, hdfProxy);
	// Patch 1
	double explicitPointsFault1Patch1[18] = { 250, 0, 300, 250, 100, 300, 250, 200, 300,
		300, 0, 350, 300, 100, 350, 300, 200, 350 };
	unsigned int triangleNodeIndexFaultPatch1[12] = { 6, 10, 9, 6, 7, 10, 7, 8, 10, 8, 11, 10 };
	f1i1triRep->pushBackTrianglePatch(6, explicitPointsFault1Patch1, 4, triangleNodeIndexFaultPatch1, hdfProxy);
	// Patch 2
	double explicitPointsFault1Patch2[18] = { 300, 0, 350, 300, 100, 350, 300, 200, 350,
		450, 0, 500, 450, 100, 500, 450, 200, 500 };
	unsigned int triangleNodeIndexFaultPatch2[12] = { 12, 16, 15, 12, 13, 16, 13, 14, 16, 14, 17, 16 };
	f1i1triRep->pushBackTrianglePatch(6, explicitPointsFault1Patch2, 4, triangleNodeIndexFaultPatch2, hdfProxy);
	// Patch 3
	double explicitPointsFault1Patch3[18] = { 450, 0, 500, 450, 100, 500, 450, 200, 500,
		500, 0, 550, 500, 100, 550, 500, 200, 550 };
	unsigned int triangleNodeIndexFaultPatch3[12] = { 18, 22, 21, 18, 19, 22, 19, 20, 22, 20, 23, 22 };
	f1i1triRep->pushBackTrianglePatch(6, explicitPointsFault1Patch3, 4, triangleNodeIndexFaultPatch3, hdfProxy);
	// Patch 4
	double explicitPointsFault1Patch4[18] = { 500, 0, 550, 500, 100, 550, 500, 200, 550,
		600, 0, 650, 600, 100, 650, 600, 200, 650 };
	unsigned int triangleNodeIndexFaultPatch4[12] = { 24, 28, 27, 24, 25, 28, 25, 26, 28, 26, 29, 28 };
	f1i1triRep->pushBackTrianglePatch(6, explicitPointsFault1Patch4, 4, triangleNodeIndexFaultPatch4, hdfProxy);

	// Fault polyline rep
	f1i1PolyLineRep = pck->createPolylineSetRepresentation(
		fault1Interp1,
		"65c59595-bf48-451e-94aa-120ebdf28d8b",
		"Fault1 Interp1 PolylineRep");
	unsigned int numNodesPerPolylinePerPatch[2] = { 3, 2 };
	double polylinePoints[15] = { 150, 0, 200, 300, 0, 350, 450, 0, 500, 150, 200, 200, 450, 200, 500 };
	f1i1PolyLineRep->pushBackGeometryPatch(numNodesPerPolylinePerPatch, polylinePoints, 2, false, hdfProxy);
	f1i1PolyLineRep->addSeismic3dCoordinatesToPatch(0, inlines, crosslines, 5, seismicLatticeRep, hdfProxy);

#if !defined(OFFICIAL)
	//**************
	// Properties
	//**************
	propType1 = pck->createPropertyKind("f7ad7cf5-f2e7-4daa-8b13-7b3df4edba3b", "propType1", "urn:resqml:f2i.com:testingAPI", gsoap_resqml2_0_1::resqml20__ResqmlUom__Euc, gsoap_resqml2_0_1::resqml20__ResqmlPropertyKind__continuous);
	ContinuousProperty* contProp1 = pck->createContinuousProperty(h1i1SingleGrid2dRep, "fcaccfc7-10cb-4f73-800e-a381642478cb", "Horizon1 Interp1 Grid2dRep Prop1", 2,
		gsoap_resqml2_0_1::resqml20__IndexableElements__nodes, "exoticMeter", propType1);
	double prop1Values[16] = { 301, 302, 301, 302, 351, 352, 351, 352, 301, 302, 301, 302, 351, 352, 351, 352 };
	contProp1->pushBackDoubleHdf5Array2dOfValues(prop1Values, 2, 8, hdfProxy);

	RESQML2_NS::PropertyKind * propType2 = pck->createPropertyKind("7372f8f6-b1fd-4263-b9a8-699d9cbf7da6", "propType2", "urn:resqml:f2i.com:testingAPI", gsoap_resqml2_0_1::resqml20__ResqmlUom__Euc, propType1);
	ContinuousProperty* contProp2 = pck->createContinuousProperty(h1i1SingleGrid2dRep, "d3efb337-19f8-4b91-8b4f-3698afe17f01", "Horizon1 Interp1 Grid2dRep Prop2", 1,
		gsoap_resqml2_0_1::resqml20__IndexableElements__nodes, gsoap_resqml2_0_1::resqml20__ResqmlUom__ft, propType2);
	double prop2Values[8] = { 302, 302, 352, 352, 302, 302, 352, 352 };
	contProp2->pushBackDoubleHdf5Array1dOfValues(prop2Values, 8, hdfProxy);
#endif
}

void serializeGrid(COMMON_NS::DataObjectRepository * pck, COMMON_NS::AbstractHdfProxy* hdfProxy)
{
	OrganizationFeature * earthModel = pck->createEarthModel("f2060ce0-fa3d-11e5-8620-0002a5d5c51b", "Grid");
	EarthModelInterpretation * earthModelInterp = pck->createEarthModelInterpretation(earthModel, "f5cd7520-fa3d-11e5-b65b-0002a5d5c51b", "Grid interp");

	// ONE SUGAR
	IjkGridExplicitRepresentation* singleCellIjkgrid = pck->createIjkGridExplicitRepresentation("e69bfe00-fa3d-11e5-b5eb-0002a5d5c51b", "One unfaulted sugar cube", 1, 1, 1);
	double singleCellIjkgridNodes[24] = { 0, 0, 300, 700, 0, 350, 0, 150, 300, 700, 150, 350,
		0, 0, 500, 700, 0, 550, 0, 150, 500, 700, 150, 550 };
	singleCellIjkgrid->setGeometryAsCoordinateLineNodes(gsoap_resqml2_0_1::resqml20__PillarShape__vertical, gsoap_resqml2_0_1::resqml20__KDirection__down, false, singleCellIjkgridNodes, hdfProxy);

	// TWO SUGARS EXPLICIT
	ijkgrid = pck->createIjkGridExplicitRepresentation(earthModelInterp, "df2103a0-fa3d-11e5-b8d4-0002a5d5c51b", "Two faulted sugar cubes (explicit geometry)", 2, 1, 1);
	double nodes[48] = { 0, 0, 300, 375, 0, 300, 700, 0, 350, 0, 150, 300, 375, 150, 300, 700, 150, 350, /* SPLIT*/ 375, 0, 350, 375, 150, 350,
		0, 0, 500, 375, 0, 500, 700, 0, 550, 0, 150, 500, 375, 150, 500, 700, 150, 550, /* SPLIT*/ 375, 0, 550, 375, 150, 550 };
	unsigned int pillarOfCoordinateLine[2] = { 1, 4 };
	unsigned int splitCoordinateLineColumnCumulativeCount[2] = { 1, 2 };
	unsigned int splitCoordinateLineColumns[2] = { 1, 1 };
	ijkgrid->setGeometryAsCoordinateLineNodes(gsoap_resqml2_0_1::resqml20__PillarShape__vertical, gsoap_resqml2_0_1::resqml20__KDirection__down, false, nodes, hdfProxy,
		2, pillarOfCoordinateLine, splitCoordinateLineColumnCumulativeCount, splitCoordinateLineColumns);

	// FOUR SUGARS PARAMETRIC
	IjkGridParametricRepresentation* ijkgridParametric = pck->createIjkGridParametricRepresentation(earthModelInterp, "37c45c00-fa3e-11e5-a21e-0002a5d5c51b", "Four faulted sugar cubes (parametric geometry)", 2, 1, 2);
	double parameters[24] = { 300, 300, 350, 300, 300, 350, /* SPLIT*/ 350, 350,
		400, 400, 450, 400, 400, 450, /* SPLIT*/ 450, 450,
		500, 500, 550, 500, 500, 550, /* SPLIT*/ 550, 550 };
	double controlPoints[18] = { 0, 0, 300, 375, 0, 300, 700, 0, 350, 0, 150, 300, 375, 150, 300, 700, 150, 350 };
	ijkgridParametric->setGeometryAsParametricSplittedPillarNodes(false, parameters, controlPoints, NULL, 1, 0, hdfProxy,
		2, pillarOfCoordinateLine, splitCoordinateLineColumnCumulativeCount, splitCoordinateLineColumns);

	// FOUR SUGARS PARAMETRIC STRAIGHT
	IjkGridParametricRepresentation* ijkgridParametricStraight = pck->createIjkGridParametricRepresentation(earthModelInterp, "f68235af-1d7a-4e24-93a8-10739b15ca40", "Four faulted sugar cubes (straight parametric geometry)", 2, 1, 2);
	double controlPointsParametricStraight[36] = { 0, 0, 300, 375, 0, 300, 700, 0, 350, 0, 150, 300, 375, 150, 300, 700, 150, 350,
		0, 0, 500, 375, 0, 500, 700, 0, 550, 0, 150, 500, 375, 150, 500, 700, 150, 550 };
	double controlPointsParameters[12] = { 300, 300, 350, 300, 300, 350,
		500, 500, 550, 500, 500, 550 };
	ijkgridParametricStraight->setGeometryAsParametricSplittedPillarNodes(false, parameters, controlPointsParametricStraight, controlPointsParameters, 2, 1, hdfProxy,
		2, pillarOfCoordinateLine, splitCoordinateLineColumnCumulativeCount, splitCoordinateLineColumns);

	// FOUR SUGARS PARAMETRIC different line kind and one cubic pillar
	IjkGridParametricRepresentation* ijkgridParametricNotSameLineKind = pck->createIjkGridParametricRepresentation("3ce91933-4f6f-4f35-b0ac-4ba4672f0a87", "Four faulted sugar cubes with one cubic pillar", 2, 1, 2);
	const double nan = numeric_limits<double>::quiet_NaN();
	double controlPointsNotSameLineKind[54] = { 0, 0, 300, 375, 0, 300, 700, 0, 350, 0, 150, 300, 375, 150, 300, 700, 150, 350,
		50, 30, 1000, 400, 0, 400, nan, nan, nan, nan, nan, nan, nan, nan, nan, nan, nan, nan,
		nan, nan, nan, 450, 0, 600, nan, nan, nan, nan, nan, nan, nan, nan, nan, nan, nan, nan };
	double controlPointParametersNotSameLineKind[18] = { 300, 300, 350, 300, 300, 350,
		1000, 400, nan, nan, nan, nan,
		nan, 600, nan, nan, nan, nan };
	short pillarKind[6] = { 1, 4, 0, 0, 0, 0 };
	ijkgridParametricNotSameLineKind->setGeometryAsParametricSplittedPillarNodes(gsoap_resqml2_0_1::resqml20__PillarShape__straight, false, parameters, controlPointsNotSameLineKind, controlPointParametersNotSameLineKind, 3, pillarKind, hdfProxy,
		2, pillarOfCoordinateLine, splitCoordinateLineColumnCumulativeCount, splitCoordinateLineColumns);

	// FOUR SUGARS PARAMETRIC different line kind an one cubic pillar : A copy
	IjkGridParametricRepresentation* ijkgridParametricNotSameLineKindCopy = pck->createIjkGridParametricRepresentation("46efd88c-87e1-4e00-bbdd-4c7bcc941749", "Copy of Four faulted sugar cubes with one cubic pillar", 2, 1, 2);
	const std::string hdfDatasetPrefix = "/RESQML/" + ijkgridParametricNotSameLineKind->getUuid();
	ijkgridParametricNotSameLineKindCopy->setGeometryAsParametricSplittedPillarNodesUsingExistingDatasets(gsoap_resqml2_0_1::resqml20__PillarShape__straight, gsoap_resqml2_0_1::resqml20__KDirection__down, false,
		hdfDatasetPrefix + "/PointParameters", hdfDatasetPrefix + "/ControlPoints", hdfDatasetPrefix + "/controlPointParameters", 3, hdfDatasetPrefix + "/LineKindIndices", hdfDatasetPrefix + "/PillarGeometryIsDefined", hdfProxy,
		2, hdfDatasetPrefix + "/PillarIndices",
		hdfDatasetPrefix + "/ColumnsPerSplitCoordinateLine/" + CUMULATIVE_LENGTH_DS_NAME, hdfDatasetPrefix + "/ColumnsPerSplitCoordinateLine/" + ELEMENTS_DS_NAME);

	// 4*3*2 explicit grid Left Handed
	IjkGridExplicitRepresentation* ijkgrid432 = pck->createIjkGridExplicitRepresentation("e96c2bde-e3ae-4d51-b078-a8e57fb1e667", "Four by Three by Two Left Handed", 4, 3, 2);
	double nodes432[216] = {
		0, 0, 300, 150, 0, 300, 375, 0, 300, 550, 0, 350, 700, 0, 350, //IJ0K0
		0, 50, 300, 150, 50, 300, 375, 50, 300, 550, 50, 350, 700, 50, 350, //IJ1K0
		0, 100, 300, 150, 100, 300, 375, 100, 300, 550, 100, 350, 700, 100, 350, //IJ2K0
		0, 150, 300, 150, 150, 300, 375, 150, 300, 550, 150, 350, 700, 150, 350, //IJ3K0
		375, 0, 350, 375, 50, 350, 375, 100, 350, 375, 150, 350, // SPLIT K0
		0, 0, 400, 150, 0, 400, 375, 0, 400, 550, 0, 450, 700, 0, 450, //IJ0K1
		0, 50, 400, 150, 50, 400, 375, 50, 400, 550, 50, 450, 700, 50, 450, //IJ1K1
		0, 100, 400, 150, 100, 400, 375, 100, 400, 550, 100, 450, 700, 100, 450, //IJ2K1
		0, 150, 400, 150, 150, 400, 375, 150, 400, 550, 150, 450, 700, 150, 450, //IJ3K1
		375, 0, 450, 375, 50, 450, 375, 100, 450, 375, 150, 450, // SPLIT K1
		0, 0, 500, 150, 0, 500, 375, 0, 500, 550, 0, 550, 700, 0, 550, //IJ0K2
		0, 50, 500, 150, 50, 500, 375, 50, 500, 550, 50, 550, 700, 50, 550, //IJ1K2
		0, 100, 500, 150, 100, 500, 375, 100, 500, 550, 100, 550, 700, 100, 550, //IJ2K2
		0, 150, 500, 150, 150, 500, 375, 150, 500, 550, 150, 550, 700, 150, 550, //IJ3K2
		375, 0, 550, 375, 50, 550, 375, 100, 550, 375, 150, 550 // SPLIT K2
	};
	unsigned int pillarOfCoordinateLine432[4] = { 2, 7, 12, 17 };
	unsigned int splitCoordinateLineColumnCumulativeCount432[4] = { 1, 3, 5, 6 };
	unsigned int splitCoordinateLineColumns432[6] = { 2, 2, 6, 6, 10, 10 };
	ijkgrid432->setGeometryAsCoordinateLineNodes(gsoap_resqml2_0_1::resqml20__PillarShape__vertical, gsoap_resqml2_0_1::resqml20__KDirection__down, false, nodes432, hdfProxy,
		4, pillarOfCoordinateLine432, splitCoordinateLineColumnCumulativeCount432, splitCoordinateLineColumns432);


	// 4*3*2 explicit grid Right Handed
	IjkGridExplicitRepresentation* ijkgrid432rh = pck->createIjkGridExplicitRepresentation("4fc004e1-0f7d-46a8-935e-588f790a6f84", "Four by Three by Two Right Handed", 4, 3, 2);
	double nodes432rh[216] = {
		0, 150, 300, 150, 150, 300, 375, 150, 300, 550, 150, 350, 700, 150, 350, //IJ0K0
		0, 100, 300, 150, 100, 300, 375, 100, 300, 550, 100, 350, 700, 100, 350, //IJ1K0
		0, 50, 300, 150, 50, 300, 375, 50, 300, 550, 50, 350, 700, 50, 350, //IJ2K0
		0, 0, 300, 150, 0, 300, 375, 0, 300, 550, 0, 350, 700, 0, 350, //IJ3K0
		375, 0, 350, 375, 50, 350, 375, 100, 350, 375, 150, 350, // SPLIT K0
		0, 150, 400, 150, 150, 400, 375, 150, 400, 550, 150, 450, 700, 150, 450, //IJ0K1
		0, 100, 400, 150, 100, 400, 375, 100, 400, 550, 100, 450, 700, 100, 450, //IJ1K1
		0, 50, 400, 150, 50, 400, 375, 50, 400, 550, 50, 450, 700, 50, 450, //IJ2K1
		0, 0, 400, 150, 0, 400, 375, 0, 400, 550, 0, 450, 700, 0, 450, //IJ3K1
		375, 0, 450, 375, 50, 450, 375, 100, 450, 375, 150, 450, // SPLIT K1
		0, 150, 500, 150, 150, 500, 375, 150, 500, 550, 150, 550, 700, 150, 550, //IJ0K2
		0, 100, 500, 150, 100, 500, 375, 100, 500, 550, 100, 550, 700, 100, 550, //IJ1K2
		0, 50, 500, 150, 50, 500, 375, 50, 500, 550, 50, 550, 700, 50, 550, //IJ2K2
		0, 0, 500, 150, 0, 500, 375, 0, 500, 550, 0, 550, 700, 0, 550, //IJ3K2
		375, 0, 550, 375, 50, 550, 375, 100, 550, 375, 150, 550 // SPLIT K2
	};
	unsigned int pillarOfCoordinateLine432rh[4] = { 17, 12, 7, 2 };
	unsigned int splitCoordinateLineColumnCumulativeCount432rh[4] = { 1, 3, 5, 6 };
	unsigned int splitCoordinateLineColumns432rh[6] = { 10, 10, 6, 6, 2, 2 };
	ijkgrid432rh->setGeometryAsCoordinateLineNodes(gsoap_resqml2_0_1::resqml20__PillarShape__vertical, gsoap_resqml2_0_1::resqml20__KDirection__down, true, nodes432rh, hdfProxy,
		4, pillarOfCoordinateLine432rh, splitCoordinateLineColumnCumulativeCount432rh, splitCoordinateLineColumns432rh);
	unsigned char enabledCells32rh[24] = {
		0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0
	};
	ijkgrid432rh->setEnabledCells(enabledCells32rh);

	//**************
	// Subrepresentations
	//**************
	if (fault1Interp1 != nullptr)
	{
		RESQML2_NS::SubRepresentation * faultSubRep = pck->createSubRepresentation(fault1Interp1, "ff248280-fa3d-11e5-a35c-0002a5d5c51b", "Fault Subrep In Grid");
		faultSubRep->pushBackSupportingRepresentation(ijkgrid);
		ULONG64 faultPillar[2] = { 1, 4 };
		faultSubRep->pushBackSubRepresentationPatch(gsoap_resqml2_0_1::resqml20__IndexableElements__pillars, 2, faultPillar, hdfProxy);
	}

	RESQML2_NS::SubRepresentation * actnum = pck->createSubRepresentation("323001d0-468c-41d7-abec-7d12c3c9428b", "ACTNUM");
	actnum->pushBackSupportingRepresentation(ijkgrid432);
	ULONG64 actnumValues[21] = {
		1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22
	};
	actnum->pushBackSubRepresentationPatch(gsoap_resqml2_0_1::resqml20__IndexableElements__cells, 21, actnumValues, hdfProxy);

	// Double grid subrep
	RESQML2_NS::SubRepresentation * doubleGridSubrep = pck->createSubRepresentation("f6d23b9c-e45d-4638-9601-ae3b682129a0", "TEST MULTI GRIDS SUBREP");
	doubleGridSubrep->pushBackSupportingRepresentation(ijkgrid432);
	doubleGridSubrep->pushBackSupportingRepresentation(ijkgrid);
	ULONG64 doubleGridSubrepValues[23] = {
		1, 4,
		1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
		12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22
	};
	short doubleGridSubrepSupportingRepIndices[23] = {
		0, 0,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
	};
	doubleGridSubrep->pushBackSubRepresentationPatch(gsoap_resqml2_0_1::resqml20__IndexableElements__cells, 23, doubleGridSubrepValues, hdfProxy, doubleGridSubrepSupportingRepIndices);

	//**************
	// Grid Connection
	//**************
	RESQML2_NS::GridConnectionSetRepresentation * gridConnSet = pck->createGridConnectionSetRepresentation("03bb6fc0-fa3e-11e5-8c09-0002a5d5c51b", "GridConnectionSetRepresentation");
	gridConnSet->pushBackSupportingGridRepresentation(ijkgrid);
	ULONG64 cellConn[6] = { 0, 9999, 0, 1, 9999, 1 };
	gridConnSet->setCellIndexPairs(3, cellConn, 9999, hdfProxy);
	int localFacePerCellIndexPairs[6] = { 3, 9999, 3, 5, 9999, 5 };
	gridConnSet->setLocalFacePerCellIndexPairs(3, localFacePerCellIndexPairs, 9999, hdfProxy);

	if (fault1Interp1 != nullptr)
	{
		// link to fault
		gridConnSet->pushBackInterpretation(fault1Interp1);
		unsigned int faultIndices = 0;
		gridConnSet->setConnectionInterpretationIndices(&faultIndices, 1, 9999, hdfProxy);
	}

	RESQML2_NS::GridConnectionSetRepresentation * gridConnSet432 = pck->createGridConnectionSetRepresentation("20b480a8-5e3b-4336-8f6e-1b3099c2c60f", "GridConnectionSetRepresentation");
	gridConnSet432->pushBackSupportingGridRepresentation(ijkgrid432);
	ULONG64 cellConn432[30] = {
		1, 9999, 5, 9999, 9, 9999,
		1, 2, 5, 6, 9, 10,
		13, 2, 17, 6, 21, 10,
		13, 14, 17, 18, 21, 22,
		9999, 14, 9999, 18, 9999, 22
	};
	gridConnSet432->setCellIndexPairs(15, cellConn432, 9999, hdfProxy);
	int localFacePerCellIndexPairs432[30] = {
		3, 9999, 3, 9999, 3, 9999,
		3, 5, 3, 5, 3, 5,
		3, 5, 3, 5, 3, 5,
		3, 5, 3, 5, 3, 5,
		9999, 5, 9999, 5, 9999, 5
	};
	gridConnSet432->setLocalFacePerCellIndexPairs(15, localFacePerCellIndexPairs432, 9999, hdfProxy);

	RESQML2_NS::GridConnectionSetRepresentation * gridConnSet432rh = pck->createGridConnectionSetRepresentation("a3d1462a-04e3-4374-921b-a4a1e9ba3ea3", "GridConnectionSetRepresentation");
	gridConnSet432rh->pushBackSupportingGridRepresentation(ijkgrid432rh);
	gridConnSet432rh->setCellIndexPairs(15, cellConn432, 9999, hdfProxy);
	gridConnSet432rh->setLocalFacePerCellIndexPairs(15, localFacePerCellIndexPairs432, 9999, hdfProxy);

	//**************
	// Properties
	//**************
	propType1 = pck->createPropertyKind("0a5f4400-fa3e-11e5-80a4-0002a5d5c51b", "cellIndex", "urn:resqml:f2i-consulting.com", gsoap_resqml2_0_1::resqml20__ResqmlUom__Euc, gsoap_resqml2_0_1::resqml20__ResqmlPropertyKind__discrete);
	discreteProp1 = pck->createDiscreteProperty(ijkgrid, "ee0857fe-23ad-4dd9-8300-21fa2e9fb572", "Two faulted sugar cubes cellIndex", 1,
		gsoap_resqml2_0_1::resqml20__IndexableElements__cells, propType1);
	unsigned short prop1Values[2] = { 0, 1 };
	discreteProp1->pushBackUShortHdf5Array3dOfValues(prop1Values, 2, 1, 1, hdfProxy, 1111);
	DiscreteProperty* discreteProp2 = pck->createDiscreteProperty(ijkgrid, "da73937c-2c60-4e10-8917-5154fde4ded5", "Two faulted sugar cubes other cellIndex", 1,
		gsoap_resqml2_0_1::resqml20__IndexableElements__cells, propType1);
	LONG64 prop2Values[2] = { 10, 11 };
	discreteProp2->pushBackLongHdf5Array3dOfValues(prop2Values, 2, 1, 1, hdfProxy, 1111);

	RESQML2_NS::PropertySet* propSet = pck->createPropertySet("", "Testing property set", false, true,gsoap_resqml2_0_1::resqml20__TimeSetKind__not_x0020a_x0020time_x0020set);
	propSet->pushBackProperty(discreteProp1);
	propSet->pushBackProperty(discreteProp2);

	DiscreteProperty* discreteProp1OnIjkgridParametric = pck->createDiscreteProperty(ijkgridParametric, "eb3dbf6c-5745-4e41-9d09-672f6fbab414", "Four sugar cubes cellIndex", 1,
		gsoap_resqml2_0_1::resqml20__IndexableElements__cells, propType1);
	unsigned short prop1ValuesOnIjkgridParametric[4] = { 0, 1, 2, 3 };
	discreteProp1OnIjkgridParametric->pushBackUShortHdf5Array3dOfValues(prop1ValuesOnIjkgridParametric, 2, 1, 2, hdfProxy, 1111, 0, 3);
	//Move this prop to another same ninjnk ijk grid
	discreteProp1OnIjkgridParametric->setRepresentation(ijkgridParametricNotSameLineKind);

	//**************
	// Time Series
	//**************
	RESQML2_NS::TimeSeries * timeSeries = pck->createTimeSeries("1187d8a0-fa3e-11e5-ac3a-0002a5d5c51b", "Testing time series");
	tm timeStruct;
	timeStruct.tm_hour = 15;
	timeStruct.tm_min = 2;
	timeStruct.tm_sec = 35;
	timeStruct.tm_mday = 8;
	timeStruct.tm_mon = 1;
	timeStruct.tm_year = 0;
	timeStruct.tm_isdst = -1;
	mktime(&timeStruct); 
	timeSeries->pushBackTimestamp(timeStruct);
	timeSeries->pushBackTimestamp(1409753895);
	timeSeries->pushBackTimestamp(1441289895);
	ContinuousProperty* continuousPropTime0 = pck->createContinuousProperty(ijkgrid, "18027a00-fa3e-11e5-8255-0002a5d5c51b", "Time Series Property", 1,
		gsoap_resqml2_0_1::resqml20__IndexableElements__cells, gsoap_resqml2_0_1::resqml20__ResqmlUom__m, gsoap_resqml2_0_1::resqml20__ResqmlPropertyKind__length);
	continuousPropTime0->setTimeIndex(0, timeSeries);
	double valuesTime0[2] = { 0, 1 };
	continuousPropTime0->pushBackDoubleHdf5Array3dOfValues(valuesTime0, 2, 1, 1, hdfProxy);
	ContinuousProperty* continuousPropTime1 = pck->createContinuousProperty(ijkgrid, "1ba54340-fa3e-11e5-9534-0002a5d5c51b", "Time Series Property", 1,
		gsoap_resqml2_0_1::resqml20__IndexableElements__cells, gsoap_resqml2_0_1::resqml20__ResqmlUom__m, gsoap_resqml2_0_1::resqml20__ResqmlPropertyKind__length);
	continuousPropTime1->setTimeIndex(1, timeSeries);
	double valuesTime1[2] = { 2, 3 };
	continuousPropTime1->pushBackDoubleHdf5Array3dOfValues(valuesTime1, 2, 1, 1, hdfProxy);
	ContinuousProperty* continuousPropTime2 = pck->createContinuousProperty(ijkgrid, "203db720-fa3e-11e5-bf9d-0002a5d5c51b", "Time Series Property ", 1,
		gsoap_resqml2_0_1::resqml20__IndexableElements__cells, gsoap_resqml2_0_1::resqml20__ResqmlUom__m, gsoap_resqml2_0_1::resqml20__ResqmlPropertyKind__length);
	continuousPropTime2->setTimeIndex(2, timeSeries);
	double valuesTime2[2] = { 3, 4 };
	continuousPropTime2->pushBackDoubleHdf5Array3dOfValues(valuesTime2, 2, 1, 1, hdfProxy);

	ContinuousPropertySeries* continuousPropertySeries = pck->createContinuousPropertySeries(
		ijkgrid,
		"2648f4e0-fa3e-11e5-ab1c-0002a5d5c51b", "Testing continuous property series",
		1,
		gsoap_resqml2_0_1::resqml20__IndexableElements__cells,
		gsoap_resqml2_0_1::resqml20__ResqmlUom__m,
		gsoap_resqml2_0_1::resqml20__ResqmlPropertyKind__length,
		timeSeries);
	double valuesTime[6] = { 0, 1, 2, 3, 3, 4 };
	continuousPropertySeries->pushBackDoubleHdf5Array1dOfValues(valuesTime, 6, hdfProxy);

	//**************
	// LGR
	//**************

	IjkGridExplicitRepresentation* lgrGrid = pck->createIjkGridExplicitRepresentation("2aec1720-fa3e-11e5-a116-0002a5d5c51b", "LGR", 2, 1, 3);
	lgrGrid->setParentWindow(
		0, 2, 1,
		0, 1, 1,
		0, 3, 1,
		ijkgrid);

	//**************
	// Stratigraphy
	//**************
	ULONG64 stratiUnitIndice = 0;
	ijkgrid->setIntervalAssociationWithStratigraphicOrganizationInterpretation(&stratiUnitIndice, 1000, stratiColumnRank0);

	// Partial transfer
	UnstructuredGridRepresentation* partialGrid = pck->createPartialUnstructuredGridRepresentation("", "Partial Grid");
	ContinuousProperty* continuousProp1 = pck->createContinuousProperty(partialGrid, "cd627946-0f89-48fa-b99c-bdb35d8ac4aa", "Testing partial property", 1,
		gsoap_resqml2_0_1::resqml20__IndexableElements__cells, gsoap_resqml2_0_1::resqml20__ResqmlUom__m, gsoap_resqml2_0_1::resqml20__ResqmlPropertyKind__length);
	double continuousProp1Values[6] = { 0, 1, 2, 3, 4, 5 };
	continuousProp1->pushBackDoubleHdf5Array1dOfValues(continuousProp1Values, 6, hdfProxy);

	// sub rep of a partial unstructured grid
	RESQML2_NS::SubRepresentation * subRepOfUnstructuredGrid = pck->createSubRepresentation("", "Subrep On Partial grid");
	subRepOfUnstructuredGrid->pushBackSupportingRepresentation(partialGrid);
	ULONG64 nodeIndex[2] = { 0, 1 };
	subRepOfUnstructuredGrid->pushBackSubRepresentationPatch(gsoap_resqml2_0_1::resqml20__IndexableElements__nodes, 2, nodeIndex, hdfProxy);

	// Tetra grid
	UnstructuredGridRepresentation* tetraGrid = pck->createUnstructuredGridRepresentation("9283cd33-5e52-4110-b7b1-616abde2b303", "One tetrahedron grid", 1);
	double tetraGridPoints[12] = { 0, 0, 300, 375, 0, 300, 0, 150, 300, 0, 0, 500 };
	ULONG64 faceIndicesPerCell[4] = { 0, 1, 2, 3 };
	unsigned char faceRightHandness[4] = { 0, 0, 1, 1 };
	ULONG64 nodeIndicesPerFace[12] = { 0, 1, 2, 1, 2, 3, 0, 1, 3, 0, 2, 3 };
	tetraGrid->setTetrahedraOnlyGeometry(faceRightHandness, tetraGridPoints, 4, 4, hdfProxy, faceIndicesPerCell, nodeIndicesPerFace);
}

void serializeRepresentationSetRepresentation(COMMON_NS::DataObjectRepository * pck, COMMON_NS::AbstractHdfProxy*)
{
	RESQML2_NS::RepresentationSetRepresentation* result = pck->createRepresentationSetRepresentation("", "Testing Representation set");
	cout << "is homogeneous : " << result->isHomogeneous() << endl;
	f1i1triRepSinglePatch->pushBackIntoRepresentationSet(result);
	cout << "is homogeneous : " << result->isHomogeneous() << endl;
	f1i1triRep->pushBackIntoRepresentationSet(result);
	cout << "is homogeneous : " << result->isHomogeneous() << endl;
	h1i1triRep->pushBackIntoRepresentationSet(result);
	cout << "is homogeneous : " << result->isHomogeneous() << endl;
	h2i1triRep->pushBackIntoRepresentationSet(result);
	cout << "is homogeneous : " << result->isHomogeneous() << endl;
	w1i1TrajRep->pushBackIntoRepresentationSet(result);
	cout << "is homogeneous : " << result->isHomogeneous() << endl;
}

void serializeStructuralModel(COMMON_NS::DataObjectRepository & pck, COMMON_NS::AbstractHdfProxy* hdfProxy)
{
	// =========================================================================
	// =========================================================================
	// Organization features
	OrganizationFeature * structOrg1 = pck.createStructuralModel("446068f6-33d8-40f4-9bec-2640fb7df23e", "StructuralOrg1");
	structOrg1->setOriginator("Geosiris");
	structOrg1->setEditor("F2I");

	// =========================================================================
	// =========================================================================
	// Organization interpretations
	StructuralOrganizationInterpretation * structuralOrganizationInterpretation = pck.createStructuralOrganizationInterpretationInApparentDepth(structOrg1, "456f31e2-f79a-400c-91a9-ffd1fc939d32", "StructuralOrg1 Interp1");
	structuralOrganizationInterpretation->setOriginator("Geosiris");
	structuralOrganizationInterpretation->setEditor("F2I");
	structuralOrganizationInterpretation->pushBackFaultInterpretation(fault1Interp1);
	structuralOrganizationInterpretation->pushBackHorizonInterpretation(horizon1Interp1, 0);
	structuralOrganizationInterpretation->pushBackHorizonInterpretation(horizon2Interp1, 0);

	// =========================================================================
	// =========================================================================
	// EarthModel
	OrganizationFeature * earthModelOrg = pck.createEarthModel("cb8848cd-8df6-4373-8cdd-fdf5f944b6ab", "EarthModelOrg");
	earthModelOrg->setOriginator("Geosiris");
	earthModelOrg->setEditor("F2I");
	EarthModelInterpretation * earthModel = pck.createEarthModelInterpretation(earthModelOrg, "03bb3a50-c206-4aee-8343-dbea84011850", "EarthModel");
	earthModel->setOriginator("Geosiris");
	earthModel->setEditor("F2I");
	earthModel->setStructuralOrganizationInterpretation(structuralOrganizationInterpretation);

	// =========================================================================
	// Add frontiers
	FrontierFeature* xPlusFrontier = pck.createFrontier("e062ee65-8296-44c0-adf9-14cef0c2eb27", "X Plus Frontier");
	GenericFeatureInterpretation* xPlusFrontierInterp = pck.createGenericFeatureInterpretation(xPlusFrontier, "e888c38b-86d5-41ae-9aaa-a7a41d0f4554", "X Plus Frontier Interp");
	FrontierFeature* xMinusFrontier = pck.createFrontier("0f979db7-1189-47f4-827f-f61f9d163655", "X Minus Frontier");
	GenericFeatureInterpretation* xMinusFrontierInterp = pck.createGenericFeatureInterpretation(xMinusFrontier, "cdbf2692-ab91-4529-8878-c10348a263a6", "X Minus Frontier Interp");
	FrontierFeature* yPlusFrontier = pck.createFrontier("f50de712-10cd-40e1-ae71-fe36a5a78453", "Y Plus Frontier");
	GenericFeatureInterpretation* yPlusFrontierInterp = pck.createGenericFeatureInterpretation(yPlusFrontier, "ef1b0d82-52f5-4394-9b1a-ea92fffa8548", "Y Plus Frontier Interp");
	FrontierFeature* yMinusFrontier = pck.createFrontier("d4820ef8-2698-407f-84c7-c36396e19d08", "Y Minus Frontier");
	GenericFeatureInterpretation* yMinusFrontierInterp = pck.createGenericFeatureInterpretation(yMinusFrontier, "74977015-731a-4e66-8fba-7c42cc44faa0", "Y Minus Frontier Interp");

	// =========================================================================
	// Binary contact interpretation

	// Contact 0: fault1Interp1 HANGING_WALL_SIDE SPLITS horizon1Interp1 BOTH_SIDES
	structuralOrganizationInterpretation->pushBackBinaryContact(gsoap_resqml2_0_1::resqml20__ContactRelationship__tectonic_x0020boundary_x0020to_x0020genetic_x0020boundary, fault1Interp1, gsoap_resqml2_0_1::resqml20__ContactSide__hanging_x0020wall,
		gsoap_resqml2_0_1::resqml20__ContactVerb__splits,
		horizon1Interp1, gsoap_resqml2_0_1::resqml20__ContactSide__both);
	// Contact 1: fault1Interp1 FOOT_WALL_SIDE SPLITS horizon1Interp1 BOTH_SIDES
	structuralOrganizationInterpretation->pushBackBinaryContact(gsoap_resqml2_0_1::resqml20__ContactRelationship__tectonic_x0020boundary_x0020to_x0020genetic_x0020boundary, fault1Interp1, gsoap_resqml2_0_1::resqml20__ContactSide__footwall,
		gsoap_resqml2_0_1::resqml20__ContactVerb__splits,
		horizon1Interp1, gsoap_resqml2_0_1::resqml20__ContactSide__both);
	// Contact 2: fault1Interp1 HANGING_WALL_SIDE SPLITS horizon2Interp1 BOTH_SIDES
	structuralOrganizationInterpretation->pushBackBinaryContact(gsoap_resqml2_0_1::resqml20__ContactRelationship__tectonic_x0020boundary_x0020to_x0020genetic_x0020boundary, fault1Interp1, gsoap_resqml2_0_1::resqml20__ContactSide__hanging_x0020wall,
		gsoap_resqml2_0_1::resqml20__ContactVerb__splits,
		horizon2Interp1, gsoap_resqml2_0_1::resqml20__ContactSide__both);
	// Contact 3: fault1Interp1 FOOT_WALL_SIDE SPLITS horizon2Interp1 BOTH_SIDES
	structuralOrganizationInterpretation->pushBackBinaryContact(gsoap_resqml2_0_1::resqml20__ContactRelationship__tectonic_x0020boundary_x0020to_x0020genetic_x0020boundary, fault1Interp1, gsoap_resqml2_0_1::resqml20__ContactSide__footwall,
		gsoap_resqml2_0_1::resqml20__ContactVerb__splits,
		horizon2Interp1, gsoap_resqml2_0_1::resqml20__ContactSide__both);

	// Contact 4: horizon1Interp1 STOPS AT yMinusFrontierInterp
	structuralOrganizationInterpretation->pushBackBinaryContact(gsoap_resqml2_0_1::resqml20__ContactRelationship__genetic_x0020boundary_x0020to_x0020frontier_x0020feature, horizon1Interp1,
		gsoap_resqml2_0_1::resqml20__ContactVerb__stops_x0020at,
		yMinusFrontierInterp);
	// Contact 5: fault1Interp1 STOPS AT yMinusFrontierInterp (part above horizon1Interp1)
	structuralOrganizationInterpretation->pushBackBinaryContact(gsoap_resqml2_0_1::resqml20__ContactRelationship__tectonic_x0020boundary_x0020to_x0020frontier_x0020feature, fault1Interp1,
		gsoap_resqml2_0_1::resqml20__ContactVerb__stops_x0020at,
		yMinusFrontierInterp);
	// Contact 6: horizon2Interp1 STOPS AT yMinusFrontierInterp
	structuralOrganizationInterpretation->pushBackBinaryContact(gsoap_resqml2_0_1::resqml20__ContactRelationship__genetic_x0020boundary_x0020to_x0020frontier_x0020feature, horizon2Interp1,
		gsoap_resqml2_0_1::resqml20__ContactVerb__stops_x0020at,
		yMinusFrontierInterp);

	// Contact 7: horizon1Interp1 STOPS AT yPlusFrontierInterp
	structuralOrganizationInterpretation->pushBackBinaryContact(gsoap_resqml2_0_1::resqml20__ContactRelationship__genetic_x0020boundary_x0020to_x0020frontier_x0020feature, horizon1Interp1,
		gsoap_resqml2_0_1::resqml20__ContactVerb__stops_x0020at,
		yPlusFrontierInterp);
	// Contact 8: fault1Interp1 STOPS AT yPlusFrontierInterp (part above horizon1Interp1)
	structuralOrganizationInterpretation->pushBackBinaryContact(gsoap_resqml2_0_1::resqml20__ContactRelationship__tectonic_x0020boundary_x0020to_x0020frontier_x0020feature, fault1Interp1,
		gsoap_resqml2_0_1::resqml20__ContactVerb__stops_x0020at,
		yPlusFrontierInterp);
	// Contact 9: horizon2Interp1 STOPS AT yPlusFrontierInterp
	structuralOrganizationInterpretation->pushBackBinaryContact(gsoap_resqml2_0_1::resqml20__ContactRelationship__genetic_x0020boundary_x0020to_x0020frontier_x0020feature, horizon2Interp1,
		gsoap_resqml2_0_1::resqml20__ContactVerb__stops_x0020at,
		yPlusFrontierInterp);

	// Contact 10: horizon1Interp1 STOPS AT xMinusFrontierInterp
	structuralOrganizationInterpretation->pushBackBinaryContact(gsoap_resqml2_0_1::resqml20__ContactRelationship__genetic_x0020boundary_x0020to_x0020frontier_x0020feature, horizon1Interp1,
		gsoap_resqml2_0_1::resqml20__ContactVerb__stops_x0020at,
		xMinusFrontierInterp);
	// Contact 11: horizon2Interp1 STOPS AT xMinusFrontierInterp
	structuralOrganizationInterpretation->pushBackBinaryContact(gsoap_resqml2_0_1::resqml20__ContactRelationship__genetic_x0020boundary_x0020to_x0020frontier_x0020feature, horizon2Interp1,
		gsoap_resqml2_0_1::resqml20__ContactVerb__stops_x0020at,
		xMinusFrontierInterp);
	// Contact 12: xMinusFrontierInterp STOPS AT yMinusFrontierInterp
	structuralOrganizationInterpretation->pushBackBinaryContact(gsoap_resqml2_0_1::resqml20__ContactRelationship__frontier_x0020feature_x0020to_x0020frontier_x0020feature, xMinusFrontierInterp,
		gsoap_resqml2_0_1::resqml20__ContactVerb__stops_x0020at,
		yMinusFrontierInterp);
	// Contact 13: xMinusFrontierInterp STOPS AT yPlusFrontierInterp
	structuralOrganizationInterpretation->pushBackBinaryContact(gsoap_resqml2_0_1::resqml20__ContactRelationship__frontier_x0020feature_x0020to_x0020frontier_x0020feature, xMinusFrontierInterp,
		gsoap_resqml2_0_1::resqml20__ContactVerb__stops_x0020at,
		yPlusFrontierInterp);

	// Contact 14: fault1Interp1 STOPS AT yMinusFrontierInterp (part below horizon1Interp1)
	structuralOrganizationInterpretation->pushBackBinaryContact(gsoap_resqml2_0_1::resqml20__ContactRelationship__tectonic_x0020boundary_x0020to_x0020frontier_x0020feature, fault1Interp1,
		gsoap_resqml2_0_1::resqml20__ContactVerb__stops_x0020at,
		yMinusFrontierInterp);
	// Contact 15: fault1Interp1 STOPS AT yPlusFrontierInterp (part above horizon1Interp1)
	structuralOrganizationInterpretation->pushBackBinaryContact(gsoap_resqml2_0_1::resqml20__ContactRelationship__tectonic_x0020boundary_x0020to_x0020frontier_x0020feature, fault1Interp1,
		gsoap_resqml2_0_1::resqml20__ContactVerb__stops_x0020at,
		yPlusFrontierInterp);

	// Contact 16: horizon1Interp1 STOPS AT yMinusFrontierInterp
	structuralOrganizationInterpretation->pushBackBinaryContact(gsoap_resqml2_0_1::resqml20__ContactRelationship__genetic_x0020boundary_x0020to_x0020frontier_x0020feature, horizon1Interp1,
		gsoap_resqml2_0_1::resqml20__ContactVerb__stops_x0020at,
		yMinusFrontierInterp);
	// Contact 17: fault1Interp1 STOPS AT yMinusFrontierInterp (part below horizon2Interp1)
	structuralOrganizationInterpretation->pushBackBinaryContact(gsoap_resqml2_0_1::resqml20__ContactRelationship__tectonic_x0020boundary_x0020to_x0020frontier_x0020feature, fault1Interp1,
		gsoap_resqml2_0_1::resqml20__ContactVerb__stops_x0020at,
		yMinusFrontierInterp);
	// Contact 18: horizon2Interp1 STOPS AT yMinusFrontierInterp
	structuralOrganizationInterpretation->pushBackBinaryContact(gsoap_resqml2_0_1::resqml20__ContactRelationship__genetic_x0020boundary_x0020to_x0020frontier_x0020feature, horizon2Interp1,
		gsoap_resqml2_0_1::resqml20__ContactVerb__stops_x0020at,
		yMinusFrontierInterp);

	// Contact 19: horizon1Interp1 STOPS AT yPlusFrontierInterp
	structuralOrganizationInterpretation->pushBackBinaryContact(gsoap_resqml2_0_1::resqml20__ContactRelationship__genetic_x0020boundary_x0020to_x0020frontier_x0020feature, horizon1Interp1,
		gsoap_resqml2_0_1::resqml20__ContactVerb__stops_x0020at,
		yPlusFrontierInterp);
	// Contact 20: fault1Interp1 STOPS AT yPlusFrontierInterp (part below horizon2Interp1)
	structuralOrganizationInterpretation->pushBackBinaryContact(gsoap_resqml2_0_1::resqml20__ContactRelationship__tectonic_x0020boundary_x0020to_x0020frontier_x0020feature, fault1Interp1,
		gsoap_resqml2_0_1::resqml20__ContactVerb__stops_x0020at,
		yPlusFrontierInterp);
	// Contact 21: horizon2Interp1 STOPS AT yPlusFrontierInterp
	structuralOrganizationInterpretation->pushBackBinaryContact(gsoap_resqml2_0_1::resqml20__ContactRelationship__genetic_x0020boundary_x0020to_x0020frontier_x0020feature, horizon2Interp1,
		gsoap_resqml2_0_1::resqml20__ContactVerb__stops_x0020at,
		yPlusFrontierInterp);

	// Contact 22: horizon1Interp1 STOPS AT xPlusFrontierInterp
	structuralOrganizationInterpretation->pushBackBinaryContact(gsoap_resqml2_0_1::resqml20__ContactRelationship__genetic_x0020boundary_x0020to_x0020frontier_x0020feature, horizon1Interp1,
		gsoap_resqml2_0_1::resqml20__ContactVerb__stops_x0020at,
		xPlusFrontierInterp);
	// Contact 23: horizon2Interp1 STOPS AT xPlusFrontierInterp
	structuralOrganizationInterpretation->pushBackBinaryContact(gsoap_resqml2_0_1::resqml20__ContactRelationship__genetic_x0020boundary_x0020to_x0020frontier_x0020feature, horizon2Interp1,
		gsoap_resqml2_0_1::resqml20__ContactVerb__stops_x0020at,
		xPlusFrontierInterp);
	// Contact 24: xPlusFrontierInterp STOPS AT yMinusFrontierInterp
	structuralOrganizationInterpretation->pushBackBinaryContact(gsoap_resqml2_0_1::resqml20__ContactRelationship__frontier_x0020feature_x0020to_x0020frontier_x0020feature, xPlusFrontierInterp,
		gsoap_resqml2_0_1::resqml20__ContactVerb__stops_x0020at,
		yMinusFrontierInterp);
	// Contact 25: xPlusFrontierInterp STOPS AT yPlusFrontierInterp
	structuralOrganizationInterpretation->pushBackBinaryContact(gsoap_resqml2_0_1::resqml20__ContactRelationship__frontier_x0020feature_x0020to_x0020frontier_x0020feature, xPlusFrontierInterp,
		gsoap_resqml2_0_1::resqml20__ContactVerb__stops_x0020at,
		yPlusFrontierInterp);

	// =========================================================================
	// =========================================================================
	// SurfaceFramework
	//    SurfaceFramework* structuralOrganizationInterpretationSurfaceFramework = pck.createSurfaceFramework(structuralOrganizationInterpretation, "", "StructuralOrg1 Interp1 Interp1 SurfFrmwk", true);
	//    structuralOrganizationInterpretationSurfaceFramework->pushBackRepresentation(f1i1triRep);
	//    structuralOrganizationInterpretationSurfaceFramework->pushBackRepresentation(h1i1triRep);
	//    structuralOrganizationInterpretationSurfaceFramework->pushBackRepresentation(h2i1triRep);

	// Single Patch Fault 1
	SealedSurfaceFrameworkRepresentation* singlePatchFault1SealedSurfaceFramework = pck.createSealedSurfaceFrameworkRepresentation(structuralOrganizationInterpretation, "df673451-d6f2-4e4d-ad86-eaaf131c458f", "Single Patch Fault1 StructuralOrg1 Interp1 Interp1 SealedSurfFrmwk");
	singlePatchFault1SealedSurfaceFramework->setOriginator("Geosiris");
	singlePatchFault1SealedSurfaceFramework->setEditor("F2I");
	f1i1triRepSinglePatch->pushBackIntoRepresentationSet(singlePatchFault1SealedSurfaceFramework);
	h1i1triRep->pushBackIntoRepresentationSet(singlePatchFault1SealedSurfaceFramework);
	h2i1triRep->pushBackIntoRepresentationSet(singlePatchFault1SealedSurfaceFramework);

	// Multipatch Fault 1
	sealedSurfaceFramework = pck.createSealedSurfaceFrameworkRepresentation(structuralOrganizationInterpretation, "c89011a9-0fd8-42cd-b992-96785ed01f6f", "StructuralOrg1 Interp1 Interp1 SealedSurfFrmwk");
	sealedSurfaceFramework->setOriginator("Geosiris");
	sealedSurfaceFramework->setEditor("F2I");
	f1i1triRep->pushBackIntoRepresentationSet(sealedSurfaceFramework);
	h1i1triRep->pushBackIntoRepresentationSet(sealedSurfaceFramework); // Top frontier of the corresponding sealedVolumeFramework
	h2i1triRep->pushBackIntoRepresentationSet(sealedSurfaceFramework); // Btm frontier of the corresponding sealedVolumeFramework
	xMinusFrontierRep = pck.createTriangulatedSetRepresentation(xMinusFrontierInterp, "0a327e2e-105c-4068-a325-a6c5e3de135f", "X Minus frontier rep");
	double xMinusFrontierRepNodes[18] = { 0, 0, 300, 0, 200, 300, 0, 0, 350, 0, 200, 350, 0, 0, 500, 0, 200, 500 };
	unsigned int xMinusFrontierRepTriangles[12] = { 0, 1, 2, 1, 3, 2, 2,3,4, 3,5,4 };
	xMinusFrontierRep->pushBackTrianglePatch(6, xMinusFrontierRepNodes, 4, xMinusFrontierRepTriangles, hdfProxy);
	xMinusFrontierRep->pushBackIntoRepresentationSet(sealedSurfaceFramework);
	xPlusFrontierRep = pck.createTriangulatedSetRepresentation(xPlusFrontierInterp, "6e678338-3b53-49b6-8801-faee493e0c42", "X Plus frontier rep");
	double xPlusFrontierRepRepNodes[18] = { 700, 0, 350, 700, 200, 350, 700, 0, 500, 700, 200, 500, 700, 0, 550, 700, 200, 550 };
	unsigned int xPlusFrontierRepRepTriangles[12] = { 0, 1, 2, 1, 3, 2, 2,3,4, 3,5,4 };
	xPlusFrontierRep->pushBackTrianglePatch(6, xPlusFrontierRepRepNodes, 4, xPlusFrontierRepRepTriangles, hdfProxy);
	xPlusFrontierRep->pushBackIntoRepresentationSet(sealedSurfaceFramework);
	yPlusFrontierRep = pck.createTriangulatedSetRepresentation(yPlusFrontierInterp, "e960aea8-72e2-495d-8253-60fda5620921", "Y Plus frontier rep");
	double yPlusFrontierRepNodesPatch0[18] = { 0, 200, 300, 250, 200, 300, 0, 200, 350, 300, 200, 350, 0, 200, 500, 450, 200, 500 };
	unsigned int yPlusFrontierRepTrianglesPatch0[12] = { 0,1,2, 1,3,2, 2,3,4, 3,5,4 };
	yPlusFrontierRep->pushBackTrianglePatch(6, yPlusFrontierRepNodesPatch0, 4, yPlusFrontierRepTrianglesPatch0, hdfProxy);
	double yPlusFrontierRepNodesPatch1[18] = { 300, 200, 350, 700, 200, 350, 450, 200, 500, 700, 200, 500, 500, 200, 550, 700, 200, 550 };
	unsigned int yPlusFrontierRepTrianglesPatch1[12] = { 6,7,8, 7,9,8, 8,9,10, 9,11,10 };
	yPlusFrontierRep->pushBackTrianglePatch(6, yPlusFrontierRepNodesPatch1, 4, yPlusFrontierRepTrianglesPatch1, hdfProxy);
	yPlusFrontierRep->pushBackIntoRepresentationSet(sealedSurfaceFramework);
	yMinusFrontierRep = pck.createTriangulatedSetRepresentation(yMinusFrontierInterp, "38f64a1c-356f-4d30-a9ce-4cd3b8d6ec40", "Y Minus frontier rep");
	double yMinusFrontierRepNodesPatch0[18] = { 0, 0, 300, 250, 0, 300, 0, 0, 350, 300, 0, 350, 0, 0, 500, 450, 0, 500 };
	unsigned int yMinusFrontierRepTrianglesPatch0[12] = { 0,1,2, 1,3,2, 2,3,4, 3,5,4 };
	yMinusFrontierRep->pushBackTrianglePatch(6, yMinusFrontierRepNodesPatch0, 4, yMinusFrontierRepTrianglesPatch0, hdfProxy);
	double yMinusFrontierRepNodesPatch1[18] = { 300, 0, 350, 700, 0, 350, 450, 0, 500, 700, 0, 500, 500, 0, 550, 700, 0, 550 };
	unsigned int yMinusFrontierRepTrianglesPatch1[12] = { 6,7,8, 7,9,8, 8,9,10, 9,11,10 };
	yMinusFrontierRep->pushBackTrianglePatch(6, yMinusFrontierRepNodesPatch1, 4, yMinusFrontierRepTrianglesPatch1, hdfProxy);
	yMinusFrontierRep->pushBackIntoRepresentationSet(sealedSurfaceFramework);

	// =========================================================================
	// =========================================================================
	// Binary contact representations

	// ===================================
	// Single Patch Fault 1 Representation

	// Contact 0
	// nbPatch * nbIdenticalNodes = 9
	//int contactIdenticalNodes[9] = {
	//	0, 0, 0,
	//	1, 1, 1,
	//	2, 2, 2
	//};
	//sealedSurfaceFramework->pushBackContact(gsoap_resqml2_0_1::resqml__IdentityKind__COLOCATION, 3, 3, contactIdenticalNodes, hdfProxy);
	singlePatchFault1SealedSurfaceFramework->pushBackContact(gsoap_resqml2_0_1::resqml20__IdentityKind__colocation);

	// Contact 0 Patch 0
	int indexSetContact0Patch0[3] = { 3, 4, 5 };
	singlePatchFault1SealedSurfaceFramework->pushBackContactPatch(
		0,
		indexSetContact0Patch0,
		3,
		f1i1triRepSinglePatch, hdfProxy);

	// Contact 0 Patch 1
	int indexSetContact0Patch1[3] = { 0, 1, 2 };
	singlePatchFault1SealedSurfaceFramework->pushBackContactPatch(
		0,
		indexSetContact0Patch1,
		3,
		h1i1triRep, hdfProxy);

	// Contact 0 Patch 2
	int indexSetContact0Patch2[3] = { 3, 4, 5 };
	singlePatchFault1SealedSurfaceFramework->pushBackContactPatch(
		0,
		indexSetContact0Patch2,
		3,
		f1i1triRepSinglePatch, hdfProxy);

	// Contact 1
	//sealedSurfaceFramework->pushBackContact(gsoap_resqml2_0_1::resqml__IdentityKind__COLOCATION, 3, 3, contactIdenticalNodes, hdfProxy);
	singlePatchFault1SealedSurfaceFramework->pushBackContact(gsoap_resqml2_0_1::resqml20__IdentityKind__colocation);

	// Contact 1 Patch 0
	int indexSetContact1Patch0[3] = { 6, 7, 8 };
	singlePatchFault1SealedSurfaceFramework->pushBackContactPatch(
		1,
		indexSetContact1Patch0,
		3,
		f1i1triRepSinglePatch, hdfProxy);

	// Contact 1 Patch 1
	int indexSetContact1Patch1[3] = { 5, 6, 7 };
	singlePatchFault1SealedSurfaceFramework->pushBackContactPatch(
		1,
		indexSetContact1Patch1,
		3,
		h1i1triRep, hdfProxy);

	// Contact 1 Patch 2
	int indexSetContact1Patch2[3] = { 6, 7, 8 };
	singlePatchFault1SealedSurfaceFramework->pushBackContactPatch(
		1,
		indexSetContact1Patch2,
		3,
		f1i1triRepSinglePatch, hdfProxy);

	// Contact 2
	//sealedSurfaceFramework->pushBackContact(gsoap_resqml2_0_1::resqml__IdentityKind__COLOCATION, 3, 3, contactIdenticalNodes, hdfProxy);
	singlePatchFault1SealedSurfaceFramework->pushBackContact(gsoap_resqml2_0_1::resqml20__IdentityKind__colocation);

	// Contact 2 Patch 0
	int indexSetContact2Patch0[3] = { 9, 10, 11 };
	singlePatchFault1SealedSurfaceFramework->pushBackContactPatch(
		2,
		indexSetContact2Patch0,
		3,
		f1i1triRepSinglePatch, hdfProxy);

	// Contact 2 Patch 1
	int indexSetContact2Patch1[3] = { 0, 1, 2 };
	singlePatchFault1SealedSurfaceFramework->pushBackContactPatch(
		2,
		indexSetContact2Patch1,
		3,
		h2i1triRep, hdfProxy);

	// Contact 2 Patch 2
	int indexSetContact2Patch2[3] = { 9, 10, 11 };
	singlePatchFault1SealedSurfaceFramework->pushBackContactPatch(
		2,
		indexSetContact2Patch2,
		3,
		f1i1triRepSinglePatch, hdfProxy);

	// Contact 3
	//sealedSurfaceFramework->pushBackContact(gsoap_resqml2_0_1::resqml__IdentityKind__COLOCATION, 3, 3, contactIdenticalNodes, hdfProxy);
	singlePatchFault1SealedSurfaceFramework->pushBackContact(gsoap_resqml2_0_1::resqml20__IdentityKind__colocation);

	// Contact 3 Patch 0
	int indexSetContact3Patch0[3] = { 12, 13, 14 };
	singlePatchFault1SealedSurfaceFramework->pushBackContactPatch(
		3,
		indexSetContact3Patch0,
		3,
		f1i1triRepSinglePatch, hdfProxy);

	// Contact 3 Patch 1
	int indexSetContact3Patch1[3] = { 5, 6, 7 };
	singlePatchFault1SealedSurfaceFramework->pushBackContactPatch(
		3,
		indexSetContact3Patch1,
		3,
		h2i1triRep, hdfProxy);

	// Contact 3 Patch 2
	int indexSetContact3Patch2[3] = { 12, 13, 14 };
	singlePatchFault1SealedSurfaceFramework->pushBackContactPatch(
		3,
		indexSetContact3Patch2,
		3,
		f1i1triRepSinglePatch, hdfProxy);

	// ================================
	// Multiatch Fault 1 Representation

	// Contact 0
	// nbPatch * nbIdenticalNodes = 9
	//    int contactIdenticalNodes[9] = {
	//        0,0,0,
	//        1,1,1,
	//        2,2,2
	//    };
	//sealedSurfaceFramework->pushBackContact(gsoap_resqml2_0_1::resqml__IdentityKind__COLOCATION, 3, 3, contactIdenticalNodes, hdfProxy);
	sealedSurfaceFramework->pushBackContact(gsoap_resqml2_0_1::resqml20__IdentityKind__colocation);

	// Contact 0 Patch 0
	//int * indexSetContact0Patch0 = new int[3];
	indexSetContact0Patch0[0] = 3;
	indexSetContact0Patch0[1] = 4;
	indexSetContact0Patch0[2] = 5;
	sealedSurfaceFramework->pushBackContactPatch(
		0,
		indexSetContact0Patch0,
		3,
		f1i1triRep, hdfProxy);

	// Contact 0 Patch 1
	//int * indexSetContact0Patch1 = new int[3];
	indexSetContact0Patch1[0] = 0;
	indexSetContact0Patch1[1] = 1;
	indexSetContact0Patch1[2] = 2;
	sealedSurfaceFramework->pushBackContactPatch(
		0,
		indexSetContact0Patch1,
		3,
		h1i1triRep, hdfProxy);

	// Contact 0 Patch 2
	//int * indexSetContact0Patch2 = new int[3];
	indexSetContact0Patch2[0] = 6;
	indexSetContact0Patch2[1] = 7;
	indexSetContact0Patch2[2] = 8;
	sealedSurfaceFramework->pushBackContactPatch(
		0,
		indexSetContact0Patch2,
		3,
		f1i1triRep, hdfProxy);

	// Contact 1
	//sealedSurfaceFramework->pushBackContact(gsoap_resqml2_0_1::resqml__IdentityKind__COLOCATION, 3, 3, contactIdenticalNodes, hdfProxy);
	sealedSurfaceFramework->pushBackContact(gsoap_resqml2_0_1::resqml20__IdentityKind__colocation);

	// Contact 1 Patch 0
	//int * indexSetContact1Patch0 = new int[3];
	indexSetContact1Patch0[0] = 9;
	indexSetContact1Patch0[1] = 10;
	indexSetContact1Patch0[2] = 11;
	sealedSurfaceFramework->pushBackContactPatch(
		1,
		indexSetContact1Patch0,
		3,
		f1i1triRep, hdfProxy);

	// Contact 1 Patch 1
	//int * indexSetContact1Patch1 = new int[3];
	indexSetContact1Patch1[0] = 5;
	indexSetContact1Patch1[1] = 6;
	indexSetContact1Patch1[2] = 7;
	sealedSurfaceFramework->pushBackContactPatch(
		1,
		indexSetContact1Patch1,
		3,
		h1i1triRep, hdfProxy);

	// Contact 1 Patch 2
	//int * indexSetContact1Patch2 = new int[3];
	indexSetContact1Patch2[0] = 12;
	indexSetContact1Patch2[1] = 13;
	indexSetContact1Patch2[2] = 14;
	sealedSurfaceFramework->pushBackContactPatch(
		1,
		indexSetContact1Patch2,
		3,
		f1i1triRep, hdfProxy);

	// Contact 2
	//sealedSurfaceFramework->pushBackContact(gsoap_resqml2_0_1::resqml__IdentityKind__COLOCATION, 3, 3, contactIdenticalNodes, hdfProxy);
	sealedSurfaceFramework->pushBackContact(gsoap_resqml2_0_1::resqml20__IdentityKind__colocation);

	// Contact 2 Patch 0
	//int * indexSetContact2Patch0 = new int[3];
	indexSetContact2Patch0[0] = 15;
	indexSetContact2Patch0[1] = 16;
	indexSetContact2Patch0[2] = 17;
	sealedSurfaceFramework->pushBackContactPatch(
		2,
		indexSetContact2Patch0,
		3,
		f1i1triRep, hdfProxy);

	// Contact 2 Patch 1
	//int * indexSetContact2Patch1 = new int[3];
	indexSetContact2Patch1[0] = 0;
	indexSetContact2Patch1[1] = 1;
	indexSetContact2Patch1[2] = 2;
	sealedSurfaceFramework->pushBackContactPatch(
		2,
		indexSetContact2Patch1,
		3,
		h2i1triRep, hdfProxy);

	// Contact 2 Patch 2
	//int * indexSetContact2Patch2 = new int[3];
	indexSetContact2Patch2[0] = 18;
	indexSetContact2Patch2[1] = 19;
	indexSetContact2Patch2[2] = 20;
	sealedSurfaceFramework->pushBackContactPatch(
		2,
		indexSetContact2Patch2,
		3,
		f1i1triRep, hdfProxy);

	// Contact 3
	//sealedSurfaceFramework->pushBackContact(gsoap_resqml2_0_1::resqml__IdentityKind__COLOCATION, 3, 3, contactIdenticalNodes, hdfProxy);
	sealedSurfaceFramework->pushBackContact(gsoap_resqml2_0_1::resqml20__IdentityKind__colocation);

	// Contact 3 Patch 0
	//int * indexSetContact3Patch0 = new int[3];
	indexSetContact3Patch0[0] = 21;
	indexSetContact3Patch0[1] = 22;
	indexSetContact3Patch0[2] = 23;
	sealedSurfaceFramework->pushBackContactPatch(
		3,
		indexSetContact3Patch0,
		3,
		f1i1triRep, hdfProxy);

	// Contact 3 Patch 1
	//int * indexSetContact3Patch1 = new int[3];
	indexSetContact3Patch1[0] = 5;
	indexSetContact3Patch1[1] = 6;
	indexSetContact3Patch1[2] = 7;
	sealedSurfaceFramework->pushBackContactPatch(
		3,
		indexSetContact3Patch1,
		3,
		h2i1triRep, hdfProxy);

	// Contact 3 Patch 2
	//int * indexSetContact3Patch2 = new int[3];
	indexSetContact3Patch2[0] = 24;
	indexSetContact3Patch2[1] = 25;
	indexSetContact3Patch2[2] = 26;
	sealedSurfaceFramework->pushBackContactPatch(
		3,
		indexSetContact3Patch2,
		3,
		f1i1triRep, hdfProxy);

	// Contact 4: horizon1Interp1 STOPS AT yMinusFrontierInterp
	sealedSurfaceFramework->pushBackContact(gsoap_resqml2_0_1::resqml20__IdentityKind__colocation);
	int twoIndicesContact[2] = { 3, 0 };
	sealedSurfaceFramework->pushBackContactPatch(4, twoIndicesContact, 2, h1i1triRep, hdfProxy);
	twoIndicesContact[0] = 0;
	twoIndicesContact[1] = 1;
	sealedSurfaceFramework->pushBackContactPatch(4, twoIndicesContact, 2, yMinusFrontierRep, hdfProxy);
	// Contact 5: fault1Interp1 STOPS AT yMinusFrontierInterp (part above horizon1Interp1)
	sealedSurfaceFramework->pushBackContact(gsoap_resqml2_0_1::resqml20__IdentityKind__colocation);
	twoIndicesContact[0] = 6;
	twoIndicesContact[1] = 9;
	sealedSurfaceFramework->pushBackContactPatch(5, twoIndicesContact, 2, f1i1triRep, hdfProxy);
	twoIndicesContact[0] = 1;
	twoIndicesContact[1] = 3;
	sealedSurfaceFramework->pushBackContactPatch(5, twoIndicesContact, 2, yMinusFrontierRep, hdfProxy);
	// Contact 6: horizon2Interp1 STOPS AT yMinusFrontierInterp
	sealedSurfaceFramework->pushBackContact(gsoap_resqml2_0_1::resqml20__IdentityKind__colocation);
	twoIndicesContact[0] = 3;
	twoIndicesContact[1] = 0;
	sealedSurfaceFramework->pushBackContactPatch(6, twoIndicesContact, 2, h2i1triRep, hdfProxy);
	twoIndicesContact[0] = 4;
	twoIndicesContact[1] = 5;
	sealedSurfaceFramework->pushBackContactPatch(6, twoIndicesContact, 2, yMinusFrontierRep, hdfProxy);

	// Contact 7: horizon1Interp1 STOPS AT yPlusFrontierInterp
	sealedSurfaceFramework->pushBackContact(gsoap_resqml2_0_1::resqml20__IdentityKind__colocation);
	twoIndicesContact[0] = 4;
	twoIndicesContact[1] = 2;
	sealedSurfaceFramework->pushBackContactPatch(7, twoIndicesContact, 2, h1i1triRep, hdfProxy);
	twoIndicesContact[0] = 0;
	twoIndicesContact[1] = 1;
	sealedSurfaceFramework->pushBackContactPatch(7, twoIndicesContact, 2, yPlusFrontierRep, hdfProxy);
	// Contact 8: fault1Interp1 STOPS AT yPlusFrontierInterp (part above horizon1Interp1)
	sealedSurfaceFramework->pushBackContact(gsoap_resqml2_0_1::resqml20__IdentityKind__colocation);
	twoIndicesContact[0] = 8;
	twoIndicesContact[1] = 11;
	sealedSurfaceFramework->pushBackContactPatch(8, twoIndicesContact, 2, f1i1triRep, hdfProxy);
	twoIndicesContact[0] = 1;
	twoIndicesContact[1] = 3;
	sealedSurfaceFramework->pushBackContactPatch(8, twoIndicesContact, 2, yPlusFrontierRep, hdfProxy);
	// Contact 9: horizon2Interp1 STOPS AT yPlusFrontierInterp
	sealedSurfaceFramework->pushBackContact(gsoap_resqml2_0_1::resqml20__IdentityKind__colocation);
	twoIndicesContact[0] = 4;
	twoIndicesContact[1] = 2;
	sealedSurfaceFramework->pushBackContactPatch(9, twoIndicesContact, 2, h2i1triRep, hdfProxy);
	twoIndicesContact[0] = 4;
	twoIndicesContact[1] = 5;
	sealedSurfaceFramework->pushBackContactPatch(9, twoIndicesContact, 2, yPlusFrontierRep, hdfProxy);

	// Contact 10: horizon1Interp1 STOPS AT xMinusFrontierInterp
	sealedSurfaceFramework->pushBackContact(gsoap_resqml2_0_1::resqml20__IdentityKind__colocation);
	twoIndicesContact[0] = 3;
	twoIndicesContact[1] = 4;
	sealedSurfaceFramework->pushBackContactPatch(10, twoIndicesContact, 2, h1i1triRep, hdfProxy);
	twoIndicesContact[0] = 0;
	twoIndicesContact[1] = 1;
	sealedSurfaceFramework->pushBackContactPatch(10, twoIndicesContact, 2, xMinusFrontierRep, hdfProxy);
	// Contact 11: horizon2Interp1 STOPS AT xMinusFrontierInterp
	sealedSurfaceFramework->pushBackContact(gsoap_resqml2_0_1::resqml20__IdentityKind__colocation);
	twoIndicesContact[0] = 3;
	twoIndicesContact[1] = 4;
	sealedSurfaceFramework->pushBackContactPatch(11, twoIndicesContact, 2, h2i1triRep, hdfProxy);
	twoIndicesContact[0] = 4;
	twoIndicesContact[1] = 5;
	sealedSurfaceFramework->pushBackContactPatch(11, twoIndicesContact, 2, xMinusFrontierRep, hdfProxy);
	// Contact 12: xMinusFrontierInterp STOPS AT yMinusFrontierInterp
	sealedSurfaceFramework->pushBackContact(gsoap_resqml2_0_1::resqml20__IdentityKind__colocation);
	int threeIndicesContact[3] = { 0,2,4 };
	sealedSurfaceFramework->pushBackContactPatch(12, threeIndicesContact, 2, yMinusFrontierRep, hdfProxy);
	threeIndicesContact[0] = 0;
	threeIndicesContact[1] = 2;
	threeIndicesContact[2] = 4;
	sealedSurfaceFramework->pushBackContactPatch(12, threeIndicesContact, 2, xMinusFrontierRep, hdfProxy);
	// Contact 13: xMinusFrontierInterp STOPS AT yPlusFrontierInterp
	sealedSurfaceFramework->pushBackContact(gsoap_resqml2_0_1::resqml20__IdentityKind__colocation);
	threeIndicesContact[0] = 0;
	threeIndicesContact[1] = 2;
	threeIndicesContact[2] = 4;
	sealedSurfaceFramework->pushBackContactPatch(13, threeIndicesContact, 2, yPlusFrontierRep, hdfProxy);
	threeIndicesContact[0] = 1;
	threeIndicesContact[1] = 3;
	threeIndicesContact[2] = 5;
	sealedSurfaceFramework->pushBackContactPatch(13, threeIndicesContact, 2, xMinusFrontierRep, hdfProxy);

	// Contact 14: fault1Interp1 STOPS AT yMinusFrontierInterp (part below horizon1Interp1)
	sealedSurfaceFramework->pushBackContact(gsoap_resqml2_0_1::resqml20__IdentityKind__colocation);
	twoIndicesContact[0] = 12;
	twoIndicesContact[1] = 15;
	sealedSurfaceFramework->pushBackContactPatch(14, twoIndicesContact, 2, f1i1triRep, hdfProxy);
	twoIndicesContact[0] = 3;
	twoIndicesContact[1] = 5;
	sealedSurfaceFramework->pushBackContactPatch(14, twoIndicesContact, 2, yMinusFrontierRep, hdfProxy);
	twoIndicesContact[0] = 6;
	twoIndicesContact[1] = 8;
	sealedSurfaceFramework->pushBackContactPatch(14, twoIndicesContact, 2, yMinusFrontierRep, hdfProxy);
	// Contact 15: fault1Interp1 STOPS AT yPlusFrontierInterp (part above horizon1Interp1)
	sealedSurfaceFramework->pushBackContact(gsoap_resqml2_0_1::resqml20__IdentityKind__colocation);
	twoIndicesContact[0] = 14;
	twoIndicesContact[1] = 17;
	sealedSurfaceFramework->pushBackContactPatch(15, twoIndicesContact, 2, f1i1triRep, hdfProxy);
	twoIndicesContact[0] = 3;
	twoIndicesContact[1] = 5;
	sealedSurfaceFramework->pushBackContactPatch(15, twoIndicesContact, 2, yPlusFrontierRep, hdfProxy);
	twoIndicesContact[0] = 6;
	twoIndicesContact[1] = 8;
	sealedSurfaceFramework->pushBackContactPatch(15, twoIndicesContact, 2, yPlusFrontierRep, hdfProxy);

	// Contact 16: horizon1Interp1 STOPS AT yMinusFrontierInterp
	sealedSurfaceFramework->pushBackContact(gsoap_resqml2_0_1::resqml20__IdentityKind__colocation);
	twoIndicesContact[0] = 5;
	twoIndicesContact[1] = 8;
	sealedSurfaceFramework->pushBackContactPatch(16, twoIndicesContact, 2, h1i1triRep, hdfProxy);
	twoIndicesContact[0] = 6;
	twoIndicesContact[1] = 7;
	sealedSurfaceFramework->pushBackContactPatch(16, twoIndicesContact, 2, yMinusFrontierRep, hdfProxy);
	// Contact 17: fault1Interp1 STOPS AT yMinusFrontierInterp (part below horizon2Interp1)
	sealedSurfaceFramework->pushBackContact(gsoap_resqml2_0_1::resqml20__IdentityKind__colocation);
	twoIndicesContact[0] = 18;
	twoIndicesContact[1] = 21;
	sealedSurfaceFramework->pushBackContactPatch(17, twoIndicesContact, 2, f1i1triRep, hdfProxy);
	twoIndicesContact[0] = 8;
	twoIndicesContact[1] = 10;
	sealedSurfaceFramework->pushBackContactPatch(17, twoIndicesContact, 2, yMinusFrontierRep, hdfProxy);
	// Contact 18: horizon2Interp1 STOPS AT yMinusFrontierInterp
	sealedSurfaceFramework->pushBackContact(gsoap_resqml2_0_1::resqml20__IdentityKind__colocation);
	twoIndicesContact[0] = 5;
	twoIndicesContact[1] = 8;
	sealedSurfaceFramework->pushBackContactPatch(18, twoIndicesContact, 2, h2i1triRep, hdfProxy);
	twoIndicesContact[0] = 10;
	twoIndicesContact[1] = 11;
	sealedSurfaceFramework->pushBackContactPatch(18, twoIndicesContact, 2, yMinusFrontierRep, hdfProxy);

	// Contact 19: horizon1Interp1 STOPS AT yPlusFrontierInterp
	sealedSurfaceFramework->pushBackContact(gsoap_resqml2_0_1::resqml20__IdentityKind__colocation);
	twoIndicesContact[0] = 7;
	twoIndicesContact[1] = 9;
	sealedSurfaceFramework->pushBackContactPatch(19, twoIndicesContact, 2, h1i1triRep, hdfProxy);
	twoIndicesContact[0] = 6;
	twoIndicesContact[1] = 7;
	sealedSurfaceFramework->pushBackContactPatch(19, twoIndicesContact, 2, yPlusFrontierRep, hdfProxy);
	// Contact 20: fault1Interp1 STOPS AT yPlusFrontierInterp (part below horizon2Interp1)
	sealedSurfaceFramework->pushBackContact(gsoap_resqml2_0_1::resqml20__IdentityKind__colocation);
	twoIndicesContact[0] = 20;
	twoIndicesContact[1] = 23;
	sealedSurfaceFramework->pushBackContactPatch(20, twoIndicesContact, 2, f1i1triRep, hdfProxy);
	twoIndicesContact[0] = 8;
	twoIndicesContact[1] = 10;
	sealedSurfaceFramework->pushBackContactPatch(20, twoIndicesContact, 2, yPlusFrontierRep, hdfProxy);
	// Contact 21: horizon2Interp1 STOPS AT yPlusFrontierInterp
	sealedSurfaceFramework->pushBackContact(gsoap_resqml2_0_1::resqml20__IdentityKind__colocation);
	twoIndicesContact[0] = 7;
	twoIndicesContact[1] = 9;
	sealedSurfaceFramework->pushBackContactPatch(21, twoIndicesContact, 2, h2i1triRep, hdfProxy);
	twoIndicesContact[0] = 10;
	twoIndicesContact[1] = 11;
	sealedSurfaceFramework->pushBackContactPatch(21, twoIndicesContact, 2, yPlusFrontierRep, hdfProxy);

	// Contact 22: horizon1Interp1 STOPS AT xPlusFrontierInterp
	sealedSurfaceFramework->pushBackContact(gsoap_resqml2_0_1::resqml20__IdentityKind__colocation);
	twoIndicesContact[0] = 8;
	twoIndicesContact[1] = 9;
	sealedSurfaceFramework->pushBackContactPatch(22, twoIndicesContact, 2, h1i1triRep, hdfProxy);
	twoIndicesContact[0] = 0;
	twoIndicesContact[1] = 1;
	sealedSurfaceFramework->pushBackContactPatch(22, twoIndicesContact, 2, xPlusFrontierRep, hdfProxy);
	// Contact 23: horizon2Interp1 STOPS AT xPlusFrontierInterp
	sealedSurfaceFramework->pushBackContact(gsoap_resqml2_0_1::resqml20__IdentityKind__colocation);
	twoIndicesContact[0] = 8;
	twoIndicesContact[1] = 9;
	sealedSurfaceFramework->pushBackContactPatch(23, twoIndicesContact, 2, h2i1triRep, hdfProxy);
	twoIndicesContact[0] = 4;
	twoIndicesContact[1] = 5;
	sealedSurfaceFramework->pushBackContactPatch(23, twoIndicesContact, 2, xPlusFrontierRep, hdfProxy);
	// Contact 24: xPlusFrontierInterp STOPS AT yMinusFrontierInterp
	sealedSurfaceFramework->pushBackContact(gsoap_resqml2_0_1::resqml20__IdentityKind__colocation);
	threeIndicesContact[0] = 7;
	threeIndicesContact[1] = 9;
	threeIndicesContact[2] = 11;
	sealedSurfaceFramework->pushBackContactPatch(24, threeIndicesContact, 2, yMinusFrontierRep, hdfProxy);
	threeIndicesContact[0] = 0;
	threeIndicesContact[1] = 2;
	threeIndicesContact[2] = 4;
	sealedSurfaceFramework->pushBackContactPatch(24, threeIndicesContact, 2, xPlusFrontierRep, hdfProxy);
	// Contact 25: xPlusFrontierInterp STOPS AT yPlusFrontierInterp
	sealedSurfaceFramework->pushBackContact(gsoap_resqml2_0_1::resqml20__IdentityKind__colocation);
	threeIndicesContact[0] = 7;
	threeIndicesContact[1] = 9;
	threeIndicesContact[2] = 11;
	sealedSurfaceFramework->pushBackContactPatch(25, threeIndicesContact, 2, yPlusFrontierRep, hdfProxy);
	threeIndicesContact[0] = 1;
	threeIndicesContact[1] = 3;
	threeIndicesContact[2] = 5;
	sealedSurfaceFramework->pushBackContactPatch(25, threeIndicesContact, 2, xPlusFrontierRep, hdfProxy);

	// =========================================================================
	// =========================================================================
	// Contact identities

	// ====================
	// Single Patch Fault 1

	// Contact 0 and 1 was previously colocated
	int contact_0_1_contactRepresentations[2] = { 0, 1 };
	//    int contact_0_1_IdenticalNodes[6] =
	//    {
	//        0, 0,
	//        1, 1,
	//        2, 2,
	//    };
	//    sealedSurfaceFramework->pushBackContactIdentity(
	//            gsoap_resqml2_0_1::resqml__IdentityKind__PREVIOUS_USCORECOLOCATION,
	//            2, contact_0_1_contactRepresentations,
	//            3, contact_0_1_IdenticalNodes, hdfProxy);
	singlePatchFault1SealedSurfaceFramework->pushBackContactIdentity(
		gsoap_resqml2_0_1::resqml20__IdentityKind__previous_x0020colocation,
		2, contact_0_1_contactRepresentations,
		hdfProxy);

	// Contact 2 and 3 was previously colocated
	int contact_2_3_contactRepresentations[2] = { 2, 3 };
	//    int contact_2_3_IdenticalNodes[6] =
	//    {
	//        0, 0,
	//        1, 1,
	//        2, 2,
	//    };
	//    sealedSurfaceFramework->pushBackContactIdentity(
	//            gsoap_resqml2_0_1::resqml__IdentityKind__PREVIOUS_USCORECOLOCATION,
	//            2, contact_2_3_contactRepresentations,
	//            3, contact_2_3_IdenticalNodes, hdfProxy);
	singlePatchFault1SealedSurfaceFramework->pushBackContactIdentity(
		gsoap_resqml2_0_1::resqml20__IdentityKind__previous_x0020colocation,
		2, contact_2_3_contactRepresentations,
		hdfProxy);

	// ==================
	// Multipatch Fault 1

	// Contact 0 and 1 was previously colocated
	//int * contact_0_1_contactRepresentations = new int[2];
	//contact_0_1_contactRepresentations[0] = 0;
	//contact_0_1_contactRepresentations[1] = 1;
	//    int contact_0_1_IdenticalNodes[6] =
	//    {
	//        0, 0,
	//        1, 1,
	//        2, 2,
	//    };
	//    sealedSurfaceFramework->pushBackContactIdentity(
	//            gsoap_resqml2_0_1::resqml__IdentityKind__PREVIOUS_USCORECOLOCATION,
	//            2, contact_0_1_contactRepresentations,
	//            3, contact_0_1_IdenticalNodes, hdfProxy);
	sealedSurfaceFramework->pushBackContactIdentity(
		gsoap_resqml2_0_1::resqml20__IdentityKind__previous_x0020colocation,
		2, contact_0_1_contactRepresentations,
		hdfProxy);

	// Contact 2 and 3 was previously colocated
	//int * contact_2_3_contactRepresentations = new int[2];
	//contact_2_3_contactRepresentations[0] = 2;
	//contact_2_3_contactRepresentations[1] = 3;
	//    int contact_2_3_IdenticalNodes[6] =
	//    {
	//        0, 0,
	//        1, 1,
	//        2, 2,
	//    };
	//    sealedSurfaceFramework->pushBackContactIdentity(
	//            gsoap_resqml2_0_1::resqml__IdentityKind__PREVIOUS_USCORECOLOCATION,
	//            2, contact_2_3_contactRepresentations,
	//            3, contact_2_3_IdenticalNodes, hdfProxy);
	sealedSurfaceFramework->pushBackContactIdentity(
		gsoap_resqml2_0_1::resqml20__IdentityKind__previous_x0020colocation,
		2, contact_2_3_contactRepresentations,
		hdfProxy);
}

void serializeActivities(COMMON_NS::DataObjectRepository * epcDoc)
{
	/********************
	* Activity Template
	********************/

	RESQML2_NS::ActivityTemplate* genericCreationActivityTemplate = epcDoc->createActivityTemplate("a41c63bf-78cb-454b-8018-c9df060c5cf3", "GenericCreationActivity");
	genericCreationActivityTemplate->pushBackParameter("CreationInput", true, false, 0, -1);
	genericCreationActivityTemplate->pushBackParameter("CreationOutput", false, true, 1, -1);

	/********************
	* Activities
	********************/
	RESQML2_NS::Activity* pickingActivity = epcDoc->createActivity(genericCreationActivityTemplate, "", "Seismic picking");
	pickingActivity->pushBackParameter("CreationOutput", horizon1);
	pickingActivity->pushBackParameter("CreationOutput", horizon1Interp1);
	pickingActivity->pushBackParameter("CreationOutput", h1i1SingleGrid2dRep);
	pickingActivity->pushBackParameter("CreationOutput", fault1);
	pickingActivity->pushBackParameter("CreationOutput", fault1Interp1);
	pickingActivity->pushBackParameter("CreationOutput", f1i1PolyLineRep);

	RESQML2_NS::Activity* h1triangulationActivity = epcDoc->createActivity(genericCreationActivityTemplate, "", "Triangulation");
	h1triangulationActivity->pushBackParameter("CreationInput", h1i1SingleGrid2dRep);
	h1triangulationActivity->pushBackParameter("CreationOutput", h1i1triRep);

	RESQML2_NS::Activity* f1TriangulationActivity1 = epcDoc->createActivity(genericCreationActivityTemplate, "", "Triangulation");
	f1TriangulationActivity1->pushBackParameter("CreationInput", f1i1PolyLineRep);
	f1TriangulationActivity1->pushBackParameter("CreationOutput", f1i1triRepSinglePatch);

	RESQML2_NS::Activity* f1TriangulationActivity2 = epcDoc->createActivity(genericCreationActivityTemplate, "", "Triangulation");
	f1TriangulationActivity2->pushBackParameter("CreationInput", f1i1PolyLineRep);
	f1TriangulationActivity2->pushBackParameter("CreationOutput", f1i1triRep);

}

void serializeFluidBoundary(COMMON_NS::DataObjectRepository & pck, COMMON_NS::AbstractHdfProxy*)
{
	FluidBoundaryFeature* fluidBoundary = pck.createFluidBoundaryFeature("44a4d87c-3c67-4f98-a314-9d91c4147061", "Fluid boundary", gsoap_resqml2_0_1::resqml20__FluidContact__gas_x0020oil_x0020contact);
	GenericFeatureInterpretation* interp = pck.createGenericFeatureInterpretation(fluidBoundary, "d06df5e4-3c56-4abd-836f-2abb5e58e13b", "Fluid boundary interp");
	PlaneSetRepresentation* rep = pck.createPlaneSetRepresentation(interp, "4df87ed5-ea4d-4a00-99a2-828a56c9dd02", "Fluid boundary PlaneSetRep");
	rep->pushBackTiltedPlaneGeometryPatch(100, 100, 400, 200, 200, 410, 150, 150, 450);
}

void serializeRockFluidOrganization(COMMON_NS::DataObjectRepository & pck, COMMON_NS::AbstractHdfProxy*)
{
	//Top Boundary
	FluidBoundaryFeature* fluidBoundaryTop = pck.createFluidBoundaryFeature("cd400fa2-4c8b-11e9-be79-3f8079258eaa", "Fluid boundary top", gsoap_resqml2_0_1::resqml20__FluidContact__gas_x0020oil_x0020contact);
	GenericFeatureInterpretation* interpTop = pck.createGenericFeatureInterpretation(fluidBoundaryTop, "0ab8f2f4-4c96-11e9-999e-c3449b44fef5", "Fluid boundary top interp");
	PlaneSetRepresentation* repTop = pck.createPlaneSetRepresentation(interpTop, "ae1d618c-4c96-11e9-8f12-cf7f4da2a08d", "Fluid boundary top PlaneSetRep");
	repTop->pushBackTiltedPlaneGeometryPatch(100, 100, 400, 200, 200, 410, 150, 150, 450);

	//Bottom Boundary
	FluidBoundaryFeature* fluidBoundaryBottom = pck.createFluidBoundaryFeature("d332b298-4c8b-11e9-80d8-c760b2e2530d", "Fluid boundary bottom", gsoap_resqml2_0_1::resqml20__FluidContact__gas_x0020oil_x0020contact);
	GenericFeatureInterpretation* interpBottom = pck.createGenericFeatureInterpretation(fluidBoundaryBottom, "1371efae-4c96-11e9-bcdd-37d8112fd19e", "Fluid boundary bottom interp");
	PlaneSetRepresentation* repBottom = pck.createPlaneSetRepresentation(interpBottom, "b54cc3b2-4c96-11e9-b33d-ef2c41476266", "Fluid boundary bottom PlaneSetRep");
	repBottom->pushBackTiltedPlaneGeometryPatch(100, 100, 400, 200, 200, 410, 150, 150, 450);

	// Unit construction
	RockFluidUnitFeature* rockFluidFeature = pck.createRockFluidUnit("18a714da-4bf2-11e9-a17e-e74cb7f87d2a", "Rock Fluid Unit", gsoap_resqml2_0_1::resqml20__Phase__oil_x0020column, fluidBoundaryTop, fluidBoundaryBottom);
	RockFluidUnitInterpretation *rockFluidUnit = pck.createRockFluidUnitInterpretation(rockFluidFeature, "4b73172a-4bf1-11e9-a9f6-9b2813cc56e1", "Rock Fluid Unit interp");

	// Feature
	OrganizationFeature * rockFluidOrgFeature = pck.createRockFluidModel("311587dd-7abc-425b-a364-908d0508ed61", "Rock Fluid Organization feature");
	rockFluidOrgFeature->setOriginator("Geosiris");

	// Interp
	RockFluidOrganizationInterpretation* rockFluidOrgInterp = pck.createRockFluidOrganizationInterpretation(rockFluidOrgFeature, "b5bbfe42-4a63-11e9-9eeb-4f036e6e8141", "Rock Fluid org", rockFluidUnit);

	// Link between ijk grid and rock fuid org
	IjkGridExplicitRepresentation* singleCellIjkgrid = pck.getDataObjectByUuid<IjkGridExplicitRepresentation>("e69bfe00-fa3d-11e5-b5eb-0002a5d5c51b");
	ULONG64 rockFluidUnitIndice = 0;
	singleCellIjkgrid->setCellAssociationWithRockFluidOrganizationInterpretation(&rockFluidUnitIndice, 1000, rockFluidOrgInterp);
}

void deserializePropertyKindMappingFiles(COMMON_NS::DataObjectRepository * pck)
{
	PropertyKindMapper* ptMapper = pck->getPropertyKindMapper();

	cout << "Application property kind name for azimuth : " << ptMapper->getApplicationPropertyKindNameFromResqmlStandardPropertyKindName(gsoap_resqml2_0_1::resqml20__ResqmlPropertyKind__azimuth, "Petrel") << endl;
	cout << "Application property kind name for azimuth : " << ptMapper->getApplicationPropertyKindNameFromResqmlStandardPropertyKindName(gsoap_resqml2_0_1::resqml20__ResqmlPropertyKind__gamma_x0020ray_x0020API_x0020unit, "Petrel") << endl;
	cout << "Application property kind name for azimuth : " << ptMapper->getApplicationPropertyKindNameFromResqmlStandardPropertyKindName(gsoap_resqml2_0_1::resqml20__ResqmlPropertyKind__gamma_x0020ray_x0020API_x0020unit, "Sismage") << endl;
	cout << "Application property kind name for dip : " << ptMapper->getApplicationPropertyKindNameFromResqmlStandardPropertyKindName(gsoap_resqml2_0_1::resqml20__ResqmlPropertyKind__dip, "myApplication") << endl;

	cout << "Standard property for Petrel \"Dip Azimuth\" : " << ptMapper->getResqmlStandardPropertyKindNameFromApplicationPropertyKindName("Dip Azimuth", "Petrel") << endl;
	cout << "Standard property for Application property Absolute temperature : " << ptMapper->getResqmlStandardPropertyKindNameFromApplicationPropertyKindName("Absolute temperature", "myApplication") << endl;

	cout << "Application property kind for unknown local property type uuid  : " << ptMapper->getApplicationPropertyKindNameFromResqmlLocalPropertyKindUuid("unknownUuid", "myApplication") << endl;
	cout << "Application property kind for local property type 959cf4e1-d485-5225-bbdb-f53c16cc0c3c (VShale) : " << ptMapper->getApplicationPropertyKindNameFromResqmlLocalPropertyKindUuid("959cf4e1-d485-5225-bbdb-f53c16cc0c3c", "Sismage") << endl;

	ptMapper->addResqmlLocalPropertyKindToEpcDocumentFromApplicationPropertyKindName("Elevation depth", "Petrel");
}

void deserializeActivity(COMMON_NS::AbstractObject const * resqmlObject)
{
	if (!resqmlObject->getActivitySet().empty())
		cout << "Activities for object " << resqmlObject->getTitle() << endl;
	for (size_t i = 0; i < resqmlObject->getActivitySet().size(); ++i)
	{
		RESQML2_NS::Activity const * activity = resqmlObject->getActivitySet()[i];
		cout << "Activity : " << activity->getTitle() << endl;
		cout << "Activity Template : " << activity->getActivityTemplate()->getTitle() << endl;
		for (unsigned int j = 0; j < activity->getActivityTemplate()->getParameterCount(); ++j)
		{
			string paramTitle = activity->getActivityTemplate()->getParameterTitle(j);
			cout << "Parameter : " << paramTitle << endl;
			cout << "Parameter min occurs : " << activity->getActivityTemplate()->getParameterMinOccurences(paramTitle) << endl;
			cout << "Parameter max occurs : " << activity->getActivityTemplate()->getParameterMaxOccurences(paramTitle) << endl;
			cout << "Parameter is input : " << activity->getActivityTemplate()->getParameterIsInput(paramTitle) << endl;
			cout << "Parameter is output : " << activity->getActivityTemplate()->getParameterIsOutput(paramTitle) << endl;
			if (activity->getParameterCount(paramTitle) > 0) {
				if (activity->isAFloatingPointQuantityParameter(paramTitle)) {
					vector<double> vals = activity->getFloatingPointQuantityParameterValue(paramTitle);
					for (size_t k = 0; k < vals.size(); ++k) {
						cout << "Double value : " << vals[k] << endl;
					}
				}
				else if (activity->isAnIntegerQuantityParameter(paramTitle)) {
					vector<LONG64> vals = activity->getIntegerQuantityParameterValue(paramTitle);
					for (size_t k = 0; k < vals.size(); ++k) {
						cout << "Integer value : " << vals[k] << endl;
					}
				}
				else if (activity->isAStringParameter(paramTitle)) {
					vector<string> vals = activity->getStringParameterValue(paramTitle);
					for (size_t k = 0; k < vals.size(); ++k) {
						cout << "String value : " << vals[k] << endl;
					}
				}
				else if (activity->isAResqmlObjectParameter(paramTitle)) {
					vector<COMMON_NS::AbstractObject*> vals = activity->getResqmlObjectParameterValue(paramTitle);
					for (size_t k = 0; k < vals.size(); ++k) {
						cout << "Object title : " << vals[k]->getTitle() << endl;
					}
				}
				else {
					const vector<unsigned int> & paramIndex = activity->getParameterIndexOfTitle(paramTitle);
					for (size_t k = 0; k < paramIndex.size(); ++k) {
						if (activity->isAFloatingPointQuantityParameter(paramIndex[k]))
							cout << "Floating Point value : " << activity->getFloatingPointQuantityParameterValue(paramIndex[k]);
						else if (activity->isAnIntegerQuantityParameter(paramIndex[k]))
							cout << "Integer value : " << activity->getIntegerQuantityParameterValue(paramIndex[k]);
						else if (activity->isAStringParameter(paramIndex[k]))
							cout << "String value : " << activity->getStringParameterValue(paramIndex[k]);
						else if (activity->isAResqmlObjectParameter(paramIndex[k]))
							cout << "Object title : " << activity->getResqmlObjectParameterValue(paramIndex[k])->getTitle();
					}
				}
			}
			else
				cout << "No provided parameter in the activity." << endl;
		}
		cout << endl;
	}
}

bool serialize(const string & filePath)
{
	COMMON_NS::EpcDocument pck(filePath);
	COMMON_NS::DataObjectRepository repo;

	COMMON_NS::AbstractObject::setFormat("F2I-CONSULTING", "Fesapi Example", FESAPI_VERSION);

	COMMON_NS::AbstractHdfProxy* hdfProxy = repo.createHdfProxy("", "Hdf Proxy", pck.getStorageDirectory(), pck.getName() + ".h5", COMMON_NS::DataObjectRepository::OVERWRITE);
	repo.setDefaultHdfProxy(hdfProxy);

	//CRS
	local3dCrs = repo.createLocalDepth3dCrs("", "Default local CRS", .0, .0, .0, .0, gsoap_resqml2_0_1::eml20__LengthUom__m, 23031, gsoap_resqml2_0_1::eml20__LengthUom__m, "Unknown", false);
	localTime3dCrs = repo.createLocalTime3dCrs("", "Default local time CRS", 1.0, 0.1, 15, .0, gsoap_resqml2_0_1::eml20__LengthUom__m, 23031, gsoap_resqml2_0_1::eml20__TimeUom__s, gsoap_resqml2_0_1::eml20__LengthUom__m, "Unknown", false); // CRS translation is just for testing;
	repo.setDefaultCrs(local3dCrs);

	// Comment or uncomment below domains/lines you want wether to test or not
	serializeWells(&repo, hdfProxy);
	serializePerforations(&repo);
	serializeBoundaries(&repo, hdfProxy);
	serializeGeobody(&repo, hdfProxy);
	serializeStructuralModel(repo, hdfProxy);
	serializeStratigraphicModel(&repo, hdfProxy);
	serializeGrid(&repo, hdfProxy);
	serializeActivities(&repo);
	serializeRepresentationSetRepresentation(&repo, hdfProxy);
	serializeFluidBoundary(repo, hdfProxy);
	serializeRockFluidOrganization(repo, hdfProxy);
#ifdef WITH_EXPERIMENTAL
	serializeGraphicalInformationSet(&repo, hdfProxy);
#endif
	// Add an extended core property before to serialize
	pck.setExtendedCoreProperty("F2I-ExtendedCoreProp", "TestingVersion");

	hdfProxy->close();

	cout << "Start serialization of " << pck.getName() << " in " << (pck.getStorageDirectory().empty() ? "working directory." : pck.getStorageDirectory()) << endl;
	pck.serializeFrom(repo);
	return true;
}

void showAllMetadata(COMMON_NS::AbstractObject const * obj, const std::string & prefix = "")
{
	std::cout << prefix << "Title is : " << obj->getTitle() << std::endl;
	std::cout << prefix << "Guid is : " << obj->getUuid() << std::endl;
	if (!obj->isPartial()) {
		for (unsigned int i = 0; i < obj->getAliasCount(); ++i) {
			std::cout << prefix << "Alias is : " << obj->getAliasAuthorityAtIndex(i) << ":" << obj->getAliasTitleAtIndex(i) << std::endl;
		}
		for (unsigned int i = 0; i < obj->getExtraMetadataCount(); ++i) {
			std::cout << prefix << "Extrametadata is : " << obj->getExtraMetadataKeyAtIndex(i) << ":" << obj->getExtraMetadataStringValueAtIndex(i) << std::endl;
		}
		time_t creation = obj->getCreation();
		std::cout << prefix << "Creation date is (unix timestamp) : " << creation << std::endl;
		tm creationTm = obj->getCreationAsTimeStructure();
		std::cout << prefix << "Creation date is (struct tm) : " << 1900 + creationTm.tm_year << "-" << creationTm.tm_mon + 1 << "-" << creationTm.tm_mday << "T" << creationTm.tm_hour << ":" << creationTm.tm_min << ":" << creationTm.tm_sec << std::endl;
		if (obj->hasVersion()) {
			std::cout << prefix << "version is : " << obj->getVersion() << std::endl;
		}
		std::cout << prefix << "--------------------------------------------------" << std::endl;

	}
	else {
		std::cout << prefix << "IS PARTIAL!" << std::endl;
	}
}

void showAllProperties(RESQML2_NS::AbstractRepresentation const * rep, bool* enabledCells = nullptr)
{
	std::vector<RESQML2_NS::AbstractValuesProperty *> propertyValuesSet = rep->getValuesPropertySet();
	if (!propertyValuesSet.empty()) {
		cout << "PROPERTIES" << std::endl;
	}
	for (size_t propIndex = 0; propIndex < propertyValuesSet.size(); ++propIndex) {
		std::cout << "\t--------------------------------------------------" << std::endl;
		RESQML2_NS::AbstractValuesProperty const * propVal = propertyValuesSet[propIndex];
		showAllMetadata(propVal, "\t");

		std::vector<RESQML2_NS::PropertySet *> propSets = propVal->getPropertySets();
		for (size_t propSetIndex = 0; propSetIndex < propSets.size(); ++propSetIndex) {
			RESQML2_NS::PropertySet* propSet = propSets[propSetIndex];
			std::cout << "\tContained in property set : ";
			showAllMetadata(propSet, "\t");
		}

		std::cout << "\tProperty kind is : " << propVal->getPropertyKindAsString() << std::endl;
		if (propVal->isAssociatedToOneStandardEnergisticsPropertyKind()) {
			std::cout << "\tProperty kind is an Energistics one" << std::endl;
		}
		else {
			std::cout << "\tProperty kind is not an Energistics one" << std::endl;
			if (propVal->getLocalPropertyKind()->isParentAnEnergisticsPropertyKind()) {
				std::cout << "\t\tProperty kind parent is an Energistics one" << std::endl;
				std::cout << "\t\tProperty kind parent is : " << propVal->getLocalPropertyKind()->getParentAsString() << std::endl;
			}
			else {
				std::cout << "\t\tProperty kind parent is not an Energistics one" << std::endl;
				std::cout << "\t\tProperty kind parent is : " << propVal->getLocalPropertyKind()->getParentLocalPropertyKind()->getTitle() << std::endl;
			}
		}

		// Dimension
		unsigned int dimCount = propVal->getDimensionsCountOfPatch(0);
		std::cout << "\tDimension count is : " << dimCount << std::endl;
		for (unsigned int dimIndex = 0; dimIndex < dimCount; ++dimIndex) {
			std::cout << "\tValues count in dimension " << dimIndex << " is : " << propVal->getValuesCountOfDimensionOfPatch(dimIndex, 0) << std::endl;
		}
		unsigned int valueCount = propVal->getValuesCountOfPatch(0);
		std::cout << "\tValues count in all dimensions is : " << valueCount << std::endl;

		// Datatype
		std::cout << "\tDatatype is : " << propVal->getValuesHdfDatatype() << std::endl;
		if (propVal->getValuesHdfDatatype() == 0) {
			cerr << "\tERROR !!!!! The hdf datatype is unknown" << endl;
			cout << "\tPress enter to continue..." << endl;
			cin.get();
		}
		else if (propVal->getValuesHdfDatatype() > 2) {
			if (dynamic_cast<DiscreteProperty const *>(propVal) == nullptr && dynamic_cast<CategoricalProperty const *>(propVal) == nullptr) {
				cerr << "\tERROR !!!!! The continuous property is linked to an integer HDF5 dataset." << endl;
				cout << "\tTrying to convert.." << endl;
				double* values = new double[valueCount];
				static_cast<ContinuousProperty const *>(propVal)->getDoubleValuesOfPatch(0, values);
				std::cout << "\tFirst value is " << values[0] << endl;
				std::cout << "\tSecond value is " << values[1] << endl;
				delete[] values;
				cout << "\tPress enter to continue..." << endl;
				cin.get();
			}
			else if (dynamic_cast<DiscreteProperty const *>(propVal) != nullptr) {
				DiscreteProperty const * discreteProp = static_cast<DiscreteProperty const *>(propVal);
				const LONG64 maxValue = discreteProp->getMaximumValue();
				const LONG64 minValue = discreteProp->getMinimumValue();
				std::cout << "\tMax value is " << maxValue << endl;
				std::cout << "\tMin value is " << minValue << endl;
				LONG64* values = new LONG64[valueCount];
				propVal->getLongValuesOfPatch(0, values);
				std::cout << "\tFirst value is " << values[0] << endl;
				std::cout << "\tSecond value is " << values[1] << endl;
				delete[] values;
			}
		}
		else {
			if (dynamic_cast<ContinuousProperty const *>(propVal) == nullptr) {
				cerr << "\tERROR !!!!! The discrete or categorical property is linked to a floating point HDF5 dataset." << endl;
				cout << "\tTrying to convert.." << endl;
				LONG64* values = new LONG64[valueCount];
				propVal->getLongValuesOfPatch(0, values);
				std::cout << "\tFirst value is " << values[0] << endl;
				std::cout << "\tSecond value is " << values[1] << endl;
				delete[] values;
				cout << "\tPress enter to continue..." << endl;
				cin.get();
			}
			else {
				ContinuousProperty const * continuousProp = static_cast<ContinuousProperty const *>(propVal);
				const double maxValue = continuousProp->getMaximumValue();
				const double minValue = continuousProp->getMinimumValue();
				std::cout << "\tMax value is " << maxValue << endl;
				std::cout << "\tMin value is " << minValue << endl;
				double* values = new double[valueCount];
				continuousProp->getDoubleValuesOfPatch(0, values);
				std::cout << "\tFirst value is " << values[0] << endl;
				std::cout << "\tSecond value is " << values[1] << endl;

				if (continuousProp->getElementCountPerValue() == 1) {
					for (size_t cellIndex = 0; cellIndex < valueCount; ++cellIndex) {
						if (enabledCells != nullptr && !enabledCells[cellIndex]) {
							continue;
						}
						if (values[cellIndex] > maxValue || values[cellIndex] < minValue) {
							std::cerr << "\tERROR of min max range on : cell " << cellIndex << " has value " << values[cellIndex] << endl;
						}
					}
				}

				delete[] values;
			}
		}
	}
	std::cout << "\t--------------------------------------------------" << std::endl;
}

void showAllSubRepresentations(const vector<RESQML2_NS::SubRepresentation *> & subRepSet)
{
	if (!subRepSet.empty()) {
		cout << "SUBREPRESENTATIONS" << std::endl;
	}
	std::cout << "\t--------------------------------------------------" << std::endl;
	for (size_t subRepIndex = 0; subRepIndex < subRepSet.size(); ++subRepIndex)
	{
		showAllMetadata(subRepSet[subRepIndex], "\t");
		if (!subRepSet[subRepIndex]->isPartial()) {
			const ULONG64 indiceCount = subRepSet[subRepIndex]->getElementCountOfPatch(0);

			// element indices
			ULONG64 * elementIndices = new ULONG64[indiceCount];
			subRepSet[subRepIndex]->getElementIndicesOfPatch(0, 0, elementIndices);
			for (unsigned int i = 0; i < indiceCount && i < 10; ++i) {
				std::cout << "Element indice at position " << i << " : " << elementIndices[i] << std::endl;
			}
			delete[] elementIndices;

			// Supporting rep indices
			short * supRepIndices = new short[indiceCount];
			subRepSet[subRepIndex]->getSupportingRepresentationIndicesOfPatch(0, supRepIndices);
			for (unsigned int i = 0; i < indiceCount && i < 10; ++i) {
				std::cout << "Supporting rep indice at position " << i << " : " << supRepIndices[i] << std::endl;
			}
			delete[] supRepIndices;

		}
		else {
			std::cout << "IS PARTIAL!" << std::endl;
		}

		showAllProperties(subRepSet[subRepIndex]);
	}
}

void deserializeStratiColumn(StratigraphicColumn * stratiColumn)
{
	showAllMetadata(stratiColumn);
	for (size_t i = 0; i < stratiColumn->getStratigraphicColumnRankInterpretationSet().size(); ++i)
	{
		std::cout << "\tCOLUMN RANK INTERP" << std::endl;
		StratigraphicColumnRankInterpretation const * stratiColumnRankInterp = stratiColumn->getStratigraphicColumnRankInterpretationSet()[i];
		showAllMetadata(stratiColumnRankInterp);
		if (stratiColumnRankInterp->isAChronoStratiRank() == true)
			cout << "This is a chrono rank!" << endl;
		else
			cout << "This is not a chrono rank!" << endl;
		unsigned int contactCount = stratiColumnRankInterp->getContactCount();
		for (unsigned int contactIndex = 0; contactIndex < contactCount; ++contactIndex) {
			cout << "SUBJECT : " << endl;
			showAllMetadata(stratiColumnRankInterp->getSubjectOfContact(contactIndex));
			cout << "DIRECT OBJECT : " << endl;
			showAllMetadata(stratiColumnRankInterp->getDirectObjectOfContact(contactIndex));
		}

		vector<StratigraphicOccurrenceInterpretation*> soiSet = stratiColumnRankInterp->getStratigraphicOccurrenceInterpretationSet();
		for (size_t soiIndex = 0; soiIndex < soiSet.size(); ++soiIndex) {
			vector<WellboreMarkerFrameRepresentation *> markerFrameSet = soiSet[soiIndex]->getWellboreMarkerFrameRepresentationSet();
			for (size_t markerFrameIndex = 0; markerFrameIndex < markerFrameSet.size(); ++markerFrameIndex) {
				WellboreMarkerFrameRepresentation * markerFrame = markerFrameSet[markerFrameIndex];
				showAllMetadata(markerFrame);
				vector<WellboreMarker *> markerSet = markerFrame->getWellboreMarkerSet();
				double* doubleMds = new double[markerFrame->getMdValuesCount()];
				markerFrame->getMdAsDoubleValues(doubleMds);
				for (size_t mIndex = 0; mIndex < markerSet.size(); ++mIndex) {
					if (doubleMds[mIndex] == doubleMds[mIndex]) {
						cout << doubleMds[mIndex] << endl;
					}
					else {
						cout << "NaN" << endl;
					}
				}
				delete[] doubleMds;
			}
		}
	}
}

void deserializeSealedSurfaceFramework(const COMMON_NS::DataObjectRepository & pck)
{
	const std::vector<RESQML2_0_1_NS::SealedSurfaceFrameworkRepresentation*> ssfVec = pck.getDataObjects<RESQML2_0_1_NS::SealedSurfaceFrameworkRepresentation>();

	for (size_t ssfIndex = 0; ssfIndex < ssfVec.size(); ++ssfIndex) {
		std::cout << "\tSEALED SURFACE FRAMEWORK" << std::endl;
		RESQML2_0_1_NS::SealedSurfaceFrameworkRepresentation* ssf = ssfVec[ssfIndex];
		showAllMetadata(ssf);

		std::cout << "\tCONTAINED REPRESENTATIONS" << std::endl;
		const unsigned int repCount = ssf->getRepresentationCount();
		for (unsigned int repIdx = 0; repIdx < repCount; ++repIdx) {
			showAllMetadata(ssf->getRepresentation(repIdx));
		}

		std::cout << "\tCONTACT (REPRESENTATION) IDENTITY" << std::endl;
		const unsigned int ciCount = ssf->getContactIdentityCount();
		for (unsigned int ciIdx = 0; ciIdx < ciCount; ++ciIdx) {
			std::cout << "\t\tidentity kind (0->coloc, 1->preColoc, 2->eq, 3->preEq) : " << ssf->getContactIdentityKind(ciIdx) << std::endl;
			const unsigned int ciContactRepCount = ssf->getContactCountInContactIdentity(ciIdx);
			unsigned int* cicrIndices = new unsigned int[ciContactRepCount];
			ssf->getContactIndices(ciIdx, cicrIndices);
			std::cout << "\t\tcontact rep indices : ";
			for (unsigned int cicrIdx = 0; cicrIdx < ciContactRepCount; ++cicrIdx) {
				std::cout << cicrIndices[cicrIdx] << " ";
			}
			delete[] cicrIndices;
			std::cout << std::endl;

			if (ssf->areAllContactNodesIdentical(ciIdx)) {
				std::cout << "\t\tAll nodes are identical." << endl;
			}
			else {
				const unsigned int ciIdenticalNodeCount = ssf->getIdenticalContactNodeCount(ciIdx);
				unsigned int* ciIdenticalNodeIndices = new unsigned int[ciIdenticalNodeCount];
				ssf->getIdenticalContactNodeIndices(ciIdx, ciIdenticalNodeIndices);
				std::cout << "\t\tIdentical nodes : ";
				for (unsigned int identicalNodesIdx = 0; identicalNodesIdx < ciIdenticalNodeCount; ++identicalNodesIdx) {
					std::cout << ciIdenticalNodeIndices[identicalNodesIdx] << " ";
				}
				delete[] ciIdenticalNodeIndices;
				std::cout << std::endl;
			}
		}

		std::cout << "\tCONTACT REPRESENTATIONS" << std::endl;;
		const unsigned int crCount = ssf->getContactCount();
		for (unsigned int crIdx = 0; crIdx < crCount; ++crIdx) {
			std::cout << "\t\tidentity kind (0->coloc, 1->preColoc, 2->eq, 3->preEq) : " << ssf->getContactPatchIdentityKind(crIdx) << std::endl;
			if (ssf->areAllContactPatchNodesIdentical(crIdx)) {
				std::cout << "\t\tAll nodes are identical." << endl;
			}
			else {
				const unsigned int crIdenticalNodeCount = ssf->getIdenticalContactPatchNodeCount(crIdx);
				unsigned int* crIdenticalNodeIndices = new unsigned int[crIdenticalNodeCount];
				ssf->getIdenticalContactPatchNodeIndices(crIdx, crIdenticalNodeIndices);
				std::cout << "\t\tIdentical nodes : ";
				for (unsigned int identicalNodesIdx = 0; identicalNodesIdx < crIdenticalNodeCount; ++identicalNodesIdx) {
					std::cout << crIdenticalNodeIndices[identicalNodesIdx] << " ";
				}
				delete[] crIdenticalNodeIndices;
				std::cout << std::endl;
			}

			std::cout << "\t\tCONTACT PATCHES" << std::endl;;
			const unsigned int cpCount = ssf->getContactPatchCount(crIdx);
			for (unsigned int cpIdx = 0; cpIdx < cpCount; ++cpIdx) {
				showAllMetadata(ssf->getRepresentationOfContactPatch(crIdx, cpIdx));
				const unsigned int cpNodeCount = ssf->getContactPatchNodeCount(crIdx, cpIdx);
				unsigned int* cpNodeIndices = new unsigned int[cpNodeCount];
				ssf->getContactPatchNodeIndices(crIdx, cpIdx, cpNodeIndices);
				std::cout << "\t\tcontact patch indices : ";
				for (unsigned int cpNodeIdx = 0; cpNodeIdx < cpNodeCount; ++cpNodeIdx) {
					std::cout << cpNodeIndices[cpNodeIdx] << " ";
				}
				delete[] cpNodeIndices;
				std::cout << std::endl;
			}
		}
	}
}

void deserializeSealedVolumeFramework(const COMMON_NS::DataObjectRepository & pck)
{
	const std::vector<RESQML2_0_1_NS::SealedVolumeFrameworkRepresentation*> svfVec = pck.getDataObjects<RESQML2_0_1_NS::SealedVolumeFrameworkRepresentation>();

	for (size_t svfIndex = 0; svfIndex < svfVec.size(); ++svfIndex) {
		std::cout << "\tSEALED VOLUME FRAMEWORK" << std::endl;
		RESQML2_0_1_NS::SealedVolumeFrameworkRepresentation* svf = svfVec[svfIndex];
		showAllMetadata(svf);

		std::cout << "\tis based on sealed structural framework " << endl;
		showAllMetadata(svf->getSealedStructuralFramework());

		std::cout << "\t\tCONTAINED REGIONS" << std::endl;
		const unsigned int regionCount = svf->getRegionCount();
		for (unsigned int regionIdx = 0; regionIdx < regionCount; ++regionIdx) {
			showAllMetadata(svf->getStratiUnitInterp(regionIdx));

			const unsigned int faceCount = svf->getFaceCountOfExternalShell(regionIdx);
			for (unsigned int faceIdx = 0; faceIdx < faceCount; ++faceIdx) {
				std::cout << "\t\tFace index " << faceIdx << " is the patch " << svf->getRepPatchIndexOfExternalShellFace(regionIdx, faceIdx) << " with side " << svf->getSideFlagOfExternalShellFace(regionIdx, faceIdx) << " of surface representation" << std::endl;
				showAllMetadata(svf->getRepOfExternalShellFace(regionIdx, faceIdx));
			}
		}

	}
}

void deserializeGeobody(COMMON_NS::DataObjectRepository * pck)
{
	//2d
	std::vector<GeneticBoundaryFeature*> geobodyBoundarySet = pck->getGeobodyBoundarySet();
	for (size_t i = 0; i < geobodyBoundarySet.size(); ++i) {
		showAllMetadata(geobodyBoundarySet[i]);
		cout << "interp count : " << geobodyBoundarySet[i]->getInterpretationCount() << endl;
	}

	//3d
	std::vector<GeobodyFeature*> geobodySet = pck->getGeobodySet();
	for (size_t i = 0; i < geobodySet.size(); ++i) {
		showAllMetadata(geobodySet[i]);
		cout << "interp count : " << geobodySet[i]->getInterpretationCount() << endl;
	}
}

void deserializeFluidBoundary(COMMON_NS::DataObjectRepository & pck)
{
	std::vector<FluidBoundaryFeature*> fbfSet = pck.getDataObjects<FluidBoundaryFeature>();
	for (size_t fbfIndex = 0; fbfIndex < fbfSet.size(); ++fbfIndex) {
		FluidBoundaryFeature* fluidBoundary = fbfSet[fbfIndex];
		if (fluidBoundary == nullptr) return;
		showAllMetadata(fluidBoundary);
		showAllMetadata(fluidBoundary->getInterpretation(0));
		PlaneSetRepresentation const * rep = static_cast<PlaneSetRepresentation const*>(fluidBoundary->getInterpretation(0)->getRepresentation(0));
		showAllMetadata(rep);
		ULONG64 ptCount = rep->getXyzPointCountOfAllPatches();
		double* allXyzPoints = new double[ptCount * 3];
		rep->getXyzPointsOfAllPatchesInGlobalCrs(allXyzPoints);
		for (size_t i = 0; i < ptCount; ++i) {
			std::cout << "Point " << i << " X=" << allXyzPoints[i * 3] << std::endl;
			std::cout << "Point " << i << " Y=" << allXyzPoints[i * 3 + 1] << std::endl;
			std::cout << "Point " << i << " Z=" << allXyzPoints[i * 3 + 2] << std::endl;
		}
		delete[] allXyzPoints;
	}
}

void deserializeRockFluidOrganization(COMMON_NS::DataObjectRepository & pck)
{
	std::vector<RockFluidOrganizationInterpretation*> rockFluidOrgInterpSet = pck.getDataObjects<RockFluidOrganizationInterpretation>();
	for (size_t rfoiIndex = 0; rfoiIndex < rockFluidOrgInterpSet.size(); ++rfoiIndex) {
		RockFluidOrganizationInterpretation* rockFluidOrgInterp = rockFluidOrgInterpSet[rfoiIndex];
		showAllMetadata(rockFluidOrgInterp);
		for (unsigned int i = 0; i < rockFluidOrgInterp->getGridRepresentationCount(); ++i) {
			RESQML2_NS::AbstractGridRepresentation const * grid = rockFluidOrgInterp->getGridRepresentation(i);
			showAllMetadata(grid);
		}

		for (unsigned int unitIndex = 0; unitIndex < rockFluidOrgInterp->getRockFluidUnitInterpCount(); ++unitIndex) {
			RockFluidUnitInterpretation* rockFluidInterp = rockFluidOrgInterp->getRockFluidUnitInterpretation(unitIndex);
			showAllMetadata(rockFluidInterp);

			RockFluidUnitFeature* rockFluidFeature = static_cast<RockFluidUnitFeature*>(rockFluidInterp->getInterpretedFeature());
			showAllMetadata(rockFluidFeature);

			BoundaryFeature* top = rockFluidFeature->getTop();
			showAllMetadata(top);

			BoundaryFeature* bottom = rockFluidFeature->getBottom();
			showAllMetadata(bottom);
		}
	}
}

/**
* Deserialize IJK grid explicit and parametric representations packed in a given EPC document.
* This method read grid geometry by using hyperslabbing methods. Each grid is read interface by interface
* and then layer by layer.
* @param pck	An EPC document containine the IJK grid to deserialize.
*/
void deserializeGridHyperslabbingInterfaceSequence(COMMON_NS::DataObjectRepository & pck)
{
	cout << endl << "BEGIN: IJK GRID REP (hyperslabbing)" << std::endl;
	unsigned int ijkGridCount = pck.getIjkGridRepresentationCount();
	for (unsigned int ijkGridIdx = 0; ijkGridIdx < ijkGridCount; ++ijkGridIdx)
	{
		AbstractIjkGridRepresentation* ijkGrid = pck.getIjkGridRepresentation(ijkGridIdx);

		if (ijkGrid->getGeometryKind() != AbstractIjkGridRepresentation::NO_GEOMETRY)
		{
			if (ijkGrid->getGeometryKind() == AbstractIjkGridRepresentation::EXPLICIT || ijkGrid->getGeometryKind() == AbstractIjkGridRepresentation::PARAMETRIC) {
				cout << "--------------------------------------------------" << std::endl;
				showAllMetadata(ijkGrid);
				if (ijkGrid->isPartial()) {
					continue;

				}

				ijkGrid->loadSplitInformation();

				// here, we read a grid interface by interface. Each interface is read
				// in i then j direction
				cout << "INTERFACE BY INTERFACE" << std::endl;

				for (unsigned int kInterface = 0; kInterface < ijkGrid->getKCellCount(); kInterface++)
				{
					cout << "INTERFACE: " << kInterface << std::endl;

					double* interfaceXyzPoints = new double[ijkGrid->getXyzPointCountOfKInterfaceOfPatch(0) * 3];
					ijkGrid->getXyzPointsOfKInterfaceOfPatch(kInterface, 0, interfaceXyzPoints);

					for (unsigned int j = 0; j < ijkGrid->getJCellCount(); j++)
					{
						for (unsigned int i = 0; i < ijkGrid->getICellCount(); i++)
						{
							cout << "CELL (" << i << ", " << j << ", " << kInterface << ")" << std::endl;

							ULONG64 xyzPointIndex;
							double x, y, z;
							ULONG64 indexShift = kInterface * ijkGrid->getXyzPointCountOfKInterfaceOfPatch(0) * 3;

							// Corner (0, 0, 0)
							xyzPointIndex = ijkGrid->getXyzPointIndexFromCellCorner(i, j, kInterface, 0) * 3 - indexShift;
							x = interfaceXyzPoints[xyzPointIndex];
							y = interfaceXyzPoints[xyzPointIndex + 1];
							z = interfaceXyzPoints[xyzPointIndex + 2];
							cout << "CORNER (0, 0, 0): " << x << " " << y << " " << z << std::endl;

							// Corner (1, 0, 0)
							xyzPointIndex = ijkGrid->getXyzPointIndexFromCellCorner(i, j, kInterface, 1) * 3 - indexShift;
							x = interfaceXyzPoints[xyzPointIndex];
							y = interfaceXyzPoints[xyzPointIndex + 1];
							z = interfaceXyzPoints[xyzPointIndex + 2];
							cout << "CORNER (1, 0, 0): " << x << " " << y << " " << z << std::endl;

							// Corner (1, 1, 0)
							xyzPointIndex = ijkGrid->getXyzPointIndexFromCellCorner(i, j, kInterface, 2) * 3 - indexShift;
							x = interfaceXyzPoints[xyzPointIndex];
							y = interfaceXyzPoints[xyzPointIndex + 1];
							z = interfaceXyzPoints[xyzPointIndex + 2];
							cout << "CORNER (1, 1, 0): " << x << " " << y << " " << z << std::endl;

							// Corner (0, 1, 0)
							xyzPointIndex = ijkGrid->getXyzPointIndexFromCellCorner(i, j, kInterface, 3) * 3 - indexShift;
							x = interfaceXyzPoints[xyzPointIndex];
							y = interfaceXyzPoints[xyzPointIndex + 1];
							z = interfaceXyzPoints[xyzPointIndex + 2];
							cout << "CORNER (0, 1, 0): " << x << " " << y << " " << z << std::endl;

						}
					}

					delete[] interfaceXyzPoints;
				}

				// last interface differs from the other because of  getXyzPointIndexFromCellCorner usage
				cout << "INTERFACE: " << ijkGrid->getKCellCount() << std::endl;

				double* interfaceXyzPoints = new double[ijkGrid->getXyzPointCountOfKInterfaceOfPatch(0) * 3];
				ijkGrid->getXyzPointsOfKInterfaceOfPatch(ijkGrid->getKCellCount(), 0, interfaceXyzPoints);

				for (unsigned int i = 0; i < ijkGrid->getICellCount(); i++)
				{
					for (unsigned int j = 0; j < ijkGrid->getJCellCount(); j++)
					{
						cout << "CELL (" << i << ", " << j << ", " << ijkGrid->getKCellCount() - 1 << ")" << std::endl;

						ULONG64 xyzPointIndex;
						double x, y, z;
						ULONG64 indexShift = ijkGrid->getKCellCount() * ijkGrid->getXyzPointCountOfKInterfaceOfPatch(0) * 3;

						// Corner (0, 0, 1)
						xyzPointIndex = ijkGrid->getXyzPointIndexFromCellCorner(i, j, ijkGrid->getKCellCount() - 1, 4) * 3 - indexShift;
						x = interfaceXyzPoints[xyzPointIndex];
						y = interfaceXyzPoints[xyzPointIndex + 1];
						z = interfaceXyzPoints[xyzPointIndex + 2];
						cout << "CORNER (0, 0, 1): " << x << " " << y << " " << z << std::endl;

						// Corner (1, 0, 1)
						xyzPointIndex = ijkGrid->getXyzPointIndexFromCellCorner(i, j, ijkGrid->getKCellCount() - 1, 5) * 3 - indexShift;
						x = interfaceXyzPoints[xyzPointIndex];
						y = interfaceXyzPoints[xyzPointIndex + 1];
						z = interfaceXyzPoints[xyzPointIndex + 2];
						cout << "CORNER (1, 0, 1): " << x << " " << y << " " << z << std::endl;

						// Corner (1, 1, 1)
						xyzPointIndex = ijkGrid->getXyzPointIndexFromCellCorner(i, j, ijkGrid->getKCellCount() - 1, 6) * 3 - indexShift;
						x = interfaceXyzPoints[xyzPointIndex];
						y = interfaceXyzPoints[xyzPointIndex + 1];
						z = interfaceXyzPoints[xyzPointIndex + 2];
						cout << "CORNER (1, 1, 1): " << x << " " << y << " " << z << std::endl;

						// Corner (0, 1, 1)
						xyzPointIndex = ijkGrid->getXyzPointIndexFromCellCorner(i, j, ijkGrid->getKCellCount() - 1, 7) * 3 - indexShift;
						x = interfaceXyzPoints[xyzPointIndex];
						y = interfaceXyzPoints[xyzPointIndex + 1];
						z = interfaceXyzPoints[xyzPointIndex + 2];
						cout << "CORNER (0, 1, 1): " << x << " " << y << " " << z << std::endl;
					}
				}

				delete[] interfaceXyzPoints;

				// here, we read a grid layer by layer. Each layer is read in i,
				// then j direction.
				cout << "--------------------------------------------------" << std::endl;
				cout << "LAYER BY LAYER" << std::endl;

				for (unsigned int kInterface = 0; kInterface < ijkGrid->getKCellCount(); kInterface++)
				{
					cout << "LAYER: " << kInterface << std::endl;

					double* layerXyzPoints = new double[ijkGrid->getXyzPointCountOfKInterfaceOfPatch(0) * 3 * 2];
					ijkGrid->getXyzPointsOfKInterfaceSequenceOfPatch(kInterface, kInterface + 1, 0, layerXyzPoints);

					for (unsigned int j = 0; j < ijkGrid->getJCellCount(); j++)
					{
						for (unsigned int i = 0; i < ijkGrid->getICellCount(); i++)
						{
							cout << "CELL (" << i << ", " << j << ", " << kInterface << ")" << std::endl;

							ULONG64 xyzPointIndex;
							double x, y, z;
							ULONG64 indexShift = kInterface * ijkGrid->getXyzPointCountOfKInterfaceOfPatch(0) * 3;

							// Corner (0, 0, 0)
							xyzPointIndex = ijkGrid->getXyzPointIndexFromCellCorner(i, j, kInterface, 0) * 3 - indexShift;
							x = layerXyzPoints[xyzPointIndex];
							y = layerXyzPoints[xyzPointIndex + 1];
							z = layerXyzPoints[xyzPointIndex + 2];
							cout << "CORNER (0, 0, 0): " << x << " " << y << " " << z << std::endl;

							// Corner (1, 0, 0)
							xyzPointIndex = ijkGrid->getXyzPointIndexFromCellCorner(i, j, kInterface, 1) * 3 - indexShift;
							x = layerXyzPoints[xyzPointIndex];
							y = layerXyzPoints[xyzPointIndex + 1];
							z = layerXyzPoints[xyzPointIndex + 2];
							cout << "CORNER (1, 0, 0): " << x << " " << y << " " << z << std::endl;

							// Corner (1, 1, 0)
							xyzPointIndex = ijkGrid->getXyzPointIndexFromCellCorner(i, j, kInterface, 2) * 3 - indexShift;
							x = layerXyzPoints[xyzPointIndex];
							y = layerXyzPoints[xyzPointIndex + 1];
							z = layerXyzPoints[xyzPointIndex + 2];
							cout << "CORNER (1, 1, 0): " << x << " " << y << " " << z << std::endl;

							// Corner (0, 1, 0)
							xyzPointIndex = ijkGrid->getXyzPointIndexFromCellCorner(i, j, kInterface, 3) * 3 - indexShift;
							x = layerXyzPoints[xyzPointIndex];
							y = layerXyzPoints[xyzPointIndex + 1];
							z = layerXyzPoints[xyzPointIndex + 2];
							cout << "CORNER (0, 1, 0): " << x << " " << y << " " << z << std::endl;

							// Corner (0, 0, 1)
							xyzPointIndex = ijkGrid->getXyzPointIndexFromCellCorner(i, j, kInterface, 4) * 3 - indexShift;
							x = layerXyzPoints[xyzPointIndex];
							y = layerXyzPoints[xyzPointIndex + 1];
							z = layerXyzPoints[xyzPointIndex + 2];
							cout << "CORNER (0, 0, 1): " << x << " " << y << " " << z << std::endl;

							// Corner (1, 0, 1)
							xyzPointIndex = ijkGrid->getXyzPointIndexFromCellCorner(i, j, kInterface, 5) * 3 - indexShift;
							x = layerXyzPoints[xyzPointIndex];
							y = layerXyzPoints[xyzPointIndex + 1];
							z = layerXyzPoints[xyzPointIndex + 2];
							cout << "CORNER (1, 0, 1): " << x << " " << y << " " << z << std::endl;

							// Corner (1, 1, 1)
							xyzPointIndex = ijkGrid->getXyzPointIndexFromCellCorner(i, j, kInterface, 6) * 3 - indexShift;
							x = layerXyzPoints[xyzPointIndex];
							y = layerXyzPoints[xyzPointIndex + 1];
							z = layerXyzPoints[xyzPointIndex + 2];
							cout << "CORNER (1, 1, 1): " << x << " " << y << " " << z << std::endl;

							// Corner (0, 1, 1)
							xyzPointIndex = ijkGrid->getXyzPointIndexFromCellCorner(i, j, kInterface, 7) * 3 - indexShift;
							x = layerXyzPoints[xyzPointIndex];
							y = layerXyzPoints[xyzPointIndex + 1];
							z = layerXyzPoints[xyzPointIndex + 2];
							cout << "CORNER (0, 1, 1): " << x << " " << y << " " << z << std::endl;
						}
					}

					delete[] layerXyzPoints;
				}

				ijkGrid->unloadSplitInformation();

			}
			else
				cout << "Only explicit and parametric geometry is handled yet" << std::endl;
		}
	}
	cout << endl << "END: IJK GRID REP (hyperslabbing)" << endl;
}

/**
* Display the cell geometry of an IJK grid block.
* @param ijkGrid			An IJK grid.
* @param iInterfaceStart	Sarting i interface of the block.
* @param iInterfaceEnd		Ending i interface of the block.
* @param jInterfaceStart	Sarting j interface of the block.
* @param jInterfaceEnd		Ending j interface of the block.
* @param kInterfaceStart	Sarting k interface of the block.
* @param kInterfaceEnd		Ending k interface of the block.
* @param xyzPoints			The geometry of the block.
*/
void displayBlockCellGeometry(AbstractIjkGridRepresentation* ijkGrid,
	unsigned int iInterfaceStart, unsigned int iInterfaceEnd,
	unsigned int jInterfaceStart, unsigned int jInterfaceEnd,
	unsigned int kInterfaceStart, unsigned int kInterfaceEnd,
	double* xyzPoints)
{
	if (xyzPoints == nullptr)
		throw invalid_argument("xyzPoints == nullptr");

	for (unsigned int k = kInterfaceStart; k < kInterfaceEnd; k++)
	{
		cout << "LAYER: " << k << std::endl;

		for (unsigned int j = jInterfaceStart; j < jInterfaceEnd; j++)
		{
			for (unsigned int i = iInterfaceStart; i < iInterfaceEnd; i++)
			{
				cout << "CELL (" << i << ", " << j << ", " << k << ")" << std::endl;

				double x, y, z;

				// Corner (0, 0, 0)
				ijkGrid->getXyzPointOfBlockFromCellCorner(i, j, k, 0, xyzPoints, x, y, z);
				cout << "CORNER (0, 0, 0): " << x << " " << y << " " << z << std::endl;

				// Corner (1, 0, 0)
				ijkGrid->getXyzPointOfBlockFromCellCorner(i, j, k, 1, xyzPoints, x, y, z);
				cout << "CORNER (1, 0, 0): " << x << " " << y << " " << z << std::endl;

				// Corner (1, 1, 0)
				ijkGrid->getXyzPointOfBlockFromCellCorner(i, j, k, 2, xyzPoints, x, y, z);
				cout << "CORNER (1, 1, 0): " << x << " " << y << " " << z << std::endl;

				// Corner (0, 1, 0)
				ijkGrid->getXyzPointOfBlockFromCellCorner(i, j, k, 3, xyzPoints, x, y, z);
				cout << "CORNER (0, 1, 0): " << x << " " << y << " " << z << std::endl;

				// Corner (0, 0, 1)
				ijkGrid->getXyzPointOfBlockFromCellCorner(i, j, k, 4, xyzPoints, x, y, z);
				cout << "CORNER (0, 0, 1): " << x << " " << y << " " << z << std::endl;

				// Corner (1, 0, 1)
				ijkGrid->getXyzPointOfBlockFromCellCorner(i, j, k, 5, xyzPoints, x, y, z);
				cout << "CORNER (1, 0, 1): " << x << " " << y << " " << z << std::endl;

				// Corner (1, 1, 1)
				ijkGrid->getXyzPointOfBlockFromCellCorner(i, j, k, 6, xyzPoints, x, y, z);
				cout << "CORNER (1, 1, 1): " << x << " " << y << " " << z << std::endl;

				// Corner (0, 1, 1)
				ijkGrid->getXyzPointOfBlockFromCellCorner(i, j, k, 7, xyzPoints, x, y, z);
				cout << "CORNER (0, 1, 1): " << x << " " << y << " " << z << std::endl;
			}
		}
	}
}

/**
* Deserialize IJK grid explicit representations packed in a given EPC document.
* This method read grid geometry block by using hyperslabbing methods. Each block is read
* layer by layer.
* @param pck	An EPC document containing the IJK grid to deserialize.
*/
void deserializeGridHyperslabbingBlock(COMMON_NS::DataObjectRepository & pck)
{
	cout << endl << "BEGIN: IJK GRID REP (block hyperslabbing)" << endl;

	// ONE SUGAR
	AbstractIjkGridRepresentation* ijkGrid = static_cast<AbstractIjkGridRepresentation*>(pck.getDataObjectByUuid("e69bfe00-fa3d-11e5-b5eb-0002a5d5c51b"));
	if (ijkGrid == nullptr) {
		return;
	}

	cout << "--------------------------------------------------" << std::endl;
	showAllMetadata(ijkGrid);
	cout << "--------------------------------------------------" << std::endl;

	ijkGrid->loadSplitInformation();

	unsigned int iInterfaceStart = 0;
	unsigned int iInterfaceEnd = 1;
	unsigned int jInterfaceStart = 0;
	unsigned int jInterfaceEnd = 1;
	unsigned int kInterfaceStart = 0;
	unsigned int kInterfaceEnd = 1;

	ijkGrid->loadBlockInformation(iInterfaceStart, iInterfaceEnd, jInterfaceStart, jInterfaceEnd, kInterfaceStart, kInterfaceEnd);

	ULONG64 xyzPointCountOfBlock = ijkGrid->getXyzPointCountOfBlock();

	double* xyzPoints = new double[xyzPointCountOfBlock * 3];
	ijkGrid->getXyzPointsOfBlockOfPatch(0, xyzPoints);

	// Keep for testing
	/*cout << "All xyz points:" << endl;
	for (unsigned int index = 0; index < xyzPointCountOfBlock; ++index)
	std::cout << "(" << xyzPoints[3 * index] << " " << xyzPoints[3 * index + 1] << " " << xyzPoints[3 * index + 2] << ") ";
	std::cout << std::endl;*/

	displayBlockCellGeometry(ijkGrid,
		iInterfaceStart, iInterfaceEnd,
		jInterfaceStart, jInterfaceEnd,
		kInterfaceStart, kInterfaceEnd,
		xyzPoints
	);

	delete[] xyzPoints;

	ijkGrid->unloadSplitInformation();

	// Four by Three by Two Left Handed (e96c2bde-e3ae-4d51-b078-a8e57fb1e667)
	ijkGrid = static_cast<AbstractIjkGridRepresentation*>(pck.getDataObjectByUuid("e96c2bde-e3ae-4d51-b078-a8e57fb1e667"));

	cout << std::endl;
	cout << "--------------------------------------------------" << std::endl;
	showAllMetadata(ijkGrid);
	cout << "--------------------------------------------------" << std::endl;

	ijkGrid->loadSplitInformation();

	// 2*3*2 sized block with:
	//
	// kInterface = 0
	//  -- ** ** --
	// |  *  *  *  |
	//  -- ** ** --
	// |  *  *  *  |
	//  -- ** ** --
	// |  *  *  *  |
	//  -- ** ** --
	//
	// kInterface = 1
	//  -- ** ** --
	// |  *  *  *  |
	//  -- ** ** --
	// |  *  *  *  |
	//  -- ** ** --
	// |  *  *  *  |
	//  -- ** ** --
	//
	// kInterface = 2
	//  -- ** ** --
	// |  *  *  *  |
	//  -- ** ** --
	// |  *  *  *  |
	//  -- ** ** --
	// |  *  *  *  |
	//  -- ** ** --

	std::cout << "block 2*3*2" << std::endl;

	iInterfaceStart = 1;
	iInterfaceEnd = 3;
	jInterfaceStart = 0;
	jInterfaceEnd = 3;
	kInterfaceStart = 0;
	kInterfaceEnd = 2;

	ijkGrid->loadBlockInformation(iInterfaceStart, iInterfaceEnd, jInterfaceStart, jInterfaceEnd, kInterfaceStart, kInterfaceEnd);

	xyzPointCountOfBlock = ijkGrid->getXyzPointCountOfBlock();

	xyzPoints = new double[xyzPointCountOfBlock * 3];
	ijkGrid->getXyzPointsOfBlockOfPatch(0, xyzPoints);

	displayBlockCellGeometry(ijkGrid,
		iInterfaceStart, iInterfaceEnd,
		jInterfaceStart, jInterfaceEnd,
		kInterfaceStart, kInterfaceEnd,
		xyzPoints
	);

	delete[] xyzPoints;

	cout << "--------------------------------------------------" << std::endl;

	// 1*1*1 sized block with:
	//
	// kInterface = 0
	//  -- -- -- --
	// |  |  |  |  |
	//  -- -- -- --
	// |  |  |  |  |
	//  -- -- -- --
	// |  |  |  |  |
	//  -- -- -- --
	//
	// kInterface = 1
	//  -- -- -- --
	// |  |  |  |  |
	//  -- -- ** --
	// |  |  *  *  |
	//  -- -- ** --
	// |  |  |  |  |
	//  -- -- -- --
	//
	// kInterface = 2
	//  -- -- -- --
	// |  |  |  |  |
	//  -- -- ** --
	// |  |  *  *  |
	//  -- -- ** --
	// |  |  |  |  |
	//  -- -- -- --

	std::cout << "bloc 1*1*1" << std::endl;

	iInterfaceStart = 2;
	iInterfaceEnd = 3;
	jInterfaceStart = 1;
	jInterfaceEnd = 2;
	kInterfaceStart = 1;
	kInterfaceEnd = 2;

	ijkGrid->loadBlockInformation(iInterfaceStart, iInterfaceEnd, jInterfaceStart, jInterfaceEnd, kInterfaceStart, kInterfaceEnd);

	xyzPointCountOfBlock = ijkGrid->getXyzPointCountOfBlock();

	xyzPoints = new double[xyzPointCountOfBlock * 3];
	ijkGrid->getXyzPointsOfBlockOfPatch(0, xyzPoints);

	displayBlockCellGeometry(ijkGrid,
		iInterfaceStart, iInterfaceEnd,
		jInterfaceStart, jInterfaceEnd,
		kInterfaceStart, kInterfaceEnd,
		xyzPoints
	);

	delete[] xyzPoints;

	cout << "--------------------------------------------------" << std::endl;

	// 4*3*2 sized block with:
	//
	// kInterface = 0
	//  ** ** ** **
	// *  *  *  *  *
	//  ** ** ** **
	// *  *  *  *  *
	//  ** ** ** **
	// *  *  *  *  *
	//  ** ** ** **
	//
	// kInterface = 1
	//  ** ** ** **
	// *  *  *  *  *
	//  ** ** ** **
	// *  *  *  *  *
	//  ** ** ** **
	// *  *  *  *  *
	//  ** ** ** **
	//
	// kInterface = 2
	//  ** ** ** **
	// *  *  *  *  *
	//  ** ** ** **
	// *  *  *  *  *
	//  ** ** ** **
	// *  *  *  *  *
	//  ** ** ** **

	std::cout << "bloc 4*3*2" << std::endl;

	iInterfaceStart = 0;
	iInterfaceEnd = 4;
	jInterfaceStart = 0;
	jInterfaceEnd = 3;
	kInterfaceStart = 0;
	kInterfaceEnd = 2;

	ijkGrid->loadBlockInformation(iInterfaceStart, iInterfaceEnd, jInterfaceStart, jInterfaceEnd, kInterfaceStart, kInterfaceEnd);

	xyzPointCountOfBlock = ijkGrid->getXyzPointCountOfBlock();

	xyzPoints = new double[xyzPointCountOfBlock * 3];
	ijkGrid->getXyzPointsOfBlockOfPatch(0, xyzPoints);

	displayBlockCellGeometry(ijkGrid,
		iInterfaceStart, iInterfaceEnd,
		jInterfaceStart, jInterfaceEnd,
		kInterfaceStart, kInterfaceEnd,
		xyzPoints
	);

	delete[] xyzPoints;

	ijkGrid->unloadSplitInformation();

	// Four faulted sugar cubes(parametric geometry) (37c45c00-fa3e-11e5-a21e-0002a5d5c51b)
	ijkGrid = static_cast<AbstractIjkGridRepresentation*>(pck.getDataObjectByUuid("37c45c00-fa3e-11e5-a21e-0002a5d5c51b"));

	cout << std::endl;
	cout << "--------------------------------------------------" << std::endl;
	showAllMetadata(ijkGrid);
	cout << "--------------------------------------------------" << std::endl;

	ijkGrid->loadSplitInformation();

	// 1*1*1 sized block with:
	//
	// kInterface = 0
	//  -- -- 
	// |  |  |
	//  -- --
	//
	// kInterface = 1
	//  -- ** 
	// |  *  *
	//  -- **
	//
	// kInterface = 2
	//  -- ** 
	// |  *  *
	//  -- **

	std::cout << "bloc 1*1*1" << std::endl;

	iInterfaceStart = 1;
	iInterfaceEnd = 2;
	jInterfaceStart = 0;
	jInterfaceEnd = 1;
	kInterfaceStart = 1;
	kInterfaceEnd = 2;

	ijkGrid->loadBlockInformation(iInterfaceStart, iInterfaceEnd, jInterfaceStart, jInterfaceEnd, kInterfaceStart, kInterfaceEnd);

	xyzPointCountOfBlock = ijkGrid->getXyzPointCountOfBlock();

	xyzPoints = new double[xyzPointCountOfBlock * 3];
	ijkGrid->getXyzPointsOfBlockOfPatch(0, xyzPoints);

	displayBlockCellGeometry(ijkGrid,
		iInterfaceStart, iInterfaceEnd,
		jInterfaceStart, jInterfaceEnd,
		kInterfaceStart, kInterfaceEnd,
		xyzPoints
	);

	delete[] xyzPoints;

	cout << "--------------------------------------------------" << std::endl;

	// 1*1*2 sized block with:
	//
	// kInterface = 0
	//  ** -- 
	// *  *  |
	//  ** --
	//
	// kInterface = 1
	//  ** -- 
	// *  *  |
	//  ** --
	//
	// kInterface = 2
	//  ** -- 
	// *  *  |
	//  ** --

	std::cout << "bloc 1*1*2" << std::endl;

	iInterfaceStart = 0;
	iInterfaceEnd = 1;
	jInterfaceStart = 0;
	jInterfaceEnd = 1;
	kInterfaceStart = 0;
	kInterfaceEnd = 2;

	ijkGrid->loadBlockInformation(iInterfaceStart, iInterfaceEnd, jInterfaceStart, jInterfaceEnd, kInterfaceStart, kInterfaceEnd);

	xyzPointCountOfBlock = ijkGrid->getXyzPointCountOfBlock();

	xyzPoints = new double[xyzPointCountOfBlock * 3];
	ijkGrid->getXyzPointsOfBlockOfPatch(0, xyzPoints);

	displayBlockCellGeometry(ijkGrid,
		iInterfaceStart, iInterfaceEnd,
		jInterfaceStart, jInterfaceEnd,
		kInterfaceStart, kInterfaceEnd,
		xyzPoints
	);

	delete[] xyzPoints;

	ijkGrid->unloadSplitInformation();

	// Four faulted sugar cubes (straight parametric geometry) (f68235af-1d7a-4e24-93a8-10739b15ca40)
	ijkGrid = static_cast<AbstractIjkGridRepresentation*>(pck.getDataObjectByUuid("f68235af-1d7a-4e24-93a8-10739b15ca40"));

	cout << std::endl;
	cout << "--------------------------------------------------" << std::endl;
	showAllMetadata(ijkGrid);
	cout << "--------------------------------------------------" << std::endl;

	ijkGrid->loadSplitInformation();

	// 1*1*1 sized block with:
	//
	// kInterface = 0
	//  -- -- 
	// |  |  |
	//  -- --
	//
	// kInterface = 1
	//  -- ** 
	// |  *  *
	//  -- **
	//
	// kInterface = 2
	//  -- ** 
	// |  *  *
	//  -- **

	std::cout << "bloc 1*1*1" << std::endl;

	iInterfaceStart = 1;
	iInterfaceEnd = 2;
	jInterfaceStart = 0;
	jInterfaceEnd = 1;
	kInterfaceStart = 1;
	kInterfaceEnd = 2;

	ijkGrid->loadBlockInformation(iInterfaceStart, iInterfaceEnd, jInterfaceStart, jInterfaceEnd, kInterfaceStart, kInterfaceEnd);

	xyzPointCountOfBlock = ijkGrid->getXyzPointCountOfBlock();

	xyzPoints = new double[xyzPointCountOfBlock * 3];
	ijkGrid->getXyzPointsOfBlockOfPatch(0, xyzPoints);

	// Keep for testing
	/*cout << "All xyz points:" << endl;
	for (unsigned int index = 0; index < xyzPointCountOfBlock; ++index)
	std::cout << "(" << xyzPoints[3 * index] << " " << xyzPoints[3 * index + 1] << " " << xyzPoints[3 * index + 2] << ") ";
	std::cout << std::endl;*/

	displayBlockCellGeometry(ijkGrid,
		iInterfaceStart, iInterfaceEnd,
		jInterfaceStart, jInterfaceEnd,
		kInterfaceStart, kInterfaceEnd,
		xyzPoints
	);

	delete[] xyzPoints;

	cout << "--------------------------------------------------" << std::endl;

	// 1*1*2 sized block with:
	//
	// kInterface = 0
	//  ** -- 
	// *  *  |
	//  ** --
	//
	// kInterface = 1
	//  ** -- 
	// *  *  |
	//  ** --
	//
	// kInterface = 2
	//  ** -- 
	// *  *  |
	//  ** --

	std::cout << "bloc 1*1*2" << std::endl;

	iInterfaceStart = 0;
	iInterfaceEnd = 1;
	jInterfaceStart = 0;
	jInterfaceEnd = 1;
	kInterfaceStart = 0;
	kInterfaceEnd = 2;

	ijkGrid->loadBlockInformation(iInterfaceStart, iInterfaceEnd, jInterfaceStart, jInterfaceEnd, kInterfaceStart, kInterfaceEnd);

	xyzPointCountOfBlock = ijkGrid->getXyzPointCountOfBlock();

	xyzPoints = new double[xyzPointCountOfBlock * 3];
	ijkGrid->getXyzPointsOfBlockOfPatch(0, xyzPoints);

	displayBlockCellGeometry(ijkGrid,
		iInterfaceStart, iInterfaceEnd,
		jInterfaceStart, jInterfaceEnd,
		kInterfaceStart, kInterfaceEnd,
		xyzPoints
	);

	delete[] xyzPoints;

	cout << "--------------------------------------------------" << std::endl;

	// 2*1*2 sized block with:
	//
	// kInterface = 0
	//  ** ** 
	// *  *  *
	//  ** **
	//
	// kInterface = 1
	//  ** ** 
	// *  *  *
	//  ** **
	//
	// kInterface = 2
	//  ** ** 
	// *  *  *
	//  ** **

	std::cout << "bloc 2*1*2" << std::endl;

	iInterfaceStart = 0;
	iInterfaceEnd = 2;
	jInterfaceStart = 0;
	jInterfaceEnd = 1;
	kInterfaceStart = 0;
	kInterfaceEnd = 2;

	ijkGrid->loadBlockInformation(iInterfaceStart, iInterfaceEnd, jInterfaceStart, jInterfaceEnd, kInterfaceStart, kInterfaceEnd);

	xyzPointCountOfBlock = ijkGrid->getXyzPointCountOfBlock();

	xyzPoints = new double[xyzPointCountOfBlock * 3];
	ijkGrid->getXyzPointsOfBlockOfPatch(0, xyzPoints);

	displayBlockCellGeometry(ijkGrid,
		iInterfaceStart, iInterfaceEnd,
		jInterfaceStart, jInterfaceEnd,
		kInterfaceStart, kInterfaceEnd,
		xyzPoints
	);

	delete[] xyzPoints;

	ijkGrid->unloadSplitInformation();

	cout << endl << "END: IJK GRID REP (block hyperslabbing)" << endl;
}

/**
* This method provides a time comparison betweeen hyperslabbed and non-hyperslabbed versions of IJK grid geometry reading.
* This method is relevant only if the pillar information loading is forced at each
* IjkGridParametricRepresentation::getXyzPointsOfKInterfaceSequenceOfPatch call!
* @param ijkGrid	an IJK grid representation
* @param nbIter	number of geometry reading iteration
*/
void ijkGridHyperslabingTiming(AbstractIjkGridRepresentation* ijkGrid, unsigned int nbIter)
{
	if (ijkGrid == nullptr)
		throw invalid_argument("discretePropertyHyperslabingTiming: ijkGrid is nullptr.");

	cout << endl << "BEGIN: IJK GRID REP (hyperslabbed and non-hyperslabbed geometry reading comparison)" << std::endl << std::endl;
	cout.precision(17);

	ijkGrid->loadSplitInformation();
	double* xyzPoints = new double[ijkGrid->getXyzPointCountOfAllPatches() * 3];

	unsigned int iCellCount = ijkGrid->getICellCount();
	unsigned int jCellCount = ijkGrid->getJCellCount();
	unsigned int kCellCount = ijkGrid->getKCellCount();

	std::cout << "iCellCount = " << iCellCount << std::endl;
	std::cout << "jCellCount = " << jCellCount << std::endl;
	std::cout << "kCellCount = " << kCellCount << std::endl;

	ijkGrid->loadBlockInformation(0, iCellCount, 0, jCellCount, 0, kCellCount);

	time_t timeStart, timeEnd;
	clock_t nonHyperslabClockDuration = 0;
	clock_t hyperslabClockDuration = 0;
	clock_t blockHyperslabClockDuration = 0;
	double nonHyperslabTimeDuration = 0;
	double hyperslabTimeDuration = 0;
	double blockHyperslabTimeDuration = 0;

	const unsigned int smoothingConstant = 10;

	for (unsigned int i = 0; i < smoothingConstant; ++i)
	{
		clock_t clockStart, clockEnd;

		// non hyperslabbing
		clockStart = clock();
		time(&timeStart);
		for (unsigned int n = 0; n < nbIter; ++n)
			ijkGrid->getXyzPointsOfPatch(0, xyzPoints);
		clockEnd = clock();
		time(&timeEnd);
		nonHyperslabClockDuration += clockEnd - clockStart;
		nonHyperslabTimeDuration += difftime(timeEnd, timeStart);

		//// keep track for debug
		//for (unsigned int i = 0; i < ijkGrid->getXyzPointCountOfAllPatches(); ++i)
		//	std::cout << "(" << xyzPoints[i] << ", " << xyzPoints[i + 1] << ", " << xyzPoints[i + 2] << ")";
		//std::cout << std::endl;

		// k interface sequence hyperslabbing
		clockStart = clock();
		time(&timeStart);
		for (unsigned int n = 0; n < nbIter; ++n)
			ijkGrid->getXyzPointsOfKInterfaceSequenceOfPatch(0, kCellCount, 0, xyzPoints);
		clockEnd = clock();
		time(&timeEnd);
		hyperslabClockDuration += clockEnd - clockStart;
		hyperslabTimeDuration += difftime(timeEnd, timeStart);

		// block hyperslabbing
		clockStart = clock();
		time(&timeStart);
		for (unsigned int n = 0; n < nbIter; ++n)
			ijkGrid->getXyzPointsOfBlockOfPatch(0, xyzPoints);
		clockEnd = clock();
		time(&timeEnd);
		blockHyperslabClockDuration += clockEnd - clockStart;
		blockHyperslabTimeDuration += difftime(timeEnd, timeStart);
	}

	nonHyperslabClockDuration /= smoothingConstant;
	hyperslabClockDuration /= smoothingConstant;
	blockHyperslabClockDuration /= smoothingConstant;
	nonHyperslabTimeDuration /= smoothingConstant;
	hyperslabTimeDuration /= smoothingConstant;
	blockHyperslabTimeDuration /= smoothingConstant;

	// results
	std::cout << "Be careful: in the case of parametric grids, following results are relevant only if pillar information" << std::endl;
	std::cout << " loading is forced at each call of IjkGridParametricRepresentation::getXyzPointsOfKInterfaceSequenceOfPatch method" << std::endl;
	std::cout << "Non hyperslab:	geometry of " << ijkGrid->getTitle() << " have been read " << nbIter << " times in " << nonHyperslabClockDuration << " ticks (CPU time)" << std::endl;
	std::cout << "k-interface hyperslab: geometry of " << ijkGrid->getTitle() << " have been read " << nbIter << " times in " << hyperslabClockDuration << " ticks (CPU time)" << std::endl;
	std::cout << "Block hperslab: geometry of " << ijkGrid->getTitle() << " have been read " << nbIter << " times in " << blockHyperslabClockDuration << " ticks (CPU time)" << std::endl;
	std::cout << "k-interface hyperslab version took " << (hyperslabClockDuration * 100) / nonHyperslabClockDuration << " % of non hyperslab version" << std::endl;
	std::cout << "block hyperslab version took " << (blockHyperslabClockDuration * 100) / nonHyperslabClockDuration << " % of non hyperslab version" << std::endl;
	std::cout << "block hyperslab version took " << (blockHyperslabClockDuration * 100) / hyperslabClockDuration << " % of k-interface hyperslab version" << std::endl << std::endl;;

	std::cout << "Non hyperslab: geometry of " << ijkGrid->getTitle() << " have been read " << nbIter << " times in " << fixed << nonHyperslabTimeDuration << " seconds" << std::endl;
	std::cout << "k-interface hyperslab: geometry of " << ijkGrid->getTitle() << " have been read " << nbIter << " times in " << fixed << hyperslabTimeDuration << " seconds" << std::endl;
	std::cout << "block hyperslab: geometry of " << ijkGrid->getTitle() << " have been read " << nbIter << " times in " << fixed << blockHyperslabTimeDuration << " seconds" << std::endl;
	std::cout << "k-interface hyperslab version took " << (hyperslabTimeDuration * 100) / nonHyperslabTimeDuration << " % of non hyperslab version" << std::endl;
	std::cout << "block hyperslab version took " << (blockHyperslabTimeDuration * 100) / nonHyperslabTimeDuration << " % of non hyperslab version" << std::endl;
	std::cout << "block hyperslab version took " << (blockHyperslabTimeDuration * 100) / hyperslabTimeDuration << " % of k-interface hyperslab version" << std::endl;

	ijkGrid->unloadSplitInformation();
	delete[] xyzPoints;

	std::cout << endl << "END: IJK GRID REP (hyperslabbed and non-hyperslabbed geometry reading comparison)" << std::endl;
}

/**
* This method provides a time comparison betweeen hyperslabbed and non-hyperslabbed versions of IJK grid discrete property reading.
* It is assumed that poperty is cell centric.
* @param ijkGrid	an IJK grid representation
* @param prop		a dicrete property
* @param nbIter		number of geometry reading iteration
*/
void discretePropertyHyperslabingTiming(AbstractIjkGridRepresentation* ijkGrid, DiscreteProperty* prop, unsigned int nbIter)
{
	if (ijkGrid == nullptr)
		throw invalid_argument("discretePropertyHyperslabingTiming: ijkGrid is nullptr.");
	if (prop == nullptr)
		throw invalid_argument("discretePropertyHyperslabingTiming: prop is nullptr.");

	std::cout << endl << "BEGIN: IJK GRID REP (hyperslabbed and non-hyperslabbed property reading comparison)" << std::endl << std::endl;
	std::cout.precision(17);

	int* values = new int[prop->getValuesCountOfPatch(0)];

	unsigned long long* numValuesInEachDimension = new unsigned long long[3];
	numValuesInEachDimension[0] = ijkGrid->getKCellCount();
	numValuesInEachDimension[1] = ijkGrid->getJCellCount();
	numValuesInEachDimension[2] = ijkGrid->getICellCount();
	unsigned long long* offsetInEachDimension = new unsigned long long[3];
	offsetInEachDimension[0] = 0;
	offsetInEachDimension[1] = 0;
	offsetInEachDimension[2] = 0;

	time_t timeStart, timeEnd;
	clock_t nonHyperslabClockDuration = 0;
	double nonHyperslabTImeDuration = 0;
	clock_t hyperslabClockDuration = 0;
	double hyperslabTimeDuration = 0;

	const unsigned int smoothingConstant = 20;

	for (unsigned int i = 0; i < smoothingConstant; ++i)
	{
		clock_t clockStart, clockEnd;

		// non hyperslabbing
		clockStart = clock();
		time(&timeStart);
		for (unsigned int n = 0; n < nbIter; ++n)
			prop->getIntValuesOfPatch(0, values);
		clockEnd = clock();
		time(&timeEnd);
		nonHyperslabClockDuration = clockEnd - clockStart;
		nonHyperslabTImeDuration = difftime(timeEnd, timeStart);

		// hyperslabbing
		clockStart = clock();
		time(&timeStart);
		for (unsigned int n = 0; n < nbIter; ++n)
			prop->getIntValuesOfPatch(0, values, numValuesInEachDimension, offsetInEachDimension, 3);
		clockEnd = clock();
		time(&timeEnd);
		hyperslabClockDuration = clockEnd - clockStart;
		hyperslabTimeDuration = difftime(timeEnd, timeStart);
	}

	delete[] numValuesInEachDimension;
	delete[] offsetInEachDimension;

	nonHyperslabClockDuration /= smoothingConstant;
	nonHyperslabTImeDuration /= smoothingConstant;
	hyperslabClockDuration /= smoothingConstant;
	hyperslabTimeDuration /= smoothingConstant;

	// results
	std::cout << "Non hyperslab: property " << prop->getTitle() << " have been read " << nbIter << " times in " << nonHyperslabClockDuration << " ticks (CPU time)" << std::endl;
	std::cout << "Hyperslab:     property " << prop->getTitle() << " have been read " << nbIter << " times in " << hyperslabClockDuration << " ticks (CPU time)" << std::endl;
	double result = (hyperslabClockDuration * 100) / nonHyperslabClockDuration;
	std::cout << "hyperslab version took " << result << " % of non hyperslab version" << std::endl;
	std::cout << "Non hyperslab: property " << ijkGrid->getTitle() << " have been read " << nbIter << " times in " << fixed << nonHyperslabTImeDuration << " seconds" << std::endl;
	std::cout << "Hyperslab:     property " << ijkGrid->getTitle() << " have been read " << nbIter << " times in " << fixed << hyperslabTimeDuration << " seconds" << std::endl;
	result = (hyperslabTimeDuration * 100) / nonHyperslabTImeDuration;
	std::cout << "hyperslab version took " << result << " % of non hyperslab version" << std::endl;

	delete[] values;

	std::cout << endl << "END: IJK GRID REP (hyperslabbed and non-hyperslabbed property reading comparison)" << std::endl;
}

void deserializePerforations(COMMON_NS::DataObjectRepository & pck)
{
	cout << endl << "PERFORATIONS" << endl;

	WITSML2_0_NS::WellboreCompletion* wellboreCompletion = pck.getDataObjectByUuid<WITSML2_0_NS::WellboreCompletion>("7bda8ecf-2037-4dc7-8c59-db6ca09f2008");
	if (wellboreCompletion == nullptr) {
		return;
	}

	cout << "deserializing WellboreCompletion: " << wellboreCompletion->getTitle() << " (" << wellboreCompletion->getUuid() << ")" << std::endl;

	WITSML2_0_NS::WellCompletion* wellCompletion = wellboreCompletion->getWellCompletion();
	std::cout << "Associated with witsml well completion " << wellCompletion->getTitle()
		<< " with GUID " << wellCompletion->getUuid() << " and witsml well " << wellCompletion->getWell()->getTitle()
		<< " with GUID " << wellCompletion->getWell()->getUuid() << std::endl;

	witsmlWellbore = wellboreCompletion->getWellbore();
	std::cout << "Associated with witsml well bore " << witsmlWellbore->getTitle()
		<< " with GUID " << witsmlWellbore->getUuid() << " and witsml well " << witsmlWellbore->getWell()->getTitle()
		<< " with GUID " << witsmlWellbore->getWell()->getUuid() << std::endl;

	for (unsigned int perforationIndex = 0; perforationIndex < wellboreCompletion->getPerforationCount(); ++perforationIndex)
	{
		cout << std::endl << "perforation " << perforationIndex << " with uid \"" << wellboreCompletion->getPerforationUid(perforationIndex) << "\":" << std::endl;
		if (wellboreCompletion->hasPerforationMdDatum(perforationIndex))
		{
			cout << "datum: " << wellboreCompletion->getPerforationMdDatum(perforationIndex) << std::endl;
		}
		if (wellboreCompletion->hasPerforationMdUnit(perforationIndex))
		{
			cout << "md unit: " << wellboreCompletion->getPerforationMdUnitAsString(perforationIndex) << std::endl;
		}
		if (wellboreCompletion->hasPerforationTopMd(perforationIndex))
		{
			cout << "top md: " << wellboreCompletion->getPerforationTopMd(perforationIndex) << std::endl;
		}
		if (wellboreCompletion->hasPerforationBaseMd(perforationIndex))
		{
			cout << "base md: " << wellboreCompletion->getPerforationBaseMd(perforationIndex) << std::endl;
		}

		for (unsigned int historyIndex = 0; historyIndex < wellboreCompletion->getPerforationHistoryCount(perforationIndex); ++historyIndex)
		{
			cout << "history entry " << historyIndex << ":" << std::endl;
			if (wellboreCompletion->hasPerforationHistoryStatus(historyIndex, perforationIndex))
			{
				cout << "\tstatus: " << wellboreCompletion->getPerforationHistoryStatusToString(historyIndex, perforationIndex) << std::endl;
			}
			if (wellboreCompletion->hasPerforationHistoryStartDate(historyIndex, perforationIndex))
			{
				cout << "\tstart date: " << wellboreCompletion->getPerforationHistoryStartDate(historyIndex, perforationIndex) << std::endl;
			}
			if (wellboreCompletion->hasPerforationHistoryEndDate(historyIndex, perforationIndex))
			{
				cout << "\tend date: " << wellboreCompletion->getPerforationHistoryEndDate(historyIndex, perforationIndex) << std::endl;
			}
			if (wellboreCompletion->hasPerforationHistoryMdDatum(historyIndex, perforationIndex))
			{
				cout << "\tdatum: " << wellboreCompletion->getPerforationHistoryMdDatum(historyIndex, perforationIndex) << std::endl;
			}
			if (wellboreCompletion->hasPerforationHistoryMdUnit(historyIndex, perforationIndex))
			{
				cout << "\tmd unit: " << wellboreCompletion->getPerforationHistoryMdUnitAsString(historyIndex, perforationIndex) << std::endl;
			}
			if (wellboreCompletion->hasPerforationHistoryTopMd(historyIndex, perforationIndex))
			{
				cout << "\ttop md: " << wellboreCompletion->getPerforationHistoryTopMd(historyIndex, perforationIndex) << std::endl;
			}
			if (wellboreCompletion->hasPerforationHistoryBaseMd(historyIndex, perforationIndex))
			{
				cout << "\tbase md: " << wellboreCompletion->getPerforationHistoryBaseMd(historyIndex, perforationIndex) << std::endl;
			}
		}
	}
}

#ifdef WITH_EXPERIMENTAL
void deserializeGraphicalInformationSet(COMMON_NS::DataObjectRepository & pck)
{
	std::cout << "GRAPHICAL INFORMATIONS" << std::endl;

	std::vector<COMMON_NS::GraphicalInformationSet*> gisSet = pck.getDataObjects<COMMON_NS::GraphicalInformationSet>();
	for (unsigned int gisIndex = 0; gisIndex < gisSet.size(); ++gisIndex) {
		COMMON_NS::GraphicalInformationSet* graphicalInformationSet = gisSet[gisIndex];
		for (unsigned int i = 0; i < graphicalInformationSet->getGraphicalInformationSetCount(); ++i)
		{
			COMMON_NS::AbstractObject* targetObject = graphicalInformationSet->getTargetObject(i);

			std::cout << "graphical information for: " << targetObject->getTitle() << std::endl;

			if (graphicalInformationSet->hasDefaultColor(targetObject)) {
				std::cout << "default hue: " << graphicalInformationSet->getDefaultHue(targetObject) << std::endl;
				std::cout << "default saturation: " << graphicalInformationSet->getDefaultSaturation(targetObject) << std::endl;
				std::cout << "default value: " << graphicalInformationSet->getDefaultValue(targetObject) << std::endl;
				std::cout << "default alpha: " << graphicalInformationSet->getDefaultAlpha(targetObject) << std::endl;
				if (graphicalInformationSet->hasDefaultColorTitle(targetObject)) {
					std::cout << "default color title: " << graphicalInformationSet->getDefaultColorTitle(targetObject) << std::endl;
				}
			}

			if (graphicalInformationSet->hasDiscreteColorMap(targetObject)) {
				RESQML2_2_NS::DiscreteColorMap* discreteColorMap = graphicalInformationSet->getDiscreteColorMap(targetObject);
				std::cout << "discrete color map title: " << discreteColorMap->getTitle() << std::endl;
				unsigned int r, g, b;
				for (unsigned int colorIndex = 0; colorIndex < discreteColorMap->getColorCount(); ++colorIndex) {
					discreteColorMap->getRgbColor(colorIndex, r, g, b);
					std::cout << colorIndex << ": (" << r << ", " << g << ", " << b << ", ";
					std::cout << discreteColorMap->getAlpha(colorIndex);
					if (discreteColorMap->hasColorTitle(colorIndex)) {
						std::cout << ", " << discreteColorMap->getColorTitle(colorIndex);
					}
					std::cout << ")" << std::endl;
				}
				if (graphicalInformationSet->hasColorMapMinMax(targetObject)) {
					std::cout << "min: " << graphicalInformationSet->getColorMapMin(targetObject) << std::endl;
					std::cout << "max: " << graphicalInformationSet->getColorMapMax(targetObject) << std::endl;
				}
				if (graphicalInformationSet->hasValueVectorIndex(targetObject)) {
					std::cout << "value vector index: " << graphicalInformationSet->getValueVectorIndex(targetObject) << std::endl;
				}
			}

			if (graphicalInformationSet->hasContinuousColorMap(targetObject)) {
				RESQML2_2_NS::ContinuousColorMap* continuousColorMap = graphicalInformationSet->getContinuousColorMap(targetObject);
				std::cout << "continuous color map title: " << continuousColorMap->getTitle() << std::endl;
				unsigned int r, g, b;
				for (unsigned int mapIndex = 0; mapIndex < continuousColorMap->getColorCount(); ++mapIndex) {
					continuousColorMap->getRgbColor(mapIndex, r, g, b);
					std::cout << mapIndex << ": (" << r << ", " << g << ", " << b << ", ";
					std::cout << continuousColorMap->getAlpha(mapIndex);
					if (continuousColorMap->hasColorTitle(mapIndex)) {
						std::cout << ", " << continuousColorMap->getColorTitle(mapIndex);
					}
					std::cout << ")" << std::endl;
				}
				if (graphicalInformationSet->hasColorMapMinMax(targetObject)) {
					std::cout << "min: " << graphicalInformationSet->getColorMapMin(targetObject) << std::endl;
					std::cout << "max: " << graphicalInformationSet->getColorMapMax(targetObject) << std::endl;
				}
				if (graphicalInformationSet->hasValueVectorIndex(targetObject)) {
					std::cout << "value vector index: " << graphicalInformationSet->getValueVectorIndex(targetObject) << std::endl;
				}
			}
		}
	}
}
#endif

void deserialize(const string & inputFile)
{
	COMMON_NS::EpcDocument pck(inputFile);
	//COMMON_NS::EpcDocument pck(inputFile, "C:/Users/Philippe/dev/fesapiEnv/fesapi/resources");
	//COMMON_NS::EpcDocument pck(inputFile, "/home/philippe/dev/fesapi/resources");
	cout << "Start deserialization of " << pck.getName() << " in " << (pck.getStorageDirectory().empty() ? "working directory." : pck.getStorageDirectory()) << endl;
	COMMON_NS::DataObjectRepository repo;
	string resqmlResult = pck.deserializeInto(repo);
	if (!resqmlResult.empty()) {
		cerr << resqmlResult << endl;
		throw invalid_argument("The epc document is not a valid one");
	}

	const unsigned int hdfProxyCount = repo.getHdfProxyCount();
	cout << "There are " << repo.getHdfProxyCount() << " hdf files associated to this epc document." << endl;
	for (unsigned int hdfProxyIndex = 0; hdfProxyIndex < hdfProxyCount; ++hdfProxyIndex) {
		cout << "Hdf file relative path : " << repo.getHdfProxy(hdfProxyIndex)->getRelativePath() << endl;
	}
	for (size_t warningIndex = 0; warningIndex < repo.getWarnings().size(); ++warningIndex) {
		cout << "Warning #" << warningIndex << " : " << repo.getWarnings()[warningIndex] << endl;
	}

#if !defined(OFFICIAL)
	//deserializePropertyKindMappingFiles(&pck);
#endif

	COMMON_NS::EnumStringMapper enumStrMapper;
	cout << "MAPPING ENUM VS STRING" << endl;
	cout << "rock permeability == " << enumStrMapper.getEnergisticsPropertyKindName(enumStrMapper.getEnergisticsPropertyKind("rock permeability")) << endl;
	cout << "m (meter) == " << enumStrMapper.getEnergisticsUnitOfMeasureName(enumStrMapper.getEnergisticsUnitOfMeasure("m")) << endl;

	cout << "EXTENDED CORE PROPERTIES" << endl;

	unordered_map<string, string> & extendedCoreProperty = pck.getExtendedCoreProperty();
	for (unordered_map<string, string>::const_iterator it = extendedCoreProperty.begin(); it != extendedCoreProperty.end(); ++it) {
		cout << it->first.c_str() << " " << it->second.c_str() << endl;
	}

	cout << "CRS" << endl;
	vector<LocalDepth3dCrs*> depthCrsSet = repo.getLocalDepth3dCrsSet();
	for (size_t i = 0; i < depthCrsSet.size(); ++i) {
		cout << "Title is : " << depthCrsSet[i]->getTitle() << endl;
		if (depthCrsSet[i]->isProjectedCrsDefinedWithEpsg())
			cout << "Projected : EPSG " << depthCrsSet[i]->getProjectedCrsEpsgCode() << endl;
		else if (depthCrsSet[i]->isProjectedCrsUnknown())
			cout << "Projected : Unknown." << "Reason is:" << depthCrsSet[i]->getProjectedCrsUnknownReason() << endl;
	}
	vector<LocalTime3dCrs*> timeCrsSet = repo.getLocalTime3dCrsSet();
	for (size_t i = 0; i < timeCrsSet.size(); ++i) {
		cout << "Title is : " << timeCrsSet[i]->getTitle() << endl;
		if (timeCrsSet[i]->isVerticalCrsDefinedWithEpsg())
			cout << "Vertical : EPSG one" << endl;
		else if (timeCrsSet[i]->isVerticalCrsUnknown())
			cout << "Vertical : Unknown." << "Reason is:" << timeCrsSet[i]->getVerticalCrsUnknownReason() << endl;
	}
	cout << endl;

	deserializeGeobody(&repo);
	deserializeFluidBoundary(repo);
	deserializeRockFluidOrganization(repo);

	std::vector<TectonicBoundaryFeature*> faultSet = repo.getFaultSet();
	std::vector<PolylineSetRepresentation *> faultPolyRep = repo.getFaultPolylineSetRepSet();
	std::vector<TriangulatedSetRepresentation *> faultTriRepSet = repo.getFaultTriangulatedSetRepSet();
	std::vector<Horizon*> horizonSet = repo.getHorizonSet();
	std::vector<Grid2dRepresentation *> horizonGrid2dSet = repo.getHorizonGrid2dRepSet();
	std::vector<TriangulatedSetRepresentation *> horizonTriRepSet = repo.getHorizonTriangulatedSetRepSet();
	std::vector<TriangulatedSetRepresentation*> unclassifiedTriRepSet = repo.getUnclassifiedTriangulatedSetRepSet();
	std::vector<PolylineRepresentation *> horizonSinglePolylineRepSet = repo.getHorizonPolylineRepSet();
	std::vector<WellboreFeature*> wellboreSet = repo.getWellboreSet();
	std::vector<WellboreTrajectoryRepresentation *> wellboreCubicTrajSet = repo.getWellboreTrajectoryRepresentationSet();
	std::vector<UnstructuredGridRepresentation*> unstructuredGridRepSet = repo.getUnstructuredGridRepresentationSet();
	std::vector<RESQML2_NS::TimeSeries*> timeSeriesSet = repo.getTimeSeriesSet();
	std::vector<StratigraphicColumn*> stratiColumnSet = repo.getStratigraphicColumnSet();
	std::vector<RESQML2_NS::RepresentationSetRepresentation*> representationSetRepresentationSet = repo.getRepresentationSetRepresentationSet();
	std::vector<RESQML2_NS::SubRepresentation*> subRepresentationSet = repo.getSubRepresentationSet();
	std::vector<PolylineSetRepresentation *> frontierPolyRep = repo.getFrontierPolylineSetRepSet();

	std::cout << "RepresentationSetRepresentation" << endl;
	for (size_t i = 0; i < representationSetRepresentationSet.size(); i++) {
		showAllMetadata(representationSetRepresentationSet[i]);
		std::cout << "Is homogeneous : " << representationSetRepresentationSet[i]->isHomogeneous() << std::endl;
		std::cout << "Representation count : " << representationSetRepresentationSet[i]->getRepresentationCount() << std::endl;
		showAllMetadata(representationSetRepresentationSet[i]->getRepresentation(0));
		std::cout << "--------------------------------------------------" << std::endl;
	}

	std::cout << "FAULTS" << endl;
	for (size_t i = 0; i < faultSet.size(); ++i) {
		TectonicBoundaryFeature const * faultFeature = faultSet[i];
		showAllMetadata(faultFeature);
		std::cout << "InterpretationCount : " << faultFeature->getInterpretationCount() << std::endl;
	}

	std::cout << faultPolyRep.size() << " FAULT POLYLINE SET REP" << endl;
	for (size_t i = 0; i < faultPolyRep.size(); ++i) {
		showAllMetadata(faultPolyRep[i]);
		ULONG64 nodeCount = faultPolyRep[i]->getXyzPointCountOfAllPatches();
		double* allXyzPoints = new double[nodeCount * 3];
		faultPolyRep[i]->getXyzPointsOfAllPatchesInGlobalCrs(allXyzPoints);
		for (ULONG64 nodeIndex = 0; nodeIndex < 6; nodeIndex += 3) {
			std::cout << allXyzPoints[nodeIndex] << " " << allXyzPoints[nodeIndex + 1] << " " << allXyzPoints[nodeIndex + 2] << endl;
		}
		delete[] allXyzPoints;
		deserializeActivity(faultPolyRep[i]);

		if (faultPolyRep[i]->areAllPolylinesClosedOfAllPatches()) {
			std::cout << "All polylines of the fault are closed" << endl;
		}
		/*
		std::cout << "\tSEISMIC INFO" << endl;
		double* inlines = new double[nodeCount];
		faultPolyRep[i]->getInlinesOfPointsOfPatch(0, inlines);
		for (unsigned int index = 0; index < nodeCount; index++)
		{
		std::cout << "\tinline : " << inlines[index] << std::endl;
		}
		delete [] inlines;
		double* crosslines = new double[nodeCount];;
		faultPolyRep[i]->getCrosslinesOfPointsOfPatch(0, crosslines);
		for (unsigned int index = 0; index < nodeCount; index++)
		{
		std::cout << "\tcrossline : " << crosslines[index] << std::endl;
		}
		delete [] crosslines;
		*/

		showAllProperties(faultPolyRep[i]);

		for (size_t gsrIndex = 0; gsrIndex < faultPolyRep[i]->getInterpretation()->getGridConnectionSetRepresentationSet().size(); ++gsrIndex) {
			std::cout << "This fault polyline rep is linked to a grid connection set." << std::endl;
		}
	}

	std::cout << faultPolyRep.size() << " FRONTIER POLYLINE SET REP" << endl;
	for (size_t i = 0; i < frontierPolyRep.size(); ++i) {
		showAllMetadata(frontierPolyRep[i]);

		bool* closedFlag = new bool[frontierPolyRep[i]->getPolylineCountOfPatch(0)];
		frontierPolyRep[i]->getClosedFlagPerPolylineOfPatch(0, closedFlag);
		for (unsigned int j = 0; j < frontierPolyRep[i]->getPolylineCountOfPatch(0); ++j) {
			std::cout << "closed flag : " << closedFlag[j] << endl;
		}
		delete[] closedFlag;
	}

	std::cout << "FAULTS TRI REP" << endl;
	for (size_t i = 0; i < faultTriRepSet.size(); i++) {
		showAllMetadata(faultTriRepSet[i]);

		ULONG64 pointCount = faultTriRepSet[i]->getXyzPointCountOfAllPatches();
		unsigned int triangleCount = faultTriRepSet[i]->getTriangleCountOfAllPatches();
		cout << "point Count " << pointCount << endl;
		cout << "triangle Count " << triangleCount << endl;

		std::cout << "\tFAULTS TRI REP GEOMETRY" << endl;
		double* xyzPoints = new double[pointCount * 3];
		faultTriRepSet[i]->getXyzPointsOfAllPatchesInGlobalCrs(xyzPoints);
		unsigned int * triangleIndices = new unsigned int[triangleCount * 3];
		faultTriRepSet[i]->getTriangleNodeIndicesOfAllPatches(triangleIndices);
		for (size_t j = 0; j < 5; j++) {
			std::cout << "\txyzPoints : " << xyzPoints[j] << std::endl;
			std::cout << "\ttriangleIndices : " << triangleIndices[j] << std::endl;
			std::cout << "\t--------------------------------------------------" << std::endl;
		}
		deserializeActivity(faultTriRepSet[i]);

		delete[] xyzPoints;
		delete[] triangleIndices;

		showAllProperties(faultTriRepSet[i]);
	}

	std::cout << "HORIZONS" << endl;
	for (size_t i = 0; i < horizonSet.size(); i++) {
		Horizon const * horFeature = horizonSet[i];
		showAllMetadata(horFeature);
		if (horFeature->hasAnAge()) {
			cout << "Age " << horFeature->getAge() << " years" << endl;
		}
		std::cout << std::endl;
		std::cout << "InterpretationCount : " << horFeature->getInterpretationCount() << std::endl;
	}

	std::cout << "HORIZONS GRID 2D REP" << endl;
	for (size_t i = 0; i < horizonGrid2dSet.size(); i++) {
		showAllMetadata(horizonGrid2dSet[i]);

		cout << "X origin " << horizonGrid2dSet[i]->getXOriginInGlobalCrs() << endl;
		cout << "Y origin " << horizonGrid2dSet[i]->getYOriginInGlobalCrs() << endl;

		cout << "I Node Count " << horizonGrid2dSet[i]->getNodeCountAlongIAxis() << endl;
		cout << "J Node Count " << horizonGrid2dSet[i]->getNodeCountAlongJAxis() << endl;
		double* zValues = new double[horizonGrid2dSet[i]->getNodeCountAlongIAxis() * horizonGrid2dSet[i]->getNodeCountAlongJAxis()];
		horizonGrid2dSet[i]->getZValuesInGlobalCrs(zValues);
		std::cout << "First zValue is : " << zValues[0] << std::endl;
		std::cout << "Second zValue is : " << zValues[1] << std::endl;
		std::cout << "Third zValue is : " << zValues[2] << std::endl;
		std::cout << "Fourth zValue is : " << zValues[3] << std::endl;
		std::cout << "Fifth zValue is : " << zValues[4] << std::endl;
		if (horizonGrid2dSet[i]->getSupportingRepresentation() != nullptr) {
			horizonGrid2dSet[i]->getSupportingRepresentation()->getZValuesInGlobalCrs(zValues);
			std::cout << "Supporting Representation first zValue is : " << zValues[0] << std::endl;
			std::cout << "Supporting Representation second zValue is : " << zValues[1] << std::endl;
			std::cout << "Supporting Representation third zValue is : " << zValues[2] << std::endl;
			std::cout << "Supporting Representation fourth zValue is : " << zValues[3] << std::endl;
			std::cout << "Supporting Representation fifth zValue is : " << zValues[4] << std::endl;
		}
		delete[] zValues;
		cout << "XIOffset : " << horizonGrid2dSet[i]->getXIOffsetInGlobalCrs() << endl;
		cout << "YIOffset : " << horizonGrid2dSet[i]->getYIOffsetInGlobalCrs() << endl;
		cout << "XJOffset : " << horizonGrid2dSet[i]->getXJOffsetInGlobalCrs() << endl;
		cout << "YJOffset : " << horizonGrid2dSet[i]->getYJOffsetInGlobalCrs() << endl;
		if (horizonGrid2dSet[i]->isISpacingConstant()) {
			cout << "Spacing in I is constant : " << horizonGrid2dSet[i]->getISpacing() << endl;
		}
		else {
			cout << "Spacing in I is not constant." << endl;
		}
		if (horizonGrid2dSet[i]->isJSpacingConstant()) {
			cout << "Spacing in J is constant : " << horizonGrid2dSet[i]->getJSpacing() << endl;
		}
		else {
			cout << "Spacing in J is not constant." << endl;
		}

		deserializeActivity(horizonGrid2dSet[i]);
		showAllProperties(horizonGrid2dSet[i]);
	}

	std::cout << "HORIZONS TRI REP" << endl;
	for (size_t i = 0; i < horizonTriRepSet.size(); i++) {
		showAllMetadata(horizonTriRepSet[i]);

		const ULONG64 pointCount = horizonTriRepSet[i]->getXyzPointCountOfAllPatches();
		unsigned int triangleCount = horizonTriRepSet[i]->getTriangleCountOfAllPatches();
		cout << "point Count " << pointCount << endl;
		cout << "triangle Count " << triangleCount << endl;

		std::cout << "\tHORIZONS TRI REP GEOMETRY" << endl;
		double* xyzPoints = new double[pointCount * 3];
		horizonTriRepSet[i]->getXyzPointsOfAllPatchesInGlobalCrs(xyzPoints);
		unsigned int * triangleIndices = new unsigned int[triangleCount * 3];
		horizonTriRepSet[i]->getTriangleNodeIndicesOfAllPatches(triangleIndices);
		for (unsigned int j = 0; j < 5; j++)
		{
			std::cout << "\txyzPoints : " << xyzPoints[j] << std::endl;
			std::cout << "\ttriangleIndices : " << triangleIndices[j] << std::endl;
			std::cout << "\t--------------------------------------------------" << std::endl;
		}

		delete[] xyzPoints;
		delete[] triangleIndices;

		const unsigned int patchCount = horizonTriRepSet[i]->getPatchCount();
		for (unsigned int patchIndex = 0; patchIndex < patchCount; ++patchIndex) {
			RESQML2_NS::AbstractRepresentation* seismicSupport = horizonTriRepSet[i]->getSeismicSupportOfPatch(patchIndex);
			if (seismicSupport != nullptr) {
				const ULONG64 pointCountForPatch = horizonTriRepSet[i]->getXyzPointCountOfPatch(patchIndex);
				cout << "Seismic support of patch " << patchIndex << " is : " << seismicSupport->getTitle() << endl;
				double* inlines = new double[pointCountForPatch];
				horizonTriRepSet[i]->getInlinesOfPointsOfPatch(patchIndex, inlines);
				for (unsigned int index = 0; index < pointCountForPatch; index++)
				{
					std::cout << "\tinline : " << inlines[index] << std::endl;
				}
				delete[] inlines;
				double* crosslines = new double[pointCountForPatch];;
				horizonTriRepSet[i]->getCrosslinesOfPointsOfPatch(patchIndex, crosslines);
				for (unsigned int index = 0; index < pointCountForPatch; index++)
				{
					std::cout << "\tcrossline : " << crosslines[index] << std::endl;
				}
				delete[] crosslines;
			}
			else {
				cout << "No seismic support for patch " << patchIndex << endl;
			}
		}

		deserializeActivity(horizonTriRepSet[i]);
		showAllProperties(horizonTriRepSet[i]);
	}

	std::cout << "UNCLASSIFIED TRI REP" << endl;
	for (size_t i = 0; i < unclassifiedTriRepSet.size(); i++) {
		showAllMetadata(unclassifiedTriRepSet[i]);
		deserializeActivity(unclassifiedTriRepSet[i]);
		showAllProperties(unclassifiedTriRepSet[i]);
	}

	std::cout << "HORIZONS SINGLE POLYLINE REP" << endl;
	for (size_t i = 0; i < horizonSinglePolylineRepSet.size(); i++)
	{
		showAllMetadata(horizonSinglePolylineRepSet[i]);

		const unsigned int patchCount = horizonSinglePolylineRepSet[i]->getPatchCount();
		for (unsigned int patchIndex = 0; patchIndex < patchCount; ++patchIndex) {
			RESQML2_NS::AbstractRepresentation* seismicSupport = horizonSinglePolylineRepSet[i]->getSeismicSupportOfPatch(patchIndex);
			if (seismicSupport != nullptr) {
				cout << "Seismic support of patch " << patchIndex << " is : " << seismicSupport->getTitle() << endl;
				double * lineAbscissa = new double[horizonSinglePolylineRepSet[i]->getXyzPointCountOfPatch(patchIndex)];
				horizonSinglePolylineRepSet[i]->getSeismicLineAbscissaOfPointsOfPatch(patchIndex, lineAbscissa);

				for (ULONG64 j = 0; j < horizonSinglePolylineRepSet[i]->getXyzPointCountOfPatch(patchIndex); j++)
				{
					std::cout << "line Abscissa : " << lineAbscissa[j] << std::endl;
				}
				delete[] lineAbscissa;
			}
		}

		showAllProperties(horizonSinglePolylineRepSet[i]);
	}

	deserializeSealedSurfaceFramework(repo);
	deserializeSealedVolumeFramework(repo);

	std::cout << "STRATI COLUMN" << endl;
	for (size_t i = 0; i < stratiColumnSet.size(); i++)
	{
		deserializeStratiColumn(stratiColumnSet[i]);
	}

	std::cout << "WELLBORES" << endl;
	for (size_t i = 0; i < wellboreSet.size(); i++)
	{
		showAllMetadata(wellboreSet[i]);
		witsmlWellbore = wellboreSet[i]->getWitsmlWellbore();
		if (witsmlWellbore != nullptr)
		{
			std::cout << "Associated with witsml well bore " << witsmlWellbore->getTitle()
				<< " with GUID " << witsmlWellbore->getUuid() << " and witsml well " << witsmlWellbore->getWell()->getTitle()
				<< " with GUID " << witsmlWellbore->getWell()->getUuid() << std::endl;
		}
		for (size_t j = 0; j < wellboreSet[i]->getInterpretationSet().size(); j++)
		{
			for (size_t k = 0; k < wellboreSet[i]->getInterpretationSet()[j]->getRepresentationSet().size(); k++)
			{
				if (wellboreSet[i]->getInterpretationSet()[j]->getRepresentationSet()[k]->getXmlTag() == WellboreMarkerFrameRepresentation::XML_TAG)
				{
					WellboreMarkerFrameRepresentation const * wmf = static_cast<WellboreMarkerFrameRepresentation const *>(wellboreSet[i]->getInterpretationSet()[j]->getRepresentationSet()[k]);
					vector<WellboreMarker *> marketSet = wmf->getWellboreMarkerSet();
					for (size_t markerIndex = 0; markerIndex < marketSet.size(); ++markerIndex)
					{
						std::cout << "marker : " << marketSet[markerIndex]->getTitle() << std::endl;
						if (marketSet[markerIndex]->getBoundaryFeatureInterpretation() != nullptr) {
							std::cout << "marker boundary feature : " << marketSet[markerIndex]->getBoundaryFeatureInterpretation()->getTitle() << std::endl;
						}
					}

					for (size_t l = 0; l < wmf->getPropertySet().size(); ++l)
					{
						if (wmf->getPropertySet()[l]->getXmlTag() == CategoricalProperty::XML_TAG)
						{
							CategoricalProperty const * catVal = static_cast<CategoricalProperty const *>(wmf->getPropertySet()[l]);
							if (catVal->getValuesHdfDatatype() == RESQML2_NS::AbstractValuesProperty::LONG_64)
							{
								std::cout << "Hdf datatype is NATIVE LONG" << std::endl;
								LONG64* tmp = new LONG64[wmf->getMdValuesCount()];
								catVal->getLongValuesOfPatch(0, tmp);
								for (size_t ind = 0; ind < 2; ind++)
									std::cout << "Value " << ind << " : " << tmp[ind] << std::endl;
								delete[] tmp;
							}
							else if (catVal->getValuesHdfDatatype() == RESQML2_NS::AbstractValuesProperty::INT)
							{
								std::cout << "Hdf datatype is NATIVE INT" << std::endl;
							}
						}
					}
				}
			}
		}
	}

	deserializePerforations(repo);

	std::cout << endl << "WELLBORES CUBIC TRAJ" << endl;
	for (size_t i = 0; i < wellboreCubicTrajSet.size(); ++i)
	{
		showAllMetadata(wellboreCubicTrajSet[i]);
		std::cout << "MD Datum is : " << wellboreCubicTrajSet[i]->getMdDatum()->getTitle() << std::endl;
		std::cout << "--------------------------------------------------" << std::endl;
		if (wellboreCubicTrajSet[i]->getXyzPointCountOfAllPatches() == 0) {
			break;
		}
		double* mdValues = new double[wellboreCubicTrajSet[i]->getXyzPointCountOfAllPatches()];
		wellboreCubicTrajSet[i]->getMdValues(mdValues);
		double* xyzPt = new double[wellboreCubicTrajSet[i]->getXyzPointCountOfAllPatches() * 3];
		wellboreCubicTrajSet[i]->getXyzPointsOfAllPatchesInGlobalCrs(xyzPt);
		for (size_t j = 0; j < wellboreCubicTrajSet[i]->getXyzPointCountOfAllPatches() * 3 && j < 10; j += 3)
		{
			cout << "Trajectory station : MD=" << mdValues[j / 3] << " X=" << xyzPt[j] << " Y=" << xyzPt[j + 1] << " Z=" << xyzPt[j + 2] << endl;
		}
		delete[] mdValues;
		delete[] xyzPt;
		std::cout << "LOGS" << endl;
		std::cout << "--------------------------------------------------" << std::endl;
		std::vector<WellboreFrameRepresentation *> wellboreFrameSet = wellboreCubicTrajSet[i]->getWellboreFrameRepresentationSet();
		for (size_t j = 0; j < wellboreFrameSet.size(); j++)
		{
			showAllMetadata(wellboreFrameSet[j]);
			std::cout << "Value Count : " << wellboreFrameSet[j]->getMdValuesCount() << endl;
			if (wellboreFrameSet[j]->areMdValuesRegularlySpaced())
			{
				std::cout << "Regularly spaced" << std::endl;
				std::cout << "First Value : " << wellboreFrameSet[j]->getMdFirstValue() << endl;
				std::cout << "Increment : " << wellboreFrameSet[j]->getMdConstantIncrementValue() << endl;
			}
			else
			{
				std::cout << "Iregularly spaced" << std::endl;
			}
			if (wellboreFrameSet[j]->getMdHdfDatatype() == RESQML2_NS::AbstractValuesProperty::DOUBLE)
				std::cout << "Hdf datatype is NATIVE DOUBLE" << std::endl;
			else if (wellboreFrameSet[j]->getMdHdfDatatype() == RESQML2_NS::AbstractValuesProperty::FLOAT)
				std::cout << "Hdf datatype is NATIVE FLOAT" << std::endl;
			else if (wellboreFrameSet[j]->getMdHdfDatatype() == RESQML2_NS::AbstractValuesProperty::UNKNOWN)
				std::cout << "Hdf datatype is UNKNOWN" << std::endl;
		}
	}

	std::cout << endl << "IJK GRID REP" << endl;
	unsigned int ijkGridCount = repo.getIjkGridRepresentationCount();
	for (unsigned int ijkGridIdx = 0; ijkGridIdx < ijkGridCount; ++ijkGridIdx)
	{
		AbstractIjkGridRepresentation* ijkGrid = repo.getIjkGridRepresentation(ijkGridIdx);

		showAllMetadata(ijkGrid);
		if (ijkGrid->isPartial()) {
			continue;
		}
		std::cout << "This ijk grid is ";
		if (!ijkGrid->isNodeGeometryCompressed()) {
			std::cout << "not ";
		}
		std::cout << "compressed." << std::endl;

		if (ijkGrid->getGeometryKind() != AbstractIjkGridRepresentation::NO_GEOMETRY)
		{
			std::cout << "Most complex pillar geometry is ";
			gsoap_resqml2_0_1::resqml20__PillarShape mostcomplexPillarGeom = ijkGrid->getMostComplexPillarGeometry();
			if (mostcomplexPillarGeom == gsoap_resqml2_0_1::resqml20__PillarShape__vertical) {
				std::cout << "vertical";
			}
			else if (mostcomplexPillarGeom == gsoap_resqml2_0_1::resqml20__PillarShape__straight) {
				std::cout << "straight";
			}
			else if (mostcomplexPillarGeom == gsoap_resqml2_0_1::resqml20__PillarShape__curved) {
				std::cout << "curved";
			}
			std::cout << std::endl;

			if (ijkGrid->getGeometryKind() == AbstractIjkGridRepresentation::LATTICE) {
				std::cout << "This 3d grid has a lattice geometry." << std::endl;
			}
			else {
				if (ijkGrid->getGeometryKind() == AbstractIjkGridRepresentation::PARAMETRIC)
				{
					std::cout << "This 3d grid has a parametric geometry." << std::endl;
					IjkGridParametricRepresentation* paramIjkGrid = static_cast<IjkGridParametricRepresentation*>(ijkGrid);
					if (paramIjkGrid->isParametricLineKindConstant())
					{
						std::cout << "Constant parametric line kind : " << paramIjkGrid->getConstantParametricLineKind() << std::endl;
					}
					else
					{
						std::cout << "Non constant parametric line kind" << std::endl;
						short* pillarKind = new short[paramIjkGrid->getPillarCount()];
						paramIjkGrid->getParametricLineKind(pillarKind);
						for (size_t pillarIndex = 0; pillarIndex < paramIjkGrid->getPillarCount() && pillarIndex < 10; ++pillarIndex) {
							cout << "Pillar index " << pillarIndex << " with kind " << pillarKind[pillarIndex] << endl;
						}
						delete[] pillarKind;
					}

					unsigned int patchCount = ijkGrid->getPatchCount();
					for (unsigned int currentPatch = 0; currentPatch < patchCount; ++currentPatch) {
						ULONG64 nbVertex = ijkGrid->getXyzPointCountOfPatch(currentPatch);

						double* xyzPts = new double[nbVertex * 3];
						ijkGrid->getXyzPointsOfPatch(currentPatch, xyzPts);

						for (size_t vIndex = 0; vIndex < nbVertex && vIndex < 10; ++vIndex) {
							double x = xyzPts[vIndex * 3];
							double y = xyzPts[vIndex * 3 + 1];
							double z = xyzPts[vIndex * 3 + 2];
							std::cout << x << " " << y << " " << z << std::endl;
						}

						delete[] xyzPts;
					}
					/*
					ULONG64 pointCountByInterface = paramIjkGrid->getXyzPointCountOfKInterfaceOfPatch(0);
					double* interfaceXyzPoints = new double[pointCountByInterface * 3];
					paramIjkGrid->getXyzPointsOfKInterfaceOfPatch(0, 0, interfaceXyzPoints);
					delete[] interfaceXyzPoints;
					*/
				}
				else if (ijkGrid->getGeometryKind() == AbstractIjkGridRepresentation::EXPLICIT)
				{
					std::cout << "This 3d grid has an explicit geometry." << std::endl;
				}
				else
				{
					std::cout << "This 3d grid has an unknown geometry." << std::endl;
				}

				// read points
				ULONG64 xyzPointCount = ijkGrid->getXyzPointCountOfAllPatches();
				std::cout << "\t XYZ points count :" << xyzPointCount << std::endl;
				std::cout << "\t Start reading XYZ points..." << std::endl;
				double* xyzPoints = new double[xyzPointCount * 3];
				ijkGrid->getXyzPointsOfAllPatches(xyzPoints);
				delete[] xyzPoints;
				std::cout << "\t Stop reading XYZ points :" << std::endl;

				std::cout << "Split coordinate line count is : " << ijkGrid->getSplitCoordinateLineCount() << std::endl;
			}
		}
		else
			std::cout << "This 3d grid has no geometry." << std::endl;

		if (ijkGrid->getInterpretation())
		{
			std::cout << "Interpretation is : " << ijkGrid->getInterpretation()->getTitle() << std::endl;
			if (ijkGrid->getInterpretation()->getInterpretedFeature())
				std::cout << "Feature is : " << ijkGrid->getInterpretation()->getInterpretedFeature()->getTitle() << std::endl;
			else
				std::cout << " NO Feature" << std::endl;
		}
		else
			std::cout << " NO interpretation" << std::endl;

		for (unsigned int subRepIndex = 0; subRepIndex < ijkGrid->getFaultSubRepresentationCount(); ++subRepIndex)
		{
			std::cout << "Fault Subrep is : " << ijkGrid->getFaultSubRepresentation(subRepIndex)->getTitle() << std::endl;
		}

		showAllSubRepresentations(ijkGrid->getSubRepresentationSet());

		//*****************************
		// TRUNCATION
		//*****************************
		if (ijkGrid->isTruncated()) {
			std::cout << "This grid is truncated" << std::endl;
			std::cout << "Truncated face count : " << ijkGrid->getTruncatedFaceCount() << std::endl;
			std::cout << "Truncated cell count : " << ijkGrid->getTruncatedCellCount() << std::endl;

			ULONG64* cellIndices = new ULONG64[ijkGrid->getTruncatedCellCount()];
			ijkGrid->getTruncatedCellIndices(cellIndices);
			for (ULONG64 index = 0; index < ijkGrid->getTruncatedCellCount() && index < 10; ++index) {
				cout << "truncated cell Indices : " << cellIndices[index] << endl;
			}
			delete[] cellIndices;

			ULONG64* cumNodeCount = new ULONG64[ijkGrid->getTruncatedFaceCount()];
			ijkGrid->getCumulativeNodeCountPerTruncatedFace(cumNodeCount);
			for (ULONG64 index = 0; index < ijkGrid->getTruncatedFaceCount() && index < 10; ++index) {
				cout << "CumulativeNodeCountPerTruncatedFace : " << cumNodeCount[index] << endl;
			}
			ULONG64* nodeIndicesPerTruncFace = new ULONG64[cumNodeCount[ijkGrid->getTruncatedFaceCount() - 1]];
			ijkGrid->getNodeIndicesOfTruncatedFaces(nodeIndicesPerTruncFace);
			for (ULONG64 index = 0; index < cumNodeCount[ijkGrid->getTruncatedFaceCount() - 1] && index < 10; ++index) {
				cout << "nodeIndicesPerTruncFace : " << nodeIndicesPerTruncFace[index] << endl;
			}
			delete[] cumNodeCount;
			delete[] nodeIndicesPerTruncFace;

			ULONG64* cumFaceCount = new ULONG64[ijkGrid->getTruncatedCellCount()];
			ijkGrid->getCumulativeTruncatedFaceCountPerTruncatedCell(cumFaceCount);
			for (ULONG64 index = 0; index < ijkGrid->getTruncatedCellCount() && index < 10; ++index) {
				cout << "CumulativeTruncatedFaceCountPerTruncatedCell : " << cumFaceCount[index] << endl;
			}
			ULONG64* faceIndicesPerTruncCell = new ULONG64[cumFaceCount[ijkGrid->getTruncatedCellCount() - 1]];
			ijkGrid->getTruncatedFaceIndicesOfTruncatedCells(faceIndicesPerTruncCell);
			for (ULONG64 index = 0; index < cumFaceCount[ijkGrid->getTruncatedCellCount() - 1] && index < 10; ++index) {
				cout << "faceIndicesPerTruncCell : " << faceIndicesPerTruncCell[index] << endl;
			}
			delete[] cumFaceCount;
			delete[] faceIndicesPerTruncCell;

			ULONG64* cumNonTruncFaceCount = new ULONG64[ijkGrid->getTruncatedCellCount()];
			ijkGrid->getCumulativeNonTruncatedFaceCountPerTruncatedCell(cumNonTruncFaceCount);
			for (ULONG64 index = 0; index < ijkGrid->getTruncatedCellCount() && index < 10; ++index) {
				cout << "CumulativeNonTruncatedFaceCountPerTruncatedCell : " << cumNonTruncFaceCount[index] << endl;
			}
			ULONG64* nonTruncfaceIndicesPerTruncCell = new ULONG64[cumNonTruncFaceCount[ijkGrid->getTruncatedCellCount() - 1]];
			ijkGrid->getNonTruncatedFaceIndicesOfTruncatedCells(nonTruncfaceIndicesPerTruncCell);
			for (ULONG64 index = 0; index < cumNonTruncFaceCount[ijkGrid->getTruncatedCellCount() - 1] && index < 10; ++index) {
				cout << "nonTruncfaceIndicesPerTruncCell : " << nonTruncfaceIndicesPerTruncCell[index] << endl;
			}
			delete[] cumNonTruncFaceCount;
			delete[] nonTruncfaceIndicesPerTruncCell;

			unsigned char* rightHandnessTruncFace = new unsigned char[ijkGrid->getTruncatedFaceCount()];
			ijkGrid->getTruncatedFaceIsRightHanded(rightHandnessTruncFace);
			for (ULONG64 index = 0; index < ijkGrid->getTruncatedFaceCount() && index < 10; ++index) {
				cout << "rightHandnessTruncFace : " << rightHandnessTruncFace[index] << endl;
			}
			delete[] rightHandnessTruncFace;
		}

		//*****************************
		// GRID CONNECTION SET 
		//*****************************
		unsigned int gridConnectionSetCount = ijkGrid->getGridConnectionSetRepresentationCount();
		std::cout << "Grid Connection Count is : " << gridConnectionSetCount << std::endl;
		if (gridConnectionSetCount > 0) {
			RESQML2_NS::GridConnectionSetRepresentation const * gridConnectionSet = ijkGrid->getGridConnectionSetRepresentation(0);
			unsigned int faultInterpOfGridConnCount = gridConnectionSet->getInterpretationCount();
			std::cout << "Interpretation Count of this grid connection set is : " << faultInterpOfGridConnCount << endl;
			if (faultInterpOfGridConnCount > 0)
			{
				RESQML2_NS::AbstractFeatureInterpretation* faultInterpOfGridConn = gridConnectionSet->getInterpretationFromIndex(0);
				std::cout << "Interpretation of this grid connection set is : " << faultInterpOfGridConn->getTitle() << " With UUID " << faultInterpOfGridConn->getUuid() << endl;
			}

			int* localFacePerCellIndexPairs = new int[gridConnectionSet->getCellIndexPairCount() * 2];
			LONG64 gcsNullValue = gridConnectionSet->getLocalFacePerCellIndexPairs(localFacePerCellIndexPairs);
			std::cout << "Null Value for LocalFacePerCellIndexPairs : " << gcsNullValue << endl;
			delete[] localFacePerCellIndexPairs;
		}

		//*****************************
		// LGR 
		//*****************************
		if (ijkGrid->getParentGrid() != NULL)
		{
			std::cout << "\t PARENT WINDOW" << std::endl;
			if (ijkGrid->getParentGrid()->getXmlTag() == AbstractIjkGridRepresentation::XML_TAG)
			{
				for (char dimension = 'i'; dimension < 'l'; ++dimension) {
					std::cout << "\t\t DIMENSION :" << dimension << std::endl;
					std::cout << "\t\t Regrid start at :" << ijkGrid->getRegridStartIndexOnParentGrid(dimension) << std::endl;
					std::cout << "\t\t Interval count is :" << ijkGrid->getRegridIntervalCount(dimension) << std::endl;
					if (ijkGrid->isRegridCellCountPerIntervalConstant('i', false)) {
						std::cout << "\t\t Constant parent cell count per interval :" << ijkGrid->getRegridConstantCellCountPerInterval(dimension, false) << std::endl;
					}
					else {
						std::cout << "\t\t Non constant parent cell count per interval" << std::endl;
					}
					if (ijkGrid->isRegridCellCountPerIntervalConstant('i', true)) {
						std::cout << "\t\t Constant child cell count per interval :" << ijkGrid->getRegridConstantCellCountPerInterval(dimension, true) << std::endl;
					}
					else {
						std::cout << "\t\t Non constant child cell count per interval" << std::endl;
					}
				}
			}/*
			 else if (ijkGrid->getParentGrid()->getXmlTag() == UnstructuredColumnLayerGridRepresentation::XML_TAG)
			 {
			 std::cout << "\t\t Refined columns count :" << ijkGrid->getParentColumnIndexCount() << std::endl;
			 }*/
			else if (ijkGrid->getParentGrid()->getXmlTag() == UnstructuredGridRepresentation::XML_TAG)
			{
				std::cout << "\t\t Refined cells count :" << ijkGrid->getParentCellIndexCount() << std::endl;
			}
		}

		//*****************************
		// STRATIGRAPHY 
		//*****************************
		cout << "\t STRATIGRAPHY" << std::endl;
		if (ijkGrid->hasIntervalStratigraphicUnitIndices())
		{
			cout << "\t\t Linked with strati : " << ijkGrid->getStratigraphicOrganizationInterpretation()->getTitle() << endl;
			ULONG64* stratiIndices = new ULONG64[ijkGrid->getKCellCount()];
			ijkGrid->getIntervalStratigraphicUnitIndices(stratiIndices);
			for (size_t i = 0; i < ijkGrid->getKCellCount(); ++i)
			{
				cout << "\t\t K layer " << i << " is linked to strati layer " << stratiIndices[i] << endl;
			}
			delete[] stratiIndices;
		}
		else
		{
			cout << "\t\t No link with stratigraphy." << endl;
		}

		bool * enabledCells = nullptr;
		if (ijkGrid->hasEnabledCellInformation()) {
			std::cout << "Has enabled/disabled cell information" << std::endl;
			enabledCells = new bool[ijkGrid->getCellCount()];
			ijkGrid->getEnabledCells(enabledCells);
		}
		showAllProperties(ijkGrid, enabledCells);
		if (enabledCells != nullptr) {
			delete[] enabledCells;
		}
	}

	// Testing k-layers hyperslabbing
	deserializeGridHyperslabbingInterfaceSequence(repo);

	// Testing block hyperslabbing
	deserializeGridHyperslabbingBlock(repo);


	// ====================
	// Timing hyperslabbing (time consuming)

	//// 4*3*2 explicit grid Left Handed
	//AbstractIjkGridRepresentation* ijkgrid432 = static_cast<AbstractIjkGridRepresentation*>(pck.getDataObjectByUuid("e96c2bde-e3ae-4d51-b078-a8e57fb1e667"));
	//ijkGridHyperslabingTiming(ijkgrid432, 250000);

	//// FOUR SUGARS PARAMETRIC
	//AbstractIjkGridRepresentation* ijkgridParametric = static_cast<AbstractIjkGridRepresentation*>(pck.getDataObjectByUuid("37c45c00-fa3e-11e5-a21e-0002a5d5c51b"));
	//ijkGridHyperslabingTiming(ijkgridParametric, 250000);

	//// Four sugar cubes cellIndex
	//DiscreteProperty* discreteProp1OnIjkgridParametric = static_cast<DiscreteProperty*>(pck.getDataObjectByUuid("eb3dbf6c-5745-4e41-9d09-672f6fbab414"));
	//discretePropertyHyperslabingTiming(ijkgridParametric, discreteProp1OnIjkgridParametric, 250000);

	// ====================


	std::cout << endl << "UNSTRUCTURED GRID REP" << endl;
	for (size_t i = 0; i < unstructuredGridRepSet.size(); ++i)
	{
		showAllMetadata(unstructuredGridRepSet[i]);
		if (!unstructuredGridRepSet[i]->isPartial() && unstructuredGridRepSet[i]->hasGeometry())
		{
			std::cout << "Node count is : " << unstructuredGridRepSet[i]->getXyzPointCountOfPatch(0) << std::endl;

			ULONG64 faceCount = 0;
			if (!unstructuredGridRepSet[i]->isFaceCountOfCellsConstant())
			{
				ULONG64 * faceCountOfCells = new ULONG64[unstructuredGridRepSet[i]->getCellCount()];
				unstructuredGridRepSet[i]->getCumulativeFaceCountPerCell(faceCountOfCells);
				std::cout << "Face count of cell 0 is : " << faceCountOfCells[0] << std::endl;
				if (unstructuredGridRepSet[i]->getCellCount() > 1)
					std::cout << "Face count of cell 1 is : " << faceCountOfCells[1] - faceCountOfCells[0] << std::endl;
				faceCount = faceCountOfCells[unstructuredGridRepSet[i]->getCellCount() - 1];
				delete[] faceCountOfCells;
			}
			else
			{
				std::cout << "Face count of cell is constant : " << unstructuredGridRepSet[i]->getConstantFaceCountOfCells() << std::endl;
				faceCount = unstructuredGridRepSet[i]->getConstantFaceCountOfCells() * unstructuredGridRepSet[i]->getCellCount();
			}
			if (!unstructuredGridRepSet[i]->isNodeCountOfFacesConstant())
			{
				ULONG64 * nodeCountOfFaces = new ULONG64[faceCount];
				unstructuredGridRepSet[i]->getCumulativeNodeCountPerFace(nodeCountOfFaces);
				std::cout << "Node count of face 0 is : " << nodeCountOfFaces[0] << std::endl;
				if (faceCount > 1)
					std::cout << "Node count of face 1 is : " << nodeCountOfFaces[1] - nodeCountOfFaces[0] << std::endl;
				delete[] nodeCountOfFaces;
			}
			else
			{
				std::cout << "Node count of face is constant : " << unstructuredGridRepSet[i]->getConstantNodeCountOfFaces() << std::endl;
			}


			std::cout << "Reading XYZ points" << std::endl;
			double * gridPoints = new double[unstructuredGridRepSet[i]->getXyzPointCountOfPatch(0) * 3];
			unstructuredGridRepSet[i]->getXyzPointsOfAllPatchesInGlobalCrs(gridPoints);
			std::cout << "DONE" << std::endl;
			std::cout << "--------------------------------------------------" << std::endl;
			delete[] gridPoints;

			unstructuredGridRepSet[i]->loadGeometry();

			std::cout << "(in memory) Face count of cell 0 is : " << unstructuredGridRepSet[i]->getFaceCountOfCell(0) << std::endl;
			if (unstructuredGridRepSet[i]->getCellCount() > 1)
				std::cout << "(in memory) Face count of cell 1 is : " << unstructuredGridRepSet[i]->getFaceCountOfCell(1) << std::endl;
			std::cout << "(in memory) Node count of face 0 of cell 0 is : " << unstructuredGridRepSet[i]->getNodeCountOfFaceOfCell(0, 0) << std::endl;
			std::cout << "(in memory) Node indice 0 of face 0 of cell 0 is : " << unstructuredGridRepSet[i]->getNodeIndicesOfFaceOfCell(0, 0)[0] << std::endl;
			std::cout << "(in memory) Node indice 1 of face 0 of cell 0 is : " << unstructuredGridRepSet[i]->getNodeIndicesOfFaceOfCell(0, 0)[1] << std::endl;
			std::cout << "(in memory) Node indice 2 of face 0 of cell 0 is : " << unstructuredGridRepSet[i]->getNodeIndicesOfFaceOfCell(0, 0)[2] << std::endl;
			std::cout << "(in memory) Node indice 0 of face 1 of cell 0 is : " << unstructuredGridRepSet[i]->getNodeIndicesOfFaceOfCell(0, 1)[0] << std::endl;

			unstructuredGridRepSet[i]->unloadGeometry();

			showAllProperties(unstructuredGridRepSet[i]);
		}

		showAllSubRepresentations(unstructuredGridRepSet[i]->getSubRepresentationSet());
	}

	std::cout << endl << "ONLY PARTIAL SUBREPRESENTATIONS" << endl;
	vector<RESQML2_NS::SubRepresentation *> onlyPartialSubReps;
	for (size_t i = 0; i < subRepresentationSet.size(); ++i) {
		if (subRepresentationSet[i]->isPartial()) {
			onlyPartialSubReps.push_back(subRepresentationSet[i]);
		}
	}
	showAllSubRepresentations(onlyPartialSubReps);

	std::cout << endl << "TIME SERIES" << endl;
	for (size_t i = 0; i < timeSeriesSet.size(); ++i)
	{
		showAllMetadata(timeSeriesSet[i]);
		for (unsigned int j = 0; j < timeSeriesSet[i]->getTimestampCount(); ++j) {
			time_t creation = timeSeriesSet[i]->getTimestamp(j);
			std::cout << "Timestamp " << j << " is (unix timestamp) : " << creation << std::endl;
			tm creationTm = timeSeriesSet[i]->getTimestampAsTimeStructure(j);
			std::cout << "Timestamp " << j << " is (struct tm) : " << 1900 + creationTm.tm_year << "-" << creationTm.tm_mon + 1 << "-" << creationTm.tm_mday << "T" << creationTm.tm_hour << ":" << creationTm.tm_min << ":" << creationTm.tm_sec << std::endl;
		}
		for (size_t j = 0; j < timeSeriesSet[i]->getPropertySet().size(); ++j)
		{
			std::cout << endl << "\tPROPERTIES" << endl;
			showAllMetadata(timeSeriesSet[i]->getPropertySet()[j]);
		}
	}

#ifdef WITH_EXPERIMENTAL
	// GRAPHICAL INFORMATION
	deserializeGraphicalInformationSet(repo);
#endif

	std::cout << endl << repo.getWarnings().size() << " WARNING(S)" << endl;
	for (size_t i = 0; i < repo.getWarnings().size(); ++i) {
		std::cout << i << " - " << repo.getWarnings()[i] << endl;
	}

	pck.close();
}

/*
#include <ctime>
// Performance testing
int main(int argc, char **argv)
{
string filePath("../../testingPackageCpp.epc");
COMMON_NS::EpcDocument pck(filePath, COMMON_NS::EpcDocument::OVERWRITE);
COMMON_NS::AbstractHdfProxy* hdfProxy = pck.createHdfProxy("", "Hdf Proxy", pck.getStorageDirectory(), pck.getName() + ".h5" );
unsigned long long dims2[3] = {152, 1000, 1000};
float * testingValues2 = new float[152000000];
std::clock_t startClock;
time_t startTime;
double duration;
for (unsigned int i = 0; i < 20; ++i) {
startClock = std::clock();
startTime = time(0);
ostringstream datasetName;
datasetName << "testingDataset"  << i;
hdfProxy->writeArrayNdOfFloatValues("testingGroup", datasetName.str(), testingValues2, dims2, 3);
std::cout<<"duration (clock) : "<< ( std::clock() - startClock ) / (double) CLOCKS_PER_SEC <<'\n';
std::cout<<"duration (time) : "<< difftime(time(0), startTime)<<'\n';
}
pck.serialize();
hdfProxy->close();
delete [] testingValues2;
}
*/

// filepath is defined in a macro to better check memory leak
#define filePath "../../testingPackageCpp.epc"
int main()
{
	//try {
	if (serialize(filePath)) {
		deserialize(filePath);
	}
	/*}
	catch (const std::invalid_argument & Exp)
	{
	std::cerr << "Error : " << Exp.what() << ".\n";
	return 1;
	}*/

	//cout << "Press enter to continue..." << endl;
	//cin.get();

#ifdef _WIN32
	_CrtDumpMemoryLeaks();
#endif

	return 0;
}
