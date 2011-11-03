#include "ofxOpenNIContext.h"
#include "ofxDepthGenerator.h"
#include "ofxOpenNIMacros.h"

// Startup
//----------------------------------------
ofxOpenNIContext::ofxOpenNIContext() {
	is_using_recording = false;
	enableLogging();
}

// Just initialize; use this when you"re creating nodes yourself.
//----------------------------------------
bool ofxOpenNIContext::initContext(){
	xn::EnumerationErrors errors;
	XnStatus nRetVal = context.Init();
	cout << xnGetStatusString(nRetVal) << endl;
	if(nRetVal != XN_STATUS_OK) logErrors(errors);
	CHECK_RC(nRetVal, "ofxOpenNIContext.setup()");
	
	// find devices
	NodeInfoList list;
	nRetVal = context.EnumerateProductionTrees(XN_NODE_TYPE_DEVICE, NULL, list, &errors);
	cout << xnGetStatusString(nRetVal) << endl;
	if(nRetVal != XN_STATUS_OK) logErrors(errors);
	XN_IS_STATUS_OK(nRetVal);
	
	printf("The following devices were found:\n");
	int i = 1;
	for (NodeInfoList::Iterator it = list.Begin(); it != list.End(); ++it, ++i)
	{
		NodeInfo deviceNodeInfo = *it;
		
		Device deviceNode;
		deviceNodeInfo.GetInstance(deviceNode);
		XnBool bExists = deviceNode.IsValid();
		if (!bExists)
		{
			context.CreateProductionTree(deviceNodeInfo, deviceNode);
			// this might fail.
		}
		
		if (deviceNode.IsValid()) {
			
			checkNodeAllCapabilities(deviceNode);
			
			if (checkNodeCapability(deviceNode, XN_CAPABILITY_DEVICE_IDENTIFICATION)) {
				const XnUInt32 nStringBufferSize = 256;
				XnChar strDeviceName[nStringBufferSize];
				XnChar strSerialNumber[nStringBufferSize];
				
				XnUInt32 nLength = nStringBufferSize;
				deviceNode.GetIdentificationCap().GetDeviceName(strDeviceName, nLength);
				nLength = nStringBufferSize;
				deviceNode.GetIdentificationCap().GetSerialNumber(strSerialNumber, nLength);
				printf("Device Name & Serial: [%d] %s (%s)\n", i, strDeviceName, strSerialNumber);
			}
			
		
			
		}
		else
		{
			printf("[%d] %s\n", i, deviceNodeInfo.GetCreationInfo());
		}
		
		// release the device if we created it
		if (!bExists && deviceNode.IsValid())
		{
			deviceNode.Release();
		}
	}
	printf("\n");
	printf("Choose device to open (1): ");
	
	int chosen = 1;
	scanf("%d", &chosen);
	
	// create it
	NodeInfoList::Iterator it = list.Begin();
	for (i = 1; i < chosen; ++i)
	{
		it++;
	}
	
	NodeInfo deviceNode = *it;
	nRetVal = context.CreateProductionTree(deviceNode, g_Device);
	
	XN_IS_STATUS_OK(nRetVal);
}

// Check a production node for single capability
//----------------------------------------
void ofxOpenNIContext::checkNodeAllCapabilities(ProductionNode & node) {
	checkNodeCapability(node, XN_CAPABILITY_EXTENDED_SERIALIZATION);
	checkNodeCapability(node, XN_CAPABILITY_MIRROR);
	checkNodeCapability(node, XN_CAPABILITY_ALTERNATIVE_VIEW_POINT);
	checkNodeCapability(node, XN_CAPABILITY_CROPPING);
	checkNodeCapability(node, XN_CAPABILITY_USER_POSITION);
	checkNodeCapability(node, XN_CAPABILITY_SKELETON);
	checkNodeCapability(node, XN_CAPABILITY_POSE_DETECTION);
	checkNodeCapability(node, XN_CAPABILITY_LOCK_AWARE);
	checkNodeCapability(node, XN_CAPABILITY_ERROR_STATE);
	checkNodeCapability(node, XN_CAPABILITY_FRAME_SYNC);
	checkNodeCapability(node, XN_CAPABILITY_BRIGHTNESS);
	checkNodeCapability(node, XN_CAPABILITY_CONTRAST);
	checkNodeCapability(node, XN_CAPABILITY_HUE);
	checkNodeCapability(node, XN_CAPABILITY_SATURATION);
	checkNodeCapability(node, XN_CAPABILITY_SHARPNESS);
	checkNodeCapability(node, XN_CAPABILITY_GAMMA);
	checkNodeCapability(node, XN_CAPABILITY_COLOR_TEMPERATURE);
	checkNodeCapability(node, XN_CAPABILITY_BACKLIGHT_COMPENSATION);
	checkNodeCapability(node, XN_CAPABILITY_GAIN);
	checkNodeCapability(node, XN_CAPABILITY_PAN);
	checkNodeCapability(node, XN_CAPABILITY_TILT);
	checkNodeCapability(node, XN_CAPABILITY_ROLL);
	checkNodeCapability(node, XN_CAPABILITY_ZOOM);
	checkNodeCapability(node, XN_CAPABILITY_EXPOSURE);
	checkNodeCapability(node, XN_CAPABILITY_IRIS);
	checkNodeCapability(node, XN_CAPABILITY_FOCUS);
	checkNodeCapability(node, XN_CAPABILITY_LOW_LIGHT_COMPENSATION);
	checkNodeCapability(node, XN_CAPABILITY_ANTI_FLICKER);
	checkNodeCapability(node, XN_CAPABILITY_HAND_TOUCHING_FOV_EDGE);
}

