#include "cpu_particles_editor_plugin.h"

#include "editor/plugins/spatial_editor_plugin.h"

void CPUParticlesEditor::_node_removed(Node *p_node) {
	if (p_node == node) {
		node = nullptr;
		hide();
	}
}

void CPUParticlesEditor::_notification(int p_notification) {
	if (p_notification == NOTIFICATION_ENTER_TREE) {
		options->set_icon(options->get_popup()->get_icon("CPUParticles", "EditorIcons"));
	}
}

void CPUParticlesEditor::_menu_option(int p_option) {
	switch (p_option) {
		case MENU_OPTION_CREATE_EMISSION_VOLUME_FROM_MESH: {
			emission_file_dialog->popup_centered_ratio();

		} break;

		case MENU_OPTION_CREATE_EMISSION_VOLUME_FROM_NODE: {
			emission_tree_dialog->popup_centered_ratio();

		} break;

		case MENU_OPTION_RESTART: {
			node->restart();

		} break;
	}
}

void CPUParticlesEditor::edit(CPUParticles *p_particles) {
	base_node = p_particles;
	node = p_particles;
}

void CPUParticlesEditor::_generate_emission_points() {
	/// hacer codigo aca
	PoolVector<Vector3> points;
	PoolVector<Vector3> normals;

	if (!_generate(points, normals)) {
		return;
	}

	if (normals.size() == 0) {
		node->set_emission_shape(CPUParticles::EMISSION_SHAPE_POINTS);
		node->set_emission_points(points);
	} else {
		node->set_emission_shape(CPUParticles::EMISSION_SHAPE_DIRECTED_POINTS);
		node->set_emission_points(points);
		node->set_emission_normals(normals);
	}
}

void CPUParticlesEditor::_bind_methods() {
	ClassDB::bind_method("_menu_option", &CPUParticlesEditor::_menu_option);
}

CPUParticlesEditor::CPUParticlesEditor() {
	particles_editor_hb = memnew(HBoxContainer);
	SpatialEditor::get_singleton()->add_control_to_menu_panel(particles_editor_hb);
	options = memnew(MenuButton);
	options->set_switch_on_hover(true);
	particles_editor_hb->add_child(options);
	particles_editor_hb->hide();

	options->set_text(TTR("CPUParticles"));
	options->get_popup()->add_item(TTR("Create Emission Points From Mesh"), MENU_OPTION_CREATE_EMISSION_VOLUME_FROM_MESH);
	options->get_popup()->add_item(TTR("Create Emission Points From Node"), MENU_OPTION_CREATE_EMISSION_VOLUME_FROM_NODE);
	options->get_popup()->add_separator();
	options->get_popup()->add_item(TTR("Restart"), MENU_OPTION_RESTART);
	options->get_popup()->connect("id_pressed", this, "_menu_option");
}

void CPUParticlesEditorPlugin::edit(Object *p_object) {
	particles_editor->edit(Object::cast_to<CPUParticles>(p_object));
}

bool CPUParticlesEditorPlugin::handles(Object *p_object) const {
	return p_object->is_class("CPUParticles");
}

void CPUParticlesEditorPlugin::make_visible(bool p_visible) {
	if (p_visible) {
		particles_editor->show();
		particles_editor->particles_editor_hb->show();
	} else {
		particles_editor->particles_editor_hb->hide();
		particles_editor->hide();
		particles_editor->edit(nullptr);
	}
}

CPUParticlesEditorPlugin::CPUParticlesEditorPlugin(EditorNode *p_node) {
	editor = p_node;
	particles_editor = memnew(CPUParticlesEditor);
	editor->get_viewport()->add_child(particles_editor);

	particles_editor->hide();
}

CPUParticlesEditorPlugin::~CPUParticlesEditorPlugin() {
}
