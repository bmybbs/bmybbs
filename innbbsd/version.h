/*  Version information */
/*
$Id: version.h,v 1.1.1.1 2009-03-04 06:33:28 bmybbs Exp $
*/

#ifndef _VERSION_H_
#define _VERSION_H_

#ifndef INNBBSDVERSION
#  define INNBBSDVERSION	"0.50beta-5F"
#endif

#ifndef NCMVERSION
#  define NCMVERSION    	"NoCeM_0.63"
#endif

#ifdef USE_NCM_PATCH
#  define VERSION       	INNBBSDVERSION"_"NCMVERSION
#else
#  define VERSION       	INNBBSDVERSION
#endif

#endif
