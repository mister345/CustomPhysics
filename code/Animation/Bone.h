#pragma once

#include "../Math/Vector.h"
#include "../Math/Quat.h"

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
};