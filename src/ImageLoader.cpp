// -*- C++ -*-
/*!
 * @file  ImageLoader.cpp
 * @brief Image Loader Component with Common Camera Interface
 * @date $Date$
 *
 * $Id$
 */

#include "ImageLoader.h"

// Module specification
// <rtc-template block="module_spec">
static const char* imageloader_spec[] =
  {
    "implementation_id", "ImageLoader",
    "type_name",         "ImageLoader",
    "description",       "Image Loader Component with Common Camera Interface",
    "version",           "1.0.0",
    "vendor",            "Kenichi Ohara, Meijo University",
    "category",          "ImageProcessin",
    "activity_type",     "PERIODIC",
    "kind",              "DataFlowComponent",
    "max_instance",      "1",
    "language",          "C++",
    "lang_type",         "compile",
    // Configuration variables
    "conf.default.FileName", "NULL",
    "conf.default.output_color_format", "RGB",
    "conf.default.compression_ratio", "100",
    // Widget
    "conf.__widget__.FileName", "text",
    "conf.__widget__.output_color_format", "radio",
    "conf.__widget__.compression_ratio", "slider.1",
    // Constraints
    "conf.__constraints__.output_color_format", "(RGB, JPEG, PNG)",
    "conf.__constraints__.compression_ratio", "0<=, >=100",
    ""
  };
// </rtc-template>

/*!
 * @brief constructor
 * @param manager Maneger Object
 */
ImageLoader::ImageLoader(RTC::Manager* manager)
    // <rtc-template block="initializer">
  : RTC::DataFlowComponentBase(manager),
    m_ImageOut("Image", m_Image)

    // </rtc-template>
{
}

/*!
 * @brief destructor
 */
ImageLoader::~ImageLoader()
{
}



RTC::ReturnCode_t ImageLoader::onInitialize()
{
  // Registration: InPort/OutPort/Service
  // <rtc-template block="registration">
  // Set InPort buffers
  
  // Set OutPort buffer
  addOutPort("Image", m_ImageOut);
  
  // Set service provider to Ports
  
  // Set service consumers to Ports
  
  // Set CORBA Service Ports
  
  // </rtc-template>

  // <rtc-template block="bind_config">
  // Bind variables and configuration variable
  bindParameter("FileName", m_FileName, "NULL");
  bindParameter("output_color_format", m_output_color_format, "RGB");
  bindParameter("compression_ratio", m_compression_ratio, "100");
  // </rtc-template>
  
  return RTC::RTC_OK;
}

