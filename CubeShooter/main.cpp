#include <irrlicht.h>
#include <irrKlang.h>
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#include <RakPeerInterface.h>

#include "constants.h"
#include "Helpers.cpp"
#include "World.cpp"
#include "EventReceiver.cpp"

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;
using namespace irrklang;

#ifdef _IRR_WINDOWS_
#pragma comment(lib, "Irrlicht.lib")
#pragma comment(lib, "irrKlang.lib") 
#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif

// Functions
void update();
void draw();
void drawUI();
void init();
void initIrr(EventReceiver &er);
void initBT();
void exitGame();

// Engine objects
IrrlichtDevice *irrDevice;
IVideoDriver *irrDriver;
ISceneManager *irrScene;
IGUIEnvironment *irrGUI;
IFileSystem *irrFile;
ITimer *irrTimer;
ILogger *irrLog;

// BT Objects
btDefaultCollisionConfiguration *CollisionConfiguration;
btBroadphaseInterface *BroadPhase;
btCollisionDispatcher *Dispatcher;
btSequentialImpulseConstraintSolver *Solver;

// Globals
World myWorld;
u32 TimeStamp;
u32 DeltaTime;
IGUIFont * font;
IGUIStaticText * text;
long fcount;

// Debug
IGUIStaticText * debugText;
stringw debugStr = L"";

int main() {
	init();

	// Main loop
	while (!myWorld.done) {
		myWorld.handleInput();
		update();
		draw();
	}

	myWorld.endWorld();
	exitGame();

	return 0;
}

void update() {
	DeltaTime = irrTimer->getTime() - TimeStamp;
	TimeStamp = irrTimer->getTime();

	if (DeltaTime < 1) {
		DeltaTime = 1;
	}

	if (irrDevice->isWindowActive()) {
		irrDevice->getCursorControl()->setPosition(0.5f, 0.5f);
	}

	myWorld.update();
	myWorld.UpdatePhysics(DeltaTime);
	myWorld.moveCamera();
	myWorld.followChar();

	// This is a better way of doing dynamic text that will always be there
	stringw fps = L"FPS: ";
	fps += stringw((u32)(1 / (f32)(DeltaTime / 1000.0f)));
	text->setText(fps.c_str());
	debugText->setText(debugStr.c_str());

	fcount++;
}

void draw() {

	// default stuff
	irrDriver->beginScene(true, true, SColor(255, 20, 0, 0));
	irrScene->drawAll();
	irrGUI->drawAll();

	drawUI();

	irrDriver->endScene();
	irrDevice->run();
}

void drawUI() {
	myWorld.drawUI();
	// Good for text that is temporary
	//stringw str = L"Current frame: ";
	//str += stringw(fcount);
	//font->draw(str.c_str(), rect<s32>(0, 15, 300, 15), video::SColor(255, 255, 255, 255));
}

void init() {
	initIrr(myWorld.inputHandler);
	initBT();

	myWorld.irrScene = irrScene;
	myWorld.irrDriver = irrDriver;
	myWorld.irrTimer = irrTimer;
	myWorld.irrDevice = irrDevice;

	myWorld.createWorld();

	// Create text
	IGUISkin *Skin = irrGUI->getSkin();
	Skin->setColor(EGDC_BUTTON_TEXT, SColor(255, 255, 255, 255));
	text = irrGUI->addStaticText(L"FPS", rect<s32>(0, 0, 200, 100), false);
	debugText = irrGUI->addStaticText(debugStr.c_str(), rect<s32>(0, 30, 200, 100), false);
	font = irrGUI->getFont(stringc(ASSETS_PATH) + "fontcourier.bmp");

	if (font) {
		Skin->setFont(font);
	}

	// defaults
	fcount = 0;
	TimeStamp = irrTimer->getTime();
	DeltaTime = 0;
}

void initIrr(EventReceiver &er) {
	irrDevice = createDevice(video::EDT_OPENGL, dimension2d<u32>(WIDTH, HEIGHT), 32, false, true, false, &er);
	irrGUI = irrDevice->getGUIEnvironment();
	irrTimer = irrDevice->getTimer();
	irrScene = irrDevice->getSceneManager();
	irrDriver = irrDevice->getVideoDriver();
	irrDevice->getCursorControl()->setVisible(0); // Make mouse invisible
	irrDevice->setWindowCaption(L"Cube Shooter - Single Player");
}

void initBT() {
	CollisionConfiguration = new btDefaultCollisionConfiguration();
	BroadPhase = new btAxisSweep3(btVector3(-1000, -1000, -1000), btVector3(1000, 1000, 1000));
	Dispatcher = new btCollisionDispatcher(CollisionConfiguration);
	Solver = new btSequentialImpulseConstraintSolver();
	myWorld.BTWorld = new btDiscreteDynamicsWorld(Dispatcher, BroadPhase, Solver, CollisionConfiguration);
}

void exitGame() {
	delete Solver;
	delete Dispatcher;
	delete BroadPhase;
	delete CollisionConfiguration;

	irrDevice->drop();
}