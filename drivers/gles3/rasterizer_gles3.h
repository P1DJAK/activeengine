#ifndef RASTERIZER_GLES3_H
#define RASTERIZER_GLES3_H

#include "rasterizer_canvas_gles3.h"
#include "rasterizer_scene_gles3.h"
#include "rasterizer_storage_gles3.h"
#include "servers/visual/rasterizer.h"

class RasterizerGLES3 : public Rasterizer {
	static Rasterizer *_create_current();

	RasterizerStorageGLES3 *storage;
	RasterizerCanvasGLES3 *canvas;
	RasterizerSceneGLES3 *scene;

	double time_total;
	float time_scale;

public:
	virtual RasterizerStorage *get_storage();
	virtual RasterizerCanvas *get_canvas();
	virtual RasterizerScene *get_scene();

	virtual void set_boot_image(const Ref<Image> &p_image, const Color &p_color, bool p_scale, bool p_use_filter = true);
	virtual void set_shader_time_scale(float p_scale);

	virtual void initialize();
	virtual void begin_frame(double frame_step);
	virtual void set_current_render_target(RID p_render_target);
	virtual void restore_render_target(bool p_3d_was_drawn);
	virtual void clear_render_target(const Color &p_color);
	virtual void blit_render_target_to_screen(RID p_render_target, const Rect2 &p_screen_rect, int p_screen = 0);
	virtual void output_lens_distorted_to_screen(RID p_render_target, const Rect2 &p_screen_rect, float p_k1, float p_k2, const Vector2 &p_eye_center, float p_oversample);
	virtual void end_frame(bool p_swap_buffers);
	virtual void finalize();

	static Error is_viable();
	static void make_current();
	static void register_config();

	virtual bool is_low_end() const { return false; }

	static bool gl_check_errors();

	RasterizerGLES3();
	~RasterizerGLES3();
};

#endif // RASTERIZER_GLES3_H
