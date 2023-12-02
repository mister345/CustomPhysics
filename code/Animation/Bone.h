#pragma once

struct BoneTransform {
	Quat rotation = { 0, 0, 0, 1 };
	Vec3 translation = {};

	static BoneTransform Identity() {
		return {
			{ 0, 0, 0, 1 },
			{ 0, 0, 0 }
		};
	}
	void operator *= ( const BoneTransform & other ) {
		translation += rotation.RotatePoint( other.translation );
		rotation *= other.rotation;
	}

	BoneTransform operator * ( const BoneTransform & b ) const {
		BoneTransform concatenated;
		concatenated.translation = translation + rotation.RotatePoint( b.translation );
		concatenated.rotation = rotation * b.rotation;
		return concatenated;
	}
};