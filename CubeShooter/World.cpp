#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <irrlicht.h>

#include "constants.h"
#include "Helpers.cpp"
#include "EventReceiver.cpp"
#include "CharController.cpp"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

class World {

	// Structs
	struct MyContactResultCallback : public btCollisionWorld::ContactResultCallback {
		bool * charOnGround;

		btScalar addSingleResult(btManifoldPoint& cp,
			const btCollisionObjectWrapper* colObj0Wrap,
			int partId0,
			int index0,
			const btCollisionObjectWrapper* colObj1Wrap,
			int partId1,
			int index1) {
			*charOnGround = true;
			return 0;
		}
	};

protected:
	// Game objects
	btRigidBody * floorBox;
	btRigidBody * charBox;
	btRigidBody * enemyBox;
	btRigidBody * basketBall;
	CharController * mainChar;
	ITexture* crosshair;

	// Tracking
	bool charOnGround = false;
	f32 camYaw = 0.0f;
	f32 camAngle = 1.0f / 2.0f;
	f32 camDist = DEFAULT_CAM_DIST;
	u32 JumpTime;
	bool startJump = true;


public:
	// Tracking
	bool done = false;

	// System objects
	btDiscreteDynamicsWorld *BTWorld;
	list<btRigidBody *> Objects;
	ICameraSceneNode* Camera;
	EventReceiver inputHandler;

	// Engine objects made in main
	ISceneManager *irrScene;
	IVideoDriver *irrDriver;
	ITimer *irrTimer;
	IrrlichtDevice *irrDevice;

	void createWorld() {
		ClearObjects();

		createSky("skydome.jpg");
		irrScene->setAmbientLight(SColorf(0.4f, 0.4f, 0.4f));
		ILightSceneNode*  pLight = irrDevice->getSceneManager()->addLightSceneNode();
		SLight & l = pLight->getLightData();
		l.Type = ELT_POINT;
		l.CastShadows = true;
		ISceneNode* pNode = irrDevice->getSceneManager()->addEmptySceneNode();
		pLight->setPosition(vector3df(0, 30, 60));
		pLight->setParent(pNode);

		floorBox = CreateBox(btVector3(FLOOR_X, FLOOR_Y, FLOOR_Z), vector3df(FLOOR_WIDTH, FLOOR_HEIGHT, FLOOR_LENGTH), 0.0f, "grass_texture.jpg", 16.0f, 16.0f);

		createEnemy();

		basketBall = CreateSphere(btVector3(0, -500, 0), 0.75f, 3.0f);

		createChar();

		Camera = irrScene->addCameraSceneNode(0, vector3df(0, 0, 0), vector3df(0, 0, 0));
		Camera->setFarValue(10000.0f);

		crosshair = irrDriver->getTexture(stringc(ASSETS_PATH) + "crosshair.png");
	}

	void followChar() {
		ISceneNode *charNode = static_cast<ISceneNode *>(charBox->getUserPointer());
		vector3df charPos = charNode->getAbsolutePosition();
		Camera->setTarget(charPos);
	}

	void moveCamera() {
		ISceneNode *charNode = static_cast<ISceneNode *>(charBox->getUserPointer());
		vector3df charPos = charNode->getAbsolutePosition();

		f32 n = (camDist / ROOT_3);

		f32 X = n * (1.0f - camAngle) * (cos((camYaw * DEG_RAD)) - sin((camYaw * DEG_RAD)));
		f32 Y = camAngle * (2.0f * n);
		f32 Z = n * (1.0f - camAngle) * (cos((camYaw * DEG_RAD)) + sin((camYaw * DEG_RAD)));

		X += charPos.X;
		Y += charPos.Y;
		Z += charPos.Z;

		vector3df newPos = vector3df(X, Y, Z);
		Camera->setPosition(newPos);
	}

