#include "resource_importer_obj.h"

#include "core/io/resource_saver.h"
#include "core/os/file_access.h"
#include "scene/3d/mesh_instance.h"
#include "scene/3d/spatial.h"
#include "scene/resources/mesh.h"
#include "scene/resources/surface_tool.h"

uint32_t EditorOBJImporter::get_import_flags() const {
	return IMPORT_SCENE;
}

static Error _parse_material_library(const String &p_path, Map<String, Ref<Material3D>> &material_map, List<String> *r_missing_deps) {
	FileAccessRef f = FileAccess::open(p_path, FileAccess::READ);
	ERR_FAIL_COND_V_MSG(!f, ERR_CANT_OPEN, vformat("Couldn't open MTL file '%s', it may not exist or not be readable.", p_path));

	Ref<SpatialMaterial> current;
	String current_name;
	String base_path = p_path.get_base_dir();
	while (true) {
		String l = f->get_line().strip_edges();

		if (l.begins_with("newmtl ")) {
			//vertex

			current_name = l.replace("newmtl", "").strip_edges();
			current.instance();
			current->set_name(current_name);
			material_map[current_name] = current;
		} else if (l.begins_with("Ka ")) {
			//uv
			WARN_PRINT("OBJ: Ambient light for material '" + current_name + "' is ignored in PBR");

		} else if (l.begins_with("Kd ")) {
			//normal
			ERR_FAIL_COND_V(current.is_null(), ERR_FILE_CORRUPT);
			Vector<String> v = l.split(" ", false);
			ERR_FAIL_COND_V(v.size() < 4, ERR_INVALID_DATA);
			Color c = current->get_albedo();
			c.r = v[1].to_float();
			c.g = v[2].to_float();
			c.b = v[3].to_float();
			current->set_albedo(c);
		} else if (l.begins_with("Ks ")) {
			//normal
			ERR_FAIL_COND_V(current.is_null(), ERR_FILE_CORRUPT);
			Vector<String> v = l.split(" ", false);
			ERR_FAIL_COND_V(v.size() < 4, ERR_INVALID_DATA);
			float r = v[1].to_float();
			float g = v[2].to_float();
			float b = v[3].to_float();
			float metalness = MAX(r, MAX(g, b));
			current->set_metallic(metalness);
		} else if (l.begins_with("Ns ")) {
			//normal
			ERR_FAIL_COND_V(current.is_null(), ERR_FILE_CORRUPT);
			Vector<String> v = l.split(" ", false);
			ERR_FAIL_COND_V(v.size() != 2, ERR_INVALID_DATA);
			float s = v[1].to_float();
			current->set_metallic((1000.0 - s) / 1000.0);
		} else if (l.begins_with("d ")) {
			//normal
			ERR_FAIL_COND_V(current.is_null(), ERR_FILE_CORRUPT);
			Vector<String> v = l.split(" ", false);
			ERR_FAIL_COND_V(v.size() != 2, ERR_INVALID_DATA);
			float d = v[1].to_float();
			Color c = current->get_albedo();
			c.a = d;
			current->set_albedo(c);
			if (c.a < 0.99) {
				current->set_feature(Material3D::FEATURE_TRANSPARENT, true);
			}
		} else if (l.begins_with("Tr ")) {
			//normal
			ERR_FAIL_COND_V(current.is_null(), ERR_FILE_CORRUPT);
			Vector<String> v = l.split(" ", false);
			ERR_FAIL_COND_V(v.size() != 2, ERR_INVALID_DATA);
			float d = v[1].to_float();
			Color c = current->get_albedo();
			c.a = 1.0 - d;
			current->set_albedo(c);
			if (c.a < 0.99) {
				current->set_feature(Material3D::FEATURE_TRANSPARENT, true);
			}

		} else if (l.begins_with("map_Ka ")) {
			//uv
			WARN_PRINT("OBJ: Ambient light texture for material '" + current_name + "' is ignored in PBR");

		} else if (l.begins_with("map_Kd ")) {
			//normal
			ERR_FAIL_COND_V(current.is_null(), ERR_FILE_CORRUPT);

			String p = l.replace("map_Kd", "").replace("\\", "/").strip_edges();
			String path;
			if (p.is_abs_path()) {
				path = p;
			} else {
				path = base_path.plus_file(p);
			}

			Ref<Texture> texture = ResourceLoader::load(path);

			if (texture.is_valid()) {
				current->set_texture(Material3D::TEXTURE_ALBEDO, texture);
			} else if (r_missing_deps) {
				r_missing_deps->push_back(path);
			}

		} else if (l.begins_with("map_Ks ")) {
			//normal
			ERR_FAIL_COND_V(current.is_null(), ERR_FILE_CORRUPT);

			String p = l.replace("map_Ks", "").replace("\\", "/").strip_edges();
			String path;
			if (p.is_abs_path()) {
				path = p;
			} else {
				path = base_path.plus_file(p);
			}

			Ref<Texture> texture = ResourceLoader::load(path);

			if (texture.is_valid()) {
				current->set_texture(Material3D::TEXTURE_METALLIC, texture);
			} else if (r_missing_deps) {
				r_missing_deps->push_back(path);
			}

		} else if (l.begins_with("map_Ns ")) {
			//normal
			ERR_FAIL_COND_V(current.is_null(), ERR_FILE_CORRUPT);

			String p = l.replace("map_Ns", "").replace("\\", "/").strip_edges();
			String path;
			if (p.is_abs_path()) {
				path = p;
			} else {
				path = base_path.plus_file(p);
			}

			Ref<Texture> texture = ResourceLoader::load(path);

			if (texture.is_valid()) {
				current->set_texture(Material3D::TEXTURE_ROUGHNESS, texture);
			} else if (r_missing_deps) {
				r_missing_deps->push_back(path);
			}
		} else if (l.begins_with("map_bump ")) {
			//normal
			ERR_FAIL_COND_V(current.is_null(), ERR_FILE_CORRUPT);

			String p = l.replace("map_bump", "").replace("\\", "/").strip_edges();
			String path = base_path.plus_file(p);

			Ref<Texture> texture = ResourceLoader::load(path);

			if (texture.is_valid()) {
				current->set_feature(Material3D::FEATURE_NORMAL_MAPPING, true);
				current->set_texture(Material3D::TEXTURE_NORMAL, texture);
			} else if (r_missing_deps) {
				r_missing_deps->push_back(path);
			}
		} else if (f->eof_reached()) {
			break;
		}
	}

	return OK;
}

