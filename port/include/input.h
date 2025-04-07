#ifndef _IN_INPUT_H
#define _IN_INPUT_H

#include <stdint.h>

#define INPUT_MAX_CONTROLLERS MAXCONTROLLERS
#define INPUT_MAX_CONNECTED_CONTROLLERS 8
#define INPUT_MAX_CONTROLLER_BUTTONS 32
#define INPUT_MAX_BINDS 4

#define CONT_STICK_XNEG 0x10000
#define CONT_STICK_XPOS 0x20000
#define CONT_STICK_YNEG 0x40000
#define CONT_STICK_YPOS 0x80000

#define CONT_NUM_BUTTONS 32 // not including the stick axes

#define MAXCONTROLLERS  4

/* controller errors */
#define CONT_NO_RESPONSE_ERROR          0x8
#define CONT_OVERRUN_ERROR              0x4

/* Controller type */

#define CONT_ABSOLUTE           0x0001
#define CONT_RELATIVE           0x0002
#define CONT_JOYPORT            0x0004
#define CONT_EEPROM		0x8000
#define CONT_EEP16K		0x4000
#define	CONT_TYPE_MASK		0x1f07
#define	CONT_TYPE_NORMAL	0x0005
#define	CONT_TYPE_MOUSE		0x0002
#define	CONT_TYPE_VOICE		0x0100

/* Controller status */

#define CONT_CARD_ON            0x01
#define CONT_CARD_PULL          0x02
#define CONT_ADDR_CRC_ER        0x04
#define CONT_EEPROM_BUSY	0x80

/* Buttons */
#define CONT_8000      0x80000000
#define CONT_4000      0x40000000
#define CONT_2000      0x20000000
#define CONT_GKEY      0x10000000 // Gangsta Key
#define CONT_0800      0x08000000
#define CONT_0400      0x04000000
#define CONT_0200      0x02000000
#define CONT_0100      0x01000000
#define CONT_0080      0x00800000
#define CONT_0040      0x00400000
#define CONT_0020      0x00200000
#define CONT_0010      0x00100000
#define CONT_0008      0x00080000
#define CONT_0004      0x00040000
#define CONT_0002      0x00020000
#define CONT_0001      0x00010000

#define CONT_A         0x00008000
#define CONT_B         0x00004000
#define CONT_G	       0x00002000
#define CONT_START     0x00001000
#define CONT_UP        0x00000800
#define CONT_DOWN      0x00000400
#define CONT_LEFT      0x00000200
#define CONT_RIGHT     0x00000100
#define CONT_EXTRA1    0x00000080
#define CONT_EXTRA0    0x00000040
#define CONT_L         0x00000020
#define CONT_R         0x00000010
#define CONT_E         0x00000008
#define CONT_D         0x00000004
#define CONT_C         0x00000002
#define CONT_F         0x00000001

/* Nintendo's official button names */

#define A_BUTTON		CONT_A
#define B_BUTTON		CONT_B
#define L_TRIG			CONT_L
#define R_TRIG			CONT_R
#define Z_TRIG			CONT_G
#define START_BUTTON	CONT_START
#define U_JPAD			CONT_UP
#define L_JPAD			CONT_LEFT
#define R_JPAD			CONT_RIGHT
#define D_JPAD			CONT_DOWN
#define U_CBUTTONS		CONT_E
#define L_CBUTTONS		CONT_C
#define R_CBUTTONS		CONT_F
#define D_CBUTTONS		CONT_D
#define X_BUTTON		CONT_EXTRA0
#define Y_BUTTON		CONT_EXTRA1
#define G_BUTTON        CONT_GKEY

#define PFS_ERR_NOPACK		1	/* no memory card is plugged or   */
#define PFS_ERR_NEW_PACK        2	/* ram pack has been changed to a */
#define PFS_ERR_INCONSISTENT    3	/* need to run Pfschecker 	  */
#define PFS_ERR_CONTRFAIL			CONT_OVERRUN_ERROR
#define PFS_ERR_INVALID         5	/* invalid parameter or file not exist*/
#define PFS_ERR_BAD_DATA        6       /* the data read from pack are bad*/
#define PFS_DATA_FULL           7	/* no free pages on ram pack      */
#define PFS_DIR_FULL            8	/* no free directories on ram pack*/
#define PFS_ERR_EXIST		9	/* file exists 			  */
#define PFS_ERR_ID_FATAL	10	/* dead ram pack */
#define PFS_ERR_DEVICE		11	/* wrong device type*/
#define PFS_ERR_NO_GBCART	12 	/* no gb cartridge (64GB-PAK) */
#define PFS_ERR_NEW_GBCART	13 	/* gb cartridge may be changed */