	void createSky(char * sky) {
		irrDriver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, false);
		ISceneNode* skydome = irrScene->addSkyDomeSceneNode(irrDriver->getTexture(stringc(ASSETS_PATH) + sky), 16, 8, 0.95f, 2.0f);
		irrDriver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, true);
	}

	btRigidBody * CreateBox(const btVector3 &TPosition, const vector3df &TScale, btScalar TMass, const char * texture = NULL, f32 scaleX = 1.0f, f32 scaleY = 1.0f) {

		IMeshSceneNode *Node = irrScene->addCubeSceneNode(1.0f);
		Node->setScale(TScale);
		Node->setMaterialFlag(EMF_LIGHTING, true);
		Node->setMaterialFlag(EMF_NORMALIZE_NORMALS, true);
		Node->addShadowVolumeSceneNode();

		if (texture) {
			Node->setMaterialTexture(0, irrDriver->getTexture(stringc(ASSETS_PATH) + texture));
			Node->getMaterial(0).getTextureMatrix(0).setTextureScale(scaleX, scaleY);
		}

		// Set the initial position of the object
		btTransform Transform;
		Transform.setIdentity();
		Transform.setOrigin(TPosition);

		btDefaultMotionState *MotionState = new btDefaultMotionState(Transform);

		// Create the shape
		btVector3 HalfExtents(TScale.X * 0.5f, TScale.Y * 0.5f, TScale.Z * 0.5f);
		btCollisionShape *Shape = new btBoxShape(HalfExtents);

		// Add mass
		btVector3 LocalInertia;
		Shape->calculateLocalInertia(TMass, LocalInertia);

		// Create the rigid body object
		btRigidBody *RigidBody = new btRigidBody(TMass, MotionState, Shape, LocalInertia);

		// Store a pointer to the irrlicht node so we can update it later
		RigidBody->setUserPointer((void *)(Node));

		RigidBody->setGravity(btVector3(0, GRAVITY, 0));

		// Add it to the world
		addBody(RigidBody);

		return RigidBody;
	}

	btRigidBody * CreateSphere(const btVector3 &TPosition, btScalar TRadius, btScalar TMass) {

		// Create an Irrlicht sphere
		IMeshSceneNode *Node = irrScene->addSphereSceneNode(TRadius, 32);
		Node->setMaterialFlag(video::EMF_LIGHTING, 1);
		Node->setMaterialFlag(video::EMF_NORMALIZE_NORMALS, true);
		Node->setMaterialTexture(0, irrDriver->getTexture(stringc(ASSETS_PATH) + "basketball.jpg"));
		//Node->addShadowVolumeSceneNode(); this causes a stupid amount of lag.

		// Set the initial position of the object
		btTransform Transform;
		Transform.setIdentity();
		Transform.setOrigin(TPosition);

		// Give it a default MotionState
		btDefaultMotionState *MotionState = new btDefaultMotionState(Transform);

		// Create the shape
		btCollisionShape *Shape = new btSphereShape(TRadius);

		// Add mass
		btVector3 LocalInertia;
		Shape->calculateLocalInertia(TMass, LocalInertia);

		// Create the rigid body object
		btRigidBody *RigidBody = new btRigidBody(TMass, MotionState, Shape, LocalInertia);

		// Store a pointer to the irrlicht node so we can update it later
		RigidBody->setUserPointer((void *)(Node));

		// Add it to the world
		BTWorld->addRigidBody(RigidBody);
		Objects.push_back(RigidBody);

		return RigidBody;
	}

	void UpdatePhysics(u32 TDeltaTime) {

		BTWorld->stepSimulation(TDeltaTime * 0.001f, 60);

		// Relay the object's orientation to irrlicht
		for (list<btRigidBody *>::Iterator Iterator = Objects.begin(); Iterator != Objects.end(); ++Iterator) {
			UpdateRender(*Iterator);
		}

		MyContactResultCallback callback;
		callback.charOnGround = &charOnGround;
		charOnGround = false;
		BTWorld->contactPairTest(charBox, floorBox, callback);
	}

	void UpdateRender(btRigidBody *TObject) {
		ISceneNode *Node = static_cast<ISceneNode *>(TObject->getUserPointer());

		// Set position
		btVector3 Point = TObject->getCenterOfMassPosition();
		Node->setPosition(vector3df((f32)Point[0], (f32)Point[1], (f32)Point[2]));

		// Set rotation
		vector3df Euler;
		const btQuaternion& TQuat = TObject->getOrientation();
		quaternion q(TQuat.getX(), TQuat.getY(), TQuat.getZ(), TQuat.getW());
		q.toEuler(Euler);
		Euler *= RADTODEG;
		Node->setRotation(Euler);
	}

	void ClearObjects() {

		for (list<btRigidBody *>::Iterator Iterator = Objects.begin(); Iterator != Objects.end(); ++Iterator) {
			btRigidBody *Object = *Iterator;

			// Delete irrlicht node
			ISceneNode *Node = static_cast<ISceneNode *>(Object->getUserPointer());
			Node->remove();

			// Remove the object from the world
			BTWorld->removeRigidBody(Object);

			// Free memory
			delete Object->getMotionState();
			delete Object->getCollisionShape();
			delete Object;
		}

		Objects.clear();
	}

	void update() {
		if (vectorOutOfBounds(charBox->getCenterOfMassPosition())) {
			charBox->applyCentralImpulse(btVector3(0, GRAVITY, 0));
			if (charBox->getCenterOfMassPosition().getY() < FLOOR_Y - 20) {
				removeObject(charBox);
				createChar();
			}
		}

		if (enemyBox->getCenterOfMassPosition().getY() < FLOOR_Y - 20) {
			removeObject(enemyBox);
			createEnemy();
		}

		if (!inputHandler.canFire && vectorOutOfBounds(basketBall->getCenterOfMassPosition())) {
			removeObject(basketBall);
			basketBall = CreateSphere(btVector3(0, -500, 0), 0.75f, 3.0f);
			inputHandler.canFire = true;
		}
	}

	void drawUI() {
		// Crosshair - this causes a stupid amount of frame rate loss
		//irrDriver->makeColorKeyTexture(crosshair, position2d<s32>(0, 0));
		//irrDriver->draw2DImage(crosshair, position2d<s32>(WIDTH / 2 - 30, HEIGHT / 2 - 90), rect<s32>(0, 0, 60, 60), 0, video::SColor(255, 255, 255, 255), true);
	}

	void removeObject(btRigidBody *body) {
		for (list<btRigidBody *>::Iterator Iterator = Objects.begin(); Iterator != Objects.end(); ++Iterator) {
			btRigidBody *Object = *Iterator;

			if (Object == body) {
				Objects.erase(Iterator);
				break;
			}
		}

		ISceneNode *Node = static_cast<ISceneNode *>(body->getUserPointer());
		Node->remove();

		BTWorld->removeRigidBody(body);

		delete body->getMotionState();
		delete body->getCollisionShape();
		delete body;
	}

	void createChar() {
		btCollisionShape *Shape = new btBoxShape(btVector3(1.0f, 1.0f, 1.0f));
		mainChar = new CharController(Helpers::vectorToBT(vector3df(0, 10, 0)), Shape, 6.0f);

		IMeshSceneNode *Node = irrScene->addCubeSceneNode(1.0f);
		Node->setScale(vector3df(2, 2, 2));
		Node->setMaterialFlag(EMF_LIGHTING, true);
		Node->setMaterialFlag(EMF_NORMALIZE_NORMALS, true);
		Node->setMaterialTexture(0, irrDriver->getTexture(stringc(ASSETS_PATH) + "crate.jpg"));
		Node->addShadowVolumeSceneNode();
		mainChar->setNode(Node);

		charBox = mainChar->body;
		charBox->setGravity(btVector3(0, 0, 0));
		charBox->setActivationState(DISABLE_DEACTIVATION);

		BTWorld->addRigidBody(charBox);
		Objects.push_back(charBox);
	}

	void createEnemy() {
		enemyBox = CreateBox(Helpers::vectorToBT(Helpers::randVector3(FLOOR_X - FLOOR_WIDTH / 2.0f, FLOOR_X + FLOOR_WIDTH / 2.0f, FLOOR_Y + FLOOR_HEIGHT, FLOOR_Y + FLOOR_HEIGHT + 4.0f, FLOOR_Z - FLOOR_LENGTH / 2.0f, FLOOR_Z + FLOOR_LENGTH / 2.0f)), vector3df(2, 2, 2), 6.0f, "crate.jpg");
	}

	void addBody(btRigidBody * body) {
		BTWorld->addRigidBody(body);
		Objects.push_back(body);
	}

	void handleInput() {
		// Handle jumping
		if (!charOnGround) {
			inputHandler.canJump = false;
		} else {
			inputHandler.canJump = true;
			mainChar->jumpSpeed = 0.0f;
			startJump = true;
		}

		if (inputHandler.jump && inputHandler.jumpTimePassed == 0 && startJump) {
			JumpTime = irrTimer->getTime();
			startJump = false;
		}

		inputHandler.jumpTimePassed = irrTimer->getTime() - JumpTime;

		if (inputHandler.jump) {
			mainChar->jumpSpeed = JUMP_SPEED - JUMP_GRAVITY * inputHandler.jumpTimePassed;
		}

		// Linear movement of charBox
		if (inputHandler.moveForward && !inputHandler.moveBackward) {
			if (inputHandler.strafeLeft && !inputHandler.strafeRight) {
				ISceneNode *charNode = static_cast<ISceneNode *>(charBox->getUserPointer());
				vector3df dir = Helpers::getTargetDirection(charNode);
				charBox->setLinearVelocity(btVector3(LIN_SPEED * dir.X - LIN_SPEED * dir.Z, mainChar->jumpSpeed, LIN_SPEED * dir.Z + LIN_SPEED * dir.X));
			} else if (inputHandler.strafeRight && !inputHandler.strafeLeft) {
				ISceneNode *charNode = static_cast<ISceneNode *>(charBox->getUserPointer());
				vector3df dir = Helpers::getTargetDirection(charNode);
				charBox->setLinearVelocity(btVector3(LIN_SPEED * dir.X + LIN_SPEED * dir.Z, mainChar->jumpSpeed, LIN_SPEED * dir.Z - LIN_SPEED * dir.X));
			} else {
				ISceneNode *charNode = static_cast<ISceneNode *>(charBox->getUserPointer());
				vector3df dir = Helpers::getTargetDirection(charNode);
				charBox->setLinearVelocity(btVector3(LIN_SPEED * dir.X, mainChar->jumpSpeed, LIN_SPEED * dir.Z));
			}
		} else if (inputHandler.moveBackward && !inputHandler.moveForward) {
			if (inputHandler.strafeLeft && !inputHandler.strafeRight) {
				ISceneNode *charNode = static_cast<ISceneNode *>(charBox->getUserPointer());
				vector3df dir = Helpers::getTargetDirection(charNode);
				charBox->setLinearVelocity(btVector3(-LIN_SPEED * dir.X - LIN_SPEED * dir.Z, mainChar->jumpSpeed, -LIN_SPEED * dir.Z + LIN_SPEED * dir.X));
			} else if (inputHandler.strafeRight && !inputHandler.strafeLeft) {
				ISceneNode *charNode = static_cast<ISceneNode *>(charBox->getUserPointer());
				vector3df dir = Helpers::getTargetDirection(charNode);
				charBox->setLinearVelocity(btVector3(-LIN_SPEED * dir.X + LIN_SPEED * dir.Z, mainChar->jumpSpeed, -LIN_SPEED * dir.Z - LIN_SPEED * dir.X));
			} else {
				ISceneNode *charNode = static_cast<ISceneNode *>(charBox->getUserPointer());
				vector3df dir = Helpers::getTargetDirection(charNode);
				charBox->setLinearVelocity(btVector3(-LIN_SPEED * dir.X, mainChar->jumpSpeed, -LIN_SPEED * dir.Z));
			}
		} else if (inputHandler.strafeLeft && !inputHandler.strafeRight) {
			ISceneNode *charNode = static_cast<ISceneNode *>(charBox->getUserPointer());
			vector3df dir = Helpers::getTargetDirection(charNode);
			charBox->setLinearVelocity(btVector3(-LIN_SPEED * dir.Z, mainChar->jumpSpeed, LIN_SPEED * dir.X));
		} else if (inputHandler.strafeRight && !inputHandler.strafeLeft) {
			ISceneNode *charNode = static_cast<ISceneNode *>(charBox->getUserPointer());
			vector3df dir = Helpers::getTargetDirection(charNode);
			charBox->setLinearVelocity(btVector3(LIN_SPEED * dir.Z, mainChar->jumpSpeed, -LIN_SPEED * dir.X));
		} else {
			charBox->setLinearVelocity(btVector3(0, mainChar->jumpSpeed, 0));
		}

		if (inputHandler.jumpTimePassed >= JUMP_TIME) {
			inputHandler.jump = false;
		}

		// Angular movement of charBox
		if (inputHandler.rotateCCW && !inputHandler.rotateCW) {
			charBox->setAngularVelocity(btVector3(0, -ANG_SPEED, 0));
			ISceneNode *charNode = static_cast<ISceneNode *>(charBox->getUserPointer());
			vector3df charRot = charNode->getAbsoluteTransformation().getRotationDegrees();
		} else if (inputHandler.rotateCW && !inputHandler.rotateCCW) {
			charBox->setAngularVelocity(btVector3(0, ANG_SPEED, 0));
			ISceneNode *charNode = static_cast<ISceneNode *>(charBox->getUserPointer());
			vector3df charRot = charNode->getAbsoluteTransformation().getRotationDegrees();
		} else {
			charBox->setAngularVelocity(btVector3(0, 0, 0));
		}

		// Camera yaw wrt character based on mouse movement
		if (inputHandler.mouseMovedX) {
			vector2df mousePos = irrDevice->getCursorControl()->getRelativePosition();
			camYaw -= (mousePos.X - 0.5f) * CAM_SENSITIVTY_X;

			if (camYaw >= 360) {
				camYaw = camYaw - 360;
			} else if (camYaw < 0) {
				camYaw = camYaw + 360;
			}

			inputHandler.mouseMovedX = false;
		}

		// Camera angle wrt character based on mouse movement
		if (inputHandler.mouseMovedY) {
			vector2df mousePos = irrDevice->getCursorControl()->getRelativePosition();
			f32 dMouseY = (mousePos.Y - 0.5f);
			f32 newcamAngle = camAngle + dMouseY * CAM_SENSITIVTY_Y;

			if ((newcamAngle >= MIN_CAM_HEIGHT && dMouseY < 0.0f) || (newcamAngle <= MAX_CAM_HEIGHT && dMouseY > 0.0f)) {
				camAngle = newcamAngle;
			} else if (newcamAngle <= MIN_CAM_HEIGHT) {
				camAngle = MIN_CAM_HEIGHT;
			} else if (newcamAngle >= MAX_CAM_HEIGHT) {
				camAngle = MAX_CAM_HEIGHT;
			}

			inputHandler.mouseMovedY = false;
		}

		// Camera zoom using mouse wheel
		if (abs(inputHandler.wheelDelta) >= 0.1f) {

			if ((camDist - inputHandler.wheelDelta) > MIN_CAM_DIST && (camDist - inputHandler.wheelDelta) < MAX_CAM_DIST) {
				camDist -= inputHandler.wheelDelta;
			}

			inputHandler.wheelDelta = 0.0f;
		}

		// Fire a basketball
		if (inputHandler.fire) {
			ISceneNode *charNode = static_cast<ISceneNode *>(charBox->getUserPointer());
			vector3df dir = Helpers::getTargetDirection(charNode);
			btVector3 charPos = charBox->getCenterOfMassPosition();
			f32 angle = BALL_DELTA_ANGLE * (1.0f - ((camAngle / BALL_DEADZONE) > 1.0f ? 1.0f : (camAngle / BALL_DEADZONE))) + BALL_MIN_ANGLE;

			removeObject(basketBall);
			basketBall = CreateSphere(btVector3(charPos.getX() + 2.0f * dir.X, charPos.getY(), charPos.getZ() + 2.0f * dir.Z), 0.75f, 3.0f);

			basketBall->setLinearVelocity(btVector3(dir.X * BALL_SPEED * cos(angle * DEG_RAD), BALL_SPEED * sin(angle * DEG_RAD), dir.Z * BALL_SPEED * cos(angle * DEG_RAD)));

			inputHandler.fire = false;
		}

		// Suicide
		if (inputHandler.respawn) {
			removeObject(charBox);
			createChar();
			inputHandler.respawn = false;
		}

		// Quit
		if (inputHandler.quit) {
			done = true;
			inputHandler.quit = false;
		}
	}

	bool vectorOutOfBounds(const btVector3 &v) {
		if (v.getX() > FLOOR_X + FLOOR_WIDTH / 2.0f || v.getX() < FLOOR_X - FLOOR_WIDTH / 2.0f) {
			return true;
		}

		if (v.getZ() > FLOOR_Z + FLOOR_LENGTH / 2.0f || v.getZ() < FLOOR_Z - FLOOR_LENGTH / 2.0f) {
			return true;
		}

		if (v.getY() < FLOOR_Y - FLOOR_HEIGHT) {
			return true;
		}

		return false;
	}

	void endWorld() {
		ClearObjects();
		delete BTWorld;
	}

};