static Error _parse_obj(const String &p_path, List<Ref<Mesh>> &r_meshes, bool p_single_mesh, bool p_generate_tangents, int p_compress_flags, Vector3 p_scale_mesh, Vector3 p_offset_mesh, List<String> *r_missing_deps) {
	FileAccessRef f = FileAccess::open(p_path, FileAccess::READ);
	ERR_FAIL_COND_V_MSG(!f, ERR_CANT_OPEN, vformat("Couldn't open OBJ file '%s', it may not exist or not be readable.", p_path));

	Ref<ArrayMesh> mesh;
	mesh.instance();

	bool generate_tangents = p_generate_tangents;
	Vector3 scale_mesh = p_scale_mesh;
	Vector3 offset_mesh = p_offset_mesh;

	Vector<Vector3> vertices;
	Vector<Vector3> normals;
	Vector<Vector2> uvs;
	Vector<Color> colors;
	String name;

	Map<String, Map<String, Ref<Material3D>>> material_map;

	Ref<SurfaceTool> surf_tool = memnew(SurfaceTool);
	surf_tool->begin(Mesh::PRIMITIVE_TRIANGLES);

	String current_material_library;
	String current_material;
	String current_group;

	while (true) {
		String l = f->get_line().strip_edges();
		while (l.length() && l[l.length() - 1] == '\\') {
			String add = f->get_line().strip_edges();
			l += add;
			if (add == String()) {
				break;
			}
		}

		if (l.begins_with("v ")) {
			//vertex
			Vector<String> v = l.split(" ", false);
			ERR_FAIL_COND_V(v.size() < 4, ERR_FILE_CORRUPT);
			Vector3 vtx;
			vtx.x = v[1].to_float() * scale_mesh.x + offset_mesh.x;
			vtx.y = v[2].to_float() * scale_mesh.y + offset_mesh.y;
			vtx.z = v[3].to_float() * scale_mesh.z + offset_mesh.z;
			vertices.push_back(vtx);
			//vertex colors
			if (v.size() >= 7) {
				while (colors.size() < vertices.size() - 1) {
					colors.push_back(Color(1.0, 1.0, 1.0));
				}
				Color c;
				c.r = v[4].to_float();
				c.g = v[5].to_float();
				c.b = v[6].to_float();
				colors.push_back(c);
			} else if (!colors.empty()) {
				colors.push_back(Color(1.0, 1.0, 1.0));
			}
		} else if (l.begins_with("vt ")) {
			//uv
			Vector<String> v = l.split(" ", false);
			ERR_FAIL_COND_V(v.size() < 3, ERR_FILE_CORRUPT);
			Vector2 uv;
			uv.x = v[1].to_float();
			uv.y = 1.0 - v[2].to_float();
			uvs.push_back(uv);

		} else if (l.begins_with("vn ")) {
			//normal
			Vector<String> v = l.split(" ", false);
			ERR_FAIL_COND_V(v.size() < 4, ERR_FILE_CORRUPT);
			Vector3 nrm;
			nrm.x = v[1].to_float();
			nrm.y = v[2].to_float();
			nrm.z = v[3].to_float();
			normals.push_back(nrm);
		} else if (l.begins_with("f ")) {
			//vertex

			Vector<String> v = l.split(" ", false);
			ERR_FAIL_COND_V(v.size() < 4, ERR_FILE_CORRUPT);

			//not very fast, could be sped up

			Vector<String> face[3];
			face[0] = v[1].split("/");
			face[1] = v[2].split("/");
			ERR_FAIL_COND_V(face[0].size() == 0, ERR_FILE_CORRUPT);

			ERR_FAIL_COND_V(face[0].size() != face[1].size(), ERR_FILE_CORRUPT);
			for (int i = 2; i < v.size() - 1; i++) {
				face[2] = v[i + 1].split("/");

				ERR_FAIL_COND_V(face[0].size() != face[2].size(), ERR_FILE_CORRUPT);
				for (int j = 0; j < 3; j++) {
					int idx = j;

					if (idx < 2) {
						idx = 1 ^ idx;
					}

					if (face[idx].size() == 3) {
						int norm = face[idx][2].to_int() - 1;
						if (norm < 0) {
							norm += normals.size() + 1;
						}
						ERR_FAIL_INDEX_V(norm, normals.size(), ERR_FILE_CORRUPT);
						surf_tool->add_normal(normals[norm]);
					}

					if (face[idx].size() >= 2 && face[idx][1] != String()) {
						int uv = face[idx][1].to_int() - 1;
						if (uv < 0) {
							uv += uvs.size() + 1;
						}
						ERR_FAIL_INDEX_V(uv, uvs.size(), ERR_FILE_CORRUPT);
						surf_tool->add_uv(uvs[uv]);
					}

					int vtx = face[idx][0].to_int() - 1;
					if (vtx < 0) {
						vtx += vertices.size() + 1;
					}
					ERR_FAIL_INDEX_V(vtx, vertices.size(), ERR_FILE_CORRUPT);

					Vector3 vertex = vertices[vtx];
					if (!colors.empty()) {
						surf_tool->add_color(colors[vtx]);
					}
					//if (weld_vertices)
					//	vertex.snap(Vector3(weld_tolerance, weld_tolerance, weld_tolerance));
					surf_tool->add_vertex(vertex);
				}

				face[1] = face[2];
			}
		} else if (l.begins_with("s ")) { //smoothing
			String what = l.substr(2, l.length()).strip_edges();
			if (what == "off") {
				surf_tool->add_smooth_group(false);
			} else {
				surf_tool->add_smooth_group(true);
			}
		} else if (/*l.begins_with("g ") ||*/ l.begins_with("usemtl ") || (l.begins_with("o ") || f->eof_reached())) { //commit group to mesh
			//groups are too annoying
			if (surf_tool->get_vertex_array().size()) {
				//another group going on, commit it
				if (normals.size() == 0) {
					surf_tool->generate_normals();
				}

				if (generate_tangents && uvs.size()) {
					surf_tool->generate_tangents();
				}

				surf_tool->index();

				print_verbose("OBJ: Current material library " + current_material_library + " has " + itos(material_map.has(current_material_library)));
				print_verbose("OBJ: Current material " + current_material + " has " + itos(material_map.has(current_material_library) && material_map[current_material_library].has(current_material)));

				if (material_map.has(current_material_library) && material_map[current_material_library].has(current_material)) {
					Ref<Material3D> &material = material_map[current_material_library][current_material];
					if (!colors.empty()) {
						material->set_flag(SpatialMaterial::FLAG_SRGB_VERTEX_COLOR, true);
					}
					surf_tool->set_material(material);
				}

				mesh = surf_tool->commit(mesh, p_compress_flags);

				if (current_material != String()) {
					mesh->surface_set_name(mesh->get_surface_count() - 1, current_material.get_basename());
				} else if (current_group != String()) {
					mesh->surface_set_name(mesh->get_surface_count() - 1, current_group);
				}

				print_verbose("OBJ: Added surface :" + mesh->surface_get_name(mesh->get_surface_count() - 1));
				surf_tool->clear();
				surf_tool->begin(Mesh::PRIMITIVE_TRIANGLES);
			}

			if (l.begins_with("o ") || f->eof_reached()) {
				if (!p_single_mesh) {
					mesh->set_name(name);
					r_meshes.push_back(mesh);
					mesh.instance();
					current_group = "";
					current_material = "";
				}
			}

			if (f->eof_reached()) {
				break;
			}

			if (l.begins_with("o ")) {
				name = l.substr(2, l.length()).strip_edges();
			}

			if (l.begins_with("usemtl ")) {
				current_material = l.replace("usemtl", "").strip_edges();
			}

			if (l.begins_with("g ")) {
				current_group = l.substr(2, l.length()).strip_edges();
			}

		} else if (l.begins_with("mtllib ")) { //parse material

			current_material_library = l.replace("mtllib", "").strip_edges();
			if (!material_map.has(current_material_library)) {
				Map<String, Ref<Material3D>> lib;
				String lib_path = current_material_library;
				if (lib_path.is_rel_path()) {
					lib_path = p_path.get_base_dir().plus_file(current_material_library);
				}
				Error err = _parse_material_library(lib_path, lib, r_missing_deps);
				if (err == OK) {
					material_map[current_material_library] = lib;
				}
			}
		}
	}

	if (p_single_mesh) {
		r_meshes.push_back(mesh);
	}

	return OK;
}

