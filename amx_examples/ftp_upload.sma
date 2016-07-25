#pragma dynamic 16536

#include <amxmodx>
#include <amxmisc>
#include <curl>

#define CURL_BUFFER_SIZE 512

public plugin_init()
{
   register_plugin("FTP Upload", "1.0", "Polarhigh")
   
   // for an example we will send today's amxx log to the our ftp server
   new log_str[14], log_full_path[128], conn_string[256]
   get_time("L%Y%m%d.log", log_str, charsmax(log_str))
   formatex(log_full_path, charsmax(log_full_path), "addons/amxmodx/logs/%s", log_str)
   formatex(conn_string, charsmax(conn_string), "ftp://user:password@127.0.0.1:21/amx_logs/%s", log_str)
   
   transfer_file(log_full_path, conn_string)
}

public transfer_file(file_path[], ftp_uri[])
{      
   if(file_exists(file_path))
   {
      new CURL:curl = curl_easy_init()
      if(curl)
      {
         new data[1], fsize
         fsize = file_size(file_path)
         data[0] = fopen(file_path, "rb")
         
         
         curl_easy_setopt(curl, CURLOPT_BUFFERSIZE, CURL_BUFFER_SIZE)
         curl_easy_setopt(curl, CURLOPT_URL, ftp_uri)
         curl_easy_setopt(curl, CURLOPT_READFUNCTION, "read_callback")
         curl_easy_setopt(curl, CURLOPT_READDATA, data[0])
         curl_easy_setopt(curl, CURLOPT_INFILESIZE, fsize)
         curl_easy_setopt(curl, CURLOPT_UPLOAD, 1)
         curl_easy_perform(curl, "curl_complete", data, sizeof(data))
         
         server_print("transfer begin %s", file_path)
      }
      else
         server_print("curl init error")
   }
   else
      server_print("transfer err, file not found: %s", file_path)
}

public read_callback(buffer[], size, nmemb, file)
{
   return fread_blocks(file, buffer, nmemb, BLOCK_BYTE) // we consider size == 1
}

public curl_complete(CURL:curl, CURLcode:code, data[])
{   
   if(code == CURLE_READ_ERROR)
      server_print("transfer aborted")
   else if(code == CURLE_OK)
      server_print("transfer complete")
   else
   {
      new err[64]
      curl_easy_strerror(code, err, charsmax(err))
      server_print("transfer error: %s", err)
   }
      
   fclose(data[0])
   curl_easy_cleanup(curl)
}
/* AMXX-Studio Notes - DO NOT MODIFY BELOW HERE
*{\\ rtf1\\ ansi\\ deff0{\\ fonttbl{\\ f0\\ fnil Tahoma;}}\n\\ viewkind4\\ uc1\\ pard\\ lang1049\\ f0\\ fs16 \n\\ par }
*/
