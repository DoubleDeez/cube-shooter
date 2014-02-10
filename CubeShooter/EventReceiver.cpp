#include <irrlicht.h>

using namespace irr;

#ifndef EVENTRECEIVER_CPP
#define EVENTRECEIVER_CPP

class EventReceiver : public IEventReceiver {
public:
	bool moveForward = false;
	bool moveBackward = false;
	bool strafeLeft = false;
	bool strafeRight = false;
	bool rotateCW = false;
	bool rotateCCW = false;
	bool quit = false;
	bool mouseMovedX = false;
	bool mouseMovedY = false;
	bool jump = false;
	bool canJump = true;
	bool fire = false;
	bool canFire = true;
	bool readyToFire = false;
	bool respawn = false;
	u32 jumpTimePassed = 0;
	f32 wheelDelta = 0;


	virtual bool OnEvent(const SEvent &TEvent) {

		if (TEvent.EventType == EET_KEY_INPUT_EVENT && !TEvent.KeyInput.PressedDown) {
			// KEY_UP : Called once after key pressed and released

			if (TEvent.KeyInput.Key == KEY_KEY_W) {
				moveForward = false;
				return true;
			} else if (TEvent.KeyInput.Key == KEY_KEY_S) {
				moveBackward = false;
				return true;
			} else if (TEvent.KeyInput.Key == KEY_KEY_A) {
				rotateCCW = false;
				return true;
			} else if (TEvent.KeyInput.Key == KEY_KEY_D) {
				rotateCW = false;
				return true;
			} else if (TEvent.KeyInput.Key == KEY_KEY_Q) {
				strafeLeft = false;
				return true;
			} else if (TEvent.KeyInput.Key == KEY_KEY_E) {
				strafeRight = false;
				return true;
			} else if (TEvent.KeyInput.Key == KEY_KEY_R) {
				respawn = true;
				return true;
			} else if (TEvent.KeyInput.Key == KEY_SPACE) {
				if (canJump) {
					jump = true;
					canJump = false;
					jumpTimePassed = 0;
				}
				return true;
			}
		} else if (TEvent.EventType == EET_KEY_INPUT_EVENT && TEvent.KeyInput.PressedDown) {
			// KEY_IS_DOWN : Continously called for key held down

			if (TEvent.KeyInput.Key == KEY_ESCAPE) {
				quit = true;
				return true;
			} else if (TEvent.KeyInput.Key == KEY_KEY_W) {
				moveForward = true;
				return true;
			} else if (TEvent.KeyInput.Key == KEY_KEY_S) {
				moveBackward = true;
				return true;
			} else if (TEvent.KeyInput.Key == KEY_KEY_A) {
				rotateCCW = true;
				return true;
			} else if (TEvent.KeyInput.Key == KEY_KEY_D) {
				rotateCW = true;
				return true;
			} else if (TEvent.KeyInput.Key == KEY_KEY_Q) {
				strafeLeft = true;
				return true;
			} else if (TEvent.KeyInput.Key == KEY_KEY_E) {
				strafeRight = true;
				return true;
			}
		} else if (TEvent.EventType == EET_MOUSE_INPUT_EVENT) {
			// MOUSE_EVENT : handles all mouse events
			if (TEvent.MouseInput.Event == EMIE_MOUSE_MOVED) {
				mouseMovedX = true;
				mouseMovedY = true;
				return true;
			} else if (TEvent.MouseInput.Event == EMIE_MOUSE_WHEEL) {
				wheelDelta = TEvent.MouseInput.Wheel;
				return true;
			} else if (TEvent.MouseInput.Event == EMIE_LMOUSE_PRESSED_DOWN && canFire) {
				readyToFire = true;
			} else if (TEvent.MouseInput.Event == EMIE_LMOUSE_LEFT_UP && readyToFire) {
				readyToFire = false;
				fire = true;
				canFire = false;
			}
		}

		return false;
	}
};

#endif