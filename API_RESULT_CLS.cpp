// Methods for the apiResult class, which uses cUrl to
// load JSON from the user-specified API, and offers limited parsing
// via cJSON.
//
// Paul Mount	06-MAR-2019  v1.0 Created
//

#include "api_result_cls.hpp"

APIRESULT::APIRESULT()
{
	json = NULL;
}

APIRESULT::APIRESULT(string url)
{
	bool status;

	try
	{
		status = loadJSON(url, false);
	}
	catch (...)
	{
		throw;
	}
}

APIRESULT::APIRESULT(cJSON *cJson)
{
	bool status;

	try
	{
		status = loadJson(cJson);
	}
	catch (...)
	{
		throw;
	}
}

APIRESULT::~APIRESULT()
{
	delete (json);
}

/*
 	Routine to create a cJSON object, either by filename or via API.
	Note that this method is private. The public ones call this
	and are specific to the data source.
*/
bool APIRESULT::loadJSON(string source, bool isFile)
{
	if (json != NULL)
		cJSON_Delete(json); // Plug this memory leak!
	bool status;

	try
	{
		if (isFile)
			status = loadJSONfile(source, json);
		else
			status = loadJSONapi(source, json);

		if (status)
			jsonLength = cJSON_GetArraySize(json);
		else
		{
			jsonLength = 0;
			cerr << (isFile ? "File " : "URL ") << source
				 << " could not be parsed as JSON" << endl;
			throw PARSE_FAIL;
		}
	}
	catch (...)
	{
		status = false;
		throw;
	}

	return status;
}

/*
bool APIRESULT::loadFile(string fileName)
{
    bool status;
    try{
	status = loadJSON(fileName,true);
    }
    catch(...)
    {
	status = false;
	throw;
    }

    return status;
}

bool APIRESULT::loadApi(string url)
{
    bool status;

    try{
	status = loadJSON(url,false);
    }
    catch(...)
    {
	status = false;
	throw;
    }

    return status;
}
*/
// If you have a pointer to cJSON, which you might have gotten from a
// call to getJsonArray or getJsonItem...
bool APIRESULT::loadJson(cJSON *cJson)
{
	if (json != NULL)
		cJSON_Delete(json); // Plug this memory leak!

	if (cJSON_IsInvalid(cJson))
	{
		throw PARSE_FAIL;
		return false;
	}

	json = cJson;
	jsonLength = cJSON_GetArraySize(json);

	return true;
}

// Private function to retrieve a block that covers one iteration.
cJSON *APIRESULT::subBlock(int entry)
{
	if (entry < 0 || entry > jsonLength)
		throw BAD_ENTRY;

	return (cJSON_IsArray(json) ? (entry < 0 || entry > jsonLength ? NULL : cJSON_GetArrayItem(json, entry)) : json);
}

// Private function. Give a final attribute:value pair, this function returns
// the JSON object so that it can be cast to the proper data type.
//
cJSON *APIRESULT::extractItem(string name, cJSON *jsonSection)
{
	cJSON *jsonItem;
	if (cJSON_HasObjectItem(jsonSection, name.c_str()))
	{
		if (cJSON_IsNull(cJSON_GetObjectItem(jsonSection, name.c_str())))
		{
			throw NULL_VALUE;
		}
		else
			jsonItem = cJSON_GetObjectItem(jsonSection, name.c_str());
	}
	else
	{
		if (cJSON_IsArray(jsonSection))
			jsonItem = extractItem(name, cJSON_GetArrayItem(jsonSection, 0));
		else
			throw NOT_FOUND;
	}

	return jsonItem;
}

// Private function to drill down layer-by-layer (passed as namePath) on
// a single iteration of the JSON array until the desired object is found
// and can be passed to the extractor and cast to the proper data type.

cJSON *APIRESULT::findItem(string namePath, int entry)
{
	cJSON *jsonItem = NULL;
	vector<string> subNames;
	cJSON *section = subBlock(entry);
	cJSON *thisBlock;

	if (section == NULL)
	{
		cerr << "Could not parse JSON" << endl;
		throw NOT_FOUND;
		return NULL;
	}

	Tokenize(namePath, subNames, ".");
	int level = 0;
	thisBlock = cJSON_GetObjectItem(section, subNames[0].c_str());

	while (thisBlock && thisBlock->string != subNames.back())
	{
		if (thisBlock->string == subNames[level])
		{
			if (cJSON_IsArray(thisBlock))
				thisBlock = cJSON_GetArrayItem(thisBlock, 0)->child;
			else
				thisBlock = thisBlock->child;
			level++;
		}
		else
			thisBlock = thisBlock->next;
	}

	if (thisBlock != NULL)
		jsonItem = thisBlock;
	else
		throw NOT_FOUND;

	return jsonItem;
}

