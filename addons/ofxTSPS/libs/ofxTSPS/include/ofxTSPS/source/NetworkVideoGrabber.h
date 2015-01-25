//
//  IpVideoGrabber.h
//  TSPS
//
//  Created by James Kong on 25/1/15.
//
//

#pragma once

#include "ofxTSPS/source/Source.h"
#include "IPVideoGrabber.h"
namespace ofxTSPS {
    class IPCameraDef
    {
    public:
        IPCameraDef()
        {
        }
        
        IPCameraDef(const std::string& url): _url(url)
        {
        }
        
        IPCameraDef(const std::string& name,
                    const std::string& url,
                    const std::string& username,
                    const std::string& password):
        _name(name),
        _url(url),
        _username(username),
        _password(password)
        {
        }
        
        void setName(const std::string& name) { _name = name; }
        std::string getName() const { return _name; }
        
        void setURL(const std::string& url) { _url = url; }
        std::string getURL() const { return _url; }
        
        void setUsername(const std::string& username) { _username = username; }
        std::string getUsername() const { return _username; }
        
        void setPassword(const std::string& password) { _password = password; }
        std::string getPassword() const { return _password; }
        
        
    private:
        std::string _name;
        std::string _url;
        std::string _username;
        std::string _password;
    };
    
    using ofx::Video::IPVideoGrabber;
    using ofx::Video::SharedIPVideoGrabber;
    class NetworkVideoGrabber : public Source,  public IPVideoGrabber{
    public:
        NetworkVideoGrabber(): IPVideoGrabber(){
            
            
            type = CAMERA_IPVIDEOGRABBER;

        }
        int numAvailable(){
            return 1;
        }
        
        bool openSource( int width, int height, string etc="" ){
            
            customData = etc;
//            IPVideoGrabber::makeShared();
            IPVideoGrabber::setURI(customData);
            IPVideoGrabber::connect();
//            int numCam = loadCameras(customData);
//            // initialize connection
//            if(numCam>0)
//            {
//                IPCameraDef &cam = ipcams[sourceIndex];
//                IPVideoGrabber::makeShared();
//                if(cam.getName()!=NULL)
//                {
                    IPVideoGrabber::setCameraName("");
//                }
//                if(cam.getURL()!="")
//                {
//                    IPVideoGrabber::setURI(cam.getURL());
//                }
//                IPVideoGrabber::connect(); // connect immediately
//                
//                // if desired, set up a video resize listener
//            ofAddListener(IPVideoGrabber::videoResized, this, &NetworkVideoGrabber::newworkVideoResized);
            return true;
        }
        bool isOpen()
        {
            return isConnected();
        }
        void update(){
            IPVideoGrabber::update();
#ifdef TARGET_OSX
            if ( bPublishTexture ){
                publishToSyphon( IPVideoGrabber::getTextureReference() );
            }
#endif
        }
        bool doProcessFrame(){
            return isFrameNew();
        }
        
        void closeSource(){
            IPVideoGrabber::close();
        }
        int loadCameras( string customData)
        {
            // all of these cameras were found using this google query
            // http://www.google.com/search?q=inurl%3A%22axis-cgi%2Fmjpg%22
            // some of the cameras below may no longer be valid.
            
            // to define a camera with a username / password
            //ipcams.push_back(IPCameraDef("http://148.61.142.228/axis-cgi/mjpg/video.cgi", "username", "password"));
            
            ofLog(OF_LOG_NOTICE, "---------------Loading Streams---------------");
            
            ofxXmlSettings XML;
            string filePath = (customData != "")?customData:"streams.xml";
            if(XML.loadFile(filePath))
            {
                
                XML.pushTag("streams");
                std::string tag = "stream";
                
                int nCams = XML.getNumTags(tag);
                
                for(std::size_t n = 0; n < nCams; n++)
                {
                    
                    IPCameraDef def(XML.getAttribute(tag, "name", "", n),
                                    XML.getAttribute(tag, "url", "", n),
                                    XML.getAttribute(tag, "username", "", n),
                                    XML.getAttribute(tag, "password", "", n));
                    
                    
                    std::string logMessage = "STREAM LOADED: " + def.getName() +
                    " url: " +  def.getURL() +
                    " username: " + def.getUsername() +
                    " password: " + def.getPassword();
                    
                    ofLogNotice() << logMessage;
                    
                    ipcams.push_back(def);
                    
                }
                
                XML.popTag();
                
            }
            else
            {
                ofLog(OF_LOG_ERROR, "Unable to load streams.xml.");
            }
            
            ofLog(OF_LOG_NOTICE, "-----------Loading Streams Complete----------");
            
            nextCamera = ipcams.size();
            return ipcams.size();
        }
        IPCameraDef& getNextCamera()
        {
            nextCamera = (nextCamera + 1) % ipcams.size();
            return ipcams[nextCamera];
        }
        void newworkVideoResized(const void* sender, ofResizeEventArgs& arg)
        {
            // find the camera that sent the resize event changed
//            for(std::size_t i = 0; i < ipcams.size(); i++)
//            {
//                if(sender == &grabbers[i])
//                {
//                    std::stringstream ss;
//                    ss << "videoResized: ";
//                    ss << "Camera connected to: " << grabbers[i]->getURI() + " ";
//                    ss << "New DIM = " << arg.width << "/" << arg.height;
//                    ofLogVerbose("ofApp") << ss.str();
//                }
//            }
        }

        std::vector<IPCameraDef> ipcams; // a list of IPCameras
        int nextCamera;
      
    };
    
}