// Check a production node for single capability
//----------------------------------------
bool ofxOpenNIContext::checkNodeCapability(ProductionNode & node, const XnChar* strCapabilityName) {
	if (node.IsCapabilitySupported(strCapabilityName)) {
		printf("Node %s capability: %s YES\n", node.GetName(), strCapabilityName);
		return true;
	} else {
		printf("Node %s capability: %s NO\n", node.GetName(), strCapabilityName);
		return false;
	}

}

// Initialize using an .ONI recording.
//----------------------------------------
bool ofxOpenNIContext::setupUsingRecording(std::string sFileRecording) {
	
	xn::EnumerationErrors errors;
	
	initContext();
	addLicense("PrimeSense", "0KOIk2JeIBYClPWVnMoRKn5cdY4=");
	
	is_using_recording = true;
	
	std::string file_path = ofToDataPath(sFileRecording.c_str(), true);
	
	printf("Attempting to open file: %s\n", file_path.c_str());
	
	XnStatus result = context.OpenFileRecording(file_path.c_str());
	
	if(result != XN_STATUS_OK) logErrors(errors);
	
	BOOL_RC(result, "Loading file");
}

void ofxOpenNIContext::logErrors(xn::EnumerationErrors& rErrors) {
	for(xn::EnumerationErrors::Iterator it = rErrors.Begin(); it != rErrors.End(); ++it) {
		XnChar desc[512];
		xnProductionNodeDescriptionToString(&it.Description(), desc,512);
		printf("%s failed: %s\n", desc, xnGetStatusString(it.Error()));
	}	
}

// Initialize using code only
bool ofxOpenNIContext::setup() {
	
	if (initContext()) {
		addLicense("PrimeSense", "0KOIk2JeIBYClPWVnMoRKn5cdY4=");
		return true;
	} else return false;

}

// Initialize using an XML file.
//----------------------------------------
bool ofxOpenNIContext::setupUsingXMLFile(std::string sFile) {
	
	xn::EnumerationErrors errors;
	
	if(sFile == "") sFile = ofToDataPath("openni/config/ofxopenni_config.xml",true);
	
	printf("Using file: %s\n", sFile.c_str());
	
	XnStatus result = context.InitFromXmlFile(sFile.c_str(), &errors);
	
	if(result != XN_STATUS_OK) logErrors(errors);
	
	BOOL_RC(result, "ofxOpenNIContext.setupUsingXMLFile()");
	
}

// This is used by other nodes (ofxDepthGenerator), which need to 
// use a different initialization when you"re loading an recording.
//----------------------------------------
bool ofxOpenNIContext::isUsingRecording() {
	return is_using_recording;
}

// Use these to retrieve references to various node types on the production tree
// TODO: should these be made static so retrievl is more immediate? Or is this sufficient?
//----------------------------------------
bool ofxOpenNIContext::getDepthGenerator(xn::DepthGenerator* depth_generator) {
	XnStatus result = XN_STATUS_OK;
	result = context.FindExistingNode(XN_NODE_TYPE_DEPTH, *depth_generator);
	BOOL_RC(result, "Retrieving depth generator");
}

bool ofxOpenNIContext::getImageGenerator(xn::ImageGenerator* image_generator) {
	XnStatus result = XN_STATUS_OK;
	result = context.FindExistingNode(XN_NODE_TYPE_IMAGE, *image_generator);
	BOOL_RC(result, "Retrieving image generator");
}

bool ofxOpenNIContext::getIRGenerator(xn::IRGenerator* ir_generator) {
	XnStatus result = XN_STATUS_OK;
	result = context.FindExistingNode(XN_NODE_TYPE_IR, *ir_generator);
	checkNodeAllCapabilities(*ir_generator);
	BOOL_RC(result, "Retrieving ir generator");
}

bool ofxOpenNIContext::getUserGenerator(xn::UserGenerator* user_generator) {
	XnStatus result = XN_STATUS_OK;
	result = context.FindExistingNode(XN_NODE_TYPE_USER, *user_generator);
	BOOL_RC(result, "Retrieving user generator");
}

