#pragma once

#include "Physics/Body.h"
#include "SceneUtil.h"

// debug object showing fwd, right, up axes
namespace sceneUtil {
	void MakeCoordGizmo( const Vec3 & origin, const Vec3 & fDir, const Vec3 rDir, const Vec3 & uDir, const float scale, void * bodies_ ) {
		constexpr float F_HEAD_SIZE = 0.75;
		constexpr float R_HEAD_SIZE = 0.45;
		constexpr float U_HEAD_SIZE = 0.15;

		auto makeArrow = [ origin, scale ]( std::vector< Body > & arrow, Vec3 dir, float headSize ) {
			constexpr float THICKNESS = 0.25f;
			// fill
			for ( int i = 0; i < 11; i++ ) {
				arrow.push_back( Body() );
				arrow.back().m_orientation = Quat( 0, 0, 0, 1 );
				arrow.back().m_linearVelocity.Zero();
				arrow.back().m_invMass = 0.f;
				arrow.back().m_elasticity = 0.99f;
				arrow.back().m_friction = 0.5f;
				arrow.back().m_shape = new ShapeSphere( THICKNESS * scale );
			}
			// add a knob at the end
			( ( ShapeSphere * )arrow.back().m_shape )->m_radius = headSize * scale;

			// arrange
			int bodyIdx = 0;
			for ( int i = 0; i < 11; i++ ) {
				const Vec3 pos = ( origin + dir * ( i * 0.35f ) ) * scale;
				arrow[ bodyIdx ].m_position = pos;
				bodyIdx++;
			}
			};

		std::vector< Body > & bodies = *reinterpret_cast< std::vector< Body > * >( bodies_ );
		std::vector< Body > fwd, right, up;
		makeArrow( fwd, fDir, F_HEAD_SIZE );
		makeArrow( right, rDir, R_HEAD_SIZE );
		makeArrow( up, uDir, U_HEAD_SIZE );
		bodies.insert( bodies.end(), fwd.begin(), fwd.end() );
		bodies.insert( bodies.end(), right.begin(), right.end() );
		bodies.insert( bodies.end(), up.begin(), up.end() );
	}
} // namespace sceneUtil
