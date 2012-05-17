#ifndef MAIN_H
#define MAIN_H

#include <memory.h>
#include <string>
#include <list>
#include <map>
#include <vector>
#include <sstream>
#include <iostream>
#include <exception>
using namespace std;

#include "../gambas.h"
#include "../gb_common.h"

#define DEBUG std::cerr << "XMLDBG : (" << __FILE__ << ":" <<__LINE__ << ") "
#define DEBUGH DEBUG << endl


#ifndef __MAIN_CPP
extern "C" GB_INTERFACE GB;
#endif


#endif // MAIN_H
