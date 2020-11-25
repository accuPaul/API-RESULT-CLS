// Implementation file to fetch JSON from the API via cUrl.
//
// Paul Mount	04-MAR-2019  v1.0 Created
//

#include "webservices.hpp"

bool loadJSONfile(string filename, char *&data)
{
    FILE *inFile = NULL;
    long len = 0;

    // open in read binary mode
    if ((inFile = fopen(filename.c_str(), "rb")) == NULL)
    {
        cerr << "Error loading JSON: " << filename
             << " could not be opened. " << strerror(errno) << endl;
        return false;
    }

    // get the length of the file
    fseek(inFile, 0, SEEK_END);
    len = ftell(inFile);
    fseek(inFile, 0, SEEK_SET);

    // allocate the memory for the JSON data
    try
    {
        data = (char *)malloc(len + 1);
        // read the file into the data variable
        fread(data, 1, len, inFile);
        data[len] = '\0';
    }
    catch (exception &e)
    {
        cerr << "JSON could not be loaded. Error = " << e.what() << endl;
        return false;
    }

    // close the data file
    fclose(inFile);

    return true;
}

bool loadJSONfile(string fileName, cJSON *&jsonPointer)
{
    char *dataPointer;

    try
    {
        if (!loadJSONfile(fileName, dataPointer))
            return false;
        jsonPointer = cJSON_Parse(dataPointer);
    }
    catch (exception &e)
    {
        cerr << "Error loading file in cJSON: " << e.what() << endl;
        return false;
    }
    return !cJSON_IsInvalid(jsonPointer);
}

bool loadJSONapi(string urlString, char *&dataPtr)
{
    /* curl variables */
    CURL *curl_handle;
    CURLcode res;
    struct MemoryStruct chunk;

    try
    {
        chunk.memory = (char *)malloc(1); /* will be grown as needed by the realloc above */
        chunk.size = 0;                   /* no data at this point */

        curl_global_init(CURL_GLOBAL_ALL);

        /* init the curl session */
        curl_handle = curl_easy_init();

        /* specify URL to get */
        curl_easy_setopt(curl_handle, CURLOPT_URL, urlString.c_str());

        /* send all data to this function  */
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

        /* we pass our 'chunk' struct to the callback function */
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);

        /* some servers don't like requests that are made without a user-agent
           field, so we provide one */
        curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        /* we don't care about SSL for these functions (I hope!) */
        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl_handle, CURLOPT_CERTINFO, 0L);
        curl_easy_setopt(curl_handle, CURLOPT_CAINFO, NULL);

        /* get it! */
        res = curl_easy_perform(curl_handle);

        /* check for errors */
        if (res != CURLE_OK)
        {
            cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << endl;
            return false;
        }
    }
    catch (exception &e)
    {
        cerr << "Error loading data from " << urlString << endl;
        cerr << e.what() << endl;
        return false;
    }
    /*
     * Now, our chunk.memory points to a memory block that is chunk.size
     * bytes big and contains the remote file. Return that pointer so
* calling program can do stuff with it.
      */

    dataPtr = chunk.memory;

    return true;
}

bool loadJSONapi(string urlString, cJSON *&jsonPointer)
{
    char *dataPointer;

    try
    {
        if (!loadJSONapi(urlString, dataPointer))
            return false;
        jsonPointer = cJSON_Parse(dataPointer);
    }
    catch (exception &e)
    {
        cerr << "Data could not be parsed by cJSON. " << endl;
        cerr << e.what() << endl;
        return false;
    }

    return !cJSON_IsInvalid(jsonPointer);
}

static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    try
    {
        mem->memory = (char *)realloc(mem->memory, mem->size + realsize + 1);
        if (mem->memory == NULL)
        {
            /* out of memory! */
            cerr << "not enough memory (realloc returned NULL)" << endl;
            return 0;
        }

        memcpy(&(mem->memory[mem->size]), contents, realsize);
        mem->size += realsize;
        mem->memory[mem->size] = 0;
    }
    catch (exception &e)
    {
        cerr << "Error allocating memory: " << e.what() << endl;
        return 0;
    }

    return realsize;
}
