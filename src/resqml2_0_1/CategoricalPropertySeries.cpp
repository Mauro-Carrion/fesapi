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
#include "resqml2_0_1/CategoricalPropertySeries.h"

#include <stdexcept>

#include "resqml2_0_1/TimeSeries.h"
#include "resqml2/AbstractRepresentation.h"
#include "resqml2_0_1/PropertyKind.h"
#include "resqml2_0_1/StringTableLookup.h"
#include "common/AbstractHdfProxy.h"

using namespace std;
using namespace RESQML2_0_1_NS;
using namespace gsoap_resqml2_0_1;

const char* CategoricalPropertySeries::XML_TAG = "CategoricalPropertySeries";

CategoricalPropertySeries::CategoricalPropertySeries(RESQML2_NS::AbstractRepresentation * rep, const string & guid, const string & title,
	const unsigned int & dimension, const gsoap_resqml2_0_1::resqml20__IndexableElements & attachmentKind,
	StringTableLookup* strLookup, const resqml20__ResqmlPropertyKind & energisticsPropertyKind,
	RESQML2_NS::TimeSeries * ts, const bool & useInterval)
{
	gsoapProxy2_0_1 = soap_new_resqml20__obj_USCORECategoricalPropertySeries(rep->getGsoapContext());	
	_resqml20__CategoricalPropertySeries* prop = static_cast<_resqml20__CategoricalPropertySeries*>(gsoapProxy2_0_1);
	prop->IndexableElement = attachmentKind;
	prop->Count = dimension;

	resqml20__StandardPropertyKind* xmlStandardPropKind = soap_new_resqml20__StandardPropertyKind(gsoapProxy2_0_1->soap);
	xmlStandardPropKind->Kind = energisticsPropertyKind;
	prop->PropertyKind = xmlStandardPropKind;

	initMandatoryMetadata();
	setMetadata(guid, title, std::string(), -1, std::string(), std::string(), -1, std::string());

	setRepresentation(rep);

	prop->SeriesTimeIndices = soap_new_resqml20__TimeIndices(gsoapProxy2_0_1->soap);
	prop->SeriesTimeIndices->TimeIndexCount = ts->getTimestampCount();
	prop->SeriesTimeIndices->UseInterval = useInterval;
	setTimeSeries(ts);

	getRepository()->addRelationship(this, strLookup);
	prop->Lookup = strLookup->newResqmlReference();
}

CategoricalPropertySeries::CategoricalPropertySeries(RESQML2_NS::AbstractRepresentation * rep, const string & guid, const string & title,
	const unsigned int & dimension, const gsoap_resqml2_0_1::resqml20__IndexableElements & attachmentKind,
	StringTableLookup* strLookup, RESQML2_NS::PropertyKind * localPropKind,
	RESQML2_NS::TimeSeries * ts, const bool & useInterval)
{
	gsoapProxy2_0_1 = soap_new_resqml20__obj_USCORECategoricalPropertySeries(rep->getGsoapContext());	
	_resqml20__CategoricalPropertySeries* prop = static_cast<_resqml20__CategoricalPropertySeries*>(gsoapProxy2_0_1);
	prop->IndexableElement = attachmentKind;
	prop->Count = dimension;

	initMandatoryMetadata();
	setMetadata(guid, title, std::string(), -1, std::string(), std::string(), -1, std::string());

	setRepresentation(rep);

	prop->SeriesTimeIndices = soap_new_resqml20__TimeIndices(gsoapProxy2_0_1->soap);
	prop->SeriesTimeIndices->TimeIndexCount = ts->getTimestampCount();
	prop->SeriesTimeIndices->UseInterval = useInterval;
	setTimeSeries(ts);

	setLocalPropertyKind(localPropKind);

	getRepository()->addRelationship(this, strLookup);
	prop->Lookup = strLookup->newResqmlReference();
}
