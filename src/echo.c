/*
 * echo.c --
 *
 *	Produce a page containing all FastCGI inputs
 *
 *
 * Copyright (c) 1996 Open Market, Inc.
 *
 * See the file "LICENSE.TERMS" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */
#ifndef lint
static const char rcsid[] = "$Id: echo.c,v 1.5 1999/07/28 00:29:37 roberts Exp $";
#endif /* not lint */
#include "fcgi_config.h"
#include "cJSON.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#include "make_log.h"
#include "fcgi_stdio.h"
#include "util_cgi.h"
#include <stddef.h>
#include <stdarg.h>
#include <assert.h>
#include <hiredis/hiredis.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef _WIN32
#include <process.h>
#else
#define REDIS_LOG_MODULE          "database"
#define REDIS_LOG_PROC            "redis"

#define REDIS_COMMAND_SIZE        300            /* redis Command 指令最大长度 */
#define FIELD_ID_SIZE            100            /* redis hash表field域字段长度 */
#define VALUES_ID_SIZE           1024            /* redis        value域字段长度 */
extern char **environ;
#endif
void rop_test_reply_type(redisReply *reply)
{
	switch (reply->type) {
		case REDIS_REPLY_STATUS:
			LOG(REDIS_LOG_MODULE, REDIS_LOG_PROC, "[+][GMS_REDIS]=REDIS_REPLY_STATUS=[string] use reply->str to get data, reply->len get data len\n");
			break;
		case REDIS_REPLY_ERROR:
			LOG(REDIS_LOG_MODULE, REDIS_LOG_PROC, "[+][GMS_REDIS]=REDIS_REPLY_ERROR=[string] use reply->str to get data, reply->len get date len\n");
			break;
		case REDIS_REPLY_INTEGER:
			LOG(REDIS_LOG_MODULE, REDIS_LOG_PROC, "[+][GMS_REDIS]=REDIS_REPLY_INTEGER=[long long] use reply->integer to get data\n");
			break;
		case REDIS_REPLY_NIL:
			LOG(REDIS_LOG_MODULE, REDIS_LOG_PROC, "[+][GMS_REDIS]=REDIS_REPLY_NIL=[] data not exist\n");
			break;
		case REDIS_REPLY_ARRAY:
			LOG(REDIS_LOG_MODULE, REDIS_LOG_PROC, "[+][GMS_REDIS]=REDIS_REPLY_ARRAY=[array] use reply->elements to get number of data, reply->element[index] to get (struct redisReply*) Object\n");
			break;
		case REDIS_REPLY_STRING:
			LOG(REDIS_LOG_MODULE, REDIS_LOG_PROC, "[+][GMS_REDIS]=REDIS_REPLY_string=[string] use reply->str to get data, reply->len get data len\n");
			break;
		default:
			LOG(REDIS_LOG_MODULE, REDIS_LOG_PROC, "[-][GMS_REDIS]Can't parse this type\n");
			break;
	}
}
int main ()
{
	while (FCGI_Accept() >= 0) {

		printf("Content-type: application/json\r\n\r\n");
		// 读数据库,  得到文件名和url,路径名  时间
		redisContext *conn = redisConnect("127.0.0.1", 6379);	
		// 返回的是数组
		redisReply* r = redisCommand(conn, "hkeys FILE_URL");
//		rop_test_reply_type(r);
		redisReply* r1 = redisCommand(conn, "hlen FILE_URL");
		int len = r1->integer;
		freeReplyObject(r1);
			LOG("datalog", "datalog", "len = %d\n", len);
		int i = 0;	
		// 连接数据库
		// 拼接字符串   
		// 将这些信息拼接成json字符串, print->被重写了, 会封装成html文件,传递给web服务器,web服务器传递到前端
		cJSON *root = cJSON_CreateObject();
		cJSON* thm, *fld;
		cJSON_AddItemToObject(root, "games", thm=cJSON_CreateArray());
		for(i = 0; i < len; i++)
		{
			char*name1 = (r->element)[i]->str;
			char name[64] = { 0 };
			strcpy(name, name1);
			LOG("datalog", "name", "name[%d] = %s\n", i, name);
		r1 = redisCommand(conn, "hget FILE_URL %s", name);
		char* url1 = r1->str;
		char url[64] = { 0 };
		strcpy(url, url1);
		LOG("datalog", "url", "url[%d] = %s\n", i, url);
		cJSON_AddItemToArray(thm,fld=cJSON_CreateObject());

		cJSON_AddStringToObject(fld, "id", name);
		cJSON_AddNumberToObject(fld, "kind", 2);
		cJSON_AddStringToObject(fld, "title_m", name);
		cJSON_AddStringToObject(fld, "title_s", "文件title_s");
		cJSON_AddStringToObject(fld, "descrip", "2015-04-04");
		cJSON_AddStringToObject(fld, "picurl_m", "static/file_png/avi.png");
		// 这个url要改成  数据库中对应的url
		cJSON_AddStringToObject(fld, "url", url);
		cJSON_AddNumberToObject(fld, "pv", 0);
		cJSON_AddNumberToObject(fld, "hot", 1);
		freeReplyObject(r1);
		}
		char* str = cJSON_Print(root);
		printf("%s\n", str);
		//释放工作
		free(str);
		redisFree(conn);
		freeReplyObject(r);
		cJSON_Delete(root);
	} 
	return 0;
}
