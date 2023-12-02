#pragma once

struct BoneTransform {
	Quat rotation = { 0, 0, 0, 1 };
	Vec3 translation = {};
	bool isIdentity = false;

	static BoneTransform Identity() {
		return {
			{ 0, 0, 0, 1 },
			{ 0, 0, 0 },
			true
		};
	}

	inline bool IsIdentity() const { return IsIdentity(); }
	void operator *= ( const BoneTransform & other ) {
		/*
		*	sometimes, you want a flat hierarchy of bones as in vertex animation;
		*	in that case, concatenate each transform with an identity root, ie, do nothing!
		* 
			if( IsIdentity() ){
				return;
			}
		
		*/

		translation += rotation.RotatePoint( other.translation );
		rotation *= other.rotation;
	}

	BoneTransform operator * ( const BoneTransform & b ) const {
		/*
		*	sometimes, you want a flat hierarchy of bones as in vertex animation;
		*	in that case, concatenate each transform with an identity root, ie, do nothing!
		*
			if( IsIdentity() ){
				return *this;
			}

		*/

		BoneTransform concatenated;
		concatenated.translation = translation + rotation.RotatePoint( b.translation );
		concatenated.rotation = rotation * b.rotation;
		return concatenated;
	}
};