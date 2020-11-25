#ifndef __API_RESULT_CLS_HXX__
#define __API_RESULT_CLS_HXX__

// Header file describing the apiResult class, which uses cUrl to
// load JSON from the user-specified API, and offers limited parsing
// via cJSON.
//
// Paul Mount	05-MAR-2019  v1.0 Created
//

#include <vector>
#include <string>
#include "webservices.hpp"

void Tokenize(string sourceString, vector<string> splitStrings, string delimiter);
enum API_ERROR
{
	PARSE_FAIL,
	BAD_ENTRY,
	NOT_FOUND,
	NULL_VALUE,
	TYPE_MISMATCH
};
enum DATATYPE
{
	cJSON_BOOLEAN,
	NUMBER,
	TEXT,
	ARRAY,
	OBJECT,
	EMPTY,
	MISSING
};

class APIRESULT
{

private:
	cJSON *json;
	int jsonLength;

	bool loadJSON(string source, bool isFile = false);
	cJSON *subBlock(int entry);
	string getValue(string name, cJSON *jsonSection);
	cJSON *findItem(string namePath, int entry);
	cJSON *extractItem(string name, cJSON *jsonSection);
	bool getEach(string namePath, cJSON *&jsonSection);

public:
	// Constructors throw API_ERROR on failue
	APIRESULT();
	APIRESULT(string url);
	APIRESULT(cJSON *cJson);
	~APIRESULT();

	// routines to create a cJSON object, either by filename or via API.
	bool loadFile(string filename) { return loadJSON(filename, true); }
	bool loadApi(string dataURL) { return loadJSON(dataURL, false); }
	bool loadJson(cJSON *cJson);

	// meta-data about the returned JSON block
	int getLength() { return jsonLength; }
	size_t getSize() { return sizeof(json); }
	cJSON *getJSON() { return json; } // so you can parse it yourself.
									  // first malloc(object.getSize())
	string dump() { return cJSON_Print(json); }

	// Find out info about a certain item without getting the item itself
	bool hasItem(string name, int entry = 0)
	{
		return cJSON_HasObjectItem(subBlock(entry), name.c_str());
	}
	bool isNull(string name, int entry = 0)
	{
		return hasItem(name, entry) ? cJSON_IsNull(cJSON_GetObjectItem(subBlock(entry), name.c_str())) : true;
	}
	DATATYPE getDataType(string name, int entry = 0);

	// extracting single values from JSON. You can specify a path down
	// a sub-item like so layer1.layer2...name (i.e. Sun.Rise)
	// These throw API_ERRORs, so you should catch them
	string getStringValue(string namePath)
	{
		return getStringValue(namePath, 0);
	}
	string getStringValue(string namePath, int entry);
	double getFloatValue(string namePath)
	{
		return getFloatValue(namePath, 0);
	}
	double getFloatValue(string namePath, int entry);
	bool getBoolValue(string namePath)
	{
		return getBoolValue(namePath, 0);
	}
	bool getBoolValue(string namePath, int entry);

	// These are useful if you have an array inside an object. Make
	// a new APIRESULT object using the return value of this call.
	cJSON *getJsonArray(string namePath, int entry = 0);
	cJSON *getJsonItem(int a) { return cJSON_GetArrayItem(json, a); }

	// Pull out a value sub-layers (namePath = layer1.{layerx}.name)
	// iteratively fetch values, returns null when finished
	bool getEachString(string namePath, string &returnValue);
	bool getEachFloat(string namePath, double &returnValue);
	bool getEachBool(string namePath, bool &returnValue);
	bool getEachArray(string namePath, cJSON *&returnValue);
};

#endif
/*

  LINKAGE: In order to use this class, you will need to include each
	of the following in your options file:
	
	SRC:[WEBSERVICES]WEBSERVICES
	SRC:[CJSON]CJSON
	SRC:[LIBCURL]CURLLIB.OLB_NOSSL/LIB
*/