enum virtkey {
	/* same order as SDL scancodes */
	VK_KEYBOARD_BEGIN = 0,
	VK_A = 4,
	VK_Z = 29,
	VK_1 = 30,
	VK_9 = 38,
	VK_0 = 39,
	VK_RETURN = 40,
	VK_ESCAPE = 41,
	VK_BACKSPACE = 42,
	VK_SPACE = 44,
	VK_MINUS = 45,
	VK_LEFTBRACKET = 47,
	VK_RIGHTBRACKET = 48,
	VK_SEMICOLON = 51,
	VK_GRAVE = 53,
	VK_COMMA = 54,
	VK_PERIOD = 55,
	VK_F1 = 58,
	VK_F9 = 66,
	VK_DELETE = 76,
	VK_LCTRL = 224,
	VK_LSHIFT = 225,
	VK_RCTRL = 228,
	VK_RSHIFT = 229,

	/* same order as SDL mouse buttons */
	VK_MOUSE_BEGIN = 512,
	VK_MOUSE_LEFT = VK_MOUSE_BEGIN,
	VK_MOUSE_MIDDLE,
	VK_MOUSE_RIGHT,
	VK_MOUSE_X1,
	VK_MOUSE_X2,
	VK_MOUSE_WHEEL_UP,
	VK_MOUSE_WHEEL_DN,

	/* same order as SDL gamecontroller buttons plus two buttons for triggers */
	VK_JOY_BEGIN,
	VK_JOY1_BEGIN = VK_JOY_BEGIN,
	VK_JOY1_LTRIG = VK_JOY1_BEGIN + 30,
	VK_JOY1_RTRIG = VK_JOY1_BEGIN + 31,
	VK_JOY2_BEGIN = VK_JOY1_BEGIN + INPUT_MAX_CONTROLLER_BUTTONS,
	VK_JOY3_BEGIN = VK_JOY2_BEGIN + INPUT_MAX_CONTROLLER_BUTTONS,
	VK_JOY4_BEGIN = VK_JOY3_BEGIN + INPUT_MAX_CONTROLLER_BUTTONS,

	VK_TOTAL_COUNT = VK_JOY_BEGIN + INPUT_MAX_CONTROLLERS * INPUT_MAX_CONTROLLER_BUTTONS,
};

enum keymod {
	/* same order as SDL keymods */
	KM_LSHIFT = 0x0001,
	KM_RSHIFT = 0x0002,
	KM_LCTRL = 0x0040,
	KM_RCTRL = 0x0080,
	KM_CAPS = 0x2000,
	KM_CTRL = KM_LCTRL | KM_RCTRL,
	KM_SHIFT = KM_LSHIFT | KM_RSHIFT
};

enum contkey {
	CK_C_R,
	CK_C_L,
	CK_C_D,
	CK_C_U,
	CK_RTRIG,
	CK_LTRIG,
	CK_X, // gap in CONT_
	CK_Y, // gap in CONT_
	CK_DPAD_R,
	CK_DPAD_L,
	CK_DPAD_D,
	CK_DPAD_U,
	CK_START,
	CK_ZTRIG,
	CK_B,
	CK_A,
	CK_STICK_XNEG,
	CK_STICK_XPOS,
	CK_STICK_YNEG,
	CK_STICK_YPOS,
	CK_ACCEPT,
	CK_CANCEL,
	CK_G,
	CK_0080,
	CK_0100,
	CK_0200,
	CK_0400,
	CK_0800,
	CK_GKEY,
	CK_2000,
	CK_4000,
	CK_8000,
	CK_TOTAL_COUNT
};

enum mouselockmode {
	MLOCK_OFF = 0,
	MLOCK_ON = 1,
	MLOCK_AUTO = 2
};

typedef struct {
	uint16_t     type;                   /* Controller Type */
	uint8_t      status;                 /* Controller status */
	uint8_t	errnum;
}JoyContStatus;

typedef struct {
	uint32_t     button;
	int8_t      stick_x;		/* -80 <= stick_x <= 80 */
	int8_t      stick_y;		/* -80 <= stick_y <= 80 */
	uint8_t	errnum;
#ifndef PLATFORM_N64
	int8_t      rstick_x;
	int8_t      rstick_y;
#endif
} JoyContPad;

// returns bitmask of connected controllers or -1 if failed
int inputInit(void);

// read the specified player's inputs into the N64 pad struct
// returns 0 if read, non-0 if failed
int inputReadController(int idx, JoyContPad *npad);

// returns 1 if rumble is supported for specified player's controller
int inputRumbleSupported(int idx);

// returns 1 if specified player has a live controller assigned
int inputControllerConnected(int idx);

// returns bitmask of players with assigned controllers
int inputControllerMask(void);

