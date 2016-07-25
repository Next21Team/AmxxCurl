#include <amxmodx>
#include <amxmisc>
#include <curl>

#define CURL_BUFFER_SIZE 512

public plugin_init()
{
   register_plugin("curl http get", "1.0", "Polarhigh")
   
   new data[1]
   data[0] = fopen("addons/amxmodx/alliedmods_main_page.html", "wb")
   server_print("curl start")
   
   new CURL:curl = curl_easy_init()
   curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, CURL_BUFFER_SIZE)
   curl_easy_setopt(curl, CURLOPT_URL, "https://forums.alliedmods.net")
   curl_easy_setopt(curl, CURLOPT_WRITEDATA, data[0])
   curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, "write")
   curl_easy_perform(curl, "complite", data, sizeof(data))
}

public write(data[], size, nmemb, file)
{
   new actual_size = size * nmemb;
   
   fwrite_blocks(file, data, actual_size, BLOCK_CHAR)
   
   return actual_size
}

public complite(CURL:curl, CURLcode:code, data[])
{
  if(code == CURLE_WRITE_ERROR)
     server_print("transfer aborted")
  else
     server_print("curl complete")
  
  fclose(data[0])
  curl_easy_cleanup(curl)
}
