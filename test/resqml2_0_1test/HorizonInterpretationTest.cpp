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
#include "resqml2_0_1test/HorizonInterpretationTest.h"

#include "catch.hpp"

#include "AbstractObjectTest.h"

#include "resqml2_0_1/Horizon.h"
#include "resqml2_0_1/HorizonInterpretation.h"

using namespace std;
using namespace resqml2_0_1test;
using namespace COMMON_NS;
using namespace RESQML2_0_1_NS;

const char* HorizonInterpretationTest::horizonUuid = "35ea54a9-63d8-4520-a4be-8eb7afff3a4f";
const char* HorizonInterpretationTest::horizonTitle = "Horizon";
const char* HorizonInterpretationTest::defaultUuid = "4b256b37-4013-47f9-b6c3-4460abc6e421";
const char* HorizonInterpretationTest::defaultTitle = "Horizon Interpretation";

HorizonInterpretationTest::HorizonInterpretationTest(const string & repoPath)
	: commontest::AbstractObjectTest(repoPath)
{
}

HorizonInterpretationTest::HorizonInterpretationTest(DataObjectRepository * repo, bool init)
	: commontest::AbstractObjectTest(repo)
{
	if (init)
		initRepo();
	else
		readRepo();
}


void HorizonInterpretationTest::initRepoHandler()
{
	// creating dependencies
	Horizon* horizon = repo->getDataObjectByUuid<Horizon>(horizonUuid);
	if (horizon == nullptr) {
		horizon = repo->createHorizon(horizonUuid, horizonTitle);
	}

	HorizonInterpretation* horizonInterp = repo->getDataObjectByUuid<HorizonInterpretation>(defaultUuid);
	if (horizonInterp == nullptr) {
		horizonInterp = repo->createHorizonInterpretation(horizon, defaultUuid, defaultTitle);
	}
	REQUIRE( horizonInterp != nullptr );
}

void HorizonInterpretationTest::readRepoHandler()
{
	// getting the horizon interpretation
	HorizonInterpretation* horizonInterp = repo->getDataObjectByUuid<HorizonInterpretation>(defaultUuid);
	REQUIRE(horizonInterp != nullptr);
}
