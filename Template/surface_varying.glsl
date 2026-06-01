
LAYOUT_LOCATION(0) VARYING vec4 vertex_worldPosition;

#if defined(HAS_ATTRIBUTE_TANGENTS)
LAYOUT_LOCATION(1) VARYING vec3 vertex_worldNormal;
#if defined(MATERIAL_NEEDS_TBN)
LAYOUT_LOCATION(2) VARYING vec4 vertex_worldTangent;
#endif
#endif

LAYOUT_LOCATION(3) VARYING vec4 vertex_position;

#if defined(HAS_ATTRIBUTE_COLOR)
LAYOUT_LOCATION(4) VARYING mediump vec4 vertex_color;
#endif

#if defined(HAS_ATTRIBUTE_UV0) && !defined(HAS_ATTRIBUTE_UV1)
LAYOUT_LOCATION(5) VARYING highp vec2 vertex_uv01;
#elif defined(HAS_ATTRIBUTE_UV1)
LAYOUT_LOCATION(5) VARYING highp vec4 vertex_uv01;
#endif

#if defined(VARIANT_HAS_SHADOWING) && defined(VARIANT_HAS_DIRECTIONAL_LIGHTING)
LAYOUT_LOCATION(6) VARYING highp vec4 vertex_lightSpacePosition;
#endif

LAYOUT_LOCATION(7) flat VARYING highp int instance_index;
highp int logical_instance_index;
