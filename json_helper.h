#ifndef JSON_HELPER
#define JSON_HELPER

#include "shapetype.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cjson/cJSON.h>

extern "C" void shapes_to_JSON(ShapeType *shapes, char *json_string,size_t size);
extern "C" void JSON_to_shapes(char *json_string,size_t size,ShapeType *shapes); 

#endif