// get/set Input.Player%d.SwapSticks
int inputControllerGetSticksSwapped(int cidx);
void inputControllerSetSticksSwapped(int cidx, int swapped);

// get/set Input.Player%d.StickCButtons (DualAnalog is 1 if StickCButtons is 0)
int inputControllerGetDualAnalog(int cidx);
void inputControllerSetDualAnalog(int cidx, int enable);

// get/set Input.Player%d.CancelCButtons
int inputControllerGetCancelCButtons(int cidx);
void inputControllerSetCancelCButtons(int cidx, int cancel);

// get/set sensitivity for a given player
float inputControllerGetAxisScale(int cidx, int stick, int axis);
void inputControllerSetAxisScale(int cidx, int stick, int axis, float value);

// get/set deadzone for a given player
float inputControllerGetAxisDeadzone(int cidx, int stick, int axis);
void inputControllerSetAxisDeadzone(int cidx, int stick, int axis, float value);

// writes array of up to INPUT_MAX_CONNECTED_CONTROLLERS controller IDs
// for all the controllers available on this machine into out if it's not NULL
// returns number of IDs that would've been written (or were written if out is not NULL)
int inputGetConnectedControllers(int *out);

// get name of connected controller id
// returns "Invalid" on failure
const char *inputGetConnectedControllerName(int id);

// get id of the controller currently assigned to player cidx or -1 if none
int inputGetAssignedControllerId(int cidx);

// assign connected controller id to player cidx
// if id is -1, unassigns controller from player, if any
// returns true on success, false on failure
int inputAssignController(int cidx, int id);

// vk is a value from the virtkey enum above
int inputKeyPressed(uint32_t vk);
int inputKeyJustPressed(uint32_t vk);

// idx is controller index, contbtn is one of the CONT_ constants
int inputButtonPressed(int idx, uint32_t contbtn);

// bind virtkey vk to n64 pad #idx's button/axis ck as represented by its contkey value
// if bind is -1, picks a bind slot automatically
void inputKeyBind(int idx, uint32_t ck, int bind, uint32_t vk);

const uint32_t *inputKeyGetBinds(int idx, uint32_t ck);

// get VK_ value from human-readable name
int inputGetKeyByName(const char *name);

// get human-readable name from VK_ value
const char *inputGetKeyName(int vk);

// get CK_ value from human-readable name
int inputGetContKeyByName(const char *name);

// get human-readable name from CK_ value
const char *inputGetContKeyName(uint32_t ck);

// strength is 0 .. 1; 0 strength turns it off
void inputRumble(int idx, float strength, float time);

float inputRumbleGetStrength(int cidx);
void inputRumbleSetStrength(int cidx, float val);

// locks the mouse cursor in the window and makes it invisible if argument is true
void inputLockMouse(int lock);

// returns the current state of the above
int inputMouseIsLocked(void);

// sets x, y to mouse position in native viewport coordinates (ie, 320x240 most of the time)
// returns true if mouse has moved this input frame
int inputMouseGetPosition(int *x, int *y);

// returns changes in mouse position since last frame, in window coordinates
void inputMouseGetRawDelta(int *dx, int *dy);

// returns changes in mouse position since last frame, scaled by sensitivity
// returns 0, 0 when the mouse is not locked into the window
void inputMouseGetScaledDelta(float *dx, float *dy);

// returns changes in mouse position since last frame, scaled by absolute sensitivity
// returns 0, 0 when the mouse is not locked into the window
void inputMouseGetAbsScaledDelta(float *dx, float *dy);

void inputMouseGetSpeed(float *x, float *y);
void inputMouseSetSpeed(float x, float y);

int inputMouseIsEnabled(void);
void inputMouseEnable(int enabled);

// call this every frame
void inputUpdate(void);

// call this before configSave()
void inputSaveBinds(void);

// reset given player's binds to either PC or N64 defaults
void inputSetDefaultKeyBinds(int cidx, int n64mode);

// clear or get the last pressed button
void inputClearLastKey(void);
int inputGetLastKey(void);

// get/set Input.MouseLockMode
int inputGetMouseLockMode(void);
void inputSetMouseLockMode(int lockmode);

// same as inputLockMouse but works only if mouse is enabled and lockmode == MLOCK_AUTO
int inputAutoLockMouse(int wantlock);

// show/hide mouse cursor; if mouse lock is on the cursor is always hidden
void inputMouseShowCursor(int show);

void inputStartTextInput(void);
void inputStopTextInput(void);
int inputIsTextInputActive(void);

void inputClearLastTextChar(void);
char inputGetLastTextChar(void);

int inputTextHandler(char *out, const uint32_t outSize, int *curCol, int oskCharsOnly);

void inputClearClipboard(void);
const char *inputGetClipboard(void);

// returns keymod values
uint32_t inputGetKeyModState(void);

#endif