// iteratively fetch values, returns false when done. It modifies a
// pointer to a cJSON object so each found value can be cast to the proper
// data type.

bool APIRESULT::getEach(string namePath, cJSON *&jsonObject)
{
	static int index = 0;
	bool status = true;
	static string lastPath = "";

	if (namePath != lastPath)
	{
		index = 0;
		lastPath = namePath;
	}

	if (index < jsonLength)
	{
		jsonObject = findItem(namePath, index);
		index++;
	}
	else
	{
		jsonObject = NULL;
		status = false;
	}

	return jsonObject;
}

// PUBLIC methods...

DATATYPE APIRESULT::getDataType(string name, int entry)
{

	cJSON *foundItem;

	try
	{
		foundItem = findItem(name, entry);
		if (foundItem == NULL)
			return MISSING;
	}
	catch (API_ERROR &apiErr)
	{
		if (apiErr == NOT_FOUND)
			return MISSING;
		if (apiErr == NULL_VALUE)
			return EMPTY;
	}

	if (cJSON_IsString(foundItem))
		return TEXT;
	if (cJSON_IsNumber(foundItem))
		return NUMBER;
	if (cJSON_IsBool(foundItem))
		return cJSON_BOOLEAN;
	if (cJSON_IsArray(foundItem))
		return ARRAY;
	if (cJSON_IsObject(foundItem))
		return OBJECT;

	return MISSING;
}

string APIRESULT::getStringValue(string namePath, int entry)
{
	string valueString = "";
	try
	{
		valueString = findItem(namePath, entry)->valuestring;
	}
	catch (API_ERROR &apiErr)
	{
		throw apiErr;
	}

	return valueString;
}

double APIRESULT::getFloatValue(string name, int entry)
{
	double apiValue = -999.0;
	try
	{
		apiValue = findItem(name, entry)->valuedouble;
	}
	catch (API_ERROR &apiErr)
	{
		throw apiErr;
	}

	return apiValue;
}

bool APIRESULT::getBoolValue(string name, int entry)
{
	bool apiValue = false;
	try
	{
		apiValue = cJSON_IsTrue(findItem(name, entry));
	}
	catch (API_ERROR &apiErr)
	{
		throw apiErr;
	}

	return apiValue;
}

cJSON *APIRESULT::getJsonArray(string name, int entry)
{
	cJSON *apiPointer = NULL;
	try
	{
		apiPointer = findItem(name, entry);
	}
	catch (API_ERROR &apiErr)
	{
		throw apiErr;
	}

	return apiPointer;
}

bool APIRESULT::getEachString(string namePath, string &returnValue)
{
	cJSON *foundItem;
	bool status = false;

	try
	{
		if (status = getEach(namePath, foundItem))
			returnValue = foundItem->valuestring;
		else
			returnValue = "";
	}
	catch (API_ERROR &apiErr)
	{
		throw apiErr;
	}

	return status;
}

bool APIRESULT::getEachFloat(string namePath, float &returnValue)
{
	cJSON *foundItem;
	bool status = false;

	try
	{
		if (status = getEach(namePath, foundItem))
			returnValue = foundItem->valuedouble;
		else
			returnValue = NOT_FOUND;
	}
	catch (API_ERROR &apiErr)
	{
		throw apiErr;
	}

	return status;
}

bool APIRESULT::getEachBool(string namePath, bool &returnValue)
{
	cJSON *foundItem;
	bool status = false;

	try
	{
		if (status = getEach(namePath, foundItem))
			returnValue = cJSON_IsTrue(foundItem);
		else
			returnValue = false;
	}
	catch (API_ERROR &apiErr)
	{
		throw apiErr;
	}

	return status;
}

bool APIRESULT::getEachArray(string namePath, cJSON *&jsonArray)
{
	bool status = false;

	try
	{
		status = getEach(namePath, jsonArray);
	}
	catch (API_ERROR &apiErr)
	{
		throw apiErr;
	}

	return status;
}
