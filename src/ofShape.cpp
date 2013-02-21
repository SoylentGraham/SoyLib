#include "ofShape.h"


bool ofShapeBox3::IsOutside(const vec2f& Pos) const
{
	if ( Pos.x < mMin.x )	return true;
	if ( Pos.y < mMin.y )	return true;

	if ( Pos.x > mMax.x )	return true;
	if ( Pos.y > mMax.y )	return true;

	return false;
}




TIntersection ofShape::GetIntersection(const ofShapeCircle2& a,const ofShapeCircle2& b)
{
	if ( !a.IsValid() || !b.IsValid() )
		return TIntersection(false);

	TIntersection Intersection;
	auto& Delta = Intersection.mDelta;

	Intersection.mDelta = b.mPosition - a.mPosition;
	Intersection.mDistanceSq = Delta.lengthSquared();
	float Distance = Delta.length();

	float RadTotal = a.mRadius + b.mRadius;

	Intersection.mIsValid = ( Intersection.mDistanceSq <= RadTotal*RadTotal );
	if ( !Intersection.mIsValid )
		return Intersection;

	//	work out hit points
	Intersection.mMidIntersection = a.mPosition + ( Delta*0.5f);
	Intersection.mCollisionPointA = a.mPosition + ( Delta.normalized() * a.mRadius );
	Intersection.mCollisionPointB = b.mPosition + ( Delta.normalized() * -b.mRadius );
	
	return Intersection;
}


bool ofLine2::GetIntersection(const ofLine2& Line,vec2f& Intersection) const
{
	float IntersectionAlongThis,IntersectionAlongLine;
	if ( !GetIntersection( Line, IntersectionAlongThis, IntersectionAlongLine ) )
		return false;

	Intersection = GetPoint( IntersectionAlongThis );
	return true;
}

bool ofLine2::GetIntersection(const ofLine2& Line,float& IntersectionAlongThis,float& IntersectionAlongLine) const	
{
	const vec2f& v1 = this->mStart;
	const vec2f& v2 = this->mEnd;
	const vec2f& v3 = Line.mStart;
	const vec2f& v4 = Line.mEnd;

	vec2f v2MinusV1( v2-v1 );
	vec2f v1Minusv3( v1-v3 );
	vec2f v4Minusv3( v4-v3 );

	float denom =		((v4Minusv3.y) * (v2MinusV1.x)) - ((v4Minusv3.x) * (v2MinusV1.y));
    float numerator =	((v4Minusv3.x) * (v1Minusv3.y)) - ((v4Minusv3.y) * (v1Minusv3.x));
    float numerator2 =	((v2MinusV1.x) * (v1Minusv3.y)) - ((v2MinusV1.y) * (v1Minusv3.x));

    if ( denom == 0.0f )
    {
        if ( numerator == 0.0f && numerator2 == 0.0f )
        {
            return false;//COINCIDENT;
        }
        return false;// PARALLEL;
    }

	float& ua = IntersectionAlongThis;
	float& ub = IntersectionAlongLine;

    ua = numerator / denom;
    ub = numerator2/ denom;

	//	intersection will be past the ends of these lines
	if ( ua < 0.f || ua > 1.f )	return false;
	if ( ub < 0.f || ub > 1.f )	return false;

	return true;
}

//-----------------------------------------------------------
//	get nearest point on line
//-----------------------------------------------------------
vec2f ofLine2::GetNearestPoint(const vec2f& Position,float& Time) const
{
	vec2f LineDir = GetDirection();
	float LineDirDotProduct = LineDir.dot(LineDir);
	
	//	avoid div by zero
	if ( LineDirDotProduct == 0.f )
	{
		Time = 0.f;
		return mStart;
	}

	vec2f Dist = Position - mStart;

	float LineDirDotProductDist = LineDir.dot(Dist);

	Time = LineDirDotProductDist / LineDirDotProduct;

	if ( Time <= 0.f )
		return mStart;

	if ( Time >= 1.f )
		return mEnd;

	return mStart + (LineDir * Time);
}


