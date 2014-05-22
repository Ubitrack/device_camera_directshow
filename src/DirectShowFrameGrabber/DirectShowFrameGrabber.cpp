/*
 * Ubitrack - Library for Ubiquitous Tracking
 * Copyright 2006, Technische Universitaet Muenchen, and individual
 * contributors as indicated by the @authors tag. See the
 * copyright.txt in the distribution for a full listing of individual
 * contributors.
 *
 * This is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this software; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA, or see the FSF site: http://www.fsf.org.
 */

/**
 * @ingroup vision_components
 * @file
 * Reads camera images using DirectShow
 *
 * @author Daniel Pustka <daniel.pustka@in.tum.de>
 */

#include <utUtil/CleanWindows.h>
#include <objbase.h>

#ifdef HAVE_DIRECTSHOW
	#include <DShow.h>
	#pragma include_alias( "dxtrans.h", "qedit.h" )
	#define __IDxtCompositor_INTERFACE_DEFINED__
	#define __IDxtAlphaSetter_INTERFACE_DEFINED__
	#define __IDxtJpeg_INTERFACE_DEFINED__
	#define __IDxtKey_INTERFACE_DEFINED__
	#include <qedit.h>
	#include <strmif.h>
#else
	// this include file contains just the directshow interfaces necessary to compile this component
	// if you need more, install the Windows SDK, the DirectX SDK (old version with dxtrans.h, e.g. August 2007) and
	// use the alternative above
	#include "DirectShowInterfaces.h"
#endif

#include "AutoComPtr.h"

#include <string>
#include <list>
#include <iostream>
#include <iomanip>
#include <strstream>
#include <log4cpp/Category.hh>

#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/scoped_array.hpp>

#include <utDataflow/PushSupplier.h>
#include <utDataflow/PullSupplier.h>
#include <utDataflow/Component.h>
#include <utDataflow/ComponentFactory.h>
#include <utMeasurement/Measurement.h>
#include <utMeasurement/TimestampSync.h>
#include <utUtil/OS.h>
#include <opencv/cv.h>
#include <utVision/Image.h>
#include <utVision/Undistortion.h>


// get a logger
static log4cpp::Category& logger( log4cpp::Category::getInstance( "Ubitrack.Vision.DirectShowFrameGrabber" ) );

using namespace Ubitrack;
using namespace Ubitrack::Vision;

