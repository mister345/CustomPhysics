//
//  Broadphase.cpp
//
#include "Broadphase.h"

/*
====================================================
Sweep and Prune algorithm
====================================================
*/
namespace {
struct pseudoBody_t {
	int id;
	float value;
	bool isMin;
};

int CompareSAP( const void * a, const void * b ) {
	const pseudoBody_t * ea = reinterpret_cast< const pseudoBody_t * >( a );
	const pseudoBody_t * eb = reinterpret_cast< const pseudoBody_t * >( b );

	if ( ea->value < eb->value ) {
		return -1;
	}
	return 1;
}
}

void SortBodiesBounds( const Body * bodies, const int num, pseudoBody_t * outSortedArray, const float dt_sec ) {
	Vec3 axis = Vec3( 1, 1, 1 );
	axis.Normalize();

	for ( int i = 0; i < num; i++ ) {
		const Body & body = bodies[ i ];
		Bounds bounds = body.m_shape->GetBounds( body.m_position, body.m_orientation );

		// expand each bounds by the distance it will travel in this time step
		bounds.Expand( bounds.mins + body.m_linearVelocity * dt_sec );
		bounds.Expand( bounds.maxs + body.m_linearVelocity * dt_sec );

		// expand a little bit more along sweep axis
		const float epsilon = 0.01f;
		bounds.Expand( bounds.mins + Vec3( -1, -1, -1 ) * epsilon );
		bounds.Expand( bounds.maxs + Vec3(  1,  1,  1 ) * epsilon );

		// separate each bounds into individual values, projected onto sweep axis so they can be sorted
		// mins and maxes can later be tied together bc they share the same ids= 0; 
		outSortedArray[ i * 2 + 0 ].id = i;
		outSortedArray[ i * 2 + 0 ].value = axis.Dot( bounds.mins );
		outSortedArray[ i * 2 + 0 ].isMin = true;

		outSortedArray[ i * 2 + 1 ].id = i;
		outSortedArray[ i * 2 + 1 ].value = axis.Dot( bounds.maxs );
		outSortedArray[ i * 2 + 1 ].isMin = false;
	}
	qsort( outSortedArray, num * 2, sizeof( pseudoBody_t ), CompareSAP );
}

void BuildPairs( std::vector< collisionPair_t > & collisionPairs, const pseudoBody_t * sortedBodies, const int num ) {
	collisionPairs.clear();

	for ( int i = 0; i < num * 2; i++ ) {
		const pseudoBody_t & a = sortedBodies[ i ];
		if ( !a.isMin ) {
			continue;
		}

		collisionPair_t pair;
		pair.a = a.id;

		for ( int j = i + 1; j < num * 2; j++ ) {
			const pseudoBody_t & b = sortedBodies[ j ];
			if ( b.id == a.id ) {
				break;
			}

			if ( !b.isMin ) {
				continue;
			}

			pair.b = b.id;
			collisionPairs.push_back( pair );
		}
	}
}

void SweepAndPrune1D( const Body * bodies, const int num, std::vector< collisionPair_t > & finalPairs, const float dt_sec ) {
	pseudoBody_t * sortedBodies = reinterpret_cast< pseudoBody_t * >( alloca( sizeof( pseudoBody_t ) * num * 2 ) );

	SortBodiesBounds( bodies, num, sortedBodies, dt_sec );
	BuildPairs( finalPairs, sortedBodies, num );
}

/*
====================================================
BroadPhase
====================================================
*/
void BroadPhase( const Body * bodies, const int num, std::vector< collisionPair_t > & outFinalPairs, const float dt_sec ) {
	outFinalPairs.clear();

	SweepAndPrune1D( bodies, num, outFinalPairs, dt_sec );
}