#include "quick_hull.h"

#include "core/map.h"

uint32_t QuickHull::debug_stop_after = 0xFFFFFFFF;
bool QuickHull::_flag_warnings = true;

Error QuickHull::build(const Vector<Vector3> &p_points, Geometry::MeshData &r_mesh, real_t p_over_tolerance_epsilon) {
	/* CREATE AABB VOLUME */

	AABB aabb;
	aabb.create_from_points(p_points);

	if (aabb.size == Vector3()) {
		return ERR_CANT_CREATE;
	}

	Vector<bool> valid_points;
	valid_points.resize(p_points.size());
	Set<Vector3> valid_cache;

	for (int i = 0; i < p_points.size(); i++) {
		Vector3 sp = p_points[i].snapped(Vector3(0.0001, 0.0001, 0.0001));
		if (valid_cache.has(sp)) {
			valid_points.write[i] = false;
		} else {
			valid_points.write[i] = true;
			valid_cache.insert(sp);
		}
	}

	/* CREATE INITIAL SIMPLEX */

	int longest_axis = aabb.get_longest_axis_index();

	//first two vertices are the most distant
	int simplex[4] = { 0 };

	{
		real_t max = 0, min = 0;

		for (int i = 0; i < p_points.size(); i++) {
			if (!valid_points[i]) {
				continue;
			}
			real_t d = p_points[i][longest_axis];
			if (i == 0 || d < min) {
				simplex[0] = i;
				min = d;
			}

			if (i == 0 || d > max) {
				simplex[1] = i;
				max = d;
			}
		}
	}

	//third vertex is one most further away from the line

	{
		real_t maxd = 0;
		Vector3 rel12 = p_points[simplex[0]] - p_points[simplex[1]];

		for (int i = 0; i < p_points.size(); i++) {
			if (!valid_points[i]) {
				continue;
			}

			Vector3 n = rel12.cross(p_points[simplex[0]] - p_points[i]).cross(rel12).normalized();
			real_t d = Math::abs(n.dot(p_points[simplex[0]]) - n.dot(p_points[i]));

			if (i == 0 || d > maxd) {
				maxd = d;
				simplex[2] = i;
			}
		}
	}

	//fourth vertex is the one most further away from the plane

	{
		real_t maxd = 0;
		Plane p(p_points[simplex[0]], p_points[simplex[1]], p_points[simplex[2]]);

		for (int i = 0; i < p_points.size(); i++) {
			if (!valid_points[i]) {
				continue;
			}

			real_t d = Math::abs(p.distance_to(p_points[i]));

			if (i == 0 || d > maxd) {
				maxd = d;
				simplex[3] = i;
			}
		}
	}

	//compute center of simplex, this is a point always warranted to be inside
	Vector3 center;

	for (int i = 0; i < 4; i++) {
		center += p_points[simplex[i]];
	}

	center /= 4.0;

	//add faces

	List<Face> faces;

	for (int i = 0; i < 4; i++) {
		static const int face_order[4][3] = {
			{ 0, 1, 2 },
			{ 0, 1, 3 },
			{ 0, 2, 3 },
			{ 1, 2, 3 }
		};

		Face f;
		for (int j = 0; j < 3; j++) {
			f.vertices[j] = simplex[face_order[i][j]];
		}

		Plane p(p_points[f.vertices[0]], p_points[f.vertices[1]], p_points[f.vertices[2]]);

		if (p.is_point_over(center)) {
			//flip face to clockwise if facing inwards
			SWAP(f.vertices[0], f.vertices[1]);
			p = -p;
		}

		f.plane = p;

		faces.push_back(f);
	}

	real_t over_tolerance = p_over_tolerance_epsilon * (aabb.size.x + aabb.size.y + aabb.size.z);

	/* COMPUTE AVAILABLE VERTICES */

	for (int i = 0; i < p_points.size(); i++) {
		if (i == simplex[0]) {
			continue;
		}
		if (i == simplex[1]) {
			continue;
		}
		if (i == simplex[2]) {
			continue;
		}
		if (i == simplex[3]) {
			continue;
		}
		if (!valid_points[i]) {
			continue;
		}

		for (List<Face>::Element *E = faces.front(); E; E = E->next()) {
			if (E->get().plane.distance_to(p_points[i]) > over_tolerance) {
				E->get().points_over.push_back(i);
				break;
			}
		}
	}

	faces.sort(); // sort them, so the ones with points are in the back

	/* BUILD HULL */

	//poop face (while still remain)
	//find further away point
	//find lit faces
	//determine horizon edges
	//build new faces with horizon edges, them assign points side from all lit faces
	//remove lit faces

	uint32_t debug_stop = debug_stop_after;

	while (debug_stop > 0 && faces.back()->get().points_over.size()) {
		debug_stop--;
		Face &f = faces.back()->get();

		//find vertex most outside
		int next = -1;
		real_t next_d = 0;

		for (int i = 0; i < f.points_over.size(); i++) {
			real_t d = f.plane.distance_to(p_points[f.points_over[i]]);

			if (d > next_d) {
				next_d = d;
				next = i;
			}
		}

		ERR_FAIL_COND_V(next == -1, ERR_BUG);

		Vector3 v = p_points[f.points_over[next]];

		//find lit faces and lit edges
		List<List<Face>::Element *> lit_faces; //lit face is a death sentence

		Map<Edge, FaceConnect> lit_edges; //create this on the flight, should not be that bad for performance and simplifies code a lot

		for (List<Face>::Element *E = faces.front(); E; E = E->next()) {
			if (E->get().plane.distance_to(v) > 0) {
				lit_faces.push_back(E);

				for (int i = 0; i < 3; i++) {
					uint32_t a = E->get().vertices[i];
					uint32_t b = E->get().vertices[(i + 1) % 3];
					Edge e(a, b);

					Map<Edge, FaceConnect>::Element *F = lit_edges.find(e);
					if (!F) {
						F = lit_edges.insert(e, FaceConnect());
					}
					if (e.vertices[0] == a) {
						//left
						F->get().left = E;
					} else {
						F->get().right = E;
					}
				}
			}
		}

		//create new faces from horizon edges
		List<List<Face>::Element *> new_faces; //new faces

		for (Map<Edge, FaceConnect>::Element *E = lit_edges.front(); E; E = E->next()) {
			FaceConnect &fc = E->get();
			if (fc.left && fc.right) {
				continue; //edge is uninteresting, not on horizont
			}

			//create new face!

			Face face;
			face.vertices[0] = f.points_over[next];
			face.vertices[1] = E->key().vertices[0];
			face.vertices[2] = E->key().vertices[1];

			Plane p(p_points[face.vertices[0]], p_points[face.vertices[1]], p_points[face.vertices[2]]);

			if (p.is_point_over(center)) {
				//flip face to clockwise if facing inwards
				SWAP(face.vertices[0], face.vertices[1]);
				p = -p;
			}

			face.plane = p;
			new_faces.push_back(faces.push_back(face));
		}

		//distribute points into new faces

		for (List<List<Face>::Element *>::Element *F = lit_faces.front(); F; F = F->next()) {
			Face &lf = F->get()->get();

			for (int i = 0; i < lf.points_over.size(); i++) {
				if (lf.points_over[i] == f.points_over[next]) { //do not add current one
					continue;
				}

				Vector3 p = p_points[lf.points_over[i]];
				for (List<List<Face>::Element *>::Element *E = new_faces.front(); E; E = E->next()) {
					Face &f2 = E->get()->get();
					if (f2.plane.distance_to(p) > over_tolerance) {
						f2.points_over.push_back(lf.points_over[i]);
						break;
					}
				}
			}
		}

		//erase lit faces

		while (lit_faces.size()) {
			faces.erase(lit_faces.front()->get());
			lit_faces.pop_front();
		}

		//put faces that contain no points on the front

		for (List<List<Face>::Element *>::Element *E = new_faces.front(); E; E = E->next()) {
			Face &f2 = E->get()->get();
			if (f2.points_over.size() == 0) {
				faces.move_to_front(E->get());
			}
		}

		//whew, done with iteration, go next
	}

	/* CREATE MESHDATA */

	//make a map of edges again
	Map<Edge, RetFaceConnect> ret_edges;
	List<Geometry::MeshData::Face> ret_faces;

	for (List<Face>::Element *E = faces.front(); E; E = E->next()) {
		Geometry::MeshData::Face f;
		f.plane = E->get().plane;

		for (int i = 0; i < 3; i++) {
			f.indices.push_back(E->get().vertices[i]);
		}

		List<Geometry::MeshData::Face>::Element *F = ret_faces.push_back(f);

		for (int i = 0; i < 3; i++) {
			uint32_t a = E->get().vertices[i];
			uint32_t b = E->get().vertices[(i + 1) % 3];
			Edge e(a, b);

			Map<Edge, RetFaceConnect>::Element *G = ret_edges.find(e);
			if (!G) {
				G = ret_edges.insert(e, RetFaceConnect());
			}
			if (e.vertices[0] == a) {
				//left
				G->get().left = F;
			} else {
				G->get().right = F;
			}
		}
	}

	//fill faces

	bool warning_f = false;
	bool warning_o_equal_e = false;
	bool warning_o = false;
	bool warning_not_f2 = false;

	for (List<Geometry::MeshData::Face>::Element *E = ret_faces.front(); E; E = E->next()) {
		Geometry::MeshData::Face &f = E->get();

		for (int i = 0; i < f.indices.size(); i++) {
			int a = E->get().indices[i];
			int b = E->get().indices[(i + 1) % f.indices.size()];
			Edge e(a, b);

			Map<Edge, RetFaceConnect>::Element *F = ret_edges.find(e);

			if (unlikely(!F)) {
				warning_f = true;
				continue;
			}

			List<Geometry::MeshData::Face>::Element *O = F->get().left == E ? F->get().right : F->get().left;
			if (unlikely(O == E)) {
				warning_o_equal_e = true;
				continue;
			}
			if (unlikely(!O)) {
				warning_o = true;
				continue;
			}

			if (O->get().plane.is_equal_approx(f.plane)) {
				//merge and delete edge and contiguous face, while repointing edges (uuugh!)
				int ois = O->get().indices.size();

				for (int j = 0; j < ois; j++) {
					//search a
					if (O->get().indices[j] == a) {
						//append the rest
						for (int k = 0; k < ois; k++) {
							int idx = O->get().indices[(k + j) % ois];
							int idxn = O->get().indices[(k + j + 1) % ois];
							if (idx == b && idxn == a) { //already have b!
								break;
							}
							if (idx != a) {
								f.indices.insert(i + 1, idx);
								i++;
							}
							Edge e2(idx, idxn);

							Map<Edge, RetFaceConnect>::Element *F2 = ret_edges.find(e2);

							if (unlikely(!F2)) {
								warning_not_f2 = true;
								continue;
							}

							//change faceconnect, point to this face instead
							if (F2->get().left == O) {
								F2->get().left = E;
							} else if (F2->get().right == O) {
								F2->get().right = E;
							}
						}

						break;
					}
				}

				// remove all edge connections to this face
				for (Map<Edge, RetFaceConnect>::Element *G = ret_edges.front(); G; G = G->next()) {
					if (G->get().left == O) {
						G->get().left = nullptr;
					}

					if (G->get().right == O) {
						G->get().right = nullptr;
					}
				}

				ret_edges.erase(F); //remove the edge
				ret_faces.erase(O); //remove the face
			}
		}
	}

	if (_flag_warnings) {
		if (warning_f) {
			WARN_PRINT("QuickHull : !F");
		}
		if (warning_o_equal_e) {
			WARN_PRINT("QuickHull : O == E");
		}
		if (warning_o) {
			WARN_PRINT("QuickHull : O == nullptr");
		}
		if (warning_not_f2) {
			WARN_PRINT("QuickHull : !F2");
		}
	}

	//fill mesh
	r_mesh.faces.clear();
	r_mesh.faces.resize(ret_faces.size());

	int idx = 0;
	for (List<Geometry::MeshData::Face>::Element *E = ret_faces.front(); E; E = E->next()) {
		r_mesh.faces.write[idx++] = E->get();
	}
	r_mesh.edges.resize(ret_edges.size());
	idx = 0;
	for (Map<Edge, RetFaceConnect>::Element *E = ret_edges.front(); E; E = E->next()) {
		Geometry::MeshData::Edge e;
		e.a = E->key().vertices[0];
		e.b = E->key().vertices[1];
		r_mesh.edges.write[idx++] = e;
	}

	// we are only interested in outputting the points that are used
	Vector<int> out_indices;

	for (int n = 0; n < r_mesh.faces.size(); n++) {
		Geometry::MeshData::Face face = r_mesh.faces[n];
		for (int i = 0; i < face.indices.size(); i++) {
			face.indices.set(i, find_or_create_output_index(face.indices[i], out_indices));
		}
		r_mesh.faces.set(n, face);
	}
	for (int n = 0; n < r_mesh.edges.size(); n++) {
		Geometry::MeshData::Edge e = r_mesh.edges[n];
		e.a = find_or_create_output_index(e.a, out_indices);
		e.b = find_or_create_output_index(e.b, out_indices);
		r_mesh.edges.set(n, e);
	}

	// rejig the final vertices
	r_mesh.vertices.resize(out_indices.size());
	for (int n = 0; n < out_indices.size(); n++) {
		r_mesh.vertices.set(n, p_points[out_indices[n]]);
	}

	return OK;
}
