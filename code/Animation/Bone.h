#pragma once

#include <vector>
#include "../Math/Vector.h"
#include "../Math/Quat.h"

////////////////////////////////////////////////////////////////////////////////
// BONE TRANSFORM
////////////////////////////////////////////////////////////////////////////////
struct BoneTransform {
	Quat rotation;
	Vec3 translation;
	bool isIdentity;

	BoneTransform() : 
		rotation( { 0, 0, 0, 1 } ), 
		translation( { 0, 0, 0 } ),
		isIdentity( true ) {}
	BoneTransform( const Quat & rot, const Vec3 & trans, bool isI )
		: rotation( rot ), translation( trans ), isIdentity( isI ) {}
	BoneTransform( const BoneTransform & other ) { *this = other; }

	void operator *= ( const BoneTransform & other );
	BoneTransform operator * ( const BoneTransform & b ) const;

	inline bool IsIdentity() const { return isIdentity; }
	static BoneTransform Identity();
	BoneTransform Inverse() const {
		return isIdentity ? *this : BoneTransform( rotation.Inverse(), translation * -1.f, isIdentity );
	}
};

////////////////////////////////////////////////////////////////////////////////
// KEYFRAME
////////////////////////////////////////////////////////////////////////////////
struct Keyframe {
	float timePos = 0.f;
	BoneTransform transform;
};

////////////////////////////////////////////////////////////////////////////////
// BONE ANIMATION
////////////////////////////////////////////////////////////////////////////////
struct BoneAnimation {
	float GetStartTime() const;
	float GetEndTime() const;
	void Interpolate( float t, BoneTransform & outTransform ) const;
	inline bool HasData() const { return !keyframes.empty(); }
	std::vector< Keyframe > keyframes;
};

////////////////////////////////////////////////////////////////////////////////
// BONE INFO
////////////////////////////////////////////////////////////////////////////////
class BoneInfo_t {
public:
	BoneInfo_t( int pi ) : parentIdx( pi ) {}
	inline int GetParent() const { return parentIdx; }

private:
	int parentIdx;

	// @TODO - add additional mapping data later; 
	// for now, we'll just do a one-to-one replacement for the sake of 
	// stable porting old array of indices to this new struct array

//	BoneTransform transform;
};