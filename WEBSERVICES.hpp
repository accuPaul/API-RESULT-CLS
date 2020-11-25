#ifndef __WEBSERVICES_HXX__
#define __WEBSERVICES_HXX__

// Header file to fetch JSON from the API via cUrl.
//
// Paul Mount	04-MAR-2019  v1.0 Created
//

#include <iostream>
#include <exception>
#include <string>
#include <curl/curl.h>
#include <cjson/cJSON.h>

using namespace std;

//using namespace AccuWeatherProgramControlObjects;

// This structure and function protoype are needed for cUrl. You don't need it
// for anything.
struct MemoryStruct
{
	char *memory;
	size_t size;
};
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb,
								  void *userp);

// If you've already pulled JSON into a file and want to load it into memory
// to parse (using cJSON_Parse or something else), use this routine.
bool loadJSONfile(string filename, char *&dataPtr);

// If you've already pulled JSON into a file and want to get back a cJSON
// object, use this routine.
bool loadJSONfile(string filename, cJSON *&dataPtr);

// If you've got a url and want to load the JSON into memory to parse into
// cJSON (or some other thing), use this.
bool loadJSONapi(string dataURL, char *&datPtr);

// If you've got a url and want to get back a cJSON object, use this.
bool loadJSONapi(string dataURL, cJSON *&dataJson);

#endif