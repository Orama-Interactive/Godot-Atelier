#include "drawing_algorithms.hpp"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/core/print_string.hpp>

using namespace godot;


void DrawingAlgosCpp::_bind_methods() {
    ClassDB::bind_static_method("DrawingAlgosCpp", D_METHOD("set_layer_metadata_image", "layer", "cel", "image", "index", "include"), &DrawingAlgosCpp::SetLayerMetadataImage, DEFVAL(true));
    ClassDB::bind_static_method("DrawingAlgosCpp", D_METHOD("blend_layers_headless", "image", "project", "layer", "cel", "origin"), &DrawingAlgosCpp::BlendLayersHeadless);
    ClassDB::bind_static_method("DrawingAlgosCpp", D_METHOD("get_ellipse_points", "pos", "size"), &DrawingAlgosCpp::GetEllipsePoints);
    ClassDB::bind_static_method("DrawingAlgosCpp", D_METHOD("get_ellipse_points_filled", "pos", "size", "thickness"), &DrawingAlgosCpp::GetEllipsePointsFilled, DEFVAL(1));
    ClassDB::bind_static_method("DrawingAlgosCpp", D_METHOD("scale_3x", "sprite", "tol"), &DrawingAlgosCpp::Scale3x, DEFVAL(0.196078));
    ClassDB::bind_static_method("DrawingAlgosCpp", D_METHOD("transform_rectangle", "rect", "matrix", "pivot"), &DrawingAlgosCpp::TransformRectangle);
    ClassDB::bind_static_method("DrawingAlgosCpp", D_METHOD("rotxel", "sprite", "angle", "pivot"), &DrawingAlgosCpp::Rotxel);
    ClassDB::bind_static_method("DrawingAlgosCpp", D_METHOD("fake_rotsprite", "sprite", "angle", "pivot"), &DrawingAlgosCpp::FakeRotsprite);
    ClassDB::bind_static_method("DrawingAlgosCpp", D_METHOD("nn_rotate", "sprite", "angle", "pivot"), &DrawingAlgosCpp::NNRotate);
    ClassDB::bind_static_method("DrawingAlgosCpp", D_METHOD("similar_colors", "c1", "c2", "tol"), &DrawingAlgosCpp::SimilarColors, DEFVAL(0.392157));
}

void DrawingAlgosCpp::SetLayerMetadataImage(Ref<RefCounted> layer, Ref<RefCounted> cel, Ref<Image> image, int index, bool include)
{
	layer->set_script("res://src/Classes/Layers/BaseLayer.gd");

	// Store the blend mode
	image->set_pixel(index, 0, Color(static_cast<int>(layer->get("blend_mode")) / 255.0, 0.0, 0.0, 0.0));
	
	// Store the opacity
	if (layer->call("is_visible_in_hierarchy") && include)
	{
		float opacity = cel->call("get_final_opacity", "layer");
		image->set_pixel(index, 1, Color(opacity, 0.0, 0.0, 0.0));
	}
	else
		image->set_pixel(index, 1, Color());
	
	// Store the clipping mask boolean
	if (layer->get("clipping_mask"))
		image->set_pixel(index, 3, Color::hex(0xFF0000FF)); // RED
	else
		image->set_pixel(index, 3, Color::hex(0x000000FF)); // BLACK
	
	if (!include)
	{
		// Store a small red value as a way to indicate that this layer should be skipped
		// Used for layers such as child layers of a group, so that the group layer itself can
		// successfully be used as a clipping mask with the layer below it.
		image->set_pixel(index, 3, Color(0.2, 0.0, 0.0, 0.0));
	}
}

