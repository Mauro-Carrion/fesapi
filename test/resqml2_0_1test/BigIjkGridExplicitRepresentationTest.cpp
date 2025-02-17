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
#include "resqml2_0_1test/BigIjkGridExplicitRepresentationTest.h"

#include "catch.hpp"
#include "resqml2_0_1test/LocalDepth3dCrsTest.h"

#include "resqml2_0_1/LocalDepth3dCrs.h"
#include "resqml2_0_1/IjkGridExplicitRepresentation.h"
#include "resqml2_0_1/DiscreteProperty.h"
#include "resqml2_0_1/ContinuousProperty.h"

using namespace std;
using namespace COMMON_NS;
using namespace resqml2_0_1test;
using namespace RESQML2_NS;

const char* BigIjkGridExplicitRepresentationTest::defaultUuid = "f889e5d2-249e-4827-8532-ce60a1d05b99";
const char* BigIjkGridExplicitRepresentationTest::defaultTitle = "Ijk Grid Explicit Representation";
const char* BigIjkGridExplicitRepresentationTest::discretePropertyUuid = "265343eb-6d1b-4015-b08b-8b4abbe7f84b";
const char* BigIjkGridExplicitRepresentationTest::discretePropertyTitle = "Explicit IJK Grid K Index";
const char* BigIjkGridExplicitRepresentationTest::continuousPropertyUuid = "a4e8e307-0844-47e9-b9fd-6ad0a1f99b88";
const char* BigIjkGridExplicitRepresentationTest::continuousPropertyTitle = "Explicit IJK Grid Continuous Property";

BigIjkGridExplicitRepresentationTest::BigIjkGridExplicitRepresentationTest(
	const string & repoPath,
	const unsigned int & iCount, const unsigned int & jCount, const unsigned int & kCount,
	const unsigned int & faultCount,
	const double & xMin, const double & xMax, const double & yMin, const double & yMax, const double & zMin, const double & zMax,
	const double & faultThrow)
	: AbstractBigIjkGridRepresentationTest(repoPath, iCount, jCount, kCount, faultCount, xMin, xMax, yMin, yMax, zMin, zMax, faultThrow) {
}

BigIjkGridExplicitRepresentationTest::BigIjkGridExplicitRepresentationTest(DataObjectRepository * repo, bool init,
	const unsigned int & iCount, const unsigned int & jCount, const unsigned int & kCount,
	const unsigned int & faultCount,
	const double & xMin, const double & xMax, const double & yMin, const double & yMax, const double & zMin, const double & zMax,
	const double & faultThrow)
	: AbstractBigIjkGridRepresentationTest(repo, init, iCount, jCount, kCount, faultCount, xMin, xMax, yMin, yMax, zMin, zMax, faultThrow) {
}

void BigIjkGridExplicitRepresentationTest::initRepoHandler() {
	// creating the ijk grid
	RESQML2_0_1_NS::IjkGridExplicitRepresentation* ijkGrid = repo->createIjkGridExplicitRepresentation(defaultUuid, defaultTitle, iCount, jCount, kCount);
	REQUIRE(ijkGrid != nullptr);
	unsigned int * pillarOfCoordinateLine = new unsigned int[faultCount * (jCount + 1)];
	unsigned int * splitCoordinateLineColumnCumulativeCount = new unsigned int[faultCount * (jCount + 1)];
	unsigned int * splitCoordinateLineColumns = new unsigned int[(faultCount * (jCount + 1)) + (faultCount * (jCount - 1))];
	initSplitCoordinateLine(pillarOfCoordinateLine, splitCoordinateLineColumnCumulativeCount, splitCoordinateLineColumns);
	initNodesIjkGridRepresentation(iCount, jCount, kCount, faultCount, xMin, xMax, yMin, yMax, zMin, zMax, faultThrow);
	ijkGrid->setGeometryAsCoordinateLineNodes(gsoap_resqml2_0_1::resqml20__PillarShape__vertical, gsoap_resqml2_0_1::resqml20__KDirection__down, false, nodesIjkGridRepresentation, nullptr,
		faultCount * (jCount + 1), pillarOfCoordinateLine, splitCoordinateLineColumnCumulativeCount, splitCoordinateLineColumns);

	// adding a discrete property
	RESQML2_0_1_NS::DiscreteProperty* discreteProperty = repo->createDiscreteProperty(
		ijkGrid, discretePropertyUuid, discretePropertyTitle,
		1, 
		gsoap_resqml2_0_1::resqml20__IndexableElements__cells, 
		gsoap_resqml2_0_1::resqml20__ResqmlPropertyKind__index);
	unsigned short * discretePropertyValues = new unsigned short[iCount * jCount * kCount];
	initDiscreteProperty(discretePropertyValues);
	discreteProperty->pushBackUShortHdf5Array3dOfValues(discretePropertyValues, iCount, jCount, kCount, nullptr, -1);

	// adding a continuous property
	RESQML2_0_1_NS::ContinuousProperty* continuousProperty = repo->createContinuousProperty(
		ijkGrid, continuousPropertyUuid, continuousPropertyTitle,
		1,
		gsoap_resqml2_0_1::resqml20__IndexableElements__cells,
		gsoap_resqml2_0_1::resqml20__ResqmlUom__m,
		gsoap_resqml2_0_1::resqml20__ResqmlPropertyKind__length);
	double * continuousPropertyValues = new double[iCount * jCount * kCount];
	initContinuousProperty(continuousPropertyValues);
	continuousProperty->pushBackDoubleHdf5Array1dOfValues(continuousPropertyValues, iCount * jCount * kCount, nullptr);

	// cleaning
	delete[] pillarOfCoordinateLine;
	delete[] splitCoordinateLineColumnCumulativeCount;
	delete[] splitCoordinateLineColumns;
	delete[] discretePropertyValues;
	delete[] continuousPropertyValues;
}

void BigIjkGridExplicitRepresentationTest::readRepoHandler() {
}