Node *EditorOBJImporter::import_scene(const String &p_path, uint32_t p_flags, int p_bake_fps, uint32_t p_compress_flags, List<String> *r_missing_deps, Error *r_err) {
	List<Ref<Mesh>> meshes;

	Error err = _parse_obj(p_path, meshes, false, p_flags & IMPORT_GENERATE_TANGENT_ARRAYS, p_compress_flags, Vector3(1, 1, 1), Vector3(0, 0, 0), r_missing_deps);

	if (err != OK) {
		if (r_err) {
			*r_err = err;
		}
		return nullptr;
	}

	Spatial *scene = memnew(Spatial);

	for (List<Ref<Mesh>>::Element *E = meshes.front(); E; E = E->next()) {
		MeshInstance *mi = memnew(MeshInstance);
		mi->set_mesh(E->get());
		mi->set_name(E->get()->get_name());
		scene->add_child(mi);
		mi->set_owner(scene);
	}

	if (r_err) {
		*r_err = OK;
	}

	return scene;
}
Ref<Animation> EditorOBJImporter::import_animation(const String &p_path, uint32_t p_flags, int p_bake_fps) {
	return Ref<Animation>();
}

void EditorOBJImporter::get_extensions(List<String> *r_extensions) const {
	r_extensions->push_back("obj");
}