namespace Ubitrack { namespace Drivers {

/**
 * @ingroup vision_components
 *
 * @par Input Ports
 * None.
 *
 * @par Output Ports
 * \c Output push port of type Ubitrack::Measurement::ImageMeasurement.
 *
 * @par Configuration
 * The configuration tag contains a \c <dsvl_input> configuration.
 * For details, see the DirectShow documentation...
 *
 */
class DirectShowFrameGrabber
	: public Dataflow::Component
	, protected ISampleGrabberCB
{
public:

	/** constructor */
	DirectShowFrameGrabber( const std::string& sName, boost::shared_ptr< Graph::UTQLSubgraph >  );

	/** destructor, waits until thread stops */
	~DirectShowFrameGrabber();

	/** starts the camera */
	void start();

	/** stops the camera */
	void stop();

protected:
	/** initializes the direct show filter graph */
	void initGraph();

	/** handles a frame after being converted to Vision::Image */
	void handleFrame( Measurement::Timestamp utTime, const Vision::Image& bufferImage );

	/** handler method for incoming pull requests */
	Measurement::Matrix3x3 getIntrinsic( Measurement::Timestamp t )
	{ return Measurement::Matrix3x3( t, m_undistorter->getIntrinsics() ); }

	// width of resulting image
	LONG m_sampleWidth;

	// height of resulting image
	LONG m_sampleHeight;

	// shift timestamps (ms)
	int m_timeOffset;

	// only send every nth image
	int m_divisor;

	/** desired width */
	int m_desiredWidth;

	/** desired image height */
	int m_desiredHeight;

	// desired camera name
	std::string m_desiredName;

	// desired camera index (e.g. for multiple cameras with the same name as the Vuzix HMD)
	std::string m_desiredDevicePath;


	/** exposure control */
	int m_cameraExposure;

	/** gain control */
	int m_cameraGain;

	/** brightness control */
	int m_cameraBrightness;

	/** number of frames received */
	int m_nFrames;

	/** timestamp of last frame */
	double m_lastTime;

	/** timestamp synchronizer */
	Measurement::TimestampSync m_syncer;

	/** undistorter */
	boost::shared_ptr<Vision::Undistortion> m_undistorter;

	// the ports
	Dataflow::PushSupplier< Measurement::ImageMeasurement > m_outPort;
	Dataflow::PushSupplier< Measurement::ImageMeasurement > m_colorOutPort;
	Dataflow::PullSupplier< Measurement::Matrix3x3 > m_intrinsicsPort;

	/** pointer to DirectShow filter graph */
	AutoComPtr< IMediaControl > m_pMediaControl;

    // ISampleGrabberCB: fake reference counting.
    STDMETHODIMP_(ULONG) AddRef() 
	{ return 1; }

    STDMETHODIMP_(ULONG) Release() 
	{ return 2; }

    STDMETHODIMP QueryInterface( REFIID riid, void **ppvObject )
    {
		if ( NULL == ppvObject ) 
			return E_POINTER;
		if ( riid == __uuidof( IUnknown ) )
		{
			*ppvObject = static_cast<IUnknown*>( this );
			 return S_OK;
		}
		if ( riid == __uuidof( ISampleGrabberCB ) )
		{
			*ppvObject = static_cast<ISampleGrabberCB*>( this );
			 return S_OK;
		}
		return E_NOTIMPL;
	}

	STDMETHODIMP SampleCB( double Time, IMediaSample *pSample );

	STDMETHODIMP BufferCB( double Time, BYTE *pBuffer, long BufferLen )
	{
		LOG4CPP_INFO( logger, "BufferCB called" );
		return E_NOTIMPL;
	}
};


DirectShowFrameGrabber::DirectShowFrameGrabber( const std::string& sName, boost::shared_ptr< Graph::UTQLSubgraph > subgraph )
	: Dataflow::Component( sName )
	, m_timeOffset( 0 )
	, m_divisor( 1 )
	, m_desiredWidth( 320 )
	, m_desiredHeight( 240 )
	, m_cameraExposure( 0 )
	, m_cameraGain( 0 )
	, m_cameraBrightness( 0 )
	, m_nFrames( 0 )
	, m_lastTime( -1e10 )
	, m_syncer( 1.0 )
	//, m_undistorter( *subgraph )
	, m_outPort( "Output", *this )
	, m_colorOutPort( "ColorOutput", *this )
	, m_intrinsicsPort( "Intrinsics", *this, boost::bind( &DirectShowFrameGrabber::getIntrinsic, this, _1 ) )
{
	HRESULT hRes = CoInitializeEx( NULL, COINIT_MULTITHREADED );
	if ( hRes == RPC_E_CHANGED_MODE )
	{ 
		LOG4CPP_WARN( logger, "CoInitializeEx failed with RPC_E_CHANGED_MODE, continuing..." );
	}
	else if ( FAILED( hRes ) )
	{
		std::ostringstream os;
		os << "Error in CoInitializeEx:" << std::hex << hRes;
		UBITRACK_THROW( os.str() );
	}

	subgraph->m_DataflowAttributes.getAttributeData( "timeOffset", m_timeOffset );
	subgraph->m_DataflowAttributes.getAttributeData( "divisor", m_divisor );
	subgraph->m_DataflowAttributes.getAttributeData( "imageWidth", m_desiredWidth );
	subgraph->m_DataflowAttributes.getAttributeData( "imageHeight", m_desiredHeight );
	m_desiredDevicePath = subgraph->m_DataflowAttributes.getAttributeString( "devicePath" );
	m_desiredName = subgraph->m_DataflowAttributes.getAttributeString( "cameraName" );
	
	subgraph->m_DataflowAttributes.getAttributeData( "cameraExposure", m_cameraExposure );
	subgraph->m_DataflowAttributes.getAttributeData( "cameraGain", m_cameraGain );
	subgraph->m_DataflowAttributes.getAttributeData( "cameraBrightness", m_cameraBrightness );

	std::string intrinsicFile = subgraph->m_DataflowAttributes.getAttributeString( "intrinsicMatrixFile" );
	std::string distortionFile = subgraph->m_DataflowAttributes.getAttributeString( "distortionFile" );
	
	
	m_undistorter.reset(new Vision::Undistortion(intrinsicFile, distortionFile));

	initGraph();
}


DirectShowFrameGrabber::~DirectShowFrameGrabber()
{
	if ( m_pMediaControl )
		m_pMediaControl->Stop();
}


void DirectShowFrameGrabber::start()
{
	if ( !m_running && m_pMediaControl )
		m_pMediaControl->Run();

	Component::start();
}


void DirectShowFrameGrabber::stop()
{
	if ( m_running && m_pMediaControl )
		m_pMediaControl->Pause();
	
	Component::stop();
}


void DirectShowFrameGrabber::initGraph()
{
	// Create the System Device Enumerator.
	AutoComPtr< ICreateDevEnum > pDevEnum;
	AutoComPtr< IEnumMoniker > pEnum;

	HRESULT hr = pDevEnum.CoCreateInstance( CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER );
	if ( SUCCEEDED( hr ) )
		// Create an enumerator for the video capture category.
		hr = pDevEnum->CreateClassEnumerator( CLSID_VideoInputDeviceCategory, &pEnum.p, 0 );

	AutoComPtr< IMoniker > pMoniker;
	AutoComPtr< IMoniker > pSelectedMoniker;
	std::string sSelectedCamera;
	while ( pEnum->Next( 1, &pMoniker.p, NULL ) == S_OK )
	{
		if ( !pSelectedMoniker )
			pSelectedMoniker = pMoniker;

		AutoComPtr< IPropertyBag > pPropBag;
		hr = pMoniker->BindToStorage( 0, 0, IID_IPropertyBag, (void**)(&pPropBag.p) );
		if ( FAILED( hr ) )
		{
			pMoniker.Release();
			continue;  // Skip this one, maybe the next one will work.
		} 

		// Find the device of the camera.
		VARIANT varDevicePath;
		VariantInit( &varDevicePath );
		hr = pPropBag->Read( L"DevicePath", &varDevicePath, 0 );
		char sDevicePath[ 128 ];
		if ( SUCCEEDED( hr ) )
			// could be optimized somehow ..
			WideCharToMultiByte( CP_ACP, 0, varDevicePath.bstrVal, -1, sDevicePath, 128, 0, 0 );

		// Find the description or friendly name.
		VARIANT varName;
		VariantInit( &varName );
		hr = pPropBag->Read( L"Description", &varName, 0 );
		if ( FAILED( hr ) )
			hr = pPropBag->Read( L"FriendlyName", &varName, 0 );

		

		if ( SUCCEEDED( hr ) )
		{
			char sName[ 128 ];
			WideCharToMultiByte( CP_ACP, 0, varName.bstrVal, -1, sName, 128, 0, 0 );
			LOG4CPP_INFO( logger, "Possible capture device: " << &sName[0] << " device path: " << sDevicePath );

			// select device based on name
			if ( !m_desiredName.empty() && strstr( sName, m_desiredName.c_str() ) )
			{
				if( m_desiredDevicePath.empty() )
				{
					sSelectedCamera = sName;
					pSelectedMoniker = pMoniker;
				}
				else if ( strstr( sDevicePath, m_desiredDevicePath.c_str() ) )
				{
					sSelectedCamera = sName;
					pSelectedMoniker = pMoniker;
				}
			}

			VariantClear( &varName ); 
		}

		pMoniker.Release();
	}

	// check if a capture device was found
	if ( !pSelectedMoniker )
		UBITRACK_THROW( "No video capture device found" );

	LOG4CPP_INFO( logger, "Using camera: " << sSelectedCamera );

	// create capture graph
    AutoComPtr< IGraphBuilder > pGraph;
    AutoComPtr< ICaptureGraphBuilder2 > pBuild;

    // Create the Capture Graph Builder.
    hr = pBuild.CoCreateInstance( CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC_SERVER );
    if ( SUCCEEDED( hr ) )
    {
        // Create the Filter Graph Manager.
        hr = pGraph.CoCreateInstance( CLSID_FilterGraph, 0, CLSCTX_INPROC_SERVER );
        if ( SUCCEEDED( hr ) )
            // Initialize the Capture Graph Builder.
            pBuild->SetFiltergraph( pGraph );
        else
			UBITRACK_THROW( "Error creating filter graph manager" );
    }
	else
		UBITRACK_THROW( "Error creating capture graph builder" );

	// create capture device filter
	AutoComPtr< IBaseFilter > pCaptureFilter;
	if ( FAILED( pSelectedMoniker->BindToObject( 0, 0, IID_IBaseFilter, (void**)&pCaptureFilter.p ) ) )
		UBITRACK_THROW( "Unable to create capture filter" );

	if ( FAILED( pGraph->AddFilter( pCaptureFilter, L"Capture" ) ) )
		UBITRACK_THROW( "Unable to add capture filter" );

	// find output pin for configuration
	AutoComPtr< IPin > pPin;
	if ( FAILED( pBuild->FindPin( pCaptureFilter, PINDIR_OUTPUT, &PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video, FALSE, 0, &pPin.p ) ) )
		UBITRACK_THROW( "Unable to find pin" );

	// enumerate media types
	AutoComPtr< IAMStreamConfig > pStreamConfig;
	if ( FAILED( pPin.QueryInterface< IAMStreamConfig >( pStreamConfig ) ) )
	{ LOG4CPP_WARN( logger, "Unable to get IAMStreamConfig interface" ); }
	else
	{
		int iCount, iSize;
		pStreamConfig->GetNumberOfCapabilities( &iCount, &iSize );
		boost::scoped_array< BYTE > buf( new BYTE[ iSize ] );

		bool bSet = false;
		for ( int iCap = 0; iCap < iCount; iCap++ )
		{
			AM_MEDIA_TYPE *pMediaType;
			pStreamConfig->GetStreamCaps( iCap, &pMediaType, buf.get() );

			if ( pMediaType->majortype != MEDIATYPE_Video || pMediaType->formattype != FORMAT_VideoInfo )
				continue;
			VIDEOINFOHEADER* pInfo = (VIDEOINFOHEADER*)pMediaType->pbFormat;

			LOG4CPP_INFO( logger, "Media type: fps=" << 1e7 / pInfo->AvgTimePerFrame << 
				", width=" << pInfo->bmiHeader.biWidth << ", height=" << pInfo->bmiHeader.biHeight <<
				", type=" << ( pMediaType->subtype == MEDIASUBTYPE_RGB24 ? "RGB24" : "?" ) );

			// set first format with correct size, but prefer RGB24 
			if ( ( m_desiredWidth <= 0 || pInfo->bmiHeader.biWidth == m_desiredWidth ) && 
				( m_desiredHeight <= 0 || pInfo->bmiHeader.biHeight == m_desiredHeight ) &&
				( !bSet || pMediaType->subtype == MEDIASUBTYPE_RGB24 ) )
			{
				pStreamConfig->SetFormat( pMediaType );
				if ( bSet )
					break;
				bSet = true;
			}

			// TODO: DeleteMediaType
		}
	}

	// create sample grabber filter
	AutoComPtr< IBaseFilter > pSampleGrabberFilter;
	if ( FAILED( pSampleGrabberFilter.CoCreateInstance( CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER ) ) )
		UBITRACK_THROW( "Unable to create sample grabber filter" );

	if ( FAILED( pGraph->AddFilter( pSampleGrabberFilter, L"SampleGrab" ) ) )
		UBITRACK_THROW( "Unable to add sample grabber filter" );

	// configure sample grabber
	AutoComPtr< ISampleGrabber > pSampleGrabber;
	pSampleGrabberFilter.QueryInterface( pSampleGrabber );
	pSampleGrabber->SetOneShot( FALSE );
	pSampleGrabber->SetBufferSamples( FALSE );
	pSampleGrabber->SetCallback( this, 0 ); // 0 = Use the SampleCB callback method.




	// make it picky on media types
	AM_MEDIA_TYPE mediaType;
	memset( &mediaType, 0, sizeof( mediaType ) );
	mediaType.majortype = MEDIATYPE_Video;
	mediaType.subtype = MEDIASUBTYPE_RGB24;
	pSampleGrabber->SetMediaType( &mediaType );

	// null renderer
	AutoComPtr< IBaseFilter > pNullRenderer;
	if ( FAILED( pNullRenderer.CoCreateInstance( CLSID_NullRenderer, NULL, CLSCTX_INPROC_SERVER ) ) )
		UBITRACK_THROW( "Unable to create null renderer filter" );

	if ( FAILED( pGraph->AddFilter( pNullRenderer, L"NullRender" ) ) )
		UBITRACK_THROW( "Unable to add null renderer filter" );

	// connect all filters
	hr = pBuild->RenderStream(
		&PIN_CATEGORY_CAPTURE, // Connect this pin ...
		&MEDIATYPE_Video,      // with this media type ...
		pCaptureFilter,        // on this filter ...
		pSampleGrabberFilter,  // to the Sample Grabber ...
		pNullRenderer );       // ... and finally to the Null Renderer.
	if ( FAILED( hr ) )
		UBITRACK_THROW( "Unable to render stream" );

	// get media type
	pSampleGrabber->GetConnectedMediaType( &mediaType );
	if ( mediaType.majortype != MEDIATYPE_Video || mediaType.subtype != MEDIASUBTYPE_RGB24 ||
		mediaType.formattype != FORMAT_VideoInfo )
		UBITRACK_THROW( "Unsupported MEDIATYPE" );

	VIDEOINFOHEADER* pVidInfo = (VIDEOINFOHEADER*)mediaType.pbFormat;
	m_sampleWidth = pVidInfo->bmiHeader.biWidth;
	m_sampleHeight = pVidInfo->bmiHeader.biHeight;
	double fps = (1.0 / pVidInfo->AvgTimePerFrame) * 10000000.0;
	LOG4CPP_INFO( logger, "Image dimensions: " << m_sampleWidth << "x" << m_sampleHeight << " FPS: " << fps );
	// TODO: FreeMediaType( &mediaType );



#ifdef HAVE_DIRECTSHOW
	/* additionally control camera parameters infos at:
	 * http://msdn.microsoft.com/en-us/library/dd318253(v=vs.85).aspx
	 */
	IAMCameraControl *pCameraControl; 
	//HRESULT hr; 
	hr = pCaptureFilter->QueryInterface(IID_IAMCameraControl, (void **)&pCameraControl); 
	if ( SUCCEEDED(hr) ) {
	  //hr = pCameraControl->GetRange(CameraControl_Exposure,
			//						NULL, // min
			//						NULL, // max
			//						NULL, // minstep
			//						&defaultExposureValue, // default
			//						NULL); // capflags
		if (m_cameraExposure != 0) {
			hr = pCameraControl->Set(CameraControl_Exposure, // property
									m_cameraExposure, // value
									CameraControl_Flags_Manual); 
			// check for errors
		}
	}

	IAMVideoProcAmp *pAMVideoProcAmp;
	hr = pCaptureFilter->QueryInterface(IID_IAMVideoProcAmp, (void**)&pAMVideoProcAmp);
	if (SUCCEEDED(hr)) {

		if (m_cameraGain != 0) {
			hr = pAMVideoProcAmp->Set(VideoProcAmp_Gain, m_cameraGain, VideoProcAmp_Flags_Manual);
			// check for errors
		}
		if (m_cameraBrightness != 0) {
			hr = pAMVideoProcAmp->Set(VideoProcAmp_Brightness, m_cameraBrightness, VideoProcAmp_Flags_Manual);
			// check for errors
		}
	}

#endif


	// start stream
	pGraph.QueryInterface< IMediaControl >( m_pMediaControl );
	m_pMediaControl->Pause();
}


void DirectShowFrameGrabber::handleFrame( Measurement::Timestamp utTime, const Vision::Image& bufferImage )
{
	boost::shared_ptr< Vision::Image > pColorImage;
	bool bColorImageDistorted = true;
	
	if ( ( m_desiredWidth > 0 && m_desiredHeight > 0 ) && 
		( bufferImage.width > m_desiredWidth || bufferImage.height > m_desiredHeight ) )
	{
	    LOG4CPP_DEBUG( logger, "downsampling" );
		pColorImage.reset( new Vision::Image( m_desiredWidth, m_desiredHeight, 3 ) );
		pColorImage->origin = bufferImage.origin;
		cvResize( bufferImage, *pColorImage );
	}

	if ( m_colorOutPort.isConnected() )
	{
		if ( pColorImage )
			pColorImage = m_undistorter->undistort( pColorImage );
		else
			pColorImage = m_undistorter->undistort( bufferImage );
		bColorImageDistorted = false;

		memcpy( pColorImage->channelSeq, "BGR", 4 );
		
		m_colorOutPort.send( Measurement::ImageMeasurement( utTime, pColorImage ) );
	}

	if ( m_outPort.isConnected() )
	{
		boost::shared_ptr< Vision::Image > pGreyImage;
		if ( pColorImage )
			pGreyImage = pColorImage->CvtColor( CV_BGR2GRAY, 1 );
		else
			pGreyImage = bufferImage.CvtColor( CV_BGR2GRAY, 1 );
		
		if ( bColorImageDistorted )
			pGreyImage = m_undistorter->undistort( pGreyImage );
		
		m_outPort.send( Measurement::ImageMeasurement( utTime, pGreyImage ) );
	}
}


STDMETHODIMP DirectShowFrameGrabber::SampleCB( double Time, IMediaSample *pSample )
{
	// TODO: check for double frames when using multiple cameras...
	LOG4CPP_DEBUG( logger, "SampleCB called" );

	if ( Time == m_lastTime )
	{
		// this was a problem with DSVideoLib and multiple cameras
		LOG4CPP_INFO( logger, "Got double frame" );
		return S_OK;
	}
	m_lastTime = Time;

	if ( !m_running || ( ++m_nFrames % m_divisor ) )
		return S_OK;

	if ( pSample->GetSize() < m_sampleWidth * m_sampleHeight * 3 )
	{
		LOG4CPP_INFO( logger, "Invalid sample size" );
		return S_OK;
	}

	BYTE* pBuffer;
	if ( FAILED( pSample->GetPointer( &pBuffer ) ) )
	{
		LOG4CPP_INFO( logger, "GetPointer failed" );
		return S_OK;
	}

	// create IplImage, convert and send
	Vision::Image bufferImage( m_sampleWidth, m_sampleHeight, 3, pBuffer, IPL_DEPTH_8U, 1 );
	Measurement::Timestamp utTime = m_syncer.convertNativeToLocal( Time );

	handleFrame( utTime + 1000000L * m_timeOffset, bufferImage );

	return S_OK;
}

} } // namespace Ubitrack::Driver

UBITRACK_REGISTER_COMPONENT( Dataflow::ComponentFactory* const cf ) {
	cf->registerComponent< Ubitrack::Drivers::DirectShowFrameGrabber > ( "DirectShowFrameGrabber" );
}

