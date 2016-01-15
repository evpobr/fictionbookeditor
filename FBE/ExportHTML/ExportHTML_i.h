

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0555 */
/* at Fri Jan 15 20:59:26 2016
 */
/* Compiler settings for ExportHTML.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 7.00.0555 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__


#ifndef __ExportHTML_i_h__
#define __ExportHTML_i_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __ExportHTMLPlugin_FWD_DEFINED__
#define __ExportHTMLPlugin_FWD_DEFINED__

#ifdef __cplusplus
typedef class ExportHTMLPlugin ExportHTMLPlugin;
#else
typedef struct ExportHTMLPlugin ExportHTMLPlugin;
#endif /* __cplusplus */

#endif 	/* __ExportHTMLPlugin_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"
#include "fbe.h"

#ifdef __cplusplus
extern "C"{
#endif 



#ifndef __ExportHTMLLib_LIBRARY_DEFINED__
#define __ExportHTMLLib_LIBRARY_DEFINED__

/* library ExportHTMLLib */
/* [uuid] */ 


EXTERN_C const IID LIBID_ExportHTMLLib;

EXTERN_C const CLSID CLSID_ExportHTMLPlugin;

#ifdef __cplusplus

class DECLSPEC_UUID("E242A6D3-84BF-4285-9FAA-160F95370668")
ExportHTMLPlugin;
#endif
#endif /* __ExportHTMLLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