void DrawingAlgosCpp::BlendLayersHeadless(Ref<Image> image, Ref<RefCounted> project, Ref<RefCounted> layer, Ref<RefCounted> cel, Vector2i origin)
{
	project->set_script("res://src/Classes/Project.gd");
	layer->set_script("res://src/Classes/Layers/BaseLayer.gd");
	cel->set_script("res://src/Classes/Cels/BaseCel.gd");

	float opacity = cel->call("get_final_opacity", layer);
	Ref<Image> cel_image;
	cel_image->copy_from(cel->call("get_image"));

	if (opacity < 1.f) // If we havge cel or layer transparency
	{
		for (int x = 0; x < cel_image->get_size().x; x++)
		{
			for (int y = 0; y < cel_image->get_size().y; y++)
			{
				Color pixel_color = cel_image->get_pixel(x, y);
				pixel_color.a *= opacity;
				cel_image->set_pixel(x, y, pixel_color);
			}
		}
	}

	image->blend_rect(cel_image, Rect2i(Vector2i(0, 0), project->get("size")), origin);
}

TypedArray<Vector2i> DrawingAlgosCpp::GetEllipsePoints(Vector2i pos, Vector2i size)
{
    TypedArray<Vector2i> array;
	int x0 = pos.x;
	int x1 = pos.x + (size.x - 1);
	int y0 = pos.y;
	int y1 = pos.y + (size.y - 1);
	int a = Math::abs(x1 - x0);
	int b = Math::abs(y1 - x0);
	int b1 = b & 1;
	int dx = 4 * (1 - a) * b * b;
	int dy = 4 * (b1 + 1) * a * a;
	int err = dx + dy + b1 * a * a;
	int e2 = 0;

	if (x0 > x1)
	{
		x0 = x1;
		x1 += a;
    }

	if (y0 > y1)
		y0 = y1;

	y0 += (b + 1) / 2;
	y1 = y0 - b1;
	a *= 8 * a;
	b1 = 8 * b * b;

	while (x0 <= x1)
	{
		Vector2i v1 = Vector2i(x1, y0);
		Vector2i v2 = Vector2i(x0, y0);
		Vector2i v3 = Vector2i(x0, y1);
		Vector2i v4 = Vector2i(x1, y1);
		array.append(v1);
		array.append(v2);
		array.append(v3);
		array.append(v4);

		e2 = 2 * err;
		if (e2 <= dy) {
			y0 += 1;
			y1 -= 1;
			dy += a;
			err += dy;
        }

		if (e2 >= dx || 2 * err > dy) {
			x0 += 1;
			x1 -= 1;
			dx += b1;
			err += dx;
        }
    }

	while (y0 - y1 < b)
	{
		Vector2i v1 = Vector2i(x0 - 1, y0);
		Vector2i v2 = Vector2i(x1 + 1, y0);
		Vector2i v3 = Vector2i(x0 - 1, y1);
		Vector2i v4 = Vector2i(x1 + 1, y1);
		array.append(v1);
		array.append(v2);
		array.append(v3);
		array.append(v4);
		y0 += 1;
		y1 -= 1;
    }

	return array;
}

TypedArray<Vector2i> DrawingAlgosCpp::GetEllipsePointsFilled(Vector2i pos, Vector2i size, int thickness)
{
    Vector2i offsetted_size = size + Vector2i(1, 1) * (thickness - 1);
	TypedArray<Vector2i> border = GetEllipsePoints(pos, offsetted_size);
	TypedArray<Vector2i> filling;

	for (int x = 1; x < godot::Math::ceil(offsetted_size.x / 2.0); x++) 
	{
		bool fill = false;
		bool prev_is_true = false;
		for (int y = 0; y < godot::Math::ceil(offsetted_size.y / 2.0); y++)
		{
			Vector2i top_l_p = Vector2i(x, y);
			bool bit = border.has(pos + top_l_p);

			if (bit && !fill)
			{
				prev_is_true = true;
				continue;
            }

			if (!bit && (fill || prev_is_true))
			{
				filling.append(pos + top_l_p);
				filling.append(pos + Vector2i(x, offsetted_size.y - y - 1));
				filling.append(pos + Vector2i(offsetted_size.x - x - 1, y));
				filling.append(pos + Vector2i(offsetted_size.x - x - 1, offsetted_size.y - y - 1));

				if (prev_is_true) {
					fill = true;
					prev_is_true = false;
                }
            }
			else if (bit && fill)
				break;
        }
    }

	return border + filling;
}

