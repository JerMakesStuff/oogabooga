
#define v4_expand_v2(a) a.xy, a.zw

/*
	There are abbreviated versions for f32 and s32
*/

#define v2_in_range v2f32_in_range
#define v2i_in_range v2s32_in_range

inline bool v2f32_in_range(Vector2f32 a, Vector2f32 min, Vector2f32 max) { return (a.x >= min.x && a.x <= max.x) && (a.y >= min.y && a.y <= max.y); }
inline bool v2f64_in_range(Vector2f64 a, Vector2f64 min, Vector2f64 max) { return (a.x >= min.x && a.x <= max.x) && (a.y >= min.y && a.y <= max.y); }
inline bool v2s32_in_range(Vector2s32 a, Vector2s32 min, Vector2s32 max) { return (a.x >= min.x && a.x <= max.x) && (a.y >= min.y && a.y <= max.y); }
