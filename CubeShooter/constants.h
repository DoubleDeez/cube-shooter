// Default stuff
#define WIDTH 1366
#define HEIGHT 768
#define ASSETS_PATH "assets/"

#define ROOT_3 1.73205080757f
#define PI 3.14159265359f
#define DEG_RAD PI / 180.0f

// Game specific
#define LIN_SPEED 5
#define ANG_SPEED 1
#define JUMP_SPEED 10
#define JUMP_TIME 1000
#define GRAVITY -10.0f
#define JUMP_GRAVITY 2.0f * JUMP_SPEED / JUMP_TIME

#define DEFAULT_CAM_DIST 20
#define MIN_CAM_DIST 3
#define MAX_CAM_DIST 50
#define CAM_SENSITIVTY_X 200
#define CAM_SENSITIVTY_Y 2.5f
#define MAX_CAM_HEIGHT 0.99f
#define MIN_CAM_HEIGHT 0.01f

#define BALL_SPEED 25.0f
#define BALL_MAX_ANGLE 35.0f
#define BALL_MIN_ANGLE 3.0f
#define BALL_DELTA_ANGLE (BALL_MAX_ANGLE - BALL_MIN_ANGLE)
#define BALL_DEADZONE 0.2f

#define FLOOR_X 0
#define FLOOR_Y 0
#define FLOOR_Z 0
#define FLOOR_WIDTH 60
#define FLOOR_HEIGHT 0.5f
#define FLOOR_LENGTH 60