EditorOBJImporter::EditorOBJImporter() {
}
////////////////////////////////////////////////////

String ResourceImporterOBJ::get_importer_name() const {
	return "wavefront_obj";
}
String ResourceImporterOBJ::get_visible_name() const {
	return "OBJ As Mesh";
}
void ResourceImporterOBJ::get_recognized_extensions(List<String> *p_extensions) const {
	p_extensions->push_back("obj");
}
String ResourceImporterOBJ::get_save_extension() const {
	return "mesh";
}
String ResourceImporterOBJ::get_resource_type() const {
	return "Mesh";
}

int ResourceImporterOBJ::get_preset_count() const {
	return 0;
}
String ResourceImporterOBJ::get_preset_name(int p_idx) const {
	return "";
}

void ResourceImporterOBJ::get_import_options(List<ImportOption> *r_options, int p_preset) const {
	r_options->push_back(ImportOption(PropertyInfo(Variant::VECTOR3, "scale_mesh"), Vector3(1, 1, 1)));
	r_options->push_back(ImportOption(PropertyInfo(Variant::VECTOR3, "offset_mesh"), Vector3(0, 0, 0)));
	r_options->push_back(ImportOption(PropertyInfo(Variant::INT, "optimize_mesh_flags", PROPERTY_HINT_FLAGS, "Vertex,Normal,Tangent,Color,TexUV,TexUV2,Bones,Weights,Index"), VS::ARRAY_COMPRESS_DEFAULT >> VS::ARRAY_COMPRESS_BASE));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "generate_tangents"), true));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "vertex_cache_optimization"), true));
	r_options->push_back(ImportOption(PropertyInfo(Variant::BOOL, "octahedral_compression"), true));
}
bool ResourceImporterOBJ::get_option_visibility(const String &p_option, const Map<StringName, Variant> &p_options) const {
	return true;
}

