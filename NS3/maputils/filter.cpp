#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include <cstdio>
#include <iostream>
#include <assert.h>

using namespace rapidjson;

int main() {
    // 1. Parse a JSON string into DOM.
    FILE* fp = fopen("mapr", "r");
    char readBuffer[65536];
    FileReadStream is(fp, readBuffer, sizeof(readBuffer));
    Document d;
    d.ParseStream(is);
    fclose(fp);

    Value & features = d["features"];
    assert(features.IsArray());
	int i=0;
	for (Value::ConstValueIterator itr = features.Begin(); itr != features.End(); ++itr)
	{  
		
		//const Value & v
		assert((*itr).HasMember("properties"));
		assert(itr->IsObject());  
		assert(itr->HasMember("properties"));
		//Value & prop = (*itr)["properties"];
	    	if ( !(*itr)["properties"].HasMember("highway") ) {
			//std::cout<< (*itr)["properties"]["osm_id"].GetInt() <<std::endl;
			d["features"].Erase(itr);
			itr--;
			
		} else if ( (*itr)["properties"].HasMember("area") ) {
			//std::cout<< (*itr)["properties"]["osm_id"].GetInt() <<std::endl;
			d["features"].Erase(itr);
			itr--;
			
		} 
        }

	fp = fopen("output.json", "w"); // 非 Windows 平台使用 "w"
	char writeBuffer[65536];
	FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
	Writer<FileWriteStream> writer(os);
	d.Accept(writer);
	fclose(fp);
	return 0;
}