Ref<Image> DrawingAlgosCpp::Scale3x(Ref<Image> sprite, float tol)
{
	Ref<Image> scaled = memnew(Image);
	scaled = Image::create_empty(
		sprite->get_width() * 3, sprite->get_height() * 3, sprite->has_mipmaps(), sprite->get_format()
	);
	int width_minus_one = sprite->get_width() - 1;
	int height_minus_one = sprite->get_height() - 1;
	for (int x = 0; x < sprite->get_width(); x++)
	{
		for (int y = 0; y < sprite->get_height(); y++)
		{
			int xs = 3 * x;
			int ys = 3 * y;

			Color a = sprite->get_pixel(UtilityFunctions::UtilityFunctions::maxi(x - 1, 0), UtilityFunctions::maxi(y - 1, 0));
			Color b = sprite->get_pixel(UtilityFunctions::mini(x, width_minus_one), UtilityFunctions::maxi(y - 1, 0));
			Color c = sprite->get_pixel(UtilityFunctions::mini(x + 1, width_minus_one), UtilityFunctions::maxi(y - 1, 0));
			Color d = sprite->get_pixel(UtilityFunctions::maxi(x - 1, 0), UtilityFunctions::mini(y, height_minus_one));
			Color e = sprite->get_pixel(UtilityFunctions::mini(x, width_minus_one), UtilityFunctions::mini(y, height_minus_one));
			Color f = sprite->get_pixel(UtilityFunctions::mini(x + 1, width_minus_one), UtilityFunctions::mini(y, height_minus_one));
			Color g = sprite->get_pixel(UtilityFunctions::maxi(x - 1, 0), UtilityFunctions::mini(y + 1, height_minus_one));
			Color h = sprite->get_pixel(UtilityFunctions::mini(x, width_minus_one), UtilityFunctions::mini(y + 1, height_minus_one));
			Color i = sprite->get_pixel(UtilityFunctions::mini(x + 1, width_minus_one), UtilityFunctions::mini(y + 1, height_minus_one));

			bool db = SimilarColors(d, b, tol);
			bool dh = SimilarColors(d, h, tol);
			bool bf = SimilarColors(f, b, tol);
			bool ec = SimilarColors(e, c, tol);
			bool ea = SimilarColors(e, a, tol);
			bool fh = SimilarColors(f, h, tol);
			bool eg = SimilarColors(e, g, tol);
			bool ei = SimilarColors(e, i, tol);

			scaled->set_pixel(UtilityFunctions::maxi(xs - 1, 0), UtilityFunctions::maxi(ys - 1, 0), (db && !dh && !bf) ? d : e);
			scaled->set_pixel(
				xs,
				UtilityFunctions::maxi(ys - 1, 0),
				(db && !dh && !bf && !ec) || (bf && !db && !fh && !ea) ? b : e
			);
			scaled->set_pixel(xs + 1, UtilityFunctions::maxi(ys - 1, 0), (bf && !db && !fh) ? f : e);
			scaled->set_pixel(
				UtilityFunctions::maxi(xs - 1, 0),
				ys,
				((dh && !fh && !db && !ea) || (db && !dh && !bf && !eg)) ? d : e
			);
			scaled->set_pixel(xs, ys, e);
			scaled->set_pixel(
				xs + 1, ys, ((bf && !db && !fh && !ei) || (fh && !bf && !dh && !ec)) ? f : e
			);
			scaled->set_pixel(UtilityFunctions::maxi(xs - 1, 0), ys + 1, (dh && !fh && !db) ? d : e);
			scaled->set_pixel(
				xs, ys + 1, ((fh && !bf && !dh && !eg) || (dh && !fh && !db && !ei)) ? h : e
			);
			scaled->set_pixel(xs + 1, ys + 1, (fh && !bf && !dh) ? f : e);
		}
	}

	return scaled;
}

