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
#pragma once

#include "resqml2/AbstractFeatureInterpretation.h"

namespace RESQML2_0_1_NS
{
	class GenericFeatureInterpretation : public RESQML2_NS::AbstractFeatureInterpretation
	{
	public:

		/**
		* Only to be used in partial transfer context
		*/
		GenericFeatureInterpretation(gsoap_resqml2_0_1::eml20__DataObjectReference* partialObject) : RESQML2_NS::AbstractFeatureInterpretation(partialObject) {}

		/**
		* Creates an instance of this class in a gsoap context.
		* @param feature	The feature the instance interprets.
		* @param guid		The guid to set to the interpretation. If empty then a new guid will be generated.
		* @param title		A title for the instance to create.
		*/
		GenericFeatureInterpretation(RESQML2_NS::AbstractFeature * feature, const std::string & guid, const std::string & title);

		GenericFeatureInterpretation(gsoap_resqml2_0_1::_resqml20__GenericFeatureInterpretation* fromGsoap) : RESQML2_NS::AbstractFeatureInterpretation(fromGsoap) {}

		~GenericFeatureInterpretation() {}
	
		DLL_IMPORT_OR_EXPORT static const char* XML_TAG;
		DLL_IMPORT_OR_EXPORT virtual std::string getXmlTag() const {return XML_TAG;}
	};
}
