#ifndef NOMINATIN_H
#define NOMINATIN_H

#include "type.h"

Coords forwardGeocode(const string& query);
string reverseGeocode(const Coords& coords);


#endif 