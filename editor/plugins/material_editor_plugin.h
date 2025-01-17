#ifndef MATERIAL_EDITOR_PLUGIN_H
#define MATERIAL_EDITOR_PLUGIN_H

#include "editor/property_editor.h"
#include "scene/resources/primitive_meshes.h"

#include "editor/editor_node.h"
#include "editor/editor_plugin.h"
#include "scene/3d/camera.h"
#include "scene/3d/light.h"
#include "scene/3d/mesh_instance.h"
#include "scene/resources/material.h"

class ViewportContainer;

class MaterialEditor : public Control {
	GDCLASS(MaterialEditor, Control);

	Vector2 rot = Vector2();

	ViewportContainer *vc;
	Viewport *viewport;
	Spatial *rotation;
	MeshInstance *sphere_instance;
	MeshInstance *box_instance;
	DirectionalLight *light1;
	DirectionalLight *light2;
	Camera *camera;

	Ref<SphereMesh> sphere_mesh;
	Ref<CubeMesh> box_mesh;

	TextureButton *sphere_switch;
	TextureButton *box_switch;

	TextureButton *light_1_switch;
	TextureButton *light_2_switch;

	Ref<Material> material;

	void _button_pressed(Node *p_button);
	bool first_enter;

protected:
	void _notification(int p_what);
	void _gui_input(const Ref<InputEvent> &p_event);
	void _update_rotation();

	static void _bind_methods();

public:
	void edit(Ref<Material> p_material, const Ref<Environment> &p_env);
	MaterialEditor();
};

class EditorInspectorPluginMaterial : public EditorInspectorPlugin {
	GDCLASS(EditorInspectorPluginMaterial, EditorInspectorPlugin);
	Ref<Environment> env;

public:
	virtual bool can_handle(Object *p_object);
	virtual void parse_begin(Object *p_object);

	EditorInspectorPluginMaterial();
};

class MaterialEditorPlugin : public EditorPlugin {
	GDCLASS(MaterialEditorPlugin, EditorPlugin);

public:
	virtual String get_name() const { return "Material"; }

	MaterialEditorPlugin(EditorNode *p_node);
};

class SpatialMaterialConversionPlugin : public EditorResourceConversionPlugin {
	GDCLASS(SpatialMaterialConversionPlugin, EditorResourceConversionPlugin);

public:
	virtual String converts_to() const;
	virtual bool handles(const Ref<Resource> &p_resource) const;
	virtual Ref<Resource> convert(const Ref<Resource> &p_resource) const;
};

class ParticlesMaterialConversionPlugin : public EditorResourceConversionPlugin {
	GDCLASS(ParticlesMaterialConversionPlugin, EditorResourceConversionPlugin);

public:
	virtual String converts_to() const;
	virtual bool handles(const Ref<Resource> &p_resource) const;
	virtual Ref<Resource> convert(const Ref<Resource> &p_resource) const;
};

class CanvasItemMaterialConversionPlugin : public EditorResourceConversionPlugin {
	GDCLASS(CanvasItemMaterialConversionPlugin, EditorResourceConversionPlugin);

public:
	virtual String converts_to() const;
	virtual bool handles(const Ref<Resource> &p_resource) const;
	virtual Ref<Resource> convert(const Ref<Resource> &p_resource) const;
};

#endif // MATERIAL_EDITOR_PLUGIN_H
