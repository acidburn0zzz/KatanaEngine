#ifndef __EDITORMAIN__
#define __EDITORMAIN__

#include <QtWidgets>

#define EDITOR_WINDOW_TITLE	"Katana Editor"

class EditorMainWindow : public QWidget
{
	EditorMainWindow();
	~EditorMainWindow();
};

extern EditorMainWindow *eMainWindow;
#endif