/*
RTC::ReturnCode_t ImageLoader::onFinalize()
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t ImageLoader::onStartup(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t ImageLoader::onShutdown(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/


RTC::ReturnCode_t ImageLoader::onActivated(RTC::UniqueId ec_id)
{
	loadimage = cv::imread(m_FileName.c_str());
	if(loadimage.empty())
	{
		RTC_ERROR(("Cannot open image file!\n"));
		std::cout<<"Cannot open image file!"<<std::endl;
		return RTC::RTC_ERROR;
	}

	width = loadimage.cols;
	height = loadimage.rows;
	depth = loadimage.depth();
	nchannels = loadimage.channels();

	std::cout << "Image size:" << width << " x " << height << std::endl;
	std::cout << "Depth     :" << depth << std::endl;
	std::cout << "Channles  :" << loadimage.channels() << std::endl;

	//Change color format from RGB to BGR
	cv::cvtColor(loadimage, loadimage, CV_RGB2BGR);

	return RTC::RTC_OK;
}


RTC::ReturnCode_t ImageLoader::onDeactivated(RTC::UniqueId ec_id)
{
	//Release allocated image buffer
	if(!loadimage.empty())
		loadimage.release();
	return RTC::RTC_OK;

}


RTC::ReturnCode_t ImageLoader::onExecute(RTC::UniqueId ec_id)
{
	//***********************************************************************************
	//***********************************************************************************
	//Following part is the template for common camera interface
	//Don't change following part
	//***********************************************************************************
	//***********************************************************************************
		
	//Set timestamp
	setTimestamp(m_Image);
	m_Image.data.captured_time = m_Image.tm;

	
	//Set camera parameter to output structure data
	m_Image.data.intrinsic.matrix_element[0] = 0.0;
	m_Image.data.intrinsic.matrix_element[1] = 0.0;
	m_Image.data.intrinsic.matrix_element[2] = 0.0;
	m_Image.data.intrinsic.matrix_element[3] = 0.0;
	m_Image.data.intrinsic.matrix_element[4] = 0.0;
		
	//Allocate distortion_coefficient
	m_Image.data.intrinsic.distortion_coefficient.length(5);

	//clear allocated distortion_coefficient
	for(int j(0); j < 5; ++j)
		m_Image.data.intrinsic.distortion_coefficient[j] = 0.0;

	//Copy image parameter to output data based on TimedCameraImage structure
	m_Image.data.image.width = width;
	m_Image.data.image.height = height;

	//Transmission image data creation based on selected image compression mode
	if( m_output_color_format == "RGB")
	{
		m_Image.data.image.format = Img::CF_RGB;
		m_Image.data.image.raw_data.length( width * height * nchannels);
		for( int i(0); i< height; ++i )
			memcpy(&m_Image.data.image.raw_data[ i * width * nchannels], &loadimage.data[ i * loadimage.step ], width * nchannels);
	}
/*
	else if(m_output_color_format == "JPEG")
	{
		m_Image.data.image.format = Img::CF_JPEG;
		//Jpeg encoding using OpenCV image compression function
		std::vector<int> compression_param = std::vector<int>(2); 
		compression_param[0] = CV_IMWRITE_JPEG_QUALITY;
		compression_param[1] = m_compression_ratio;
		//Encode raw image data to jpeg data
		std::vector<uchar> compressed_image;
		cv::imencode(".jpg", loadimage, compressed_image, compression_param);
		//Copy encoded jpeg data to Outport Buffer
		m_Image.data.image.raw_data.length(compressed_image.size());
		memcpy(&m_Image.data.image.raw_data[0], &compressed_image[0], sizeof(unsigned char) * compressed_image.size());
	}
	else if(m_output_color_format == "PNG")
	{
		m_Image.data.image.format = Img::CF_PNG;
		//Jpeg encoding using OpenCV image compression function
		std::vector<int> compression_param = std::vector<int>(2); 
		compression_param[0] = CV_IMWRITE_PNG_COMPRESSION;
		compression_param[1] = (int)((double)m_compression_ratio/10.0);
		if(compression_param[1] == 10)
			compression_param[1] = 9;
		std::cout<<"PNG compression ratio: "<<compression_param[1] << "\r";


		//Encode raw image data to jpeg data
		std::vector<uchar> compressed_image;
		cv::imencode(".png", loadimage, compressed_image, compression_param);
		//Copy encoded jpeg data to Outport Buffer
		m_Image.data.image.raw_data.length(compressed_image.size());
		memcpy(&m_Image.data.image.raw_data[0], &compressed_image[0], sizeof(unsigned char) * compressed_image.size());
	}
*/
	//Output image data via OutPort
	m_ImageOut.write();
	//***********************************************************************************
	//End of the template part
	//***********************************************************************************
	return RTC::RTC_OK;
}

/*
RTC::ReturnCode_t ImageLoader::onAborting(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t ImageLoader::onError(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t ImageLoader::onReset(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t ImageLoader::onStateUpdate(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/

/*
RTC::ReturnCode_t ImageLoader::onRateChanged(RTC::UniqueId ec_id)
{
  return RTC::RTC_OK;
}
*/



extern "C"
{
 
  void ImageLoaderInit(RTC::Manager* manager)
  {
    coil::Properties profile(imageloader_spec);
    manager->registerFactory(profile,
                             RTC::Create<ImageLoader>,
                             RTC::Delete<ImageLoader>);
  }
  
};