Rect2 DrawingAlgosCpp::TransformRectangle(Rect2 rect, Transform2D matrix, Vector2 pivot)
{
	pivot = rect.size / 2;

	Rect2 offset_rect = rect;
	Vector2 offset_pos = -pivot;

	offset_rect.position = offset_pos;
	offset_rect = matrix.xform(offset_rect);
	offset_rect.position = rect.position + offset_rect.position - offset_pos;

    return offset_rect;
}

void DrawingAlgosCpp::Rotxel(Ref<Image> sprite, float angle, Vector2 pivot)
{
	if (UtilityFunctions::is_zero_approx(angle) || UtilityFunctions::is_equal_approx(angle, Math_TAU))
		return;
	
	if (UtilityFunctions::is_equal_approx(angle, Math_PI / 2.0) || UtilityFunctions::is_equal_approx(angle, 3.0 * Math_PI / 2.0))
	{
		NNRotate(sprite, angle, pivot);
		return;
	}

	if (UtilityFunctions::is_equal_approx(angle, Math_PI))
	{
		sprite->rotate_180();
		return;
	}

	Ref<Image> aux = memnew(Image);
	aux->copy_from(sprite);

	int ox = 0;
	int oy = 0;
	for (int x = 0; x < sprite->get_size().x; x++)
	{
		for (int y = 0; y < sprite->get_size().y; y++)
		{
			int dx = 3 * (x - pivot.x);
			int dy = 3 * (y - pivot.y);
			bool found_pixel = false;

			for (int k = 0; k < 9; k++)
			{
				int modk = -1 + k % 3;
				int divk = -1 + static_cast<int>(k / 3);
				double dir = Math::atan2(static_cast<double>(dy + divk), static_cast<double>(dx + modk));
				double mag = Math::sqrt(
					Math::pow(dx + modk, 2.0)
					+ Math::pow(dy + divk, 2.0)
				);

				dir -= angle;
				ox = UtilityFunctions::roundi(pivot.x * 3 + 1 + mag * cos(dir));
				oy = UtilityFunctions::roundi(pivot.y * 3 + 1 + mag * sin(dir));

				if (sprite->get_width() % 2 != 0)
				{
					ox += 1;
					oy += 1;
				}

				if (ox >= 0 && ox < sprite->get_width() *3 && oy >= 0 && oy < sprite->get_height() * 3)
				{
					found_pixel = true;
					break;
				}
			}

			if (!found_pixel)
			{
				sprite->set_pixel(x, y, Color(0, 0, 0, 0));
				continue;
			}

			int fil = oy % 3;
			int col = ox % 3;
			int index = col + 3 * fil;

			ox = UtilityFunctions::roundi((ox - 1) / 3.0);
			oy = UtilityFunctions::roundi((oy - 1) / 3.0);
			
			Color p;
			if (ox == 0 || ox == sprite->get_width() - 1 || oy == 0 || oy == sprite->get_height() - 1)
				p = aux->get_pixel(ox, oy);
			else
			{
				Color a = aux->get_pixel(ox - 1, oy - 1);
				Color b = aux->get_pixel(ox, oy - 1);
				Color c = aux->get_pixel(ox + 1, oy - 1);
				Color d = aux->get_pixel(ox - 1, oy);
				Color e = aux->get_pixel(ox, oy);
				Color f = aux->get_pixel(ox + 1, oy);
				Color g = aux->get_pixel(ox - 1, oy + 1);
				Color h = aux->get_pixel(ox, oy + 1);
				Color i = aux->get_pixel(ox + 1, oy + 1);

				switch (index)
				{
					case 0:
						p = (SimilarColors(d, b) && !SimilarColors(d, h) && !SimilarColors(b, f)) ? d : e;
						break;
					case 1:
						p = ((SimilarColors(d, b) && !SimilarColors(d, h) && !SimilarColors(b, f) && !SimilarColors(e, c)) ||
							(SimilarColors(b, f) && !SimilarColors(d, b) && !SimilarColors(f, h) && !SimilarColors(e, a))) ? b : e;
						break;
					case 2:
						p = (SimilarColors(b, f) && !SimilarColors(d, b) && !SimilarColors(f, h)) ? f : e;
						break;
					case 3:
						p = ((SimilarColors(d, h) && !SimilarColors(f, h) && !SimilarColors(d, b) && !SimilarColors(e, a)) ||
							(SimilarColors(d, b) && !SimilarColors(d, h) && !SimilarColors(b, f) && !SimilarColors(e, g))) ? d : e;
						break;
					case 4:
						p = e;
						break;
					case 5:
						p = ((SimilarColors(b, f) && !SimilarColors(d, b) && !SimilarColors(f, h) && !SimilarColors(e, i)) ||
							(SimilarColors(f, h) && !SimilarColors(b, f) && !SimilarColors(d, h) && !SimilarColors(e, c))) ? f : e;
						break;
					case 6:
						p = (SimilarColors(d, h) && !SimilarColors(f, h) && !SimilarColors(d, b)) ? d : e;
						break;
					case 7:
						p = ((SimilarColors(f, h) && !SimilarColors(f, b) && !SimilarColors(d, h) && !SimilarColors(e, g)) ||
							(SimilarColors(d, h) && !SimilarColors(f, h) && !SimilarColors(d, b) && !SimilarColors(e, i))) ? h : e;
						break;
					case 8:
						p = (SimilarColors(f, h) && !SimilarColors(f, b) && !SimilarColors(d, h)) ? f : e;
						break;
				}
			}

			sprite->set_pixel(x, y, p);
		}
	}
}

