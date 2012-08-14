

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 7.00.0500 */
/* at Thu Jun 05 11:36:21 2008
 */
/* Compiler settings for DirectShowInterfaces.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, IID_IMediaControl,0x56a868b1,0x0ad4,0x11ce,0xb0,0x3a,0x00,0x20,0xaf,0x0b,0xa7,0x70);


MIDL_DEFINE_GUID(IID, IID_IPin,0x56a86891,0x0ad4,0x11ce,0xb0,0x3a,0x00,0x20,0xaf,0x0b,0xa7,0x70);


MIDL_DEFINE_GUID(IID, IID_ICreateDevEnum,0x29840822,0x5B84,0x11D0,0xBD,0x3B,0x00,0xA0,0xC9,0x11,0xCE,0x86);


MIDL_DEFINE_GUID(IID, IID_IFilterGraph,0x56a8689f,0x0ad4,0x11ce,0xb0,0x3a,0x00,0x20,0xaf,0x0b,0xa7,0x70);


MIDL_DEFINE_GUID(IID, IID_IGraphBuilder,0x56a868a9,0x0ad4,0x11ce,0xb0,0x3a,0x00,0x20,0xaf,0x0b,0xa7,0x70);


MIDL_DEFINE_GUID(IID, IID_ICaptureGraphBuilder2,0x93E5A4E0,0x2D50,0x11d2,0xAB,0xFA,0x00,0xA0,0xC9,0xC6,0xE3,0x8D);


MIDL_DEFINE_GUID(IID, IID_IMediaFilter,0x56a86899,0x0ad4,0x11ce,0xb0,0x3a,0x00,0x20,0xaf,0x0b,0xa7,0x70);


MIDL_DEFINE_GUID(IID, IID_IBaseFilter,0x56a86895,0x0ad4,0x11ce,0xb0,0x3a,0x00,0x20,0xaf,0x0b,0xa7,0x70);


MIDL_DEFINE_GUID(IID, IID_IAMStreamConfig,0xC6E13340,0x30AC,0x11d0,0xA1,0x8C,0x00,0xA0,0xC9,0x11,0x89,0x56);


MIDL_DEFINE_GUID(IID, IID_ISampleGrabberCB,0x0579154A,0x2B53,0x4994,0xB0,0xD0,0xE7,0x73,0x14,0x8E,0xFF,0x85);


MIDL_DEFINE_GUID(IID, IID_ISampleGrabber,0x6B652FFF,0x11FE,0x4fce,0x92,0xAD,0x02,0x66,0xB5,0xD7,0xC7,0x8F);


MIDL_DEFINE_GUID(IID, IID_IMediaSample,0x56a8689a,0x0ad4,0x11ce,0xb0,0x3a,0x00,0x20,0xaf,0x0b,0xa7,0x70);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



