#ifndef EDITOR_RUN_SCRIPT_H
#define EDITOR_RUN_SCRIPT_H

#include "core/reference.h"
#include "editor_plugin.h"
class EditorNode;
class EditorScript : public Reference {
	GDCLASS(EditorScript, Reference);

	EditorNode *editor;

protected:
	static void _bind_methods();

public:
	void add_root_node(Node *p_node);
	Node *get_scene();
	EditorInterface *get_editor_interface();
	virtual void _run();

	void set_editor(EditorNode *p_editor);
	EditorScript();
};

#endif // EDITOR_RUN_SCRIPT_H