Error ResourceImporterOBJ::import(const String &p_source_file, const String &p_save_path, const Map<StringName, Variant> &p_options, List<String> *r_platform_variants, List<String> *r_gen_files, Variant *r_metadata) {
	List<Ref<Mesh>> meshes;

	uint32_t compress_flags = int(p_options["optimize_mesh_flags"]) << VS::ARRAY_COMPRESS_BASE;
	if (bool(p_options["octahedral_compression"])) {
		compress_flags |= VS::ARRAY_FLAG_USE_OCTAHEDRAL_COMPRESSION;
	}
	if (bool(p_options["vertex_cache_optimization"])) {
		compress_flags |= VS::ARRAY_FLAG_USE_VERTEX_CACHE_OPTIMIZATION;
	}
	Error err = _parse_obj(p_source_file, meshes, true, p_options["generate_tangents"], compress_flags, p_options["scale_mesh"], p_options["offset_mesh"], nullptr);

	ERR_FAIL_COND_V(err != OK, err);
	ERR_FAIL_COND_V(meshes.size() != 1, ERR_BUG);

	String save_path = p_save_path + ".mesh";

	err = ResourceSaver::save(save_path, meshes.front()->get());

	ERR_FAIL_COND_V_MSG(err != OK, err, "Cannot save Mesh to file '" + save_path + "'.");

	r_gen_files->push_back(save_path);

	return OK;
}

ResourceImporterOBJ::ResourceImporterOBJ() {
}
