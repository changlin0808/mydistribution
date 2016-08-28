#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include <chagnlin.c>
#include <unistd.h>

int main()
{
	cJSON* json = cJSON_CreateObject();
	cJSON_AddNumberToObject(json, "访问量", 100);
	char* out = cJSON_Print(json);
	printf("%s\n", out);
	cJSON_Delete(json);
	free(out);

	return 0;
}