TIntersection ofShape::GetIntersection(const ofShapePolygon2& a,const ofShapeCircle2& b)
{
	if ( !a.IsValid() || !b.IsValid() )
		return TIntersection(false);

	ofLine2 ab( a.mTriangle[0], a.mTriangle[1] );
	ofLine2 bc( a.mTriangle[1], a.mTriangle[2] );
	ofLine2 ca( a.mTriangle[2], a.mTriangle[0] );
	
	//	see if circle intersects edge
	//	get circle -> triangle edge nearest point
	vec2f abnear = ab.GetNearestPoint( b.GetCenter() ) - b.GetCenter();
	vec2f bcnear = bc.GetNearestPoint( b.GetCenter() ) - b.GetCenter();
	vec2f canear = ca.GetNearestPoint( b.GetCenter() ) - b.GetCenter();
	float abneardistsq = abnear.lengthSquared();
	float bcneardistsq = bcnear.lengthSquared();
	float caneardistsq = canear.lengthSquared();
	float RadSq = b.mRadius * b.mRadius;

	//	find nearest valid edge intersection
	vec2f* pnear = NULL;
	float* pneardistsq = NULL;
	if ( abneardistsq < bcneardistsq && abneardistsq < caneardistsq && abneardistsq <= RadSq )
	{
		pnear = &abnear;
		pneardistsq = &abneardistsq;
	}
	else if ( bcneardistsq < caneardistsq && bcneardistsq <= RadSq )
	{
		pnear = &bcnear;
		pneardistsq = &bcneardistsq;
	}
	else if ( caneardistsq <= RadSq )
	{
		pnear = &canear;
		pneardistsq = &caneardistsq;
	}

	//	got nearest intersection
	if ( pnear )
	{
		TIntersection Intersection(true);
		Intersection.mDelta = pnear->normalized();
		Intersection.mCollisionPointA = *pnear + b.GetCenter();	//	triangle
		Intersection.mCollisionPointB = b.GetCenter() + (Intersection.mDelta*b.mRadius);	//	circle
		Intersection.mDistanceSq = *pneardistsq;
		Intersection.mMidIntersection = ( b.GetCenter() + a.GetCenter() ) * 0.5f;
		return Intersection;
	}

	//	see if the circle is wholly inside the triangle
	if ( !a.IsInside( b.GetCenter() ) )
		return TIntersection(false);

	TIntersection Intersection(true);
	Intersection.mCollisionPointA = a.GetCenter();	//	triangle
	Intersection.mCollisionPointB = b.GetCenter();	//	circle
	Intersection.mDelta = Intersection.mCollisionPointB - Intersection.mCollisionPointA;
	Intersection.mDistanceSq = Intersection.mDelta.lengthSquared();
	Intersection.mMidIntersection = ( Intersection.mCollisionPointA + Intersection.mCollisionPointB ) * 0.5f;
	return Intersection;

}

bool ofShapePolygon2::IsInside(const vec2f& Pos) const
{
	auto& v1 = mTriangle[0];
	auto& v2 = mTriangle[1];
	auto& v3 = mTriangle[2];

	bool absign = ((v2.y - v1.y)*(Pos.x - v1.x) - (v2.x - v1.x)*(Pos.y - v1.y)) >= 0;
	bool bcsign = ((v3.y - v2.y)*(Pos.x - v2.x) - (v3.x - v2.x)*(Pos.y - v2.y)) >= 0;
	bool casign = ((v1.y - v3.y)*(Pos.x - v3.x) - (v1.x - v3.x)*(Pos.x - v3.x)) >= 0;

	//	all same side (doesn't matter about winding then)
	return (absign == bcsign && absign == casign);
}


