#pragma once

#include <godot_cpp/classes/ref_counted.hpp>

namespace godot {

class Image;

class DrawingAlgosCpp : public RefCounted
{
    GDCLASS(DrawingAlgosCpp, RefCounted)

protected:
    static void _bind_methods();

public:
    ~DrawingAlgosCpp() = default;

    static void SetLayerMetadataImage(Ref<RefCounted> layer, Ref<RefCounted> cel, Ref<Image>, int index, bool include = true); // TODO: Broken
    static void BlendLayersHeadless(Ref<Image> image, Ref<RefCounted> project, Ref<RefCounted> layer, Ref<RefCounted> cel, Vector2i origin);
    static TypedArray<Vector2i> GetEllipsePoints(Vector2i pos, Vector2i size);
    static TypedArray<Vector2i> GetEllipsePointsFilled(Vector2i pos, Vector2i size, int thickness);
    static Ref<Image> Scale3x(Ref<Image> sprite, float tol = 0.196078);
    static Rect2 TransformRectangle(Rect2 rect, Transform2D matrix, Vector2 pivot);
    static void Rotxel(Ref<Image> sprite, float angle, Vector2 pivot);
    static void FakeRotsprite(Ref<Image> sprite, float angle, Vector2 pivot);
    static void NNRotate(Ref<Image> sprite, float angle, Vector2 pivot);
    static bool SimilarColors(Color c1, Color c2, float tol = 0.392157);
};

}
