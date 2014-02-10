#include <irrlicht.h>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

using namespace irr;
using namespace core;
using namespace scene;

#ifndef HELPERS_CPP
#define HELPERS_CPP

class Helpers {

public:

	static btVector3 vectorToBT(const vector3df &v) {
		return btVector3(v.X, v.Y, v.Z);
	}

	static vector3df btToVector(const btVector3 &v) {
		return vector3df(v.getX(), v.getY(), v.getZ());
	}

	static vector3df getTargetDirection(ISceneNode* node) {
		matrix4 mat = node->getRelativeTransformation();
		vector3df dir(mat[8], mat[9], mat[10]);
		dir.normalize();
		return dir;
	}

	static f32 GetRandInt(f32 max, f32 min) {
		return rand() % (int)(max - min) + min;
	}

	static vector3df randVector3(f32 minX, f32 maxX, f32 minY, f32 maxY, f32 minZ, f32 maxZ) {
		return vector3df(GetRandInt(maxX, minX), GetRandInt(maxY, minY), GetRandInt(maxZ, minZ));
	}
};

#endif