TIntersection ofShape::GetIntersection(const ofShapePolygon2& a,const ofShapePolygon2& b)
{
	if ( !a.IsValid() || !b.IsValid() )
		return TIntersection(false);

	//	test line intersections
	for ( int i=0;	i<a.mTriangle.GetSize();	i++ )
	{
		ofLine2 aedge( a.mTriangle[i], a.mTriangle[(i+1)%a.mTriangle.GetSize()] );
		for ( int j=0;	j<b.mTriangle.GetSize();	j++ )
		{
			ofLine2 bedge( b.mTriangle[j], b.mTriangle[(j+1)%b.mTriangle.GetSize()] );

			vec2f CrossPos;
			if ( !aedge.GetIntersection( bedge, CrossPos ) )
				continue;

		}
	}
	
	//	check in case one is wholly inside the other,
	//	we just need to check any point so use first
	vec2f acenter = b.GetCenter();
	vec2f bcenter = b.GetCenter();
	if ( a.IsInside( bcenter ) || b.IsInside( acenter ) )
	{
		TIntersection Intersection(true);
		Intersection.mCollisionPointA = acenter;	//	triangle
		Intersection.mCollisionPointB = bcenter;	//	circle
		Intersection.mDelta = Intersection.mCollisionPointB - Intersection.mCollisionPointA;
		Intersection.mDistanceSq = Intersection.mDelta.lengthSquared();
		Intersection.mMidIntersection = ( Intersection.mCollisionPointA + Intersection.mCollisionPointB ) * 0.5f;
		return Intersection;
	}

	return TIntersection(false);
}


ofShapeCircle2 ofShapePolygon2::GetBounds() const
{
	ofShapeCircle2 Circle;
	Circle.mPosition = GetCenter();

	//	find longest point from center
	for ( int i=0;	i<mTriangle.GetSize();	i++ )
	{
		vec2f Delta = mTriangle[i] - Circle.mPosition;
		Circle.mRadius = ofMax( Circle.mRadius, Delta.length() );
	}
	return Circle;
}
	
bool ofShapeCircle2::IsInside(const vec2f& Point) const
{
	vec2f Delta = Point - mPosition;
	float DeltaLenSq = Delta.lengthSquared();
	return ( DeltaLenSq <= mRadius*mRadius );
}


bool ofShapeCircle2::IsInside(const ofShapeCircle2& Shape) const
{
	vec2f Delta = Shape.mPosition - mPosition;
	float DeltaLenSq = Delta.lengthSquared();
	float Rad = mRadius + Shape.mRadius;
	return ( DeltaLenSq <= Rad*Rad );
}


void ofShapeCircle2::Accumulate(const ofShapeCircle2& that)
{
	//	make no change
	if ( !that.IsValid() )
		return;

	//	just use argument
	if ( !IsValid() )
	{
		*this = that;
		return;
	}

	//	get furthest point on both spheres from each other
	vec2f DirToSphere( that.GetCenter() - this->GetCenter() );
	float LengthSq = DirToSphere.lengthSquared();
	
	//	overlapping, so just get a new larger radius
	if ( LengthSq < ofNearZero )
	{
		mRadius = ofMax( that.mRadius, this->mRadius );
		return;
	}

	//	normalise dir
	DirToSphere.normalize();

	//	get the furthest point away on the sphere
	vec2f FurthestPointOnSphere = that.GetCenter() + ( DirToSphere * that.mRadius );

	//	if its already inside this sphere then we dont need to change anything
	if ( IsInside( FurthestPointOnSphere ) )
		return;

	//	get furthest point away on the existing sphere
	vec2f FurthestPointOnThis = this->GetCenter() - ( DirToSphere * this->mRadius );

	//	new sphere center is midpoint between furthest points
	mPosition = (FurthestPointOnSphere + FurthestPointOnThis) * 0.5f;

	//	new radius is half length from furthest point to furthest point
	mRadius = (FurthestPointOnSphere - FurthestPointOnThis).length() * 0.5f;
}
