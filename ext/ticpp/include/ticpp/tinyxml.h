/*
www.sourceforge.net/projects/tinyxml
Original code (2.0 and earlier )copyright (c) 2000-2006 Lee Thomason (www.grinninglizard.com)

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/

/*
 * THIS FILE WAS ALTERED BY Matt Janisz, 12. October 2012.
 *
 * - added ticppapi.h include and TICPP_API dll-interface to support building DLL using VS200X
 */

#pragma once

#include <ticpp/visitor.h>
#include <ticpp/base.h>
#include <ticpp/printer.h>
#include <ticpp/node.h>
#include <ticpp/document.h>
#include <ticpp/attribute.h>
#include <ticpp/element.h>
#include <ticpp/comment.h>
#include <ticpp/text.h>
#include <ticpp/declaration.h>
#include <ticpp/unknown.h>
#include <ticpp/handle.h>
#include <ticpp/stylesheet.h>
