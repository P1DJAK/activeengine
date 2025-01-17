#ifndef EDITOR_PROPERTIES_ARRAY_DICT_H
#define EDITOR_PROPERTIES_ARRAY_DICT_H

#include "editor/editor_inspector.h"
#include "editor/editor_spin_slider.h"
#include "editor/filesystem_dock.h"
#include "scene/gui/button.h"

class EditorPropertyArrayObject : public Reference {
	GDCLASS(EditorPropertyArrayObject, Reference);

	Variant array;

protected:
	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;

public:
	void set_array(const Variant &p_array);
	Variant get_array();

	EditorPropertyArrayObject();
};

class EditorPropertyDictionaryObject : public Reference {
	GDCLASS(EditorPropertyDictionaryObject, Reference);

	Variant new_item_key;
	Variant new_item_value;
	Dictionary dict;

protected:
	bool _set(const StringName &p_name, const Variant &p_value);
	bool _get(const StringName &p_name, Variant &r_ret) const;

public:
	void set_dict(const Dictionary &p_dict);
	Dictionary get_dict();

	void set_new_item_key(const Variant &p_new_item);
	Variant get_new_item_key();

	void set_new_item_value(const Variant &p_new_item);
	Variant get_new_item_value();

	EditorPropertyDictionaryObject();
};

class EditorPropertyArray : public EditorProperty {
	GDCLASS(EditorPropertyArray, EditorProperty);

	PopupMenu *change_type;
	bool updating;
	bool dropping;

	Ref<EditorPropertyArrayObject> object;
	int page_length = 20;
	int page_index = 0;
	int changing_type_index;
	Button *edit;
	VBoxContainer *vbox;
	EditorSpinSlider *size_slider;
	EditorSpinSlider *page_slider;
	HBoxContainer *page_hbox;
	Variant::Type array_type;
	Variant::Type subtype;
	PropertyHint subtype_hint;
	String subtype_hint_string;

	int reorder_from_index = -1;
	int reorder_to_index = -1;
	float reorder_mouse_y_delta = 0.0f;
	HBoxContainer *reorder_selected_element_hbox = nullptr;
	Button *reorder_selected_button = nullptr;

	void _page_changed(double p_page);
	void _length_changed(double p_page);
	void _edit_pressed();
	void _property_changed(const String &p_prop, Variant p_value, const String &p_name = String(), bool changing = false);
	void _change_type(Object *p_button, int p_index);
	void _change_type_menu(int p_index);

	void _object_id_selected(const String &p_property, ObjectID p_id);
	void _remove_pressed(int p_index);

	void _reorder_button_gui_input(const Ref<InputEvent> &p_event);
	void _reorder_button_down(int p_index);
	void _reorder_button_up();

	void _button_draw();
	bool _is_drop_valid(const Dictionary &p_drag_data) const;
	bool can_drop_data_fw(const Point2 &p_point, const Variant &p_data, Control *p_from) const;
	void drop_data_fw(const Point2 &p_point, const Variant &p_data, Control *p_from);

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	void setup(Variant::Type p_array_type, const String &p_hint_string = "");
	virtual void update_property();
	EditorPropertyArray();
};

class EditorPropertyDictionary : public EditorProperty {
	GDCLASS(EditorPropertyDictionary, EditorProperty);

	PopupMenu *change_type;
	bool updating;

	Ref<EditorPropertyDictionaryObject> object;
	int page_length = 20;
	int page_index = 0;
	int changing_type_index;
	Button *edit;
	VBoxContainer *vbox;
	EditorSpinSlider *size_slider;
	EditorSpinSlider *page_slider;
	HBoxContainer *page_hbox;
	Button *button_add_item;

	void _page_changed(double p_page);
	void _edit_pressed();
	void _property_changed(const String &p_prop, Variant p_value, const String &p_name = String(), bool changing = false);
	void _change_type(Object *p_button, int p_index);
	void _change_type_menu(int p_index);

	void _add_key_value();
	void _object_id_selected(const String &p_property, ObjectID p_id);

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	virtual void update_property();
	EditorPropertyDictionary();
};

#endif // EDITOR_PROPERTIES_ARRAY_DICT_H
