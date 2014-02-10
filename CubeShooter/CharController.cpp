#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <irrlicht.h>

#include "constants.h"

using namespace irr;
using namespace core;
using namespace scene;

#ifndef CHARCONTROLLER_CPP
#define CHARCONTROLLER_CPP

class CharController {
public:
	btRigidBody * body;
	f32 jumpSpeed = GRAVITY;

	CharController(const btVector3 &pos, btCollisionShape *shape, btScalar mass) {
		// set defaults
		btVector3 localIntertia = btVector3(1, 0, 1);
		//shape->calculateLocalInertia(mass, localIntertia);
		btTransform startTransform = btTransform::getIdentity();
		startTransform.setOrigin(pos);
		btDefaultMotionState *state = new btDefaultMotionState(startTransform);

		body = new btRigidBody(mass, state, shape, localIntertia);
		body->setFriction(0.5f);
	}

	~CharController() {
		delete body;
	}

	void CharController::setNode(IMeshSceneNode *node) {
		body->setUserPointer((void *)(node));
	}

	void* CharController::operator new(size_t size){
		void* p = _aligned_malloc(size, 16);

		if (!p) throw std::bad_alloc();

		return p;
	}

	void CharController::operator delete (void *p) {
		CharController *ptr = static_cast<CharController*>(p);
		_aligned_free(p);
	}
};

#endif