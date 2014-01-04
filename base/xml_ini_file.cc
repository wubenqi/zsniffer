// Copyright (C) 2012 by wubenqi
// Distributable under the terms of either the Apache License (Version 2.0) or 
// the GNU Lesser General Public License, as specified in the COPYING file.
//
// By: wubenqi<wubenqi@gmail.com>
//

#include "base/xml_ini_file.h"

XMLIniFile::XMLIniFile(void) : 
	inited_(false),
	updated_(false) {
	ini_file_name_[0] = '\0';
}

XMLIniFile::~XMLIniFile(void) {
	Flush();
}

bool XMLIniFile::LoadXMLIniFile(const char* pstrFilename) {
    if(!pstrFilename)
		return false;
	strcpy( ini_file_name_, pstrFilename );
	return inited_ = xml_ini_parser_.Load(pstrFilename);
}

bool XMLIniFile::GetString( const char* pstrSection, const char* pstrKey, char* pstrValue,  unsigned int cchMax, const char* pstrDefault) {
	if (!inited_ || !pstrSection || !pstrKey )
		return false;
	std::string strValue;
	if( FindEntry( pstrSection, pstrKey, strValue ) ) {
		if (strValue.length()+1 > cchMax) {
			strncpy( pstrValue, strValue.c_str(), cchMax-1 );
			pstrValue[cchMax-1] = 0;
		} else
			strcpy( pstrValue, strValue.c_str() );
	} else {
		if ( !pstrDefault )
			pstrValue[0] = '\0';
		else
			strcpy( pstrValue, pstrDefault );
	}
	return true;
}

bool XMLIniFile::GetString(const char* pstrSection, const char* pstrKey, std::string& sValue, const char* pstrDefault ) {      
	if (!inited_ || !pstrSection || !pstrKey )
		return false;
	if (!FindEntry( pstrSection, pstrKey, sValue )) {
		if ( !pstrDefault )
			sValue = "";
		else
			sValue = pstrDefault;
	}
	return true;
}

bool XMLIniFile::GetStrings( const char* pstrSection, const char* pstrKey, std::vector<std::string>& sValue ) {
	if (!inited_ || !pstrSection || !pstrKey )
		return false;
	FindEntrys( pstrSection, pstrKey, sValue );
	return true;
}

bool XMLIniFile::GetInt(const char* pstrSection, const char* pstrKey, int& iValue,  int iDefault) {
	if (!inited_ || !pstrSection || !pstrKey )
		return false;
	std::string strValue;
	if( FindEntry( pstrSection, pstrKey, strValue ) )
		iValue = atoi(strValue.c_str());
	else
		iValue = iDefault;
	return true;
}

bool XMLIniFile::GetInts( const char* pstrSection, const char* pstrKey, std::vector<int>& iValue ) {
  if (!inited_ || !pstrSection || !pstrKey )
    return false;
  FindEntrys( pstrSection, pstrKey, iValue );
  return true;
}


bool XMLIniFile::GetBool(const char* pstrSection, const char* pstrKey, bool& bValue, bool bDefault ) {
	if (!inited_ || !pstrSection  || !pstrKey)
		return false;
	std::string strValue;
	if( FindEntry( pstrSection, pstrKey, strValue ) ) {
		//char szValue[2] = { 0 };
		switch( strValue[0] ) {
		case 'y': // Yes
		case 'Y':
		case 't': // True
		case 'T':
		case '1': // 1
			bValue = true;
			break;
		case 'n': // No
		case 'N':
		case 'f': // False
		case 'F':
		case '0': // 0
			bValue = false;
			break;
		default:
			bValue = bDefault;
		}
	}
	return true;
}

bool XMLIniFile::GetBools( const char* pstrSection, const char* pstrKey, std::vector<bool>& bValue ) {
	//todo
	return false;
}


bool XMLIniFile::PutString(const char* pstrSection, const char* pstrKey, const char* pstrValue) {
	if ( !inited_ || !pstrSection || !pstrKey )
		return false;
	SetEntry( pstrSection, pstrKey, pstrValue );
	updated_ = true;
    return true;
}

bool XMLIniFile::PutInt(const char* pstrSection, const char* pstrKey, int iValue) {
	if ( !inited_ || !pstrSection || !pstrKey)
		return false;
	char szVal[25];
	sprintf( szVal, "%d", iValue );
	SetEntry( pstrSection, pstrKey, szVal );
	updated_ = true;
    return true;
}

bool XMLIniFile::PutBool(const char* pstrSection, const char* pstrKey, bool bValue) {
	if (!inited_ || !pstrSection || !pstrKey)
		return false;
	std::string strVal = "false";
	if(bValue)
		strVal = "true";
  	SetEntry( pstrSection, pstrKey, strVal );
	updated_ = true;
	return true;
}

