/*****************************************************************************
// Filename: Vianova.CityGML.Reader.Types.cpp
//
// @Author: Chu Van Huyen, ViaNova Systems AS
// Copyright (C) 2015, ViaNova Systems AS.
//
//******************************************************************************/
#include "LandXml.Reader.Types.h"


BEGIN_LANDXML_READER
///////////////////////////////////////////////////////////////////////////////
std::string getCityObjectsClassName(LandXmlObjectsTypeMask mask)
{
#define GETLANDXMLNAME( _k_,_t_ ) if ( mask & LDT_ ## _t_ ) ss << _k_ << "|";
	std::stringstream ss;
	GETLANDXMLNAME("GenericLandXmlObject", GenericLandXmlObject);
	GETLANDXMLNAME("Units", Units);
	GETLANDXMLNAME("CoordinateSystem", CoordinateSystem);
	GETLANDXMLNAME("FeatureDictionary", FeatureDictionary);
	GETLANDXMLNAME("Project", Project);
	GETLANDXMLNAME("Application", Application);
	GETLANDXMLNAME("CgPoints", CgPoints);
	GETLANDXMLNAME("Survey", Survey);
	GETLANDXMLNAME("Surfaces", Surfaces);
#undef GETLANDXMLNAME
	std::string s = ss.str();
	if (s != "") s.erase(s.length() - 1, 1); // remove the last | char
	return s;
};

// std::string tokenizer helper
std::vector<std::string> tokenize(const std::string& str, const std::string& delimiters)
{
	std::vector<std::string> tokens;
	std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	std::string::size_type pos = str.find_first_of(delimiters, lastPos);

	while (std::wstring::npos != pos || std::wstring::npos != lastPos)
	{
		tokens.push_back(str.substr(lastPos, pos - lastPos));
		lastPos = str.find_first_not_of(delimiters, pos);
		pos = str.find_first_of(delimiters, lastPos);
	}
	return tokens;
}

bool caseInsensitiveStringCompare(const std::string& str1, const std::string& str2)
{
	std::string s1(str1);
	std::transform(s1.begin(), s1.end(), s1.begin(), ::towlower);
	std::string s2(str2);
	std::transform(s2.begin(), s2.end(), s2.begin(), ::towlower);
	return (s1 == s2);
}

LandXmlObjectsTypeMask getLandXmlObjectsTypeMaskFromString(const std::string& stringMask)
{
	LandXmlObjectsTypeMask mask = 0;

	std::vector<std::string> tokens = tokenize(stringMask);

#define COMPARELANDXMLNAMEMASK( _k_,_t_ ) {\
	bool neg = ( tokens[i][0] == '~' || tokens[i][0] == '!');\
	if ( caseInsensitiveStringCompare(_k_, neg ? tokens[i].substr(1) : tokens[i] ) ) { mask = neg ? ( mask & (~ LDT_ ## _t_ )) : ( mask | LDT_ ## _t_ );}\
	}

	for (unsigned int i = 0; i < tokens.size(); i++)
	{
		if (tokens[i].length() == 0) continue;

		COMPARELANDXMLNAMEMASK("GenericLandXmlObject", GenericLandXmlObject);
		COMPARELANDXMLNAMEMASK("Units", Units);
		COMPARELANDXMLNAMEMASK("CoordinateSystem", CoordinateSystem);
		COMPARELANDXMLNAMEMASK("FeatureDictionary", FeatureDictionary);
		COMPARELANDXMLNAMEMASK("Project", Project);
		COMPARELANDXMLNAMEMASK("Application", Application);
		COMPARELANDXMLNAMEMASK("CgPoints", CgPoints);
		COMPARELANDXMLNAMEMASK("Survey", Survey);
		COMPARELANDXMLNAMEMASK("Surfaces", Surfaces);
	}
#undef COMPARELANDXMLNAMEMASK
	return mask;
}
END_LANDXML_READER