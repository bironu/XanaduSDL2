#ifndef keystate_H
#define keystate_H

#ifdef __WIN32__

// 非同期キー入力
#define get_keystate(vkey)	(GetAsyncKeyState(vkey) < 0)

#define VK_RETURN	0x0D
#define VK_SHIFT	0x10
#define VK_CONTROL	0x11
#define VK_ESCAPE	0x1B
#define VK_SPACE	0x20
#define VK_PRIOR	0x21
#define VK_NEXT		0x22
#define VK_END		0x23
#define VK_HOME		0x24
#define VK_LEFT		0x25
#define VK_UP		0x26
#define VK_RIGHT	0x27
#define VK_DOWN		0x28
#define VK_NUMPAD0	0x60
#define VK_NUMPAD1	0x61
#define VK_NUMPAD2	0x62
#define VK_NUMPAD3	0x63
#define VK_NUMPAD4	0x64
#define VK_NUMPAD5	0x65
#define VK_NUMPAD6	0x66
#define VK_NUMPAD7	0x67
#define VK_NUMPAD8	0x68
#define VK_NUMPAD9	0x69
#define VK_NUMLOCK	0x90

__declspec(dllimport) short __stdcall GetAsyncKeyState(int);

#else

#define VK_SHIFT	0
#define VK_CONTROL	1

#define VK_RETURN	2
#define VK_ESCAPE	3

#define VK_HOME		4
#define VK_LEFT		5
#define VK_UP		6
#define VK_RIGHT	7
#define VK_DOWN		8
#define VK_PRIOR	9
#define VK_NEXT		10
#define VK_END		11

#define VK_SPACE	0x20

extern int keystate_vector[256];

#define get_keystate(vkey) (keystate_vector[vkey])

#endif

#endif // keystate_H
