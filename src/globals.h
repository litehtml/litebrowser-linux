#pragma once

#include <fstream>
#include <string>
#include <cerrno>
#include <clocale>
#include <vector>
#include <iostream>
#include <cstdlib>
#if defined __APPLE__ || __OpenBSD__ 
#include <stdlib.h>
#else
#include <malloc.h>
#endif
#include <memory.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include <cairo.h>
#include <gtkmm.h>
#include <gtkmm/application.h>
#include <gtkmm/window.h>
#include "../litehtml/include/litehtml.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <fontconfig/fontconfig.h>
#include <cairo-ft.h>
#include <gdk/gdk.h>
#include <cairomm/context.h>
#include <curl/curl.h>
#include <Poco/URI.h>
