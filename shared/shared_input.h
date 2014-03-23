/*	Copyright (C) 2011-2014 OldTimes Software
*/
#ifndef __SHAREDINPUT__
#define	__SHAREDINPUT__

// These are the key numbers that should be passed to Key_Event.
#define	INPUT_KEY_TAB		9
#define	INPUT_KEY_ENTER	    13
#define	K_ESCAPE		27
#define	K_SPACE			32

// Normal keys should be passed as lowercased ascii
#define	K_BACKSPACE		127
#define	K_UPARROW		128
#define	K_DOWNARROW		129
#define	K_LEFTARROW		130
#define	K_RIGHTARROW	131
#define	K_ALT			132
#define	K_CTRL			133
#define	K_SHIFT			134
#define	K_F1			135
#define	K_F2			136
#define	K_F3			137
#define	K_F4			138
#define	K_F5			139
#define	K_F6			140
#define	K_F7			141
#define	K_F8			142
#define	K_F9			143
#define	K_F10			144
#define	K_F11			145
#define	K_F12			146
#define	K_INS			147
#define	K_DEL			148
#define	K_PGDN			149
#define	K_PGUP			150
#define	K_HOME			151
#define	K_END			152

//johnfitz -- keypad
#define	KP_NUMLOCK		153
#define	KP_SLASH		154
#define	KP_STAR			155
#define	KP_MINUS		156
#define	KP_HOME			157
#define	KP_UPARROW		158
#define	KP_PGUP			159
#define	KP_PLUS			160
#define	KP_LEFTARROW	161
#define	KP_5			162
#define	KP_RIGHTARROW	163
#define	KP_END			164
#define	KP_DOWNARROW	165
#define	KP_PGDN			166
#define	KP_ENTER		167
#define	KP_INS			168
#define	KP_DEL			169
//johnfitz

#define K_PAUSE			255

/*  Mouse buttons generate virtual keys.
*/
#define	K_MOUSE1		200
#define	K_MOUSE2		201
#define	K_MOUSE3		202

// Joystick buttons
#define	K_JOY1			203
#define	K_JOY2			204
#define	K_JOY3			205
#define	K_JOY4			206

/*  Aux keys are for multi-buttoned joysticks to generate so they can use
    the normal binding process.
*/
#define	K_AUX1			207
#define	K_AUX2			208
#define	K_AUX3			209
#define	K_AUX4			210
#define	K_AUX5			211
#define	K_AUX6			212
#define	K_AUX7			213
#define	K_AUX8			214
#define	K_AUX9			215
#define	K_AUX10			216
#define	K_AUX11			217
#define	K_AUX12			218
#define	K_AUX13			219
#define	K_AUX14			220
#define	K_AUX15			221
#define	K_AUX16			222
#define	K_AUX17			223
#define	K_AUX18			224
#define	K_AUX19			225
#define	K_AUX20			226
#define	K_AUX21			227
#define	K_AUX22			228
#define	K_AUX23			229
#define	K_AUX24			230
#define	K_AUX25			231
#define	K_AUX26			232
#define	K_AUX27			233
#define	K_AUX28			234
#define	K_AUX29			235
#define	K_AUX30			236
#define	K_AUX31			237
#define	K_AUX32			238
#define K_MWHEELUP		239
#define K_MWHEELDOWN	240
#define	K_CAPSLOCK		300

#endif