bool ofxOpenNIContext::getGestureGenerator(xn::GestureGenerator* gesture_generator) {
	XnStatus result = XN_STATUS_OK;
	result = context.FindExistingNode(XN_NODE_TYPE_GESTURE, *gesture_generator);
	BOOL_RC(result, "Retrieving gesture generator");
}

bool ofxOpenNIContext::getHandsGenerator(xn::HandsGenerator* hands_generator) {
	XnStatus result = XN_STATUS_OK;
	result = context.FindExistingNode(XN_NODE_TYPE_HANDS, *hands_generator);
	BOOL_RC(result, "Retrieving hands generator");
}

// we need to programmatically add a license when playing back a recording
// file otherwise the skeleton tracker will throw an error and not work
void ofxOpenNIContext::addLicense(std::string sVendor, std::string sKey) {
	
	XnLicense license = {0};
	XnStatus status = XN_STATUS_OK;
	
	status = xnOSStrNCopy(license.strVendor, sVendor.c_str(),sVendor.size(), sizeof(license.strVendor));
	if(status != XN_STATUS_OK) {
		printf("ofxOpenNIContext error creating license (vendor)\n");
		return;
	}
	
	status = xnOSStrNCopy(license.strKey, sKey.c_str(), sKey.size(), sizeof(license.strKey));
	if(status != XN_STATUS_OK) {
		printf("ofxOpenNIContext error creating license (key)\n");
		return;
	}	
	
	status = context.AddLicense(license);
	SHOW_RC(status, "AddLicense");
	
	xnPrintRegisteredLicenses();
	
}

// TODO: check this is working and use it with ONI recordings??
void ofxOpenNIContext::enableLogging() {
	
	XnStatus result = xnLogSetConsoleOutput(true);
	SHOW_RC(result, "Set console output");
	
	result = xnLogSetSeverityFilter(XN_LOG_ERROR);	// TODO: set different log levels with code; enable and disable functionality
	SHOW_RC(result, "Set log level");

	xnLogSetMaskState(XN_LOG_MASK_ALL, TRUE);
	
}

// Update all nodes, should be call in the ofTestApp::update()
//----------------------------------------
void ofxOpenNIContext::update(){
	XnStatus nRetVal = context.WaitAnyUpdateAll();	
}

// Allow us to mirror the image_gen/depth_gen
bool ofxOpenNIContext::toggleMirror() {
	return setMirror(!context.GetGlobalMirror());
}

bool ofxOpenNIContext::setMirror(XnBool mirroring) {
	XnStatus result = context.SetGlobalMirror(mirroring);
	BOOL_RC(result, "Set mirroring");
}

bool ofxOpenNIContext::toggleRegisterViewport() {
	
	// get refs to depth and image generators TODO: make work with IR generator
	xn::DepthGenerator depth_generator;
	getDepthGenerator(&depth_generator);
	
	xn::ImageGenerator image_generator;
	if (!getImageGenerator(&image_generator)) {
		printf("No Image generator found: cannot register viewport");
		return false;
	}
	
	// Toggle registering view point to image map
	if (depth_generator.IsCapabilitySupported(XN_CAPABILITY_ALTERNATIVE_VIEW_POINT))
	{
		
		if(depth_generator.GetAlternativeViewPointCap().IsViewPointAs(image_generator)) {
			unregisterViewport();
		} else {
			registerViewport();
		}
		
	} else return false;
	
	return true;
}

bool ofxOpenNIContext::registerViewport() {
	
	// get refs to depth and image generators TODO: make work with IR generator
	xn::DepthGenerator depth_generator;
	getDepthGenerator(&depth_generator);
	
	xn::ImageGenerator image_generator;
	if (!getImageGenerator(&image_generator)) {
		printf("No Image generator found: cannot register viewport");
		return false;
	}
	
	// Register view point to image map
	if (depth_generator.IsCapabilitySupported(XN_CAPABILITY_ALTERNATIVE_VIEW_POINT)) {
		
		XnStatus result = depth_generator.GetAlternativeViewPointCap().SetViewPoint(image_generator);
		CHECK_RC(result, "Register viewport");
		
	} else return false;
	
	return true;
}

bool ofxOpenNIContext::unregisterViewport() {
	
	// get refs to depth generator
	xn::DepthGenerator depth_generator;
	getDepthGenerator(&depth_generator);
	
	// Unregister view point from (image) any map	
	if (depth_generator.IsCapabilitySupported(XN_CAPABILITY_ALTERNATIVE_VIEW_POINT)) {
		XnStatus result = depth_generator.GetAlternativeViewPointCap().ResetViewPoint();
		CHECK_RC(result, "Unregister viewport");
		
	} else return false;
	
	return true;
}

// Get a reference to the xn::Context.
//----------------------------------------
xn::Context& ofxOpenNIContext::getXnContext(){
	return context;
}

void ofxOpenNIContext::shutdown() {
	printf("Shutdown context\n");
	context.Shutdown();
}

// Shutdown.
//----------------------------------------
ofxOpenNIContext::~ofxOpenNIContext(){
	shutdown();
}