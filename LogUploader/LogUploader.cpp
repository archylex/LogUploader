// LogUploader.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>
#include <string.h>
#include <curl/curl.h>

#define BUFFER_SIZE 1024

#pragma comment(lib, "libcurl_a.lib")

using namespace std;

void getContentFromFile(const char * filename, char *data) {
	FILE* file;
	char line[BUFFER_SIZE];
	errno_t errFile;

	errFile = fopen_s(&file, filename, "rb");

	if (errFile == 0) {
		while (fgets(line, BUFFER_SIZE, file)) {			
			strcat(data, line);
		}

		fclose(file);

		printf("*** The file '%s' is open.\n", filename);
	} else {
		printf("*** The file '%s' doesn't open.\n", filename);
	}
}

void sendFile(const char* url, const char* filename) {
	CURL* curl;
	CURLcode res;
	struct curl_httppost* post = NULL;
	struct curl_httppost* last = NULL;
	struct curl_slist* headerlist = NULL;
	static const char buf[] = "Expect:";		
	static char data[6400];

	getContentFromFile(filename, data);

	curl_global_init(CURL_GLOBAL_ALL);

	curl_formadd(&post, &last,
		CURLFORM_COPYNAME, "cache-control:",
		CURLFORM_COPYCONTENTS, "no-cache",
		CURLFORM_END);

	curl_formadd(&post, &last,
		CURLFORM_COPYNAME, "content-type:",
		CURLFORM_COPYCONTENTS, "multipart/form-data",
		CURLFORM_END);

	curl_formadd(&post, &last,
		CURLFORM_COPYNAME, "uploaded_file",
		CURLFORM_BUFFER, filename,
		CURLFORM_BUFFERPTR, data,
		CURLFORM_BUFFERLENGTH, strlen(data),
		CURLFORM_END);

	curl = curl_easy_init();

	headerlist = curl_slist_append(headerlist, buf);

	if (curl) {
		printf("*** The response from server:\n");
		curl_easy_setopt(curl, CURLOPT_URL, url);
		curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);

		res = curl_easy_perform(curl);

		if (res != CURLE_OK)
			fprintf(stderr, "*** %s\n*** There was an error uploading the file.\n",
				curl_easy_strerror(res));
		else {
			long response_code;
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

			if (response_code == 200)
				printf("\n*** The file '%s' has been uploaded.\n", filename);
			else
				printf("*** There was an error uploading the file.\n");
		}

		curl_easy_cleanup(curl);

		curl_formfree(post);

		curl_slist_free_all(headerlist);
	}
	else {
		printf("*** We have problems with CURL.\n");
	}
}

void distributionOfFile(const char* filename, const char* page) {
	FILE* file;
	char line[BUFFER_SIZE];
	errno_t errFile;

	errFile = fopen_s(&file, "mirrors.list", "r");

	if (errFile == 0) {
		printf("*** Distribution of a file to mirrors:\n");

		while (fgets(line, BUFFER_SIZE, file)) {
			if (!feof(file))
				line[strlen(line) - 1] = '\0';

			printf("*** Try to send to %s \n", line);			

			if (strlen(page) > 0)
				strncat_s(line, page, BUFFER_SIZE - strlen(line) - 1);
						
			sendFile(line, filename);
		}

		fclose(file);		
	} else {
		printf("*** The file 'mirrors.list' doesn't open.\n");
	}
}

int main()
{
	const char* page = "upload.php";
	const char* filename = "info.log";

	distributionOfFile(filename, page);
		
	return 0;
}