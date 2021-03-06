#pragma once


#include <ofxSoylent.h>

class ofTrueTypeFont;
class ofShapeTriangle2;



namespace TRender
{
	extern ofTrueTypeFont	Font;
	bool					InitFont();
};

//	push/pop style for immediate rendering
class TRenderSceneScope
{
public:
	TRenderSceneScope(const char* SceneName);
	~TRenderSceneScope();
};


class ofStyleScope
{
public:
	ofStyleScope()	{	ofPushStyle();	}
	~ofStyleScope()	{	ofPopStyle();	}
};

inline void ofClearDepth()
{
	glClear( GL_DEPTH_BUFFER_BIT );
}


inline void ofClearViewport(const ofRectangle& Viewport,const ofColor& ClearColour)
{
	ofClearDepth();
	ofPushStyle();
	if ( ClearColour.a > 0 )
	{
		ofSetColor( ClearColour );
		ofRect( Viewport );
	}
	ofPopStyle();
}

template<class ARRAY>
void ofDrawPathClosed(const ARRAY& PathPoints)
{
	for ( int a=0;	a<PathPoints.GetSize();	a++ )
	{
		const auto& va = PathPoints[a];
		int b = (a+1) % PathPoints.GetSize();
		const auto& vb = PathPoints[b];
		ofLine( va, vb );
	}
}

void ofDrawTriangles(const ArrayBridge<ofShapeTriangle2>& Triangles);
void ofDrawTriangles(const ArrayBridge<ofShapeTriangle2>& Triangles,ofTexture& Texture,const ArrayBridge<ofShapeTriangle2>& TexCoords);

class ofShapeCapsule2;
void ofCapsule(const ofShapeCapsule2& Capsule,float z=0.f,bool DrawCenterLine=false);

inline void ofBezier(ofPoint p0,ofPoint p1,ofPoint p2,ofPoint p3)
{
	ofBezier( p0.x, p0.y, p0.z, p1.x, p1.y, p1.z, p2.x, p2.y, p2.z, p3.x, p3.y, p3.z );
}