void XMLIniFile::DeleteKey(const char* pstrSection, const char* pstrKey) {
	// Loop through sections
	xml_ini_parser_.ResetPos();
	while ( xml_ini_parser_.FindChildElem("Section") ) {
#ifdef WIN32
		// Is this the right section?
		if ( !_stricmp(xml_ini_parser_.GetChildAttrib("name").c_str(), pstrSection ) )
#else
    if ( !strcasecmp(xml_ini_parser_.GetChildAttrib("name").c_str(), pstrSection ) )
#endif
		{
			// Check entries in this section
			xml_ini_parser_.IntoElem();
			while ( xml_ini_parser_.FindChildElem("Item") ) {
				// Is this the right entry?
#ifdef WIN32
				if ( !_stricmp (xml_ini_parser_.GetChildAttrib("name").c_str(), pstrKey) )
#else
				if ( !strcasecmp (xml_ini_parser_.GetChildAttrib("name").c_str(), pstrKey) )
#endif
				{
					xml_ini_parser_.RemoveChildElem();
					return;
				}
			}
			break; // don't check any other sections
		}
	}
	updated_ = true;
}

void XMLIniFile::DeleteSection(const char* pstrSection)
{
	if ( !inited_ || !pstrSection )
		return;

	// Loop through sections
	xml_ini_parser_.ResetPos();
	while ( xml_ini_parser_.FindChildElem("Section") ) {
		// Is this the right section?
#ifdef WIN32
		if ( !_stricmp( xml_ini_parser_.GetChildAttrib("name").c_str(), pstrSection) )
#else
		if ( !strcasecmp( xml_ini_parser_.GetChildAttrib("name").c_str(), pstrSection) )
#endif
		{
			xml_ini_parser_.RemoveChildElem();
			break;
		}
	}
	updated_ = true;
}

void XMLIniFile::Flush() {
	if (inited_ && updated_)
        xml_ini_parser_.Save( ini_file_name_ );	
}

void XMLIniFile::SetEntry( const std::string& strSection, const std::string& strItem, const std::string& strValue ) {
	// Find/Create root element of xml document
	xml_ini_parser_.ResetPos();
	if ( ! xml_ini_parser_.FindElem() )
		xml_ini_parser_.AddElem( "XMLIni" ); // or whatever root element name is

	// Find/Create section
	bool bFoundSection = false;
	while ( xml_ini_parser_.FindChildElem("Section") ) {
		// Is this the right section?
		if ( xml_ini_parser_.GetChildAttrib("name") == strSection ) {
			bFoundSection = true;
			break;
		}
	}
	if ( ! bFoundSection ) {
		xml_ini_parser_.AddChildElem( "Section" );
		xml_ini_parser_.SetChildAttrib( "name", strSection.c_str() );
	}

	// Find/Create entry
	xml_ini_parser_.IntoElem();
	bool bFoundEntry = false;
	while ( xml_ini_parser_.FindChildElem("Item") ) {
		// Is this the right entry?
		if ( xml_ini_parser_.GetChildAttrib("name") == strItem ) {
			bFoundEntry = true;
			break;
		}
	}
	if ( ! bFoundEntry ) {
		xml_ini_parser_.AddChildElem( "Item" );
		xml_ini_parser_.SetChildAttrib( "name", strItem.c_str() );
	}

	// Set value
	xml_ini_parser_.SetChildData( strValue.c_str() );
}

bool XMLIniFile::FindEntry( const std::string& strSection, const std::string& strItem, std::string& strValue ) {
	// Loop through sections
	xml_ini_parser_.ResetPos();
	while ( xml_ini_parser_.FindChildElem("Section") ) {
		// Is this the right section?
		if (  xml_ini_parser_.GetChildAttrib("name") == strSection ) {
			// Check entries in this section
			xml_ini_parser_.IntoElem();
			while ( xml_ini_parser_.FindChildElem("Item") ) {
				// Is this the right entry?
				if ( xml_ini_parser_.GetChildAttrib("name") == strItem ) {
					strValue = xml_ini_parser_.GetChildData();
					return true;
				}
			}
			break; // don't check any other sections
		}
	}
	return false;
}

bool XMLIniFile::FindEntrys( const std::string& strSection, const std::string& strItem, std::vector<std::string>& strValues ) {
	// Loop through sections
	xml_ini_parser_.ResetPos();
	while ( xml_ini_parser_.FindChildElem("Section") ) {
		// Is this the right section?
		if (  xml_ini_parser_.GetChildAttrib("name") == strSection ) {
			// Check entries in this section
			xml_ini_parser_.IntoElem();
			while ( xml_ini_parser_.FindChildElem("Item") ) {
				// Is this the right entry?
				if ( xml_ini_parser_.GetChildAttrib("name") == strItem ) {
      strValues.push_back(xml_ini_parser_.GetChildData());
					//return true;
				}
			}
			break; // don't check any other sections
		}
	}
	return false;
}

bool XMLIniFile::FindEntrys( const std::string& strSection, const std::string& strItem, std::vector<int>& iValues ) {
  // Loop through sections
  xml_ini_parser_.ResetPos();
  while ( xml_ini_parser_.FindChildElem("Section") ) {
    // Is this the right section?
    if (  xml_ini_parser_.GetChildAttrib("name") == strSection ) {
      // Check entries in this section
      xml_ini_parser_.IntoElem();
      while ( xml_ini_parser_.FindChildElem("Item") ) {
        // Is this the right entry?
        if ( xml_ini_parser_.GetChildAttrib("name") == strItem ) {
          iValues.push_back(atoi(xml_ini_parser_.GetChildData().c_str()));
          //return true;
        }
      }
      break; // don't check any other sections
    }
  }
  return false;
}
