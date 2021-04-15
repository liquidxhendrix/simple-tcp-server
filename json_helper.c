#include "json_helper.h"

void shapes_to_JSON(ShapeType *shapes, char *json_string,size_t size){

/*
char* curr=json_string;

curr+=snprintf(curr,size,"{");
curr+=snprintf(curr,size,"color:'%s'",shapes->color);
curr+=snprintf(curr,size,",");
curr+=snprintf(curr,size,"x:%lu",shapes->x);
curr+=snprintf(curr,size,",");
curr+=snprintf(curr,size,"y:%lu",shapes->y);
curr+=snprintf(curr,size,",");
curr+=snprintf(curr,size,"shapesize:%lu",shapes->shapesize);
curr+=snprintf(curr,size,"}");
curr+=snprintf(curr,size,"\n");
*/


 cJSON *color = NULL;
 cJSON *x = NULL;
 cJSON *y = NULL;
 cJSON *shapesize = NULL;
 char *string = NULL;

 cJSON *shapeData = cJSON_CreateObject();

if (NULL == shapeData)
{
    goto end;
}

if (NULL==cJSON_AddStringToObject(shapeData, "color", shapes->color))
{
    goto end;
}


if (NULL==cJSON_AddNumberToObject(shapeData, "x", shapes->x))
{
    goto end;
}


if (NULL==cJSON_AddNumberToObject(shapeData, "y", shapes->y))
{
    goto end;
}


if (NULL==cJSON_AddNumberToObject(shapeData, "shapesize", shapes->shapesize))
{
    goto end;
}

string = cJSON_Print(shapeData);

strncpy(json_string,string,size);


end:
    free(string);
    cJSON_Delete(shapeData);

}


void JSON_to_shapes(char *json_string,size_t size,ShapeType *shapes)
{

cJSON *shapes_json = cJSON_Parse(json_string);

cJSON *color = cJSON_GetObjectItemCaseSensitive(shapes_json, "color");
cJSON *x = cJSON_GetObjectItemCaseSensitive(shapes_json, "x");
cJSON *y = cJSON_GetObjectItemCaseSensitive(shapes_json, "y");
cJSON *shapesize = cJSON_GetObjectItemCaseSensitive(shapes_json, "shapesize");


shapes->color = color->valuestring;
shapes->x = x->valuedouble;
shapes->y = y->valuedouble;
shapes->shapesize = shapesize->valuedouble;

}