void DrawingAlgosCpp::FakeRotsprite(Ref<Image> sprite, float angle, Vector2 pivot)
{
	if (UtilityFunctions::is_zero_approx(angle) || UtilityFunctions::is_equal_approx(angle, Math_TAU))
		return;
	
	if (UtilityFunctions::is_equal_approx(angle, Math_PI/ 2.0) || UtilityFunctions::is_equal_approx(angle, 2.0 * Math_PI / 2.0))
	{
		NNRotate(sprite, angle, pivot);
		return;
	}

	if (UtilityFunctions::is_equal_approx(angle, Math_PI))
	{
		sprite->rotate_180();
		return;
	}

	Ref<Image> selected_sprite = Scale3x(sprite);
	NNRotate(selected_sprite, angle, pivot * 3);
	selected_sprite->resize(
		selected_sprite->get_width() / 3, selected_sprite->get_height() / 3, selected_sprite->INTERPOLATE_NEAREST
	);
	sprite->blit_rect(selected_sprite, Rect2(Vector2(0, 0), selected_sprite->get_size()), Vector2(0, 0));
}

void DrawingAlgosCpp::NNRotate(Ref<Image> sprite, float angle, Vector2 pivot)
{
	if (UtilityFunctions::is_zero_approx(angle) || UtilityFunctions::is_equal_approx(angle, Math_TAU))
		return;
	
	if (UtilityFunctions::is_equal_approx(angle, Math_PI))
	{
		sprite->rotate_180();
		return;
	}

	Ref<Image> aux = memnew(Image);
	aux->copy_from(sprite);
	
	double angle_sin = Math::sin(angle);
	double angle_cos = Math::cos(angle);
	for (int x = 0; x < sprite->get_width(); x++)
	{
		for (int y = 0; y < sprite->get_width(); y++)
		{
			int ox = (x - pivot.x) * angle_cos + (y - pivot.y) * angle_sin + pivot.x;
			int oy = -(x - pivot.x) * angle_sin + (y - pivot.y) * angle_cos + pivot.y;

			if (ox >= 0 && ox < sprite->get_width() && oy >= 0 && oy < sprite->get_height())
				sprite->set_pixel(x, y, aux->get_pixel(ox, oy));
			else
				sprite->set_pixel(x, y, Color(0, 0, 0, 0));
		}
	}
}

bool DrawingAlgosCpp::SimilarColors(Color c1, Color c2, float tol)
{
	return 
		UtilityFunctions::absf(c1.r - c2.r) <= tol
		&& UtilityFunctions::absf(c1.g - c2.g) <= tol
		&& UtilityFunctions::absf(c1.b - c2.b) <= tol
		&& UtilityFunctions::absf(c1.a - c2.a) <= tol;
}

