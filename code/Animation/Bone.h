#pragma once

#include <vector>
#include "../Math/Vector.h"
#include "../Math/Quat.h"

namespace fbxsdk {
	class FbxScene;
	class FbxNode;
	class FbxAnimLayer;
	class FbxQuaternion;
	class FbxVector4;
}

////////////////////////////////////////////////////////////////////////////////
// BONE TRANSFORM
////////////////////////////////////////////////////////////////////////////////
struct BoneTransform {
	Quat rotation;
	Vec3 translation;
	bool isIdentity;

	BoneTransform() : rotation( { 0, 0, 0, 1 } ), translation( { 0, 0, 0 } ), isIdentity( true ) 
	{}
	BoneTransform( const Quat & rot, const Vec3 & trans, bool isI ) : rotation( rot ), translation( trans ), isIdentity( isI ) 
	{}
	BoneTransform( const BoneTransform & other ) { *this = other; }
	BoneTransform( fbxsdk::FbxQuaternion * q, const fbxsdk::FbxVector4 * t );

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
////////////////////////////////////////////////////////////s////////////////////
struct BoneAnimation {
	BoneAnimation() = default;
	BoneAnimation( const BoneAnimation & oth ) { *this = oth; }
	BoneAnimation( fbxsdk::FbxScene * fbxScene, fbxsdk::FbxNode * boneNode, bool isGlobal );

	float GetStartTime() const;
	float GetEndTime() const;
	void Interpolate( float t, BoneTransform & outTransform, int myIdx ) const;
	inline bool HasData() const { return !keyframes.empty(); }
	std::vector< Keyframe > keyframes;
	mutable int lastKeyframeIdx = 0;
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
};