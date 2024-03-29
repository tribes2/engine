//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _PLATFORMGL_H_
#define _PLATFORMGL_H_


typedef unsigned int	GLenum;
typedef unsigned char	GLboolean;
typedef unsigned int	GLbitfield;
typedef void		GLvoid;
typedef signed char	GLbyte;		/* 1-byte signed */
typedef short		GLshort;	/* 2-byte signed */
typedef int		GLint;		/* 4-byte signed */
typedef unsigned char	GLubyte;	/* 1-byte unsigned */
typedef unsigned short	GLushort;	/* 2-byte unsigned */
typedef unsigned int	GLuint;		/* 4-byte unsigned */
typedef int		GLsizei;	/* 4-byte signed */
typedef float		GLfloat;	/* single precision float */
typedef float		GLclampf;	/* single precision float in [0,1] */
typedef double		GLdouble;	/* double precision float */
typedef double		GLclampd;	/* double precision float in [0,1] */


/* Boolean values */
#define GL_FALSE				0x0
#define GL_TRUE					0x1

/* Data types */
#define GL_BYTE					0x1400
#define GL_UNSIGNED_BYTE			0x1401
#define GL_SHORT				0x1402
#define GL_UNSIGNED_SHORT			0x1403
#define GL_INT					0x1404
#define GL_UNSIGNED_INT				0x1405
#define GL_FLOAT				0x1406
#define GL_DOUBLE				0x140A
#define GL_2_BYTES				0x1407
#define GL_3_BYTES				0x1408
#define GL_4_BYTES				0x1409

/* Primitives */
#define GL_POINTS				0x0000
#define GL_LINES				0x0001
#define GL_LINE_LOOP				0x0002
#define GL_LINE_STRIP				0x0003
#define GL_TRIANGLES				0x0004
#define GL_TRIANGLE_STRIP			0x0005
#define GL_TRIANGLE_FAN				0x0006
#define GL_QUADS				0x0007
#define GL_QUAD_STRIP				0x0008
#define GL_POLYGON				0x0009

/* Vertex Arrays */
#define GL_VERTEX_ARRAY				0x8074
#define GL_NORMAL_ARRAY				0x8075
#define GL_COLOR_ARRAY				0x8076
#define GL_INDEX_ARRAY				0x8077
#define GL_TEXTURE_COORD_ARRAY			0x8078
#define GL_EDGE_FLAG_ARRAY			0x8079
#define GL_VERTEX_ARRAY_SIZE			0x807A
#define GL_VERTEX_ARRAY_TYPE			0x807B
#define GL_VERTEX_ARRAY_STRIDE			0x807C
#define GL_NORMAL_ARRAY_TYPE			0x807E
#define GL_NORMAL_ARRAY_STRIDE			0x807F
#define GL_COLOR_ARRAY_SIZE			0x8081
#define GL_COLOR_ARRAY_TYPE			0x8082
#define GL_COLOR_ARRAY_STRIDE			0x8083
#define GL_INDEX_ARRAY_TYPE			0x8085
#define GL_INDEX_ARRAY_STRIDE			0x8086
#define GL_TEXTURE_COORD_ARRAY_SIZE		0x8088
#define GL_TEXTURE_COORD_ARRAY_TYPE		0x8089
#define GL_TEXTURE_COORD_ARRAY_STRIDE		0x808A
#define GL_EDGE_FLAG_ARRAY_STRIDE		0x808C
#define GL_VERTEX_ARRAY_POINTER			0x808E
#define GL_NORMAL_ARRAY_POINTER			0x808F
#define GL_COLOR_ARRAY_POINTER			0x8090
#define GL_INDEX_ARRAY_POINTER			0x8091
#define GL_TEXTURE_COORD_ARRAY_POINTER		0x8092
#define GL_EDGE_FLAG_ARRAY_POINTER		0x8093
#define GL_V2F					0x2A20
#define GL_V3F					0x2A21
#define GL_C4UB_V2F				0x2A22
#define GL_C4UB_V3F				0x2A23
#define GL_C3F_V3F				0x2A24
#define GL_N3F_V3F				0x2A25
#define GL_C4F_N3F_V3F				0x2A26
#define GL_T2F_V3F				0x2A27
#define GL_T4F_V4F				0x2A28
#define GL_T2F_C4UB_V3F				0x2A29
#define GL_T2F_C3F_V3F				0x2A2A
#define GL_T2F_N3F_V3F				0x2A2B
#define GL_T2F_C4F_N3F_V3F			0x2A2C
#define GL_T4F_C4F_N3F_V4F			0x2A2D

/* Matrix Mode */
#define GL_MATRIX_MODE				0x0BA0
#define GL_MODELVIEW				0x1700
#define GL_PROJECTION				0x1701
#define GL_TEXTURE				0x1702

/* Points */
#define GL_POINT_SMOOTH				0x0B10
#define GL_POINT_SIZE				0x0B11
#define GL_POINT_SIZE_GRANULARITY 		0x0B13
#define GL_POINT_SIZE_RANGE			0x0B12

/* Lines */
#define GL_LINE_SMOOTH				0x0B20
#define GL_LINE_STIPPLE				0x0B24
#define GL_LINE_STIPPLE_PATTERN			0x0B25
#define GL_LINE_STIPPLE_REPEAT			0x0B26
#define GL_LINE_WIDTH				0x0B21
#define GL_LINE_WIDTH_GRANULARITY		0x0B23
#define GL_LINE_WIDTH_RANGE			0x0B22

/* Polygons */
#define GL_POINT				0x1B00
#define GL_LINE					0x1B01
#define GL_FILL					0x1B02
#define GL_CW					0x0900
#define GL_CCW					0x0901
#define GL_FRONT				0x0404
#define GL_BACK					0x0405
#define GL_POLYGON_MODE				0x0B40
#define GL_POLYGON_SMOOTH			0x0B41
#define GL_POLYGON_STIPPLE			0x0B42
#define GL_EDGE_FLAG				0x0B43
#define GL_CULL_FACE				0x0B44
#define GL_CULL_FACE_MODE			0x0B45
#define GL_FRONT_FACE				0x0B46
#define GL_POLYGON_OFFSET_FACTOR		0x8038
#define GL_POLYGON_OFFSET_UNITS			0x2A00
#define GL_POLYGON_OFFSET_POINT			0x2A01
#define GL_POLYGON_OFFSET_LINE			0x2A02
#define GL_POLYGON_OFFSET_FILL			0x8037

/* Display Lists */
#define GL_COMPILE				0x1300
#define GL_COMPILE_AND_EXECUTE			0x1301
#define GL_LIST_BASE				0x0B32
#define GL_LIST_INDEX				0x0B33
#define GL_LIST_MODE				0x0B30

/* Depth buffer */
#define GL_NEVER				0x0200
#define GL_LESS					0x0201
#define GL_EQUAL				0x0202
#define GL_LEQUAL				0x0203
#define GL_GREATER				0x0204
#define GL_NOTEQUAL				0x0205
#define GL_GEQUAL				0x0206
#define GL_ALWAYS				0x0207
#define GL_DEPTH_TEST				0x0B71
#define GL_DEPTH_BITS				0x0D56
#define GL_DEPTH_CLEAR_VALUE			0x0B73
#define GL_DEPTH_FUNC				0x0B74
#define GL_DEPTH_RANGE				0x0B70
#define GL_DEPTH_WRITEMASK			0x0B72
#define GL_DEPTH_COMPONENT			0x1902

/* Lighting */
#define GL_LIGHTING				0x0B50
#define GL_LIGHT0				0x4000
#define GL_LIGHT1				0x4001
#define GL_LIGHT2				0x4002
#define GL_LIGHT3				0x4003
#define GL_LIGHT4				0x4004
#define GL_LIGHT5				0x4005
#define GL_LIGHT6				0x4006
#define GL_LIGHT7				0x4007
#define GL_SPOT_EXPONENT			0x1205
#define GL_SPOT_CUTOFF				0x1206
#define GL_CONSTANT_ATTENUATION			0x1207
#define GL_LINEAR_ATTENUATION			0x1208
#define GL_QUADRATIC_ATTENUATION		0x1209
#define GL_AMBIENT				0x1200
#define GL_DIFFUSE				0x1201
#define GL_SPECULAR				0x1202
#define GL_SHININESS				0x1601
#define GL_EMISSION				0x1600
#define GL_POSITION				0x1203
#define GL_SPOT_DIRECTION			0x1204
#define GL_AMBIENT_AND_DIFFUSE			0x1602
#define GL_COLOR_INDEXES			0x1603
#define GL_LIGHT_MODEL_TWO_SIDE			0x0B52
#define GL_LIGHT_MODEL_LOCAL_VIEWER		0x0B51
#define GL_LIGHT_MODEL_AMBIENT			0x0B53
#define GL_FRONT_AND_BACK			0x0408
#define GL_SHADE_MODEL				0x0B54
#define GL_FLAT					0x1D00
#define GL_SMOOTH				0x1D01
#define GL_COLOR_MATERIAL			0x0B57
#define GL_COLOR_MATERIAL_FACE			0x0B55
#define GL_COLOR_MATERIAL_PARAMETER		0x0B56
#define GL_NORMALIZE				0x0BA1

/* User clipping planes */
#define GL_CLIP_PLANE0				0x3000
#define GL_CLIP_PLANE1				0x3001
#define GL_CLIP_PLANE2				0x3002
#define GL_CLIP_PLANE3				0x3003
#define GL_CLIP_PLANE4				0x3004
#define GL_CLIP_PLANE5				0x3005

/* Accumulation buffer */
#define GL_ACCUM_RED_BITS			0x0D58
#define GL_ACCUM_GREEN_BITS			0x0D59
#define GL_ACCUM_BLUE_BITS			0x0D5A
#define GL_ACCUM_ALPHA_BITS			0x0D5B
#define GL_ACCUM_CLEAR_VALUE			0x0B80
#define GL_ACCUM				0x0100
#define GL_ADD					0x0104
#define GL_LOAD					0x0101
#define GL_MULT					0x0103
#define GL_RETURN				0x0102

/* Alpha testing */
#define GL_ALPHA_TEST				0x0BC0
#define GL_ALPHA_TEST_REF			0x0BC2
#define GL_ALPHA_TEST_FUNC			0x0BC1

/* Blending */
#define GL_BLEND				0x0BE2
#define GL_BLEND_SRC				0x0BE1
#define GL_BLEND_DST				0x0BE0
#define GL_ZERO					0x0
#define GL_ONE					0x1
#define GL_SRC_COLOR				0x0300
#define GL_ONE_MINUS_SRC_COLOR			0x0301
#define GL_DST_COLOR				0x0306
#define GL_ONE_MINUS_DST_COLOR			0x0307
#define GL_SRC_ALPHA				0x0302
#define GL_ONE_MINUS_SRC_ALPHA			0x0303
#define GL_DST_ALPHA				0x0304
#define GL_ONE_MINUS_DST_ALPHA			0x0305
#define GL_SRC_ALPHA_SATURATE			0x0308
#define GL_CONSTANT_COLOR			0x8001
#define GL_ONE_MINUS_CONSTANT_COLOR		0x8002
#define GL_CONSTANT_ALPHA			0x8003
#define GL_ONE_MINUS_CONSTANT_ALPHA		0x8004

/* Render Mode */
#define GL_FEEDBACK				0x1C01
#define GL_RENDER				0x1C00
#define GL_SELECT				0x1C02

/* Feedback */
#define GL_2D					0x0600
#define GL_3D					0x0601
#define GL_3D_COLOR				0x0602
#define GL_3D_COLOR_TEXTURE			0x0603
#define GL_4D_COLOR_TEXTURE			0x0604
#define GL_POINT_TOKEN				0x0701
#define GL_LINE_TOKEN				0x0702
#define GL_LINE_RESET_TOKEN			0x0707
#define GL_POLYGON_TOKEN			0x0703
#define GL_BITMAP_TOKEN				0x0704
#define GL_DRAW_PIXEL_TOKEN			0x0705
#define GL_COPY_PIXEL_TOKEN			0x0706
#define GL_PASS_THROUGH_TOKEN			0x0700
#define GL_FEEDBACK_BUFFER_POINTER		0x0DF0
#define GL_FEEDBACK_BUFFER_SIZE			0x0DF1
#define GL_FEEDBACK_BUFFER_TYPE			0x0DF2

/* Selection */
#define GL_SELECTION_BUFFER_POINTER		0x0DF3
#define GL_SELECTION_BUFFER_SIZE		0x0DF4

/* Fog */
#define GL_FOG					0x0B60
#define GL_FOG_MODE				0x0B65
#define GL_FOG_DENSITY				0x0B62
#define GL_FOG_COLOR				0x0B66
#define GL_FOG_INDEX				0x0B61
#define GL_FOG_START				0x0B63
#define GL_FOG_END				0x0B64
#define GL_LINEAR				0x2601
#define GL_EXP					0x0800
#define GL_EXP2					0x0801

/* Logic Ops */
#define GL_LOGIC_OP				0x0BF1
#define GL_INDEX_LOGIC_OP			0x0BF1
#define GL_COLOR_LOGIC_OP			0x0BF2
#define GL_LOGIC_OP_MODE			0x0BF0
#define GL_CLEAR				0x1500
#define GL_SET					0x150F
#define GL_COPY					0x1503
#define GL_COPY_INVERTED			0x150C
#define GL_NOOP					0x1505
#define GL_INVERT				0x150A
#define GL_AND					0x1501
#define GL_NAND					0x150E
#define GL_OR					0x1507
#define GL_NOR					0x1508
#define GL_XOR					0x1506
#define GL_EQUIV				0x1509
#define GL_AND_REVERSE				0x1502
#define GL_AND_INVERTED				0x1504
#define GL_OR_REVERSE				0x150B
#define GL_OR_INVERTED				0x150D

/* Stencil */
#define GL_STENCIL_TEST				0x0B90
#define GL_STENCIL_WRITEMASK			0x0B98
#define GL_STENCIL_BITS				0x0D57
#define GL_STENCIL_FUNC				0x0B92
#define GL_STENCIL_VALUE_MASK			0x0B93
#define GL_STENCIL_REF				0x0B97
#define GL_STENCIL_FAIL				0x0B94
#define GL_STENCIL_PASS_DEPTH_PASS		0x0B96
#define GL_STENCIL_PASS_DEPTH_FAIL		0x0B95
#define GL_STENCIL_CLEAR_VALUE			0x0B91
#define GL_STENCIL_INDEX			0x1901
#define GL_KEEP					0x1E00
#define GL_REPLACE				0x1E01
#define GL_INCR					0x1E02
#define GL_DECR					0x1E03

/* Buffers, Pixel Drawing/Reading */
#define GL_NONE					0x0
#define GL_LEFT					0x0406
#define GL_RIGHT				0x0407
/*GL_FRONT					0x0404 */
/*GL_BACK					0x0405 */
/*GL_FRONT_AND_BACK				0x0408 */
#define GL_FRONT_LEFT				0x0400
#define GL_FRONT_RIGHT				0x0401
#define GL_BACK_LEFT				0x0402
#define GL_BACK_RIGHT				0x0403
#define GL_AUX0					0x0409
#define GL_AUX1					0x040A
#define GL_AUX2					0x040B
#define GL_AUX3					0x040C
#define GL_COLOR_INDEX				0x1900
#define GL_RED					0x1903
#define GL_GREEN				0x1904
#define GL_BLUE					0x1905
#define GL_ALPHA				0x1906
#define GL_LUMINANCE				0x1909
#define GL_LUMINANCE_ALPHA			0x190A
#define GL_ALPHA_BITS				0x0D55
#define GL_RED_BITS				0x0D52
#define GL_GREEN_BITS				0x0D53
#define GL_BLUE_BITS				0x0D54
#define GL_INDEX_BITS				0x0D51
#define GL_SUBPIXEL_BITS			0x0D50
#define GL_AUX_BUFFERS				0x0C00
#define GL_READ_BUFFER				0x0C02
#define GL_DRAW_BUFFER				0x0C01
#define GL_DOUBLEBUFFER				0x0C32
#define GL_STEREO				0x0C33
#define GL_BITMAP				0x1A00
#define GL_COLOR				0x1800
#define GL_DEPTH				0x1801
#define GL_STENCIL				0x1802
#define GL_DITHER				0x0BD0
#define GL_RGB					0x1907
#define GL_RGBA					0x1908

/* Implementation limits */
#define GL_MAX_LIST_NESTING			0x0B31
#define GL_MAX_ATTRIB_STACK_DEPTH		0x0D35
#define GL_MAX_MODELVIEW_STACK_DEPTH		0x0D36
#define GL_MAX_NAME_STACK_DEPTH			0x0D37
#define GL_MAX_PROJECTION_STACK_DEPTH		0x0D38
#define GL_MAX_TEXTURE_STACK_DEPTH		0x0D39
#define GL_MAX_EVAL_ORDER			0x0D30
#define GL_MAX_LIGHTS				0x0D31
#define GL_MAX_CLIP_PLANES			0x0D32
#define GL_MAX_TEXTURE_SIZE			0x0D33
#define GL_MAX_PIXEL_MAP_TABLE			0x0D34
#define GL_MAX_VIEWPORT_DIMS			0x0D3A
#define GL_MAX_CLIENT_ATTRIB_STACK_DEPTH	0x0D3B

/* Gets */
#define GL_ATTRIB_STACK_DEPTH			0x0BB0
#define GL_CLIENT_ATTRIB_STACK_DEPTH		0x0BB1
#define GL_COLOR_CLEAR_VALUE			0x0C22
#define GL_COLOR_WRITEMASK			0x0C23
#define GL_CURRENT_INDEX			0x0B01
#define GL_CURRENT_COLOR			0x0B00
#define GL_CURRENT_NORMAL			0x0B02
#define GL_CURRENT_RASTER_COLOR			0x0B04
#define GL_CURRENT_RASTER_DISTANCE		0x0B09
#define GL_CURRENT_RASTER_INDEX			0x0B05
#define GL_CURRENT_RASTER_POSITION		0x0B07
#define GL_CURRENT_RASTER_TEXTURE_COORDS	0x0B06
#define GL_CURRENT_RASTER_POSITION_VALID	0x0B08
#define GL_CURRENT_TEXTURE_COORDS		0x0B03
#define GL_INDEX_CLEAR_VALUE			0x0C20
#define GL_INDEX_MODE				0x0C30
#define GL_INDEX_WRITEMASK			0x0C21
#define GL_MODELVIEW_MATRIX			0x0BA6
#define GL_MODELVIEW_STACK_DEPTH		0x0BA3
#define GL_NAME_STACK_DEPTH			0x0D70
#define GL_PROJECTION_MATRIX			0x0BA7
#define GL_PROJECTION_STACK_DEPTH		0x0BA4
#define GL_RENDER_MODE				0x0C40
#define GL_RGBA_MODE				0x0C31
#define GL_TEXTURE_MATRIX			0x0BA8
#define GL_TEXTURE_STACK_DEPTH			0x0BA5
#define GL_VIEWPORT				0x0BA2

/* Evaluators */
#define GL_AUTO_NORMAL				0x0D80
#define GL_MAP1_COLOR_4				0x0D90
#define GL_MAP1_GRID_DOMAIN			0x0DD0
#define GL_MAP1_GRID_SEGMENTS			0x0DD1
#define GL_MAP1_INDEX				0x0D91
#define GL_MAP1_NORMAL				0x0D92
#define GL_MAP1_TEXTURE_COORD_1			0x0D93
#define GL_MAP1_TEXTURE_COORD_2			0x0D94
#define GL_MAP1_TEXTURE_COORD_3			0x0D95
#define GL_MAP1_TEXTURE_COORD_4			0x0D96
#define GL_MAP1_VERTEX_3			0x0D97
#define GL_MAP1_VERTEX_4			0x0D98
#define GL_MAP2_COLOR_4				0x0DB0
#define GL_MAP2_GRID_DOMAIN			0x0DD2
#define GL_MAP2_GRID_SEGMENTS			0x0DD3
#define GL_MAP2_INDEX				0x0DB1
#define GL_MAP2_NORMAL				0x0DB2
#define GL_MAP2_TEXTURE_COORD_1			0x0DB3
#define GL_MAP2_TEXTURE_COORD_2			0x0DB4
#define GL_MAP2_TEXTURE_COORD_3			0x0DB5
#define GL_MAP2_TEXTURE_COORD_4			0x0DB6
#define GL_MAP2_VERTEX_3			0x0DB7
#define GL_MAP2_VERTEX_4			0x0DB8
#define GL_COEFF				0x0A00
#define GL_DOMAIN				0x0A02
#define GL_ORDER				0x0A01

/* Hints */
#define GL_FOG_HINT				0x0C54
#define GL_LINE_SMOOTH_HINT			0x0C52
#define GL_PERSPECTIVE_CORRECTION_HINT		0x0C50
#define GL_POINT_SMOOTH_HINT			0x0C51
#define GL_POLYGON_SMOOTH_HINT			0x0C53
#define GL_DONT_CARE				0x1100
#define GL_FASTEST				0x1101
#define GL_NICEST				0x1102

/* Scissor box */
#define GL_SCISSOR_TEST				0x0C11
#define GL_SCISSOR_BOX				0x0C10

/* Pixel Mode / Transfer */
#define GL_MAP_COLOR				0x0D10
#define GL_MAP_STENCIL				0x0D11
#define GL_INDEX_SHIFT				0x0D12
#define GL_INDEX_OFFSET				0x0D13
#define GL_RED_SCALE				0x0D14
#define GL_RED_BIAS				0x0D15
#define GL_GREEN_SCALE				0x0D18
#define GL_GREEN_BIAS				0x0D19
#define GL_BLUE_SCALE				0x0D1A
#define GL_BLUE_BIAS				0x0D1B
#define GL_ALPHA_SCALE				0x0D1C
#define GL_ALPHA_BIAS				0x0D1D
#define GL_DEPTH_SCALE				0x0D1E
#define GL_DEPTH_BIAS				0x0D1F
#define GL_PIXEL_MAP_S_TO_S_SIZE		0x0CB1
#define GL_PIXEL_MAP_I_TO_I_SIZE		0x0CB0
#define GL_PIXEL_MAP_I_TO_R_SIZE		0x0CB2
#define GL_PIXEL_MAP_I_TO_G_SIZE		0x0CB3
#define GL_PIXEL_MAP_I_TO_B_SIZE		0x0CB4
#define GL_PIXEL_MAP_I_TO_A_SIZE		0x0CB5
#define GL_PIXEL_MAP_R_TO_R_SIZE		0x0CB6
#define GL_PIXEL_MAP_G_TO_G_SIZE		0x0CB7
#define GL_PIXEL_MAP_B_TO_B_SIZE		0x0CB8
#define GL_PIXEL_MAP_A_TO_A_SIZE		0x0CB9
#define GL_PIXEL_MAP_S_TO_S			0x0C71
#define GL_PIXEL_MAP_I_TO_I			0x0C70
#define GL_PIXEL_MAP_I_TO_R			0x0C72
#define GL_PIXEL_MAP_I_TO_G			0x0C73
#define GL_PIXEL_MAP_I_TO_B			0x0C74
#define GL_PIXEL_MAP_I_TO_A			0x0C75
#define GL_PIXEL_MAP_R_TO_R			0x0C76
#define GL_PIXEL_MAP_G_TO_G			0x0C77
#define GL_PIXEL_MAP_B_TO_B			0x0C78
#define GL_PIXEL_MAP_A_TO_A			0x0C79
#define GL_PACK_ALIGNMENT			0x0D05
#define GL_PACK_LSB_FIRST			0x0D01
#define GL_PACK_ROW_LENGTH			0x0D02
#define GL_PACK_SKIP_PIXELS			0x0D04
#define GL_PACK_SKIP_ROWS			0x0D03
#define GL_PACK_SWAP_BYTES			0x0D00
#define GL_UNPACK_ALIGNMENT			0x0CF5
#define GL_UNPACK_LSB_FIRST			0x0CF1
#define GL_UNPACK_ROW_LENGTH			0x0CF2
#define GL_UNPACK_SKIP_PIXELS			0x0CF4
#define GL_UNPACK_SKIP_ROWS			0x0CF3
#define GL_UNPACK_SWAP_BYTES			0x0CF0
#define GL_ZOOM_X				0x0D16
#define GL_ZOOM_Y				0x0D17

/* Texture mapping */
#define GL_TEXTURE_ENV				0x2300
#define GL_TEXTURE_ENV_MODE			0x2200
#define GL_TEXTURE_1D				0x0DE0
#define GL_TEXTURE_2D				0x0DE1
#define GL_TEXTURE_WRAP_S			0x2802
#define GL_TEXTURE_WRAP_T			0x2803
#define GL_TEXTURE_MAG_FILTER			0x2800
#define GL_TEXTURE_MIN_FILTER			0x2801
#define GL_TEXTURE_ENV_COLOR			0x2201
#define GL_TEXTURE_GEN_S			0x0C60
#define GL_TEXTURE_GEN_T			0x0C61
#define GL_TEXTURE_GEN_MODE			0x2500
#define GL_TEXTURE_BORDER_COLOR			0x1004
#define GL_TEXTURE_WIDTH			0x1000
#define GL_TEXTURE_HEIGHT			0x1001
#define GL_TEXTURE_BORDER			0x1005
#define GL_TEXTURE_COMPONENTS			0x1003
#define GL_TEXTURE_RED_SIZE			0x805C
#define GL_TEXTURE_GREEN_SIZE			0x805D
#define GL_TEXTURE_BLUE_SIZE			0x805E
#define GL_TEXTURE_ALPHA_SIZE			0x805F
#define GL_TEXTURE_LUMINANCE_SIZE		0x8060
#define GL_TEXTURE_INTENSITY_SIZE		0x8061
#define GL_NEAREST_MIPMAP_NEAREST		0x2700
#define GL_NEAREST_MIPMAP_LINEAR		0x2702
#define GL_LINEAR_MIPMAP_NEAREST		0x2701
#define GL_LINEAR_MIPMAP_LINEAR			0x2703
#define GL_OBJECT_LINEAR			0x2401
#define GL_OBJECT_PLANE				0x2501
#define GL_EYE_LINEAR				0x2400
#define GL_EYE_PLANE				0x2502
#define GL_SPHERE_MAP				0x2402
#define GL_DECAL				0x2101
#define GL_MODULATE				0x2100
#define GL_NEAREST				0x2600
#define GL_REPEAT				0x2901
#define GL_CLAMP				0x2900
#define GL_S					0x2000
#define GL_T					0x2001
#define GL_R					0x2002
#define GL_Q					0x2003
#define GL_TEXTURE_GEN_R			0x0C62
#define GL_TEXTURE_GEN_Q			0x0C63

/* GL 1.1 texturing */
#define GL_PROXY_TEXTURE_1D			0x8063
#define GL_PROXY_TEXTURE_2D			0x8064
#define GL_TEXTURE_PRIORITY			0x8066
#define GL_TEXTURE_RESIDENT			0x8067
#define GL_TEXTURE_BINDING_1D			0x8068
#define GL_TEXTURE_BINDING_2D			0x8069
#define GL_TEXTURE_INTERNAL_FORMAT		0x1003

/* GL 1.2 texturing */
#define GL_PACK_SKIP_IMAGES			0x806B
#define GL_PACK_IMAGE_HEIGHT			0x806C
#define GL_UNPACK_SKIP_IMAGES			0x806D
#define GL_UNPACK_IMAGE_HEIGHT			0x806E
#define GL_TEXTURE_3D				0x806F
#define GL_PROXY_TEXTURE_3D			0x8070
#define GL_TEXTURE_DEPTH			0x8071
#define GL_TEXTURE_WRAP_R			0x8072
#define GL_MAX_3D_TEXTURE_SIZE			0x8073
#define GL_TEXTURE_BINDING_3D			0x806A

/* Internal texture formats (GL 1.1) */
#define GL_ALPHA4				0x803B
#define GL_ALPHA8				0x803C
#define GL_ALPHA12				0x803D
#define GL_ALPHA16				0x803E
#define GL_LUMINANCE4				0x803F
#define GL_LUMINANCE8				0x8040
#define GL_LUMINANCE12				0x8041
#define GL_LUMINANCE16				0x8042
#define GL_LUMINANCE4_ALPHA4			0x8043
#define GL_LUMINANCE6_ALPHA2			0x8044
#define GL_LUMINANCE8_ALPHA8			0x8045
#define GL_LUMINANCE12_ALPHA4			0x8046
#define GL_LUMINANCE12_ALPHA12			0x8047
#define GL_LUMINANCE16_ALPHA16			0x8048
#define GL_INTENSITY				0x8049
#define GL_INTENSITY4				0x804A
#define GL_INTENSITY8				0x804B
#define GL_INTENSITY12				0x804C
#define GL_INTENSITY16				0x804D
#define GL_R3_G3_B2				0x2A10
#define GL_RGB4					0x804F
#define GL_RGB5					0x8050
#define GL_RGB8					0x8051
#define GL_RGB10				0x8052
#define GL_RGB12				0x8053
#define GL_RGB16				0x8054
#define GL_RGBA2				0x8055
#define GL_RGBA4				0x8056
#define GL_RGB5_A1				0x8057
#define GL_RGBA8				0x8058
#define GL_RGB10_A2				0x8059
#define GL_RGBA12				0x805A
#define GL_RGBA16				0x805B

/* Utility */
#define GL_VENDOR				0x1F00
#define GL_RENDERER				0x1F01
#define GL_VERSION				0x1F02
#define GL_EXTENSIONS				0x1F03

/* Errors */
#define GL_NO_ERROR 				0x0
#define GL_INVALID_VALUE			0x0501
#define GL_INVALID_ENUM				0x0500
#define GL_INVALID_OPERATION			0x0502
#define GL_STACK_OVERFLOW			0x0503
#define GL_STACK_UNDERFLOW			0x0504
#define GL_OUT_OF_MEMORY			0x0505


/* OpenGL 1.2 */
#define GL_RESCALE_NORMAL			0x803A
#define GL_CLAMP_TO_EDGE			0x812F
#define GL_MAX_ELEMENTS_VERTICES		0x80E8
#define GL_MAX_ELEMENTS_INDICES			0x80E9
#define GL_BGR					0x80E0
#define GL_BGRA					0x80E1
#define GL_UNSIGNED_BYTE_3_3_2			0x8032
#define GL_UNSIGNED_BYTE_2_3_3_REV		0x8362
#define GL_UNSIGNED_SHORT_5_6_5			0x8363
#define GL_UNSIGNED_SHORT_5_6_5_REV		0x8364
#define GL_UNSIGNED_SHORT_4_4_4_4		0x8033
#define GL_UNSIGNED_SHORT_4_4_4_4_REV		0x8365
#define GL_UNSIGNED_SHORT_5_5_5_1		0x8034
#define GL_UNSIGNED_SHORT_1_5_5_5_REV		0x8366
#define GL_UNSIGNED_INT_8_8_8_8			0x8035
#define GL_UNSIGNED_INT_8_8_8_8_REV		0x8367
#define GL_UNSIGNED_INT_10_10_10_2		0x8036
#define GL_UNSIGNED_INT_2_10_10_10_REV		0x8368
#define GL_LIGHT_MODEL_COLOR_CONTROL		0x81F8
#define GL_SINGLE_COLOR				0x81F9
#define GL_SEPARATE_SPECULAR_COLOR		0x81FA
#define GL_TEXTURE_MIN_LOD			0x813A
#define GL_TEXTURE_MAX_LOD			0x813B
#define GL_TEXTURE_BASE_LEVEL			0x813C
#define GL_TEXTURE_MAX_LEVEL			0x813D
#define GL_SMOOTH_POINT_SIZE_RANGE		0x0B12
#define GL_SMOOTH_POINT_SIZE_GRANULARITY	0x0B13
#define GL_SMOOTH_LINE_WIDTH_RANGE		0x0B22
#define GL_SMOOTH_LINE_WIDTH_GRANULARITY	0x0B23
#define GL_ALIASED_POINT_SIZE_RANGE		0x846D
#define GL_ALIASED_LINE_WIDTH_RANGE		0x846E



/*
 * OpenGL 1.2 imaging subset (NOT IMPLEMENTED BY MESA)
 */
/* GL_EXT_color_table */
#define GL_COLOR_TABLE				0x80D0
#define GL_POST_CONVOLUTION_COLOR_TABLE		0x80D1
#define GL_POST_COLOR_MATRIX_COLOR_TABLE	0x80D2
#define GL_PROXY_COLOR_TABLE			0x80D3
#define GL_PROXY_POST_CONVOLUTION_COLOR_TABLE	0x80D4
#define GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE	0x80D5
#define GL_COLOR_TABLE_SCALE			0x80D6
#define GL_COLOR_TABLE_BIAS			0x80D7
#define GL_COLOR_TABLE_FORMAT			0x80D8
#define GL_COLOR_TABLE_WIDTH			0x80D9
#define GL_COLOR_TABLE_RED_SIZE			0x80DA
#define GL_COLOR_TABLE_GREEN_SIZE		0x80DB
#define GL_COLOR_TABLE_BLUE_SIZE		0x80DC
#define GL_COLOR_TABLE_ALPHA_SIZE		0x80DD
#define GL_COLOR_TABLE_LUMINANCE_SIZE		0x80DE
#define GL_COLOR_TABLE_INTENSITY_SIZE		0x80DF
/* GL_EXT_convolution and GL_HP_convolution_border_modes */
#define GL_CONVOLUTION_1D			0x8010
#define GL_CONVOLUTION_2D			0x8011
#define GL_SEPARABLE_2D				0x8012
#define GL_CONVOLUTION_BORDER_MODE		0x8013
#define GL_CONVOLUTION_FILTER_SCALE		0x8014
#define GL_CONVOLUTION_FILTER_BIAS		0x8015
#define GL_REDUCE				0x8016
#define GL_CONVOLUTION_FORMAT			0x8017
#define GL_CONVOLUTION_WIDTH			0x8018
#define GL_CONVOLUTION_HEIGHT			0x8019
#define GL_MAX_CONVOLUTION_WIDTH		0x801A
#define GL_MAX_CONVOLUTION_HEIGHT		0x801B
#define GL_POST_CONVOLUTION_RED_SCALE		0x801C
#define GL_POST_CONVOLUTION_GREEN_SCALE		0x801D
#define GL_POST_CONVOLUTION_BLUE_SCALE		0x801E
#define GL_POST_CONVOLUTION_ALPHA_SCALE		0x801F
#define GL_POST_CONVOLUTION_RED_BIAS		0x8020
#define GL_POST_CONVOLUTION_GREEN_BIAS		0x8021
#define GL_POST_CONVOLUTION_BLUE_BIAS		0x8022
#define GL_POST_CONVOLUTION_ALPHA_BIAS		0x8023
#define GL_CONSTANT_BORDER			0x8151
#define GL_REPLICATE_BORDER			0x8153
#define GL_CONVOLUTION_BORDER_COLOR		0x8154
/* GL_SGI_color_matrix */
#define GL_COLOR_MATRIX				0x80B1
#define GL_COLOR_MATRIX_STACK_DEPTH		0x80B2
#define GL_MAX_COLOR_MATRIX_STACK_DEPTH		0x80B3
#define GL_POST_COLOR_MATRIX_RED_SCALE		0x80B4
#define GL_POST_COLOR_MATRIX_GREEN_SCALE	0x80B5
#define GL_POST_COLOR_MATRIX_BLUE_SCALE		0x80B6
#define GL_POST_COLOR_MATRIX_ALPHA_SCALE	0x80B7
#define GL_POST_COLOR_MATRIX_RED_BIAS		0x80B8
#define GL_POST_COLOR_MATRIX_GREEN_BIAS		0x80B9
#define GL_POST_COLOR_MATRIX_BLUE_BIAS		0x80BA
#define GL_POST_COLOR_MATRIX_ALPHA_BIAS		0x80BB
/* GL_EXT_histogram */
#define GL_HISTOGRAM				0x8024
#define GL_PROXY_HISTOGRAM			0x8025
#define GL_HISTOGRAM_WIDTH			0x8026
#define GL_HISTOGRAM_FORMAT			0x8027
#define GL_HISTOGRAM_RED_SIZE			0x8028
#define GL_HISTOGRAM_GREEN_SIZE			0x8029
#define GL_HISTOGRAM_BLUE_SIZE			0x802A
#define GL_HISTOGRAM_ALPHA_SIZE			0x802B
#define GL_HISTOGRAM_LUMINANCE_SIZE		0x802C
#define GL_HISTOGRAM_SINK			0x802D
#define GL_MINMAX				0x802E
#define GL_MINMAX_FORMAT			0x802F
#define GL_MINMAX_SINK				0x8030
#define GL_TABLE_TOO_LARGE			0x8031
/* GL_EXT_blend_color, GL_EXT_blend_minmax */
#define GL_BLEND_EQUATION			0x8009
#define GL_MIN					0x8007
#define GL_MAX					0x8008
#define GL_FUNC_ADD				0x8006
#define GL_FUNC_SUBTRACT			0x800A
#define GL_FUNC_REVERSE_SUBTRACT		0x800B
#define	GL_BLEND_COLOR				0x8005


/* glPush/PopAttrib bits */
#define GL_CURRENT_BIT				0x00000001
#define GL_POINT_BIT				0x00000002
#define GL_LINE_BIT				0x00000004
#define GL_POLYGON_BIT				0x00000008
#define GL_POLYGON_STIPPLE_BIT			0x00000010
#define GL_PIXEL_MODE_BIT			0x00000020
#define GL_LIGHTING_BIT				0x00000040
#define GL_FOG_BIT				0x00000080
#define GL_DEPTH_BUFFER_BIT			0x00000100
#define GL_ACCUM_BUFFER_BIT			0x00000200
#define GL_STENCIL_BUFFER_BIT			0x00000400
#define GL_VIEWPORT_BIT				0x00000800
#define GL_TRANSFORM_BIT			0x00001000
#define GL_ENABLE_BIT				0x00002000
#define GL_COLOR_BUFFER_BIT			0x00004000
#define GL_HINT_BIT				0x00008000
#define GL_EVAL_BIT				0x00010000
#define GL_LIST_BIT				0x00020000
#define GL_TEXTURE_BIT				0x00040000
#define GL_SCISSOR_BIT				0x00080000
#define GL_ALL_ATTRIB_BITS			0x000FFFFF


#define GL_CLIENT_PIXEL_STORE_BIT		0x00000001
#define GL_CLIENT_VERTEX_ARRAY_BIT		0x00000002
#define GL_CLIENT_ALL_ATTRIB_BITS 		0xFFFFFFFF


extern void (*glClearIndex)( GLfloat c );

extern void (*glClearColor)( GLclampf red,
				     GLclampf green,
				     GLclampf blue,
				     GLclampf alpha );

extern void (*glClear)( GLbitfield mask );

extern void (*glIndexMask)( GLuint mask );

extern void (*glColorMask)( GLboolean red, GLboolean green,
			 GLboolean blue, GLboolean alpha );

extern void (*glAlphaFunc)( GLenum func, GLclampf ref );

extern void (*glBlendFunc)( GLenum sfactor, GLenum dfactor );

extern void (*glLogicOp)( GLenum opcode );

extern void (*glCullFace)( GLenum mode );

extern void (*glFrontFace)( GLenum mode );

extern void (*glPointSize)( GLfloat size );

extern void (*glLineWidth)( GLfloat width );

extern void (*glLineStipple)( GLint factor, GLushort pattern );

extern void (*glPolygonMode)( GLenum face, GLenum mode );

extern void (*glPolygonOffset)( GLfloat factor, GLfloat units );

extern void (*glPolygonStipple)( const GLubyte *mask );

extern void (*glGetPolygonStipple)( GLubyte *mask );

extern void (*glEdgeFlag)( GLboolean flag );

extern void (*glEdgeFlagv)( const GLboolean *flag );

extern void (*glScissor)( GLint x, GLint y,
                                   GLsizei width, GLsizei height);

extern void (*glClipPlane)( GLenum plane, const GLdouble *equation );

extern void (*glGetClipPlane)( GLenum plane, GLdouble *equation );

extern void (*glDrawBuffer)( GLenum mode );

extern void (*glReadBuffer)( GLenum mode );

extern void (*glEnable)( GLenum cap );

extern void (*glDisable)( GLenum cap );

extern GLboolean (*glIsEnabled)( GLenum cap );


extern void (*glEnableClientState)( GLenum cap );  /* 1.1 */

extern void (*glDisableClientState)( GLenum cap );  /* 1.1 */


extern void (*glGetBooleanv)( GLenum pname, GLboolean *params );

extern void (*glGetDoublev)( GLenum pname, GLdouble *params );

extern void (*glGetFloatv)( GLenum pname, GLfloat *params );

extern void (*glGetIntegerv)( GLenum pname, GLint *params );


extern void (*glPushAttrib)( GLbitfield mask );

extern void (*glPopAttrib)( void );


extern void (*glPushClientAttrib)( GLbitfield mask );  /* 1.1 */

extern void (*glPopClientAttrib)( void );  /* 1.1 */


extern GLint (*glRenderMode)( GLenum mode );

extern GLenum (*glGetError)( void );

extern const GLubyte* (*glGetString)( GLenum name );

extern void (*glFinish)( void );

extern void (*glFlush)( void );

extern void (*glHint)( GLenum target, GLenum mode );



/*
 * Depth Buffer
 */

extern void (*glClearDepth)( GLclampd depth );

extern void (*glDepthFunc)( GLenum func );

extern void (*glDepthMask)( GLboolean flag );

extern void (*glDepthRange)( GLclampd near_val, GLclampd far_val );


/*
 * Accumulation Buffer
 */

extern void (*glClearAccum)( GLfloat red, GLfloat green,
                                      GLfloat blue, GLfloat alpha );

extern void (*glAccum)( GLenum op, GLfloat value );



/*
 * Transformation
 */

extern void (*glMatrixMode)( GLenum mode );

extern void (*glOrtho)( GLdouble left, GLdouble right,
                                 GLdouble bottom, GLdouble top,
                                 GLdouble near_val, GLdouble far_val );

extern void (*glFrustum)( GLdouble left, GLdouble right,
                                   GLdouble bottom, GLdouble top,
                                   GLdouble near_val, GLdouble far_val );

extern void (*glViewport)( GLint x, GLint y,
                                    GLsizei width, GLsizei height );

extern void (*glPushMatrix)( void );

extern void (*glPopMatrix)( void );

extern void (*glLoadIdentity)( void );

extern void (*glLoadMatrixd)( const GLdouble *m );
extern void (*glLoadMatrixf)( const GLfloat *m );

extern void (*glMultMatrixd)( const GLdouble *m );
extern void (*glMultMatrixf)( const GLfloat *m );

extern void (*glRotated)( GLdouble angle,
                                   GLdouble x, GLdouble y, GLdouble z );
extern void (*glRotatef)( GLfloat angle,
                                   GLfloat x, GLfloat y, GLfloat z );

extern void (*glScaled)( GLdouble x, GLdouble y, GLdouble z );
extern void (*glScalef)( GLfloat x, GLfloat y, GLfloat z );

extern void (*glTranslated)( GLdouble x, GLdouble y, GLdouble z );
extern void (*glTranslatef)( GLfloat x, GLfloat y, GLfloat z );



/*
 * Display Lists
 */

extern GLboolean (*glIsList)( GLuint list );

extern void (*glDeleteLists)( GLuint list, GLsizei range );

extern GLuint (*glGenLists)( GLsizei range );

extern void (*glNewList)( GLuint list, GLenum mode );

extern void (*glEndList)( void );

extern void (*glCallList)( GLuint list );

extern void (*glCallLists)( GLsizei n, GLenum type,
                                     const GLvoid *lists );

extern void (*glListBase)( GLuint base );



/*
 * Drawing Functions
 */

extern void (*glBegin)( GLenum mode );

extern void (*glEnd)( void );


extern void (*glVertex2d)( GLdouble x, GLdouble y );
extern void (*glVertex2f)( GLfloat x, GLfloat y );
extern void (*glVertex2i)( GLint x, GLint y );
extern void (*glVertex2s)( GLshort x, GLshort y );

extern void (*glVertex3d)( GLdouble x, GLdouble y, GLdouble z );
extern void (*glVertex3f)( GLfloat x, GLfloat y, GLfloat z );
extern void (*glVertex3i)( GLint x, GLint y, GLint z );
extern void (*glVertex3s)( GLshort x, GLshort y, GLshort z );

extern void (*glVertex4d)( GLdouble x, GLdouble y, GLdouble z, GLdouble w );
extern void (*glVertex4f)( GLfloat x, GLfloat y, GLfloat z, GLfloat w );
extern void (*glVertex4i)( GLint x, GLint y, GLint z, GLint w );
extern void (*glVertex4s)( GLshort x, GLshort y, GLshort z, GLshort w );

extern void (*glVertex2dv)( const GLdouble *v );
extern void (*glVertex2fv)( const GLfloat *v );
extern void (*glVertex2iv)( const GLint *v );
extern void (*glVertex2sv)( const GLshort *v );

extern void (*glVertex3dv)( const GLdouble *v );
extern void (*glVertex3fv)( const GLfloat *v );
extern void (*glVertex3iv)( const GLint *v );
extern void (*glVertex3sv)( const GLshort *v );

extern void (*glVertex4dv)( const GLdouble *v );
extern void (*glVertex4fv)( const GLfloat *v );
extern void (*glVertex4iv)( const GLint *v );
extern void (*glVertex4sv)( const GLshort *v );


extern void (*glNormal3b)( GLbyte nx, GLbyte ny, GLbyte nz );
extern void (*glNormal3d)( GLdouble nx, GLdouble ny, GLdouble nz );
extern void (*glNormal3f)( GLfloat nx, GLfloat ny, GLfloat nz );
extern void (*glNormal3i)( GLint nx, GLint ny, GLint nz );
extern void (*glNormal3s)( GLshort nx, GLshort ny, GLshort nz );

extern void (*glNormal3bv)( const GLbyte *v );
extern void (*glNormal3dv)( const GLdouble *v );
extern void (*glNormal3fv)( const GLfloat *v );
extern void (*glNormal3iv)( const GLint *v );
extern void (*glNormal3sv)( const GLshort *v );


extern void (*glIndexd)( GLdouble c );
extern void (*glIndexf)( GLfloat c );
extern void (*glIndexi)( GLint c );
extern void (*glIndexs)( GLshort c );
extern void (*glIndexub)( GLubyte c );  /* 1.1 */

extern void (*glIndexdv)( const GLdouble *c );
extern void (*glIndexfv)( const GLfloat *c );
extern void (*glIndexiv)( const GLint *c );
extern void (*glIndexsv)( const GLshort *c );
extern void (*glIndexubv)( const GLubyte *c );  /* 1.1 */

extern void (*glColor3b)( GLbyte red, GLbyte green, GLbyte blue );
extern void (*glColor3d)( GLdouble red, GLdouble green, GLdouble blue );
extern void (*glColor3f)( GLfloat red, GLfloat green, GLfloat blue );
extern void (*glColor3i)( GLint red, GLint green, GLint blue );
extern void (*glColor3s)( GLshort red, GLshort green, GLshort blue );
extern void (*glColor3ub)( GLubyte red, GLubyte green, GLubyte blue );
extern void (*glColor3ui)( GLuint red, GLuint green, GLuint blue );
extern void (*glColor3us)( GLushort red, GLushort green, GLushort blue );

extern void (*glColor4b)( GLbyte red, GLbyte green,
                                   GLbyte blue, GLbyte alpha );
extern void (*glColor4d)( GLdouble red, GLdouble green,
                                   GLdouble blue, GLdouble alpha );
extern void (*glColor4f)( GLfloat red, GLfloat green,
                                   GLfloat blue, GLfloat alpha );
extern void (*glColor4i)( GLint red, GLint green,
                                   GLint blue, GLint alpha );
extern void (*glColor4s)( GLshort red, GLshort green,
                                   GLshort blue, GLshort alpha );
extern void (*glColor4ub)( GLubyte red, GLubyte green,
                                    GLubyte blue, GLubyte alpha );
extern void (*glColor4ui)( GLuint red, GLuint green,
                                    GLuint blue, GLuint alpha );
extern void (*glColor4us)( GLushort red, GLushort green,
                                    GLushort blue, GLushort alpha );


extern void (*glColor3bv)( const GLbyte *v );
extern void (*glColor3dv)( const GLdouble *v );
extern void (*glColor3fv)( const GLfloat *v );
extern void (*glColor3iv)( const GLint *v );
extern void (*glColor3sv)( const GLshort *v );
extern void (*glColor3ubv)( const GLubyte *v );
extern void (*glColor3uiv)( const GLuint *v );
extern void (*glColor3usv)( const GLushort *v );

extern void (*glColor4bv)( const GLbyte *v );
extern void (*glColor4dv)( const GLdouble *v );
extern void (*glColor4fv)( const GLfloat *v );
extern void (*glColor4iv)( const GLint *v );
extern void (*glColor4sv)( const GLshort *v );
extern void (*glColor4ubv)( const GLubyte *v );
extern void (*glColor4uiv)( const GLuint *v );
extern void (*glColor4usv)( const GLushort *v );


extern void (*glTexCoord1d)( GLdouble s );
extern void (*glTexCoord1f)( GLfloat s );
extern void (*glTexCoord1i)( GLint s );
extern void (*glTexCoord1s)( GLshort s );

extern void (*glTexCoord2d)( GLdouble s, GLdouble t );
extern void (*glTexCoord2f)( GLfloat s, GLfloat t );
extern void (*glTexCoord2i)( GLint s, GLint t );
extern void (*glTexCoord2s)( GLshort s, GLshort t );

extern void (*glTexCoord3d)( GLdouble s, GLdouble t, GLdouble r );
extern void (*glTexCoord3f)( GLfloat s, GLfloat t, GLfloat r );
extern void (*glTexCoord3i)( GLint s, GLint t, GLint r );
extern void (*glTexCoord3s)( GLshort s, GLshort t, GLshort r );

extern void (*glTexCoord4d)( GLdouble s, GLdouble t, GLdouble r, GLdouble q );
extern void (*glTexCoord4f)( GLfloat s, GLfloat t, GLfloat r, GLfloat q );
extern void (*glTexCoord4i)( GLint s, GLint t, GLint r, GLint q );
extern void (*glTexCoord4s)( GLshort s, GLshort t, GLshort r, GLshort q );

extern void (*glTexCoord1dv)( const GLdouble *v );
extern void (*glTexCoord1fv)( const GLfloat *v );
extern void (*glTexCoord1iv)( const GLint *v );
extern void (*glTexCoord1sv)( const GLshort *v );

extern void (*glTexCoord2dv)( const GLdouble *v );
extern void (*glTexCoord2fv)( const GLfloat *v );
extern void (*glTexCoord2iv)( const GLint *v );
extern void (*glTexCoord2sv)( const GLshort *v );

extern void (*glTexCoord3dv)( const GLdouble *v );
extern void (*glTexCoord3fv)( const GLfloat *v );
extern void (*glTexCoord3iv)( const GLint *v );
extern void (*glTexCoord3sv)( const GLshort *v );

extern void (*glTexCoord4dv)( const GLdouble *v );
extern void (*glTexCoord4fv)( const GLfloat *v );
extern void (*glTexCoord4iv)( const GLint *v );
extern void (*glTexCoord4sv)( const GLshort *v );


extern void (*glRasterPos2d)( GLdouble x, GLdouble y );
extern void (*glRasterPos2f)( GLfloat x, GLfloat y );
extern void (*glRasterPos2i)( GLint x, GLint y );
extern void (*glRasterPos2s)( GLshort x, GLshort y );

extern void (*glRasterPos3d)( GLdouble x, GLdouble y, GLdouble z );
extern void (*glRasterPos3f)( GLfloat x, GLfloat y, GLfloat z );
extern void (*glRasterPos3i)( GLint x, GLint y, GLint z );
extern void (*glRasterPos3s)( GLshort x, GLshort y, GLshort z );

extern void (*glRasterPos4d)( GLdouble x, GLdouble y, GLdouble z, GLdouble w );
extern void (*glRasterPos4f)( GLfloat x, GLfloat y, GLfloat z, GLfloat w );
extern void (*glRasterPos4i)( GLint x, GLint y, GLint z, GLint w );
extern void (*glRasterPos4s)( GLshort x, GLshort y, GLshort z, GLshort w );

extern void (*glRasterPos2dv)( const GLdouble *v );
extern void (*glRasterPos2fv)( const GLfloat *v );
extern void (*glRasterPos2iv)( const GLint *v );
extern void (*glRasterPos2sv)( const GLshort *v );

extern void (*glRasterPos3dv)( const GLdouble *v );
extern void (*glRasterPos3fv)( const GLfloat *v );
extern void (*glRasterPos3iv)( const GLint *v );
extern void (*glRasterPos3sv)( const GLshort *v );

extern void (*glRasterPos4dv)( const GLdouble *v );
extern void (*glRasterPos4fv)( const GLfloat *v );
extern void (*glRasterPos4iv)( const GLint *v );
extern void (*glRasterPos4sv)( const GLshort *v );


extern void (*glRectd)( GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2 );
extern void (*glRectf)( GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2 );
extern void (*glRecti)( GLint x1, GLint y1, GLint x2, GLint y2 );
extern void (*glRects)( GLshort x1, GLshort y1, GLshort x2, GLshort y2 );


extern void (*glRectdv)( const GLdouble *v1, const GLdouble *v2 );
extern void (*glRectfv)( const GLfloat *v1, const GLfloat *v2 );
extern void (*glRectiv)( const GLint *v1, const GLint *v2 );
extern void (*glRectsv)( const GLshort *v1, const GLshort *v2 );



/*
 * Vertex Arrays  (1.1)
 */

extern void (*glVertexPointer)( GLint size, GLenum type,
                                       GLsizei stride, const GLvoid *ptr );

extern void (*glNormalPointer)( GLenum type, GLsizei stride,
                                       const GLvoid *ptr );

extern void (*glColorPointer)( GLint size, GLenum type,
                                      GLsizei stride, const GLvoid *ptr );

extern void (*glIndexPointer)( GLenum type, GLsizei stride,
                                      const GLvoid *ptr );

extern void (*glTexCoordPointer)( GLint size, GLenum type,
                                         GLsizei stride, const GLvoid *ptr );

extern void (*glEdgeFlagPointer)( GLsizei stride, const GLvoid *ptr );

extern void (*glGetPointerv)( GLenum pname, void **params );

extern void (*glArrayElement)( GLint i );

extern void (*glDrawArrays)( GLenum mode, GLint first, GLsizei count );

extern void (*glDrawElements)( GLenum mode, GLsizei count,
                                      GLenum type, const GLvoid *indices );

extern void (*glInterleavedArrays)( GLenum format, GLsizei stride,
                                           const GLvoid *pointer );


/*
 * Lighting
 */

extern void (*glShadeModel)( GLenum mode );

extern void (*glLightf)( GLenum light, GLenum pname, GLfloat param );
extern void (*glLighti)( GLenum light, GLenum pname, GLint param );
extern void (*glLightfv)( GLenum light, GLenum pname,
                                 const GLfloat *params );
extern void (*glLightiv)( GLenum light, GLenum pname,
                                 const GLint *params );

extern void (*glGetLightfv)( GLenum light, GLenum pname,
                                    GLfloat *params );
extern void (*glGetLightiv)( GLenum light, GLenum pname,
                                    GLint *params );

extern void (*glLightModelf)( GLenum pname, GLfloat param );
extern void (*glLightModeli)( GLenum pname, GLint param );
extern void (*glLightModelfv)( GLenum pname, const GLfloat *params );
extern void (*glLightModeliv)( GLenum pname, const GLint *params );

extern void (*glMaterialf)( GLenum face, GLenum pname, GLfloat param );
extern void (*glMateriali)( GLenum face, GLenum pname, GLint param );
extern void (*glMaterialfv)( GLenum face, GLenum pname, const GLfloat *params );
extern void (*glMaterialiv)( GLenum face, GLenum pname, const GLint *params );

extern void (*glGetMaterialfv)( GLenum face, GLenum pname, GLfloat *params );
extern void (*glGetMaterialiv)( GLenum face, GLenum pname, GLint *params );

extern void (*glColorMaterial)( GLenum face, GLenum mode );




/*
 * Raster functions
 */

extern void (*glPixelZoom)( GLfloat xfactor, GLfloat yfactor );

extern void (*glPixelStoref)( GLenum pname, GLfloat param );
extern void (*glPixelStorei)( GLenum pname, GLint param );

extern void (*glPixelTransferf)( GLenum pname, GLfloat param );
extern void (*glPixelTransferi)( GLenum pname, GLint param );

extern void (*glPixelMapfv)( GLenum map, GLint mapsize,
                                    const GLfloat *values );
extern void (*glPixelMapuiv)( GLenum map, GLint mapsize,
                                     const GLuint *values );
extern void (*glPixelMapusv)( GLenum map, GLint mapsize,
                                     const GLushort *values );

extern void (*glGetPixelMapfv)( GLenum map, GLfloat *values );
extern void (*glGetPixelMapuiv)( GLenum map, GLuint *values );
extern void (*glGetPixelMapusv)( GLenum map, GLushort *values );

extern void (*glBitmap)( GLsizei width, GLsizei height,
                                GLfloat xorig, GLfloat yorig,
                                GLfloat xmove, GLfloat ymove,
                                const GLubyte *bitmap );

extern void (*glReadPixels)( GLint x, GLint y,
                                    GLsizei width, GLsizei height,
                                    GLenum format, GLenum type,
                                    GLvoid *pixels );

extern void (*glDrawPixels)( GLsizei width, GLsizei height,
                                    GLenum format, GLenum type,
                                    const GLvoid *pixels );

extern void (*glCopyPixels)( GLint x, GLint y,
                                    GLsizei width, GLsizei height,
                                    GLenum type );



/*
 * Stenciling
 */

extern void (*glStencilFunc)( GLenum func, GLint ref, GLuint mask );

extern void (*glStencilMask)( GLuint mask );

extern void (*glStencilOp)( GLenum fail, GLenum zfail, GLenum zpass );

extern void (*glClearStencil)( GLint s );



/*
 * Texture mapping
 */

extern void (*glTexGend)( GLenum coord, GLenum pname, GLdouble param );
extern void (*glTexGenf)( GLenum coord, GLenum pname, GLfloat param );
extern void (*glTexGeni)( GLenum coord, GLenum pname, GLint param );

extern void (*glTexGendv)( GLenum coord, GLenum pname, const GLdouble *params );
extern void (*glTexGenfv)( GLenum coord, GLenum pname, const GLfloat *params );
extern void (*glTexGeniv)( GLenum coord, GLenum pname, const GLint *params );

extern void (*glGetTexGendv)( GLenum coord, GLenum pname, GLdouble *params );
extern void (*glGetTexGenfv)( GLenum coord, GLenum pname, GLfloat *params );
extern void (*glGetTexGeniv)( GLenum coord, GLenum pname, GLint *params );


extern void (*glTexEnvf)( GLenum target, GLenum pname, GLfloat param );
extern void (*glTexEnvi)( GLenum target, GLenum pname, GLint param );

extern void (*glTexEnvfv)( GLenum target, GLenum pname, const GLfloat *params );
extern void (*glTexEnviv)( GLenum target, GLenum pname, const GLint *params );

extern void (*glGetTexEnvfv)( GLenum target, GLenum pname, GLfloat *params );
extern void (*glGetTexEnviv)( GLenum target, GLenum pname, GLint *params );


extern void (*glTexParameterf)( GLenum target, GLenum pname, GLfloat param );
extern void (*glTexParameteri)( GLenum target, GLenum pname, GLint param );

extern void (*glTexParameterfv)( GLenum target, GLenum pname,
                                          const GLfloat *params );
extern void (*glTexParameteriv)( GLenum target, GLenum pname,
                                          const GLint *params );

extern void (*glGetTexParameterfv)( GLenum target,
                                           GLenum pname, GLfloat *params);
extern void (*glGetTexParameteriv)( GLenum target,
                                           GLenum pname, GLint *params );

extern void (*glGetTexLevelParameterfv)( GLenum target, GLint level,
                                                GLenum pname, GLfloat *params );
extern void (*glGetTexLevelParameteriv)( GLenum target, GLint level,
                                                GLenum pname, GLint *params );


extern void (*glTexImage1D)( GLenum target, GLint level,
                                    GLint internalFormat,
                                    GLsizei width, GLint border,
                                    GLenum format, GLenum type,
                                    const GLvoid *pixels );

extern void (*glTexImage2D)( GLenum target, GLint level,
                                    GLint internalFormat,
                                    GLsizei width, GLsizei height,
                                    GLint border, GLenum format, GLenum type,
                                    const GLvoid *pixels );

extern void (*glGetTexImage)( GLenum target, GLint level,
                                     GLenum format, GLenum type,
                                     GLvoid *pixels );



/* 1.1 functions */

extern void (*glGenTextures)( GLsizei n, GLuint *textures );

extern void (*glDeleteTextures)( GLsizei n, const GLuint *textures);

extern void (*glBindTexture)( GLenum target, GLuint texture );

extern void (*glPrioritizeTextures)( GLsizei n,
                                            const GLuint *textures,
                                            const GLclampf *priorities );

extern GLboolean (*glAreTexturesResident)( GLsizei n,
                                                  const GLuint *textures,
                                                  GLboolean *residences );

extern GLboolean (*glIsTexture)( GLuint texture );


extern void (*glTexSubImage1D)( GLenum target, GLint level,
                                       GLint xoffset,
                                       GLsizei width, GLenum format,
                                       GLenum type, const GLvoid *pixels );


extern void (*glTexSubImage2D)( GLenum target, GLint level,
                                       GLint xoffset, GLint yoffset,
                                       GLsizei width, GLsizei height,
                                       GLenum format, GLenum type,
                                       const GLvoid *pixels );


extern void (*glCopyTexImage1D)( GLenum target, GLint level,
                                        GLenum internalformat,
                                        GLint x, GLint y,
                                        GLsizei width, GLint border );


extern void (*glCopyTexImage2D)( GLenum target, GLint level,
                                        GLenum internalformat,
                                        GLint x, GLint y,
                                        GLsizei width, GLsizei height,
                                        GLint border );


extern void (*glCopyTexSubImage1D)( GLenum target, GLint level,
                                           GLint xoffset, GLint x, GLint y,
                                           GLsizei width );


extern void (*glCopyTexSubImage2D)( GLenum target, GLint level,
                                           GLint xoffset, GLint yoffset,
                                           GLint x, GLint y,
                                           GLsizei width, GLsizei height );




/*
 * Evaluators
 */

extern void (*glMap1d)( GLenum target, GLdouble u1, GLdouble u2,
                               GLint stride,
                               GLint order, const GLdouble *points );
extern void (*glMap1f)( GLenum target, GLfloat u1, GLfloat u2,
                               GLint stride,
                               GLint order, const GLfloat *points );

extern void (*glMap2d)( GLenum target,
		     GLdouble u1, GLdouble u2, GLint ustride, GLint uorder,
		     GLdouble v1, GLdouble v2, GLint vstride, GLint vorder,
		     const GLdouble *points );
extern void (*glMap2f)( GLenum target,
		     GLfloat u1, GLfloat u2, GLint ustride, GLint uorder,
		     GLfloat v1, GLfloat v2, GLint vstride, GLint vorder,
		     const GLfloat *points );

extern void (*glGetMapdv)( GLenum target, GLenum query, GLdouble *v );
extern void (*glGetMapfv)( GLenum target, GLenum query, GLfloat *v );
extern void (*glGetMapiv)( GLenum target, GLenum query, GLint *v );

extern void (*glEvalCoord1d)( GLdouble u );
extern void (*glEvalCoord1f)( GLfloat u );

extern void (*glEvalCoord1dv)( const GLdouble *u );
extern void (*glEvalCoord1fv)( const GLfloat *u );

extern void (*glEvalCoord2d)( GLdouble u, GLdouble v );
extern void (*glEvalCoord2f)( GLfloat u, GLfloat v );

extern void (*glEvalCoord2dv)( const GLdouble *u );
extern void (*glEvalCoord2fv)( const GLfloat *u );

extern void (*glMapGrid1d)( GLint un, GLdouble u1, GLdouble u2 );
extern void (*glMapGrid1f)( GLint un, GLfloat u1, GLfloat u2 );

extern void (*glMapGrid2d)( GLint un, GLdouble u1, GLdouble u2,
                                   GLint vn, GLdouble v1, GLdouble v2 );
extern void (*glMapGrid2f)( GLint un, GLfloat u1, GLfloat u2,
                                   GLint vn, GLfloat v1, GLfloat v2 );

extern void (*glEvalPoint1)( GLint i );

extern void (*glEvalPoint2)( GLint i, GLint j );

extern void (*glEvalMesh1)( GLenum mode, GLint i1, GLint i2 );

extern void (*glEvalMesh2)( GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2 );



/*
 * Fog
 */

extern void (*glFogf)( GLenum pname, GLfloat param );

extern void (*glFogi)( GLenum pname, GLint param );

extern void (*glFogfv)( GLenum pname, const GLfloat *params );

extern void (*glFogiv)( GLenum pname, const GLint *params );



/*
 * Selection and Feedback
 */

extern void (*glFeedbackBuffer)( GLsizei size, GLenum type, GLfloat *buffer );

extern void (*glPassThrough)( GLfloat token );

extern void (*glSelectBuffer)( GLsizei size, GLuint *buffer );

extern void (*glInitNames)( void );

extern void (*glLoadName)( GLuint name );

extern void (*glPushName)( GLuint name );

extern void (*glPopName)( void );



/* 1.2 functions */
extern void (*glDrawRangeElements)( GLenum mode, GLuint start,
	GLuint end, GLsizei count, GLenum type, const GLvoid *indices );

extern void (*glTexImage3D)( GLenum target, GLint level,
                                      GLint internalFormat,
                                      GLsizei width, GLsizei height,
                                      GLsizei depth, GLint border,
                                      GLenum format, GLenum type,
                                      const GLvoid *pixels );

extern void (*glTexSubImage3D)( GLenum target, GLint level,
                                         GLint xoffset, GLint yoffset,
                                         GLint zoffset, GLsizei width,
                                         GLsizei height, GLsizei depth,
                                         GLenum format,
                                         GLenum type, const GLvoid *pixels);

extern void (*glCopyTexSubImage3D)( GLenum target, GLint level,
                                             GLint xoffset, GLint yoffset,
                                             GLint zoffset, GLint x,
                                             GLint y, GLsizei width,
                                             GLsizei height );


/* 1.2 imaging extension functions */

extern void (*glColorTable)( GLenum target, GLenum internalformat,
                                    GLsizei width, GLenum format,
                                    GLenum type, const GLvoid *table );

extern void (*glColorSubTable)( GLenum target,
                                       GLsizei start, GLsizei count,
                                       GLenum format, GLenum type,
                                       const GLvoid *data );

extern void (*glColorTableParameteriv)( GLenum target, GLenum pname,
                                              const GLint *params);

extern void (*glColorTableParameterfv)( GLenum target, GLenum pname,
                                              const GLfloat *params);

extern void (*glCopyColorSubTable)( GLenum target, GLsizei start,
                                           GLint x, GLint y, GLsizei width );

extern void (*glCopyColorTable)( GLenum target, GLenum internalformat,
                                        GLint x, GLint y, GLsizei width );

extern void (*glGetColorTable)( GLenum target, GLenum format,
                                       GLenum type, GLvoid *table );

extern void (*glGetColorTableParameterfv)( GLenum target, GLenum pname,
                                                  GLfloat *params );

extern void (*glGetColorTableParameteriv)( GLenum target, GLenum pname,
                                                  GLint *params );

extern void (*glBlendEquation)( GLenum mode );

extern void (*glBlendColor)( GLclampf red, GLclampf green,
                                    GLclampf blue, GLclampf alpha );

extern void (*glHistogram)( GLenum target, GLsizei width,
				   GLenum internalformat, GLboolean sink );

extern void (*glResetHistogram)( GLenum target );

extern void (*glGetHistogram)( GLenum target, GLboolean reset,
				      GLenum format, GLenum type,
				      GLvoid *values );

extern void (*glGetHistogramParameterfv)( GLenum target, GLenum pname,
						 GLfloat *params );

extern void (*glGetHistogramParameteriv)( GLenum target, GLenum pname,
						 GLint *params );

extern void (*glMinmax)( GLenum target, GLenum internalformat,
				GLboolean sink );

extern void (*glResetMinmax)( GLenum target );

extern void (*glGetMinmax)( GLenum target, GLboolean reset,
                                   GLenum format, GLenum types,
                                   GLvoid *values );

extern void (*glGetMinmaxParameterfv)( GLenum target, GLenum pname,
					      GLfloat *params );

extern void (*glGetMinmaxParameteriv)( GLenum target, GLenum pname,
					      GLint *params );

extern void (*glConvolutionFilter1D)( GLenum target,
	GLenum internalformat, GLsizei width, GLenum format, GLenum type,
	const GLvoid *image );

extern void (*glConvolutionFilter2D)( GLenum target,
	GLenum internalformat, GLsizei width, GLsizei height, GLenum format,
	GLenum type, const GLvoid *image );

extern void (*glConvolutionParameterf)( GLenum target, GLenum pname,
	GLfloat params );

extern void (*glConvolutionParameterfv)( GLenum target, GLenum pname,
	const GLfloat *params );

extern void (*glConvolutionParameteri)( GLenum target, GLenum pname,
	GLint params );

extern void (*glConvolutionParameteriv)( GLenum target, GLenum pname,
	const GLint *params );

extern void (*glCopyConvolutionFilter1D)( GLenum target,
	GLenum internalformat, GLint x, GLint y, GLsizei width );

extern void (*glCopyConvolutionFilter2D)( GLenum target,
	GLenum internalformat, GLint x, GLint y, GLsizei width,
	GLsizei height);

extern void (*glGetConvolutionFilter)( GLenum target, GLenum format,
	GLenum type, GLvoid *image );

extern void (*glGetConvolutionParameterfv)( GLenum target, GLenum pname,
	GLfloat *params );

extern void (*glGetConvolutionParameteriv)( GLenum target, GLenum pname,
	GLint *params );

extern void (*glSeparableFilter2D)( GLenum target,
	GLenum internalformat, GLsizei width, GLsizei height, GLenum format,
	GLenum type, const GLvoid *row, const GLvoid *column );

extern void (*glGetSeparableFilter)( GLenum target, GLenum format,
	GLenum type, GLvoid *row, GLvoid *column, GLvoid *span );



/*
 * GL_ARB_multitexture (ARB extension 1 and OpenGL 1.2.1)
 */
#ifndef GL_ARB_multitexture
#define GL_ARB_multitexture 1

#define GL_TEXTURE0_ARB				0x84C0
#define GL_TEXTURE1_ARB				0x84C1
#define GL_TEXTURE2_ARB				0x84C2
#define GL_TEXTURE3_ARB				0x84C3
#define GL_TEXTURE4_ARB				0x84C4
#define GL_TEXTURE5_ARB				0x84C5
#define GL_TEXTURE6_ARB				0x84C6
#define GL_TEXTURE7_ARB				0x84C7
#define GL_TEXTURE8_ARB				0x84C8
#define GL_TEXTURE9_ARB				0x84C9
#define GL_TEXTURE10_ARB			0x84CA
#define GL_TEXTURE11_ARB			0x84CB
#define GL_TEXTURE12_ARB			0x84CC
#define GL_TEXTURE13_ARB			0x84CD
#define GL_TEXTURE14_ARB			0x84CE
#define GL_TEXTURE15_ARB			0x84CF
#define GL_TEXTURE16_ARB			0x84D0
#define GL_TEXTURE17_ARB			0x84D1
#define GL_TEXTURE18_ARB			0x84D2
#define GL_TEXTURE19_ARB			0x84D3
#define GL_TEXTURE20_ARB			0x84D4
#define GL_TEXTURE21_ARB			0x84D5
#define GL_TEXTURE22_ARB			0x84D6
#define GL_TEXTURE23_ARB			0x84D7
#define GL_TEXTURE24_ARB			0x84D8
#define GL_TEXTURE25_ARB			0x84D9
#define GL_TEXTURE26_ARB			0x84DA
#define GL_TEXTURE27_ARB			0x84DB
#define GL_TEXTURE28_ARB			0x84DC
#define GL_TEXTURE29_ARB			0x84DD
#define GL_TEXTURE30_ARB			0x84DE
#define GL_TEXTURE31_ARB			0x84DF
#define GL_ACTIVE_TEXTURE_ARB			0x84E0
#define GL_CLIENT_ACTIVE_TEXTURE_ARB		0x84E1
#define GL_MAX_TEXTURE_UNITS_ARB		0x84E2

extern void (*glActiveTextureARB)( GLenum texture);
extern void (*glClientActiveTextureARB)( GLenum texture);
extern void (*glMultiTexCoord1dARB)( GLenum target, GLdouble s);
extern void (*glMultiTexCoord1dvARB)( GLenum target, const GLdouble *v);
extern void (*glMultiTexCoord1fARB)( GLenum target, GLfloat s);
extern void (*glMultiTexCoord1fvARB)( GLenum target, const GLfloat *v);
extern void (*glMultiTexCoord1iARB)( GLenum target, GLint s);
extern void (*glMultiTexCoord1ivARB)( GLenum target, const GLint *v);
extern void (*glMultiTexCoord1sARB)( GLenum target, GLshort s);
extern void (*glMultiTexCoord1svARB)( GLenum target, const GLshort *v);
extern void (*glMultiTexCoord2dARB)( GLenum target, GLdouble s, GLdouble t);
extern void (*glMultiTexCoord2dvARB)( GLenum target, const GLdouble *v);
extern void (*glMultiTexCoord2fARB)( GLenum target, GLfloat s, GLfloat t);
extern void (*glMultiTexCoord2fvARB)( GLenum target, const GLfloat *v);
extern void (*glMultiTexCoord2iARB)( GLenum target, GLint s, GLint t);
extern void (*glMultiTexCoord2ivARB)( GLenum target, const GLint *v);
extern void (*glMultiTexCoord2sARB)( GLenum target, GLshort s, GLshort t);
extern void (*glMultiTexCoord2svARB)( GLenum target, const GLshort *v);
extern void (*glMultiTexCoord3dARB)( GLenum target, GLdouble s, GLdouble t, GLdouble r);
extern void (*glMultiTexCoord3dvARB)( GLenum target, const GLdouble *v);
extern void (*glMultiTexCoord3fARB)( GLenum target, GLfloat s, GLfloat t, GLfloat r);
extern void (*glMultiTexCoord3fvARB)( GLenum target, const GLfloat *v);
extern void (*glMultiTexCoord3iARB)( GLenum target, GLint s, GLint t, GLint r);
extern void (*glMultiTexCoord3ivARB)( GLenum target, const GLint *v);
extern void (*glMultiTexCoord3sARB)( GLenum target, GLshort s, GLshort t, GLshort r);
extern void (*glMultiTexCoord3svARB)( GLenum target, const GLshort *v);
extern void (*glMultiTexCoord4dARB)( GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q);
extern void (*glMultiTexCoord4dvARB)( GLenum target, const GLdouble *v);
extern void (*glMultiTexCoord4fARB)( GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
extern void (*glMultiTexCoord4fvARB)( GLenum target, const GLfloat *v);
extern void (*glMultiTexCoord4iARB)( GLenum target, GLint s, GLint t, GLint r, GLint q);
extern void (*glMultiTexCoord4ivARB)( GLenum target, const GLint *v);
extern void (*glMultiTexCoord4sARB)( GLenum target, GLshort s, GLshort t, GLshort r, GLshort q);
extern void (*glMultiTexCoord4svARB)( GLenum target, const GLshort *v);

#endif /* GL_ARB_multitexture */


/*
 * GLU function pointers
 */
extern GLint (*gluProject)( GLdouble objx, GLdouble objy, GLdouble objz,
			    const GLdouble modelMatrix[16],
			    const GLdouble projMatrix[16],
			    const GLint viewport[4],
			    GLdouble *winx, GLdouble *winy,
			    GLdouble *winz );

extern GLint (*gluUnProject)( GLdouble winx, GLdouble winy,
			      GLdouble winz,
			      const GLdouble modelMatrix[16],
			      const GLdouble projMatrix[16],
			      const GLint viewport[4],
			      GLdouble *objx, GLdouble *objy,
			      GLdouble *objz );

/*
 * GLext
 */
#ifndef GL_ARB_transpose_matrix
#define GL_TRANSPOSE_MODELVIEW_MATRIX_ARB 0x84E3
#define GL_TRANSPOSE_PROJECTION_MATRIX_ARB 0x84E4
#define GL_TRANSPOSE_TEXTURE_MATRIX_ARB   0x84E5
#define GL_TRANSPOSE_COLOR_MATRIX_ARB     0x84E6
#endif

#ifndef GL_ARB_multisample
#define GL_MULTISAMPLE_ARB                0x809D
#define GL_SAMPLE_ALPHA_TO_COVERAGE_ARB   0x809E
#define GL_SAMPLE_ALPHA_TO_ONE_ARB        0x809F
#define GL_SAMPLE_COVERAGE_ARB            0x80A0
#define GL_SAMPLE_BUFFERS_ARB             0x80A8
#define GL_SAMPLES_ARB                    0x80A9
#define GL_SAMPLE_COVERAGE_VALUE_ARB      0x80AA
#define GL_SAMPLE_COVERAGE_INVERT_ARB     0x80AB
#define GL_MULTISAMPLE_BIT_ARB            0x20000000
#endif

#ifndef GL_ARB_texture_cube_map
#define GL_NORMAL_MAP_ARB                 0x8511
#define GL_REFLECTION_MAP_ARB             0x8512
#define GL_TEXTURE_CUBE_MAP_ARB           0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP_ARB   0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB 0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB 0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB 0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB 0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB 0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB 0x851A
#define GL_PROXY_TEXTURE_CUBE_MAP_ARB     0x851B
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB  0x851C
#endif

#ifndef GL_ARB_texture_compression
#define GL_COMPRESSED_ALPHA_ARB           0x84E9
#define GL_COMPRESSED_LUMINANCE_ARB       0x84EA
#define GL_COMPRESSED_LUMINANCE_ALPHA_ARB 0x84EB
#define GL_COMPRESSED_INTENSITY_ARB       0x84EC
#define GL_COMPRESSED_RGB_ARB             0x84ED
#define GL_COMPRESSED_RGBA_ARB            0x84EE
#define GL_TEXTURE_COMPRESSION_HINT_ARB   0x84EF
#define GL_TEXTURE_COMPRESSED_IMAGE_SIZE_ARB         0x86A0
#define GL_TEXTURE_COMPRESSED_ARB         0x86A1
#define GL_NUM_COMPRESSED_TEXTURE_FORMATS_ARB 0x86A2
#define GL_COMPRESSED_TEXTURE_FORMATS_ARB 0x86A3
#endif

#ifndef GL_EXT_abgr
#define GL_ABGR_EXT                       0x8000
#endif

#ifndef GL_EXT_blend_color
#define GL_CONSTANT_COLOR_EXT             0x8001
#define GL_ONE_MINUS_CONSTANT_COLOR_EXT   0x8002
#define GL_CONSTANT_ALPHA_EXT             0x8003
#define GL_ONE_MINUS_CONSTANT_ALPHA_EXT   0x8004
#define GL_BLEND_COLOR_EXT                0x8005
#endif

#ifndef GL_EXT_polygon_offset
#define GL_POLYGON_OFFSET_EXT             0x8037
#define GL_POLYGON_OFFSET_FACTOR_EXT      0x8038
#define GL_POLYGON_OFFSET_BIAS_EXT        0x8039
#endif

#ifndef GL_EXT_texture
#define GL_ALPHA4_EXT                     0x803B
#define GL_ALPHA8_EXT                     0x803C
#define GL_ALPHA12_EXT                    0x803D
#define GL_ALPHA16_EXT                    0x803E
#define GL_LUMINANCE4_EXT                 0x803F
#define GL_LUMINANCE8_EXT                 0x8040
#define GL_LUMINANCE12_EXT                0x8041
#define GL_LUMINANCE16_EXT                0x8042
#define GL_LUMINANCE4_ALPHA4_EXT          0x8043
#define GL_LUMINANCE6_ALPHA2_EXT          0x8044
#define GL_LUMINANCE8_ALPHA8_EXT          0x8045
#define GL_LUMINANCE12_ALPHA4_EXT         0x8046
#define GL_LUMINANCE12_ALPHA12_EXT        0x8047
#define GL_LUMINANCE16_ALPHA16_EXT        0x8048
#define GL_INTENSITY_EXT                  0x8049
#define GL_INTENSITY4_EXT                 0x804A
#define GL_INTENSITY8_EXT                 0x804B
#define GL_INTENSITY12_EXT                0x804C
#define GL_INTENSITY16_EXT                0x804D
#define GL_RGB2_EXT                       0x804E
#define GL_RGB4_EXT                       0x804F
#define GL_RGB5_EXT                       0x8050
#define GL_RGB8_EXT                       0x8051
#define GL_RGB10_EXT                      0x8052
#define GL_RGB12_EXT                      0x8053
#define GL_RGB16_EXT                      0x8054
#define GL_RGBA2_EXT                      0x8055
#define GL_RGBA4_EXT                      0x8056
#define GL_RGB5_A1_EXT                    0x8057
#define GL_RGBA8_EXT                      0x8058
#define GL_RGB10_A2_EXT                   0x8059
#define GL_RGBA12_EXT                     0x805A
#define GL_RGBA16_EXT                     0x805B
#define GL_TEXTURE_RED_SIZE_EXT           0x805C
#define GL_TEXTURE_GREEN_SIZE_EXT         0x805D
#define GL_TEXTURE_BLUE_SIZE_EXT          0x805E
#define GL_TEXTURE_ALPHA_SIZE_EXT         0x805F
#define GL_TEXTURE_LUMINANCE_SIZE_EXT     0x8060
#define GL_TEXTURE_INTENSITY_SIZE_EXT     0x8061
#define GL_REPLACE_EXT                    0x8062
#define GL_PROXY_TEXTURE_1D_EXT           0x8063
#define GL_PROXY_TEXTURE_2D_EXT           0x8064
#define GL_TEXTURE_TOO_LARGE_EXT          0x8065
#endif

#ifndef GL_EXT_texture3D
#define GL_PACK_SKIP_IMAGES               0x806B
#define GL_PACK_SKIP_IMAGES_EXT           0x806B
#define GL_PACK_IMAGE_HEIGHT              0x806C
#define GL_PACK_IMAGE_HEIGHT_EXT          0x806C
#define GL_UNPACK_SKIP_IMAGES             0x806D
#define GL_UNPACK_SKIP_IMAGES_EXT         0x806D
#define GL_UNPACK_IMAGE_HEIGHT            0x806E
#define GL_UNPACK_IMAGE_HEIGHT_EXT        0x806E
#define GL_TEXTURE_3D                     0x806F
#define GL_TEXTURE_3D_EXT                 0x806F
#define GL_PROXY_TEXTURE_3D               0x8070
#define GL_PROXY_TEXTURE_3D_EXT           0x8070
#define GL_TEXTURE_DEPTH                  0x8071
#define GL_TEXTURE_DEPTH_EXT              0x8071
#define GL_TEXTURE_WRAP_R                 0x8072
#define GL_TEXTURE_WRAP_R_EXT             0x8072
#define GL_MAX_3D_TEXTURE_SIZE            0x8073
#define GL_MAX_3D_TEXTURE_SIZE_EXT        0x8073
#endif

#ifndef GL_SGIS_texture_filter4
#define GL_FILTER4_SGIS                   0x8146
#define GL_TEXTURE_FILTER4_SIZE_SGIS      0x8147
#endif

#ifndef GL_EXT_subtexture
#endif

#ifndef GL_EXT_copy_texture
#endif

#ifndef GL_EXT_histogram
#define GL_HISTOGRAM_EXT                  0x8024
#define GL_PROXY_HISTOGRAM_EXT            0x8025
#define GL_HISTOGRAM_WIDTH_EXT            0x8026
#define GL_HISTOGRAM_FORMAT_EXT           0x8027
#define GL_HISTOGRAM_RED_SIZE_EXT         0x8028
#define GL_HISTOGRAM_GREEN_SIZE_EXT       0x8029
#define GL_HISTOGRAM_BLUE_SIZE_EXT        0x802A
#define GL_HISTOGRAM_ALPHA_SIZE_EXT       0x802B
#define GL_HISTOGRAM_LUMINANCE_SIZE_EXT   0x802C
#define GL_HISTOGRAM_SINK_EXT             0x802D
#define GL_MINMAX_EXT                     0x802E
#define GL_MINMAX_FORMAT_EXT              0x802F
#define GL_MINMAX_SINK_EXT                0x8030
#define GL_TABLE_TOO_LARGE_EXT            0x8031
#endif

#ifndef GL_EXT_convolution
#define GL_CONVOLUTION_1D_EXT             0x8010
#define GL_CONVOLUTION_2D_EXT             0x8011
#define GL_SEPARABLE_2D_EXT               0x8012
#define GL_CONVOLUTION_BORDER_MODE_EXT    0x8013
#define GL_CONVOLUTION_FILTER_SCALE_EXT   0x8014
#define GL_CONVOLUTION_FILTER_BIAS_EXT    0x8015
#define GL_REDUCE_EXT                     0x8016
#define GL_CONVOLUTION_FORMAT_EXT         0x8017
#define GL_CONVOLUTION_WIDTH_EXT          0x8018
#define GL_CONVOLUTION_HEIGHT_EXT         0x8019
#define GL_MAX_CONVOLUTION_WIDTH_EXT      0x801A
#define GL_MAX_CONVOLUTION_HEIGHT_EXT     0x801B
#define GL_POST_CONVOLUTION_RED_SCALE_EXT 0x801C
#define GL_POST_CONVOLUTION_GREEN_SCALE_EXT 0x801D
#define GL_POST_CONVOLUTION_BLUE_SCALE_EXT 0x801E
#define GL_POST_CONVOLUTION_ALPHA_SCALE_EXT 0x801F
#define GL_POST_CONVOLUTION_RED_BIAS_EXT  0x8020
#define GL_POST_CONVOLUTION_GREEN_BIAS_EXT 0x8021
#define GL_POST_CONVOLUTION_BLUE_BIAS_EXT 0x8022
#define GL_POST_CONVOLUTION_ALPHA_BIAS_EXT 0x8023
#endif

#ifndef GL_SGI_color_matrix
#define GL_COLOR_MATRIX_SGI               0x80B1
#define GL_COLOR_MATRIX_STACK_DEPTH_SGI   0x80B2
#define GL_MAX_COLOR_MATRIX_STACK_DEPTH_SGI 0x80B3
#define GL_POST_COLOR_MATRIX_RED_SCALE_SGI 0x80B4
#define GL_POST_COLOR_MATRIX_GREEN_SCALE_SGI 0x80B5
#define GL_POST_COLOR_MATRIX_BLUE_SCALE_SGI 0x80B6
#define GL_POST_COLOR_MATRIX_ALPHA_SCALE_SGI 0x80B7
#define GL_POST_COLOR_MATRIX_RED_BIAS_SGI 0x80B8
#define GL_POST_COLOR_MATRIX_GREEN_BIAS_SGI 0x80B9
#define GL_POST_COLOR_MATRIX_BLUE_BIAS_SGI 0x80BA
#define GL_POST_COLOR_MATRIX_ALPHA_BIAS_SGI 0x80BB
#endif

#ifndef GL_SGI_color_table
#define GL_COLOR_TABLE_SGI                0x80D0
#define GL_POST_CONVOLUTION_COLOR_TABLE_SGI 0x80D1
#define GL_POST_COLOR_MATRIX_COLOR_TABLE_SGI 0x80D2
#define GL_PROXY_COLOR_TABLE_SGI          0x80D3
#define GL_PROXY_POST_CONVOLUTION_COLOR_TABLE_SGI 0x80D4
#define GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE_SGI 0x80D5
#define GL_COLOR_TABLE_SCALE_SGI          0x80D6
#define GL_COLOR_TABLE_BIAS_SGI           0x80D7
#define GL_COLOR_TABLE_FORMAT_SGI         0x80D8
#define GL_COLOR_TABLE_WIDTH_SGI          0x80D9
#define GL_COLOR_TABLE_RED_SIZE_SGI       0x80DA
#define GL_COLOR_TABLE_GREEN_SIZE_SGI     0x80DB
#define GL_COLOR_TABLE_BLUE_SIZE_SGI      0x80DC
#define GL_COLOR_TABLE_ALPHA_SIZE_SGI     0x80DD
#define GL_COLOR_TABLE_LUMINANCE_SIZE_SGI 0x80DE
#define GL_COLOR_TABLE_INTENSITY_SIZE_SGI 0x80DF
#endif

#ifndef GL_SGIS_pixel_texture
#define GL_PIXEL_TEXTURE_SGIS             0x8353
#define GL_PIXEL_FRAGMENT_RGB_SOURCE_SGIS 0x8354
#define GL_PIXEL_FRAGMENT_ALPHA_SOURCE_SGIS 0x8355
#define GL_PIXEL_GROUP_COLOR_SGIS         0x8356
#endif

#ifndef GL_SGIX_pixel_texture
#define GL_PIXEL_TEX_GEN_SGIX             0x8139
#define GL_PIXEL_TEX_GEN_MODE_SGIX        0x832B
#endif

#ifndef GL_SGIS_texture4D
#define GL_PACK_SKIP_VOLUMES_SGIS         0x8130
#define GL_PACK_IMAGE_DEPTH_SGIS          0x8131
#define GL_UNPACK_SKIP_VOLUMES_SGIS       0x8132
#define GL_UNPACK_IMAGE_DEPTH_SGIS        0x8133
#define GL_TEXTURE_4D_SGIS                0x8134
#define GL_PROXY_TEXTURE_4D_SGIS          0x8135
#define GL_TEXTURE_4DSIZE_SGIS            0x8136
#define GL_TEXTURE_WRAP_Q_SGIS            0x8137
#define GL_MAX_4D_TEXTURE_SIZE_SGIS       0x8138
#define GL_TEXTURE_4D_BINDING_SGIS        0x814F
#endif

#ifndef GL_SGI_texture_color_table
#define GL_TEXTURE_COLOR_TABLE_SGI        0x80BC
#define GL_PROXY_TEXTURE_COLOR_TABLE_SGI  0x80BD
#endif

#ifndef GL_EXT_cmyka
#define GL_CMYK_EXT                       0x800C
#define GL_CMYKA_EXT                      0x800D
#define GL_PACK_CMYK_HINT_EXT             0x800E
#define GL_UNPACK_CMYK_HINT_EXT           0x800F
#endif

#ifndef GL_EXT_texture_object
#define GL_TEXTURE_PRIORITY_EXT           0x8066
#define GL_TEXTURE_RESIDENT_EXT           0x8067
#define GL_TEXTURE_1D_BINDING_EXT         0x8068
#define GL_TEXTURE_2D_BINDING_EXT         0x8069
#define GL_TEXTURE_3D_BINDING_EXT         0x806A
#endif

#ifndef GL_SGIS_detail_texture
#define GL_DETAIL_TEXTURE_2D_SGIS         0x8095
#define GL_DETAIL_TEXTURE_2D_BINDING_SGIS 0x8096
#define GL_LINEAR_DETAIL_SGIS             0x8097
#define GL_LINEAR_DETAIL_ALPHA_SGIS       0x8098
#define GL_LINEAR_DETAIL_COLOR_SGIS       0x8099
#define GL_DETAIL_TEXTURE_LEVEL_SGIS      0x809A
#define GL_DETAIL_TEXTURE_MODE_SGIS       0x809B
#define GL_DETAIL_TEXTURE_FUNC_POINTS_SGIS 0x809C
#endif

#ifndef GL_SGIS_sharpen_texture
#define GL_LINEAR_SHARPEN_SGIS            0x80AD
#define GL_LINEAR_SHARPEN_ALPHA_SGIS      0x80AE
#define GL_LINEAR_SHARPEN_COLOR_SGIS      0x80AF
#define GL_SHARPEN_TEXTURE_FUNC_POINTS_SGIS 0x80B0
#endif

#ifndef GL_EXT_packed_pixels
#define GL_UNSIGNED_BYTE_3_3_2_EXT        0x8032
#define GL_UNSIGNED_SHORT_4_4_4_4_EXT     0x8033
#define GL_UNSIGNED_SHORT_5_5_5_1_EXT     0x8034
#define GL_UNSIGNED_INT_8_8_8_8_EXT       0x8035
#define GL_UNSIGNED_INT_10_10_10_2_EXT    0x8036
#endif

#ifndef GL_SGIS_texture_lod
#define GL_TEXTURE_MIN_LOD_SGIS           0x813A
#define GL_TEXTURE_MAX_LOD_SGIS           0x813B
#define GL_TEXTURE_BASE_LEVEL_SGIS        0x813C
#define GL_TEXTURE_MAX_LEVEL_SGIS         0x813D
#endif

#ifndef GL_SGIS_multisample
#define GL_MULTISAMPLE_SGIS               0x809D
#define GL_SAMPLE_ALPHA_TO_MASK_SGIS      0x809E
#define GL_SAMPLE_ALPHA_TO_ONE_SGIS       0x809F
#define GL_SAMPLE_MASK_SGIS               0x80A0
#define GL_1PASS_SGIS                     0x80A1
#define GL_2PASS_0_SGIS                   0x80A2
#define GL_2PASS_1_SGIS                   0x80A3
#define GL_4PASS_0_SGIS                   0x80A4
#define GL_4PASS_1_SGIS                   0x80A5
#define GL_4PASS_2_SGIS                   0x80A6
#define GL_4PASS_3_SGIS                   0x80A7
#define GL_SAMPLE_BUFFERS_SGIS            0x80A8
#define GL_SAMPLES_SGIS                   0x80A9
#define GL_SAMPLE_MASK_VALUE_SGIS         0x80AA
#define GL_SAMPLE_MASK_INVERT_SGIS        0x80AB
#define GL_SAMPLE_PATTERN_SGIS            0x80AC
#endif

#ifndef GL_EXT_rescale_normal
#define GL_RESCALE_NORMAL_EXT             0x803A
#endif

#ifndef GL_EXT_vertex_array
#define GL_VERTEX_ARRAY_EXT               0x8074
#define GL_NORMAL_ARRAY_EXT               0x8075
#define GL_COLOR_ARRAY_EXT                0x8076
#define GL_INDEX_ARRAY_EXT                0x8077
#define GL_TEXTURE_COORD_ARRAY_EXT        0x8078
#define GL_EDGE_FLAG_ARRAY_EXT            0x8079
#define GL_VERTEX_ARRAY_SIZE_EXT          0x807A
#define GL_VERTEX_ARRAY_TYPE_EXT          0x807B
#define GL_VERTEX_ARRAY_STRIDE_EXT        0x807C
#define GL_VERTEX_ARRAY_COUNT_EXT         0x807D
#define GL_NORMAL_ARRAY_TYPE_EXT          0x807E
#define GL_NORMAL_ARRAY_STRIDE_EXT        0x807F
#define GL_NORMAL_ARRAY_COUNT_EXT         0x8080
#define GL_COLOR_ARRAY_SIZE_EXT           0x8081
#define GL_COLOR_ARRAY_TYPE_EXT           0x8082
#define GL_COLOR_ARRAY_STRIDE_EXT         0x8083
#define GL_COLOR_ARRAY_COUNT_EXT          0x8084
#define GL_INDEX_ARRAY_TYPE_EXT           0x8085
#define GL_INDEX_ARRAY_STRIDE_EXT         0x8086
#define GL_INDEX_ARRAY_COUNT_EXT          0x8087
#define GL_TEXTURE_COORD_ARRAY_SIZE_EXT   0x8088
#define GL_TEXTURE_COORD_ARRAY_TYPE_EXT   0x8089
#define GL_TEXTURE_COORD_ARRAY_STRIDE_EXT 0x808A
#define GL_TEXTURE_COORD_ARRAY_COUNT_EXT  0x808B
#define GL_EDGE_FLAG_ARRAY_STRIDE_EXT     0x808C
#define GL_EDGE_FLAG_ARRAY_COUNT_EXT      0x808D
#define GL_VERTEX_ARRAY_POINTER_EXT       0x808E
#define GL_NORMAL_ARRAY_POINTER_EXT       0x808F
#define GL_COLOR_ARRAY_POINTER_EXT        0x8090
#define GL_INDEX_ARRAY_POINTER_EXT        0x8091
#define GL_TEXTURE_COORD_ARRAY_POINTER_EXT 0x8092
#define GL_EDGE_FLAG_ARRAY_POINTER_EXT    0x8093
#endif

#ifndef GL_EXT_misc_attribute
#endif

#ifndef GL_SGIS_generate_mipmap
#define GL_GENERATE_MIPMAP_SGIS           0x8191
#define GL_GENERATE_MIPMAP_HINT_SGIS      0x8192
#endif

#ifndef GL_SGIX_clipmap
#define GL_LINEAR_CLIPMAP_LINEAR_SGIX     0x8170
#define GL_TEXTURE_CLIPMAP_CENTER_SGIX    0x8171
#define GL_TEXTURE_CLIPMAP_FRAME_SGIX     0x8172
#define GL_TEXTURE_CLIPMAP_OFFSET_SGIX    0x8173
#define GL_TEXTURE_CLIPMAP_VIRTUAL_DEPTH_SGIX 0x8174
#define GL_TEXTURE_CLIPMAP_LOD_OFFSET_SGIX 0x8175
#define GL_TEXTURE_CLIPMAP_DEPTH_SGIX     0x8176
#define GL_MAX_CLIPMAP_DEPTH_SGIX         0x8177
#define GL_MAX_CLIPMAP_VIRTUAL_DEPTH_SGIX 0x8178
#define GL_NEAREST_CLIPMAP_NEAREST_SGIX   0x844D
#define GL_NEAREST_CLIPMAP_LINEAR_SGIX    0x844E
#define GL_LINEAR_CLIPMAP_NEAREST_SGIX    0x844F
#endif

#ifndef GL_SGIX_shadow
#define GL_TEXTURE_COMPARE_SGIX           0x819A
#define GL_TEXTURE_COMPARE_OPERATOR_SGIX  0x819B
#define GL_TEXTURE_LEQUAL_R_SGIX          0x819C
#define GL_TEXTURE_GEQUAL_R_SGIX          0x819D
#endif

#ifndef GL_SGIS_texture_edge_clamp
#define GL_CLAMP_TO_EDGE_SGIS             0x812F
#endif

#ifndef GL_SGIS_texture_border_clamp
#define GL_CLAMP_TO_BORDER_SGIS           0x812D
#endif

#ifndef GL_EXT_blend_minmax
#define GL_FUNC_ADD_EXT                   0x8006
#define GL_MIN_EXT                        0x8007
#define GL_MAX_EXT                        0x8008
#define GL_BLEND_EQUATION_EXT             0x8009
#endif

#ifndef GL_EXT_blend_subtract
#define GL_FUNC_SUBTRACT_EXT              0x800A
#define GL_FUNC_REVERSE_SUBTRACT_EXT      0x800B
#endif

#ifndef GL_EXT_blend_logic_op
#endif

#ifndef GL_SGIX_interlace
#define GL_INTERLACE_SGIX                 0x8094
#endif

#ifndef GL_SGIX_pixel_tiles
#define GL_PIXEL_TILE_BEST_ALIGNMENT_SGIX 0x813E
#define GL_PIXEL_TILE_CACHE_INCREMENT_SGIX 0x813F
#define GL_PIXEL_TILE_WIDTH_SGIX          0x8140
#define GL_PIXEL_TILE_HEIGHT_SGIX         0x8141
#define GL_PIXEL_TILE_GRID_WIDTH_SGIX     0x8142
#define GL_PIXEL_TILE_GRID_HEIGHT_SGIX    0x8143
#define GL_PIXEL_TILE_GRID_DEPTH_SGIX     0x8144
#define GL_PIXEL_TILE_CACHE_SIZE_SGIX     0x8145
#endif

#ifndef GL_SGIS_texture_select
#define GL_DUAL_ALPHA4_SGIS               0x8110
#define GL_DUAL_ALPHA8_SGIS               0x8111
#define GL_DUAL_ALPHA12_SGIS              0x8112
#define GL_DUAL_ALPHA16_SGIS              0x8113
#define GL_DUAL_LUMINANCE4_SGIS           0x8114
#define GL_DUAL_LUMINANCE8_SGIS           0x8115
#define GL_DUAL_LUMINANCE12_SGIS          0x8116
#define GL_DUAL_LUMINANCE16_SGIS          0x8117
#define GL_DUAL_INTENSITY4_SGIS           0x8118
#define GL_DUAL_INTENSITY8_SGIS           0x8119
#define GL_DUAL_INTENSITY12_SGIS          0x811A
#define GL_DUAL_INTENSITY16_SGIS          0x811B
#define GL_DUAL_LUMINANCE_ALPHA4_SGIS     0x811C
#define GL_DUAL_LUMINANCE_ALPHA8_SGIS     0x811D
#define GL_QUAD_ALPHA4_SGIS               0x811E
#define GL_QUAD_ALPHA8_SGIS               0x811F
#define GL_QUAD_LUMINANCE4_SGIS           0x8120
#define GL_QUAD_LUMINANCE8_SGIS           0x8121
#define GL_QUAD_INTENSITY4_SGIS           0x8122
#define GL_QUAD_INTENSITY8_SGIS           0x8123
#define GL_DUAL_TEXTURE_SELECT_SGIS       0x8124
#define GL_QUAD_TEXTURE_SELECT_SGIS       0x8125
#endif

#ifndef GL_SGIX_sprite
#define GL_SPRITE_SGIX                    0x8148
#define GL_SPRITE_MODE_SGIX               0x8149
#define GL_SPRITE_AXIS_SGIX               0x814A
#define GL_SPRITE_TRANSLATION_SGIX        0x814B
#define GL_SPRITE_AXIAL_SGIX              0x814C
#define GL_SPRITE_OBJECT_ALIGNED_SGIX     0x814D
#define GL_SPRITE_EYE_ALIGNED_SGIX        0x814E
#endif

#ifndef GL_SGIX_texture_multi_buffer
#define GL_TEXTURE_MULTI_BUFFER_HINT_SGIX 0x812E
#endif

#ifndef GL_SGIS_point_parameters
#define GL_POINT_SIZE_MIN_EXT             0x8126
#define GL_POINT_SIZE_MIN_SGIS            0x8126
#define GL_POINT_SIZE_MAX_EXT             0x8127
#define GL_POINT_SIZE_MAX_SGIS            0x8127
#define GL_POINT_FADE_THRESHOLD_SIZE_EXT  0x8128
#define GL_POINT_FADE_THRESHOLD_SIZE_SGIS 0x8128
#define GL_DISTANCE_ATTENUATION_EXT       0x8129
#define GL_DISTANCE_ATTENUATION_SGIS      0x8129
#endif

#ifndef GL_SGIX_instruments
#define GL_INSTRUMENT_BUFFER_POINTER_SGIX 0x8180
#define GL_INSTRUMENT_MEASUREMENTS_SGIX   0x8181
#endif

#ifndef GL_SGIX_texture_scale_bias
#define GL_POST_TEXTURE_FILTER_BIAS_SGIX  0x8179
#define GL_POST_TEXTURE_FILTER_SCALE_SGIX 0x817A
#define GL_POST_TEXTURE_FILTER_BIAS_RANGE_SGIX 0x817B
#define GL_POST_TEXTURE_FILTER_SCALE_RANGE_SGIX 0x817C
#endif

#ifndef GL_SGIX_framezoom
#define GL_FRAMEZOOM_SGIX                 0x818B
#define GL_FRAMEZOOM_FACTOR_SGIX          0x818C
#define GL_MAX_FRAMEZOOM_FACTOR_SGIX      0x818D
#endif

#ifndef GL_SGIX_tag_sample_buffer
#endif

#ifndef GL_SGIX_reference_plane
#define GL_REFERENCE_PLANE_SGIX           0x817D
#define GL_REFERENCE_PLANE_EQUATION_SGIX  0x817E
#endif

#ifndef GL_SGIX_flush_raster
#endif

#ifndef GL_SGIX_depth_texture
#define GL_DEPTH_COMPONENT16_SGIX         0x81A5
#define GL_DEPTH_COMPONENT24_SGIX         0x81A6
#define GL_DEPTH_COMPONENT32_SGIX         0x81A7
#endif

#ifndef GL_SGIS_fog_function
#define GL_FOG_FUNC_SGIS                  0x812A
#define GL_FOG_FUNC_POINTS_SGIS           0x812B
#define GL_MAX_FOG_FUNC_POINTS_SGIS       0x812C
#endif

#ifndef GL_SGIX_fog_offset
#define GL_FOG_OFFSET_SGIX                0x8198
#define GL_FOG_OFFSET_VALUE_SGIX          0x8199
#endif

#ifndef GL_HP_image_transform
#define GL_IMAGE_SCALE_X_HP               0x8155
#define GL_IMAGE_SCALE_Y_HP               0x8156
#define GL_IMAGE_TRANSLATE_X_HP           0x8157
#define GL_IMAGE_TRANSLATE_Y_HP           0x8158
#define GL_IMAGE_ROTATE_ANGLE_HP          0x8159
#define GL_IMAGE_ROTATE_ORIGIN_X_HP       0x815A
#define GL_IMAGE_ROTATE_ORIGIN_Y_HP       0x815B
#define GL_IMAGE_MAG_FILTER_HP            0x815C
#define GL_IMAGE_MIN_FILTER_HP            0x815D
#define GL_IMAGE_CUBIC_WEIGHT_HP          0x815E
#define GL_CUBIC_HP                       0x815F
#define GL_AVERAGE_HP                     0x8160
#define GL_IMAGE_TRANSFORM_2D_HP          0x8161
#define GL_POST_IMAGE_TRANSFORM_COLOR_TABLE_HP 0x8162
#define GL_PROXY_POST_IMAGE_TRANSFORM_COLOR_TABLE_HP 0x8163
#endif

#ifndef GL_HP_convolution_border_modes
#define GL_IGNORE_BORDER_HP               0x8150
#define GL_CONSTANT_BORDER_HP             0x8151
#define GL_REPLICATE_BORDER_HP            0x8153
#define GL_CONVOLUTION_BORDER_COLOR_HP    0x8154
#endif

#ifndef GL_INGR_palette_buffer
#endif

#ifndef GL_SGIX_texture_add_env
#define GL_TEXTURE_ENV_BIAS_SGIX          0x80BE
#endif

#ifndef GL_EXT_color_subtable
#endif

#ifndef GL_PGI_vertex_hints
#define GL_VERTEX_DATA_HINT_PGI           0x1A22A
#define GL_VERTEX_CONSISTENT_HINT_PGI     0x1A22B
#define GL_MATERIAL_SIDE_HINT_PGI         0x1A22C
#define GL_MAX_VERTEX_HINT_PGI            0x1A22D
#define GL_COLOR3_BIT_PGI                 0x00010000
#define GL_COLOR4_BIT_PGI                 0x00020000
#define GL_EDGEFLAG_BIT_PGI               0x00040000
#define GL_INDEX_BIT_PGI                  0x00080000
#define GL_MAT_AMBIENT_BIT_PGI            0x00100000
#define GL_MAT_AMBIENT_AND_DIFFUSE_BIT_PGI 0x00200000
#define GL_MAT_DIFFUSE_BIT_PGI            0x00400000
#define GL_MAT_EMISSION_BIT_PGI           0x00800000
#define GL_MAT_COLOR_INDEXES_BIT_PGI      0x01000000
#define GL_MAT_SHININESS_BIT_PGI          0x02000000
#define GL_MAT_SPECULAR_BIT_PGI           0x04000000
#define GL_NORMAL_BIT_PGI                 0x08000000
#define GL_TEXCOORD1_BIT_PGI              0x10000000
#define GL_TEXCOORD2_BIT_PGI              0x20000000
#define GL_TEXCOORD3_BIT_PGI              0x40000000
#define GL_TEXCOORD4_BIT_PGI              0x80000000
#define GL_VERTEX23_BIT_PGI               0x00000004
#define GL_VERTEX4_BIT_PGI                0x00000008
#endif

#ifndef GL_PGI_misc_hints
#define GL_PREFER_DOUBLEBUFFER_HINT_PGI   0x1A1F8
#define GL_CONSERVE_MEMORY_HINT_PGI       0x1A1FD
#define GL_RECLAIM_MEMORY_HINT_PGI        0x1A1FE
#define GL_NATIVE_GRAPHICS_HANDLE_PGI     0x1A202
#define GL_NATIVE_GRAPHICS_BEGIN_HINT_PGI 0x1A203
#define GL_NATIVE_GRAPHICS_END_HINT_PGI   0x1A204
#define GL_ALWAYS_FAST_HINT_PGI           0x1A20C
#define GL_ALWAYS_SOFT_HINT_PGI           0x1A20D
#define GL_ALLOW_DRAW_OBJ_HINT_PGI        0x1A20E
#define GL_ALLOW_DRAW_WIN_HINT_PGI        0x1A20F
#define GL_ALLOW_DRAW_FRG_HINT_PGI        0x1A210
#define GL_ALLOW_DRAW_MEM_HINT_PGI        0x1A211
#define GL_STRICT_DEPTHFUNC_HINT_PGI      0x1A216
#define GL_STRICT_LIGHTING_HINT_PGI       0x1A217
#define GL_STRICT_SCISSOR_HINT_PGI        0x1A218
#define GL_FULL_STIPPLE_HINT_PGI          0x1A219
#define GL_CLIP_NEAR_HINT_PGI             0x1A220
#define GL_CLIP_FAR_HINT_PGI              0x1A221
#define GL_WIDE_LINE_HINT_PGI             0x1A222
#define GL_BACK_NORMALS_HINT_PGI          0x1A223
#endif

#ifndef GL_EXT_paletted_texture
#define GL_COLOR_INDEX1_EXT               0x80E2
#define GL_COLOR_INDEX2_EXT               0x80E3
#define GL_COLOR_INDEX4_EXT               0x80E4
#define GL_COLOR_INDEX8_EXT               0x80E5
#define GL_COLOR_INDEX12_EXT              0x80E6
#define GL_COLOR_INDEX16_EXT              0x80E7
#define GL_TEXTURE_INDEX_SIZE_EXT         0x80ED
#endif

#ifndef GL_EXT_clip_volume_hint
#define GL_CLIP_VOLUME_CLIPPING_HINT_EXT  0x80F0
#endif

#ifndef GL_SGIX_list_priority
#define GL_LIST_PRIORITY_SGIX             0x8182
#endif

#ifndef GL_SGIX_ir_instrument1
#define GL_IR_INSTRUMENT1_SGIX            0x817F
#endif

#ifndef GL_SGIX_calligraphic_fragment
#define GL_CALLIGRAPHIC_FRAGMENT_SGIX     0x8183
#endif

#ifndef GL_SGIX_texture_lod_bias
#define GL_TEXTURE_LOD_BIAS_S_SGIX        0x818E
#define GL_TEXTURE_LOD_BIAS_T_SGIX        0x818F
#define GL_TEXTURE_LOD_BIAS_R_SGIX        0x8190
#endif

#ifndef GL_SGIX_shadow_ambient
#define GL_SHADOW_AMBIENT_SGIX            0x80BF
#endif

#ifndef GL_EXT_index_texture
#endif

#ifndef GL_EXT_index_material
#define GL_INDEX_MATERIAL_EXT             0x81B8
#define GL_INDEX_MATERIAL_PARAMETER_EXT   0x81B9
#define GL_INDEX_MATERIAL_FACE_EXT        0x81BA
#endif

#ifndef GL_EXT_index_func
#define GL_INDEX_TEST_EXT                 0x81B5
#define GL_INDEX_TEST_FUNC_EXT            0x81B6
#define GL_INDEX_TEST_REF_EXT             0x81B7
#endif

#ifndef GL_EXT_index_array_formats
#define GL_IUI_V2F_EXT                    0x81AD
#define GL_IUI_V3F_EXT                    0x81AE
#define GL_IUI_N3F_V2F_EXT                0x81AF
#define GL_IUI_N3F_V3F_EXT                0x81B0
#define GL_T2F_IUI_V2F_EXT                0x81B1
#define GL_T2F_IUI_V3F_EXT                0x81B2
#define GL_T2F_IUI_N3F_V2F_EXT            0x81B3
#define GL_T2F_IUI_N3F_V3F_EXT            0x81B4
#endif

#ifndef GL_EXT_compiled_vertex_array
#define GL_ARRAY_ELEMENT_LOCK_FIRST_EXT   0x81A8
#define GL_ARRAY_ELEMENT_LOCK_COUNT_EXT   0x81A9
#endif

#ifndef GL_EXT_cull_vertex
#define GL_CULL_VERTEX_EXT                0x81AA
#define GL_CULL_VERTEX_EYE_POSITION_EXT   0x81AB
#define GL_CULL_VERTEX_OBJECT_POSITION_EXT 0x81AC
#endif

#ifndef GL_SGIX_ycrcb
#define GL_YCRCB_422_SGIX                 0x81BB
#define GL_YCRCB_444_SGIX                 0x81BC
#endif

#ifndef GL_SGIX_fragment_lighting
#define GL_FRAGMENT_LIGHTING_SGIX         0x8400
#define GL_FRAGMENT_COLOR_MATERIAL_SGIX   0x8401
#define GL_FRAGMENT_COLOR_MATERIAL_FACE_SGIX 0x8402
#define GL_FRAGMENT_COLOR_MATERIAL_PARAMETER_SGIX 0x8403
#define GL_MAX_FRAGMENT_LIGHTS_SGIX       0x8404
#define GL_MAX_ACTIVE_LIGHTS_SGIX         0x8405
#define GL_CURRENT_RASTER_NORMAL_SGIX     0x8406
#define GL_LIGHT_ENV_MODE_SGIX            0x8407
#define GL_FRAGMENT_LIGHT_MODEL_LOCAL_VIEWER_SGIX 0x8408
#define GL_FRAGMENT_LIGHT_MODEL_TWO_SIDE_SGIX 0x8409
#define GL_FRAGMENT_LIGHT_MODEL_AMBIENT_SGIX 0x840A
#define GL_FRAGMENT_LIGHT_MODEL_NORMAL_INTERPOLATION_SGIX 0x840B
#define GL_FRAGMENT_LIGHT0_SGIX           0x840C
#define GL_FRAGMENT_LIGHT1_SGIX           0x840D
#define GL_FRAGMENT_LIGHT2_SGIX           0x840E
#define GL_FRAGMENT_LIGHT3_SGIX           0x840F
#define GL_FRAGMENT_LIGHT4_SGIX           0x8410
#define GL_FRAGMENT_LIGHT5_SGIX           0x8411
#define GL_FRAGMENT_LIGHT6_SGIX           0x8412
#define GL_FRAGMENT_LIGHT7_SGIX           0x8413
#endif

#ifndef GL_IBM_rasterpos_clip
#define GL_RASTER_POSITION_UNCLIPPED_IBM  0x19262
#endif

#ifndef GL_HP_texture_lighting
#define GL_TEXTURE_LIGHTING_MODE_HP       0x8167
#define GL_TEXTURE_POST_SPECULAR_HP       0x8168
#define GL_TEXTURE_PRE_SPECULAR_HP        0x8169
#endif

#ifndef GL_EXT_draw_range_elements
#define GL_MAX_ELEMENTS_VERTICES_EXT      0x80E8
#define GL_MAX_ELEMENTS_INDICES_EXT       0x80E9
#endif

#ifndef GL_WIN_phong_shading
#define GL_PHONG_WIN                      0x80EA
#define GL_PHONG_HINT_WIN                 0x80EB
#endif

#ifndef GL_WIN_specular_fog
#define GL_FOG_SPECULAR_TEXTURE_WIN       0x80EC
#endif

#ifndef GL_EXT_light_texture
#define GL_FRAGMENT_MATERIAL_EXT          0x8349
#define GL_FRAGMENT_NORMAL_EXT            0x834A
#define GL_FRAGMENT_COLOR_EXT             0x834C
#define GL_ATTENUATION_EXT                0x834D
#define GL_SHADOW_ATTENUATION_EXT         0x834E
#define GL_TEXTURE_APPLICATION_MODE_EXT   0x834F
#define GL_TEXTURE_LIGHT_EXT              0x8350
#define GL_TEXTURE_MATERIAL_FACE_EXT      0x8351
#define GL_TEXTURE_MATERIAL_PARAMETER_EXT 0x8352
/* reuse GL_FRAGMENT_DEPTH_EXT */
#endif

#ifndef GL_SGIX_blend_alpha_minmax
#define GL_ALPHA_MIN_SGIX                 0x8320
#define GL_ALPHA_MAX_SGIX                 0x8321
#endif

#ifndef GL_EXT_bgra
#define GL_BGR_EXT                        0x80E0
#define GL_BGRA_EXT                       0x80E1
#endif

#ifndef GL_INTEL_texture_scissor
#endif

#ifndef GL_INTEL_parallel_arrays
#define GL_PARALLEL_ARRAYS_INTEL          0x83F4
#define GL_VERTEX_ARRAY_PARALLEL_POINTERS_INTEL 0x83F5
#define GL_NORMAL_ARRAY_PARALLEL_POINTERS_INTEL 0x83F6
#define GL_COLOR_ARRAY_PARALLEL_POINTERS_INTEL 0x83F7
#define GL_TEXTURE_COORD_ARRAY_PARALLEL_POINTERS_INTEL 0x83F8
#endif

#ifndef GL_HP_occlusion_test
#define GL_OCCLUSION_TEST_HP              0x8165
#define GL_OCCLUSION_TEST_RESULT_HP       0x8166
#endif

#ifndef GL_EXT_pixel_transform
#define GL_PIXEL_TRANSFORM_2D_EXT         0x8330
#define GL_PIXEL_MAG_FILTER_EXT           0x8331
#define GL_PIXEL_MIN_FILTER_EXT           0x8332
#define GL_PIXEL_CUBIC_WEIGHT_EXT         0x8333
#define GL_CUBIC_EXT                      0x8334
#define GL_AVERAGE_EXT                    0x8335
#define GL_PIXEL_TRANSFORM_2D_STACK_DEPTH_EXT 0x8336
#define GL_MAX_PIXEL_TRANSFORM_2D_STACK_DEPTH_EXT 0x8337
#define GL_PIXEL_TRANSFORM_2D_MATRIX_EXT  0x8338
#endif

#ifndef GL_EXT_pixel_transform_color_table
#endif

#ifndef GL_EXT_shared_texture_palette
#define GL_SHARED_TEXTURE_PALETTE_EXT     0x81FB
#endif

#ifndef GL_EXT_separate_specular_color
#define GL_LIGHT_MODEL_COLOR_CONTROL_EXT  0x81F8
#define GL_SINGLE_COLOR_EXT               0x81F9
#define GL_SEPARATE_SPECULAR_COLOR_EXT    0x81FA
#endif

#ifndef GL_EXT_secondary_color
#define GL_COLOR_SUM_EXT                  0x8458
#define GL_CURRENT_SECONDARY_COLOR_EXT    0x8459
#define GL_SECONDARY_COLOR_ARRAY_SIZE_EXT 0x845A
#define GL_SECONDARY_COLOR_ARRAY_TYPE_EXT 0x845B
#define GL_SECONDARY_COLOR_ARRAY_STRIDE_EXT 0x845C
#define GL_SECONDARY_COLOR_ARRAY_POINTER_EXT 0x845D
#define GL_SECONDARY_COLOR_ARRAY_EXT      0x845E
#endif

#ifndef GL_EXT_texture_perturb_normal
#define GL_PERTURB_EXT                    0x85AE
#define GL_TEXTURE_NORMAL_EXT             0x85AF
#endif

#ifndef GL_EXT_multi_draw_arrays
#endif

#ifndef GL_EXT_fog_coord
#define GL_FOG_COORDINATE_SOURCE_EXT      0x8450
#define GL_FOG_COORDINATE_EXT             0x8451
#define GL_FRAGMENT_DEPTH_EXT             0x8452
#define GL_CURRENT_FOG_COORDINATE_EXT     0x8453
#define GL_FOG_COORDINATE_ARRAY_TYPE_EXT  0x8454
#define GL_FOG_COORDINATE_ARRAY_STRIDE_EXT 0x8455
#define GL_FOG_COORDINATE_ARRAY_POINTER_EXT 0x8456
#define GL_FOG_COORDINATE_ARRAY_EXT       0x8457
#endif

#ifndef GL_REND_screen_coordinates
#define GL_SCREEN_COORDINATES_REND        0x8490
#define GL_INVERTED_SCREEN_W_REND         0x8491
#endif

#ifndef GL_EXT_coordinate_frame
#define GL_TANGENT_ARRAY_EXT              0x8439
#define GL_BINORMAL_ARRAY_EXT             0x843A
#define GL_CURRENT_TANGENT_EXT            0x843B
#define GL_CURRENT_BINORMAL_EXT           0x843C
#define GL_TANGENT_ARRAY_TYPE_EXT         0x843E
#define GL_TANGENT_ARRAY_STRIDE_EXT       0x843F
#define GL_BINORMAL_ARRAY_TYPE_EXT        0x8440
#define GL_BINORMAL_ARRAY_STRIDE_EXT      0x8441
#define GL_TANGENT_ARRAY_POINTER_EXT      0x8442
#define GL_BINORMAL_ARRAY_POINTER_EXT     0x8443
#define GL_MAP1_TANGENT_EXT               0x8444
#define GL_MAP2_TANGENT_EXT               0x8445
#define GL_MAP1_BINORMAL_EXT              0x8446
#define GL_MAP2_BINORMAL_EXT              0x8447
#endif

#ifndef GL_EXT_texture_env_combine
#define GL_COMBINE_EXT                    0x8570
#define GL_COMBINE_RGB_EXT                0x8571
#define GL_COMBINE_ALPHA_EXT              0x8572
#define GL_RGB_SCALE_EXT                  0x8573
#define GL_ADD_SIGNED_EXT                 0x8574
#define GL_INTERPOLATE_EXT                0x8575
#define GL_CONSTANT_EXT                   0x8576
#define GL_PRIMARY_COLOR_EXT              0x8577
#define GL_PREVIOUS_EXT                   0x8578
#define GL_SOURCE0_RGB_EXT                0x8580
#define GL_SOURCE1_RGB_EXT                0x8581
#define GL_SOURCE2_RGB_EXT                0x8582
#define GL_SOURCE3_RGB_EXT                0x8583
#define GL_SOURCE4_RGB_EXT                0x8584
#define GL_SOURCE5_RGB_EXT                0x8585
#define GL_SOURCE6_RGB_EXT                0x8586
#define GL_SOURCE7_RGB_EXT                0x8587
#define GL_SOURCE0_ALPHA_EXT              0x8588
#define GL_SOURCE1_ALPHA_EXT              0x8589
#define GL_SOURCE2_ALPHA_EXT              0x858A
#define GL_SOURCE3_ALPHA_EXT              0x858B
#define GL_SOURCE4_ALPHA_EXT              0x858C
#define GL_SOURCE5_ALPHA_EXT              0x858D
#define GL_SOURCE6_ALPHA_EXT              0x858E
#define GL_SOURCE7_ALPHA_EXT              0x858F
#define GL_OPERAND0_RGB_EXT               0x8590
#define GL_OPERAND1_RGB_EXT               0x8591
#define GL_OPERAND2_RGB_EXT               0x8592
#define GL_OPERAND3_RGB_EXT               0x8593
#define GL_OPERAND4_RGB_EXT               0x8594
#define GL_OPERAND5_RGB_EXT               0x8595
#define GL_OPERAND6_RGB_EXT               0x8596
#define GL_OPERAND7_RGB_EXT               0x8597
#define GL_OPERAND0_ALPHA_EXT             0x8598
#define GL_OPERAND1_ALPHA_EXT             0x8599
#define GL_OPERAND2_ALPHA_EXT             0x859A
#define GL_OPERAND3_ALPHA_EXT             0x859B
#define GL_OPERAND4_ALPHA_EXT             0x859C
#define GL_OPERAND5_ALPHA_EXT             0x859D
#define GL_OPERAND6_ALPHA_EXT             0x859E
#define GL_OPERAND7_ALPHA_EXT             0x859F
#endif

#ifndef GL_APPLE_specular_vector
#define GL_LIGHT_MODEL_SPECULAR_VECTOR_APPLE 0x85B0
#endif

#ifndef GL_APPLE_transform_hint
#define GL_TRANSFORM_HINT_APPLE           0x85B1
#endif

#ifndef GL_SGIX_fog_scale
#define GL_FOG_SCALE_SGIX                 0x81FC
#define GL_FOG_SCALE_VALUE_SGIX           0x81FD
#endif

#ifndef GL_SUNX_constant_data
#define GL_UNPACK_CONSTANT_DATA_SUNX      0x81D5
#define GL_TEXTURE_CONSTANT_DATA_SUNX     0x81D6
#endif

#ifndef GL_SUN_global_alpha
#define GL_GLOBAL_ALPHA_SUN               0x81D9
#define GL_GLOBAL_ALPHA_FACTOR_SUN        0x81DA
#endif

#ifndef GL_SUN_triangle_list
#define GL_RESTART_SUN                    0x01
#define GL_REPLACE_MIDDLE_SUN             0x02
#define GL_REPLACE_OLDEST_SUN             0x03
#define GL_TRIANGLE_LIST_SUN              0x81D7
#define GL_REPLACEMENT_CODE_SUN           0x81D8
#define GL_REPLACEMENT_CODE_ARRAY_SUN     0x85C0
#define GL_REPLACEMENT_CODE_ARRAY_TYPE_SUN 0x85C1
#define GL_REPLACEMENT_CODE_ARRAY_STRIDE_SUN 0x85C2
#define GL_REPLACEMENT_CODE_ARRAY_POINTER_SUN 0x85C3
#define GL_R1UI_V3F_SUN                   0x85C4
#define GL_R1UI_C4UB_V3F_SUN              0x85C5
#define GL_R1UI_C3F_V3F_SUN               0x85C6
#define GL_R1UI_N3F_V3F_SUN               0x85C7
#define GL_R1UI_C4F_N3F_V3F_SUN           0x85C8
#define GL_R1UI_T2F_V3F_SUN               0x85C9
#define GL_R1UI_T2F_N3F_V3F_SUN           0x85CA
#define GL_R1UI_T2F_C4F_N3F_V3F_SUN       0x85CB
#endif

#ifndef GL_SUN_vertex
#endif

#ifndef GL_EXT_blend_func_separate
#define GL_BLEND_DST_RGB_EXT              0x80C8
#define GL_BLEND_SRC_RGB_EXT              0x80C9
#define GL_BLEND_DST_ALPHA_EXT            0x80CA
#define GL_BLEND_SRC_ALPHA_EXT            0x80CB
#endif

#ifndef GL_INGR_color_clamp
#define GL_RED_MIN_CLAMP_INGR             0x8560
#define GL_GREEN_MIN_CLAMP_INGR           0x8561
#define GL_BLUE_MIN_CLAMP_INGR            0x8562
#define GL_ALPHA_MIN_CLAMP_INGR           0x8563
#define GL_RED_MAX_CLAMP_INGR             0x8564
#define GL_GREEN_MAX_CLAMP_INGR           0x8565
#define GL_BLUE_MAX_CLAMP_INGR            0x8566
#define GL_ALPHA_MAX_CLAMP_INGR           0x8567
#endif

#ifndef GL_INGR_interlace_read
#define GL_INTERLACE_READ_INGR            0x8568
#endif

#ifndef GL_EXT_stencil_wrap
#define GL_INCR_WRAP_EXT                  0x8507
#define GL_DECR_WRAP_EXT                  0x8508
#endif

#ifndef GL_EXT_422_pixels
#define GL_422_EXT                        0x80CC
#define GL_422_REV_EXT                    0x80CD
#define GL_422_AVERAGE_EXT                0x80CE
#define GL_422_REV_AVERAGE_EXT            0x80CF
#endif

#ifndef GL_NV_texgen_reflection
#define GL_NORMAL_MAP_NV                  0x8511
#define GL_REFLECTION_MAP_NV              0x8512
#endif

#ifndef GL_EXT_texture_cube_map
#define GL_NORMAL_MAP_EXT                 0x8511
#define GL_REFLECTION_MAP_EXT             0x8512
#define GL_TEXTURE_CUBE_MAP_EXT           0x8513
#define GL_TEXTURE_BINDING_CUBE_MAP_EXT   0x8514
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT 0x8515
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_X_EXT 0x8516
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Y_EXT 0x8517
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_EXT 0x8518
#define GL_TEXTURE_CUBE_MAP_POSITIVE_Z_EXT 0x8519
#define GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_EXT 0x851A
#define GL_PROXY_TEXTURE_CUBE_MAP_EXT     0x851B
#define GL_MAX_CUBE_MAP_TEXTURE_SIZE_EXT  0x851C
#endif

#ifndef GL_SUN_convolution_border_modes
#define GL_WRAP_BORDER_SUN                0x81D4
#endif

#ifndef GL_EXT_texture_env_add
#endif

#ifndef GL_EXT_texture_lod_bias
#define GL_MAX_TEXTURE_LOD_BIAS_EXT       0x84FD
#define GL_TEXTURE_FILTER_CONTROL_EXT     0x8500
#define GL_TEXTURE_LOD_BIAS_EXT           0x8501
#endif

#ifndef GL_EXT_texture_filter_anisotropic
#define GL_TEXTURE_MAX_ANISOTROPY_EXT     0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF
#endif

#ifndef GL_EXT_vertex_weighting
#define GL_MODELVIEW0_STACK_DEPTH_EXT     GL_MODELVIEW_STACK_DEPTH
#define GL_MODELVIEW1_STACK_DEPTH_EXT     0x8502
#define GL_MODELVIEW0_MATRIX_EXT          GL_MODELVIEW_MATRIX
#define GL_MODELVIEW_MATRIX1_EXT          0x8506
#define GL_VERTEX_WEIGHTING_EXT           0x8509
#define GL_MODELVIEW0_EXT                 GL_MODELVIEW
#define GL_MODELVIEW1_EXT                 0x850A
#define GL_CURRENT_VERTEX_WEIGHT_EXT      0x850B
#define GL_VERTEX_WEIGHT_ARRAY_EXT        0x850C
#define GL_VERTEX_WEIGHT_ARRAY_SIZE_EXT   0x850D
#define GL_VERTEX_WEIGHT_ARRAY_TYPE_EXT   0x850E
#define GL_VERTEX_WEIGHT_ARRAY_STRIDE_EXT 0x850F
#define GL_VERTEX_WEIGHT_ARRAY_POINTER_EXT 0x8510
#endif

#ifndef GL_NV_light_max_exponent
#define GL_MAX_SHININESS_NV               0x8504
#define GL_MAX_SPOT_EXPONENT_NV           0x8505
#endif

#ifndef GL_NV_vertex_array_range
#define GL_VERTEX_ARRAY_RANGE_NV          0x851D
#define GL_VERTEX_ARRAY_RANGE_LENGTH_NV   0x851E
#define GL_VERTEX_ARRAY_RANGE_VALID_NV    0x851F
#define GL_MAX_VERTEX_ARRAY_RANGE_ELEMENT_NV 0x8520
#define GL_VERTEX_ARRAY_RANGE_POINTER_NV  0x8521
#endif

#ifndef GL_NV_register_combiners
#define GL_REGISTER_COMBINERS_NV          0x8522
#define GL_VARIABLE_A_NV                  0x8523
#define GL_VARIABLE_B_NV                  0x8524
#define GL_VARIABLE_C_NV                  0x8525
#define GL_VARIABLE_D_NV                  0x8526
#define GL_VARIABLE_E_NV                  0x8527
#define GL_VARIABLE_F_NV                  0x8528
#define GL_VARIABLE_G_NV                  0x8529
#define GL_CONSTANT_COLOR0_NV             0x852A
#define GL_CONSTANT_COLOR1_NV             0x852B
#define GL_PRIMARY_COLOR_NV               0x852C
#define GL_SECONDARY_COLOR_NV             0x852D
#define GL_SPARE0_NV                      0x852E
#define GL_SPARE1_NV                      0x852F
#define GL_DISCARD_NV                     0x8530
#define GL_E_TIMES_F_NV                   0x8531
#define GL_SPARE0_PLUS_SECONDARY_COLOR_NV 0x8532
#define GL_UNSIGNED_IDENTITY_NV           0x8536
#define GL_UNSIGNED_INVERT_NV             0x8537
#define GL_EXPAND_NORMAL_NV               0x8538
#define GL_EXPAND_NEGATE_NV               0x8539
#define GL_HALF_BIAS_NORMAL_NV            0x853A
#define GL_HALF_BIAS_NEGATE_NV            0x853B
#define GL_SIGNED_IDENTITY_NV             0x853C
#define GL_SIGNED_NEGATE_NV               0x853D
#define GL_SCALE_BY_TWO_NV                0x853E
#define GL_SCALE_BY_FOUR_NV               0x853F
#define GL_SCALE_BY_ONE_HALF_NV           0x8540
#define GL_BIAS_BY_NEGATIVE_ONE_HALF_NV   0x8541
#define GL_COMBINER_INPUT_NV              0x8542
#define GL_COMBINER_MAPPING_NV            0x8543
#define GL_COMBINER_COMPONENT_USAGE_NV    0x8544
#define GL_COMBINER_AB_DOT_PRODUCT_NV     0x8545
#define GL_COMBINER_CD_DOT_PRODUCT_NV     0x8546
#define GL_COMBINER_MUX_SUM_NV            0x8547
#define GL_COMBINER_SCALE_NV              0x8548
#define GL_COMBINER_BIAS_NV               0x8549
#define GL_COMBINER_AB_OUTPUT_NV          0x854A
#define GL_COMBINER_CD_OUTPUT_NV          0x854B
#define GL_COMBINER_SUM_OUTPUT_NV         0x854C
#define GL_MAX_GENERAL_COMBINERS_NV       0x854D
#define GL_NUM_GENERAL_COMBINERS_NV       0x854E
#define GL_COLOR_SUM_CLAMP_NV             0x854F
#define GL_COMBINER0_NV                   0x8550
#define GL_COMBINER1_NV                   0x8551
#define GL_COMBINER2_NV                   0x8552
#define GL_COMBINER3_NV                   0x8553
#define GL_COMBINER4_NV                   0x8554
#define GL_COMBINER5_NV                   0x8555
#define GL_COMBINER6_NV                   0x8556
#define GL_COMBINER7_NV                   0x8557
/* reuse GL_TEXTURE0_ARB */
/* reuse GL_TEXTURE1_ARB */
/* reuse GL_ZERO */
/* reuse GL_NONE */
/* reuse GL_FOG */
#endif

#ifndef GL_NV_fog_distance
#define GL_FOG_DISTANCE_MODE_NV           0x855A
#define GL_EYE_RADIAL_NV                  0x855B
#define GL_EYE_PLANE_ABSOLUTE_NV          0x855C
/* reuse GL_EYE_PLANE */
#endif

#ifndef GL_NV_texgen_emboss
#define GL_EMBOSS_LIGHT_NV                0x855D
#define GL_EMBOSS_CONSTANT_NV             0x855E
#define GL_EMBOSS_MAP_NV                  0x855F
#endif

#ifndef GL_NV_blend_square
#endif

#ifndef GL_NV_texture_env_combine4
#define GL_COMBINE4_NV                    0x8503
#define GL_SOURCE3_RGB_NV                 0x8583
#define GL_SOURCE3_ALPHA_NV               0x858B
#define GL_OPERAND3_RGB_NV                0x8593
#define GL_OPERAND3_ALPHA_NV              0x859B
#endif

#ifndef GL_MESA_resize_buffers
#endif

#ifndef GL_MESA_window_pos
#endif

#ifndef GL_EXT_texture_compression_s3tc
#define GL_COMPRESSED_RGB_S3TC_DXT1_EXT   0x83F0
#define GL_COMPRESSED_RGBA_S3TC_DXT1_EXT  0x83F1
#define GL_COMPRESSED_RGBA_S3TC_DXT3_EXT  0x83F2
#define GL_COMPRESSED_RGBA_S3TC_DXT5_EXT  0x83F3
#endif

#ifndef GL_IBM_cull_vertex
#define GL_CULL_VERTEX_IBM                103050
#endif

#ifndef GL_IBM_multimode_draw_arrays
#endif

#ifndef GL_IBM_vertex_array_lists
#define GL_VERTEX_ARRAY_LIST_IBM          103070
#define GL_NORMAL_ARRAY_LIST_IBM          103071
#define GL_COLOR_ARRAY_LIST_IBM           103072
#define GL_INDEX_ARRAY_LIST_IBM           103073
#define GL_TEXTURE_COORD_ARRAY_LIST_IBM   103074
#define GL_EDGE_FLAG_ARRAY_LIST_IBM       103075
#define GL_FOG_COORDINATE_ARRAY_LIST_IBM  103076
#define GL_SECONDARY_COLOR_ARRAY_LIST_IBM 103077
#define GL_VERTEX_ARRAY_LIST_STRIDE_IBM   103080
#define GL_NORMAL_ARRAY_LIST_STRIDE_IBM   103081
#define GL_COLOR_ARRAY_LIST_STRIDE_IBM    103082
#define GL_INDEX_ARRAY_LIST_STRIDE_IBM    103083
#define GL_TEXTURE_COORD_ARRAY_LIST_STRIDE_IBM 103084
#define GL_EDGE_FLAG_ARRAY_LIST_STRIDE_IBM 103085
#define GL_FOG_COORDINATE_ARRAY_LIST_STRIDE_IBM 103086
#define GL_SECONDARY_COLOR_ARRAY_LIST_STRIDE_IBM 103087
#endif

#ifndef GL_SGIX_subsample
#define GL_PACK_SUBSAMPLE_RATE_SGIX       0x85A0
#define GL_UNPACK_SUBSAMPLE_RATE_SGIX     0x85A1
#define GL_PIXEL_SUBSAMPLE_4444_SGIX      0x85A2
#define GL_PIXEL_SUBSAMPLE_2424_SGIX      0x85A3
#define GL_PIXEL_SUBSAMPLE_4242_SGIX      0x85A4
#endif

#ifndef GL_SGIX_ycrcb_subsample
#endif

#ifndef GL_SGIX_ycrcba
#define GL_YCRCB_SGIX                     0x8318
#define GL_YCRCBA_SGIX                    0x8319
#endif

#ifndef GL_SGI_depth_pass_instrument
#define GL_DEPTH_PASS_INSTRUMENT_SGIX     0x8310
#define GL_DEPTH_PASS_INSTRUMENT_COUNTERS_SGIX 0x8311
#define GL_DEPTH_PASS_INSTRUMENT_MAX_SGIX 0x8312
#endif

#ifndef GL_3DFX_texture_compression_FXT1
#define GL_COMPRESSED_RGB_FXT1_3DFX       0x86B0
#define GL_COMPRESSED_RGBA_FXT1_3DFX      0x86B1
#endif

#ifndef GL_3DFX_multisample
#define GL_MULTISAMPLE_3DFX               0x86B2
#define GL_SAMPLE_BUFFERS_3DFX            0x86B3
#define GL_SAMPLES_3DFX                   0x86B4
#define GL_MULTISAMPLE_BIT_3DFX           0x20000000
#endif

#ifndef GL_3DFX_tbuffer
#endif

#ifndef GL_EXT_multisample
#define GL_MULTISAMPLE_EXT                0x809D
#define GL_SAMPLE_ALPHA_TO_MASK_EXT       0x809E
#define GL_SAMPLE_ALPHA_TO_ONE_EXT        0x809F
#define GL_SAMPLE_MASK_EXT                0x80A0
#define GL_1PASS_EXT                      0x80A1
#define GL_2PASS_0_EXT                    0x80A2
#define GL_2PASS_1_EXT                    0x80A3
#define GL_4PASS_0_EXT                    0x80A4
#define GL_4PASS_1_EXT                    0x80A5
#define GL_4PASS_2_EXT                    0x80A6
#define GL_4PASS_3_EXT                    0x80A7
#define GL_SAMPLE_BUFFERS_EXT             0x80A8
#define GL_SAMPLES_EXT                    0x80A9
#define GL_SAMPLE_MASK_VALUE_EXT          0x80AA
#define GL_SAMPLE_MASK_INVERT_EXT         0x80AB
#define GL_SAMPLE_PATTERN_EXT             0x80AC
#endif

#ifndef GL_SGIX_vertex_preclip
#define GL_VERTEX_PRECLIP_SGIX            0x83EE
#define GL_VERTEX_PRECLIP_HINT_SGIX       0x83EF
#endif

#ifndef GL_SGIX_convolution_accuracy
#define GL_CONVOLUTION_HINT_SGIX          0x8316
#endif

#ifndef GL_SGIX_resample
#define GL_PACK_RESAMPLE_SGIX             0x842C
#define GL_UNPACK_RESAMPLE_SGIX           0x842D
#define GL_RESAMPLE_REPLICATE_SGIX        0x842E
#define GL_RESAMPLE_ZERO_FILL_SGIX        0x842F
#define GL_RESAMPLE_DECIMATE_SGIX         0x8430
#endif

#ifndef GL_SGIS_point_line_texgen
#define GL_EYE_DISTANCE_TO_POINT_SGIS     0x81F0
#define GL_OBJECT_DISTANCE_TO_POINT_SGIS  0x81F1
#define GL_EYE_DISTANCE_TO_LINE_SGIS      0x81F2
#define GL_OBJECT_DISTANCE_TO_LINE_SGIS   0x81F3
#define GL_EYE_POINT_SGIS                 0x81F4
#define GL_OBJECT_POINT_SGIS              0x81F5
#define GL_EYE_LINE_SGIS                  0x81F6
#define GL_OBJECT_LINE_SGIS               0x81F7
#endif

#ifndef GL_SGIS_texture_color_mask
#define GL_TEXTURE_COLOR_WRITEMASK_SGIS   0x81EF
#endif

/* A forgotten token. */
#define GL_CLAMP_TO_EDGE_EXT     0x812F

#ifndef GL_ARB_transpose_matrix
#define GL_ARB_transpose_matrix 1
extern void (*glLoadTransposeMatrixfARB) (const GLfloat *);
extern void (*glLoadTransposeMatrixdARB) (const GLdouble *);
extern void (*glMultTransposeMatrixfARB) (const GLfloat *);
extern void (*glMultTransposeMatrixdARB) (const GLdouble *);
#endif

#ifndef GL_ARB_multisample
#define GL_ARB_multisample 1
extern void (*glSampleCoverageARB) (GLclampf, GLboolean);
extern void (*glSamplePassARB) (GLenum);
#endif

#ifndef GL_ARB_texture_env_add
#define GL_ARB_texture_env_add 1
#endif

#ifndef GL_ARB_texture_cube_map
#define GL_ARB_texture_cube_map 1
#endif

#ifndef GL_ARB_texture_compression
#define GL_ARB_texture_compression 1
extern void (*glCompressedTexImage3DARB) (GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLsizei, const GLvoid *);
extern void (*glCompressedTexImage2DARB) (GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLsizei, const GLvoid *);
extern void (*glCompressedTexImage1DARB) (GLenum, GLint, GLint, GLsizei, GLint, GLsizei, const GLvoid *);
extern void (*glCompressedTexSubImage3DARB) (GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid *);
extern void (*glCompressedTexSubImage2DARB) (GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid *);
extern void (*glCompressedTexSubImage1DARB) (GLenum, GLint, GLint, GLsizei, GLenum, GLsizei, const GLvoid *);
extern void (*glGetCompressedTexImageARB) (GLenum, GLint, void *);
#endif

#ifndef GL_EXT_abgr
#define GL_EXT_abgr 1
#endif

#ifndef GL_EXT_blend_color
#define GL_EXT_blend_color 1
extern void (*glBlendColorEXT) (GLclampf, GLclampf, GLclampf, GLclampf);
#endif

#ifndef GL_EXT_polygon_offset
#define GL_EXT_polygon_offset 1
extern void (*glPolygonOffsetEXT) (GLfloat, GLfloat);
#endif

#ifndef GL_EXT_texture
#define GL_EXT_texture 1
#endif

#ifndef GL_EXT_texture3D
#define GL_EXT_texture3D 1
extern void (*glTexImage3DEXT) (GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *);
#endif

#ifndef GL_EXT_subtexture
extern void (*glTexSubImage3DEXT) (GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
#endif

#ifndef GL_SGIS_texture_filter4
#define GL_SGIS_texture_filter4 1
extern void (*glGetTexFilterFuncSGIS) (GLenum, GLenum, GLfloat *);
extern void (*glTexFilterFuncSGIS) (GLenum, GLenum, GLsizei, const GLfloat *);
#endif

#ifndef GL_EXT_subtexture
#define GL_EXT_subtexture 1
extern void (*glTexSubImage1DEXT) (GLenum, GLint, GLint, GLsizei, GLenum, GLenum, const GLvoid *);
extern void (*glTexSubImage2DEXT) (GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
#endif

#ifndef GL_EXT_copy_texture
#define GL_EXT_copy_texture 1
extern void (*glCopyTexImage1DEXT) (GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLint);
extern void (*glCopyTexImage2DEXT) (GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint);
extern void (*glCopyTexSubImage1DEXT) (GLenum, GLint, GLint, GLint, GLint, GLsizei);
extern void (*glCopyTexSubImage2DEXT) (GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
extern void (*glCopyTexSubImage3DEXT) (GLenum, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei);
#endif

#ifndef GL_EXT_histogram
#define GL_EXT_histogram 1
extern void (*glGetHistogramEXT) (GLenum, GLboolean, GLenum, GLenum, GLvoid *);
extern void (*glGetHistogramParameterfvEXT) (GLenum, GLenum, GLfloat *);
extern void (*glGetHistogramParameterivEXT) (GLenum, GLenum, GLint *);
extern void (*glGetMinmaxEXT) (GLenum, GLboolean, GLenum, GLenum, GLvoid *);
extern void (*glGetMinmaxParameterfvEXT) (GLenum, GLenum, GLfloat *);
extern void (*glGetMinmaxParameterivEXT) (GLenum, GLenum, GLint *);
extern void (*glHistogramEXT) (GLenum, GLsizei, GLenum, GLboolean);
extern void (*glMinmaxEXT) (GLenum, GLenum, GLboolean);
extern void (*glResetHistogramEXT) (GLenum);
extern void (*glResetMinmaxEXT) (GLenum);
#endif

#ifndef GL_EXT_convolution
#define GL_EXT_convolution 1
extern void (*glConvolutionFilter1DEXT) (GLenum, GLenum, GLsizei, GLenum, GLenum, const GLvoid *);
extern void (*glConvolutionFilter2DEXT) (GLenum, GLenum, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
extern void (*glConvolutionParameterfEXT) (GLenum, GLenum, GLfloat);
extern void (*glConvolutionParameterfvEXT) (GLenum, GLenum, const GLfloat *);
extern void (*glConvolutionParameteriEXT) (GLenum, GLenum, GLint);
extern void (*glConvolutionParameterivEXT) (GLenum, GLenum, const GLint *);
extern void (*glCopyConvolutionFilter1DEXT) (GLenum, GLenum, GLint, GLint, GLsizei);
extern void (*glCopyConvolutionFilter2DEXT) (GLenum, GLenum, GLint, GLint, GLsizei, GLsizei);
extern void (*glGetConvolutionFilterEXT) (GLenum, GLenum, GLenum, GLvoid *);
extern void (*glGetConvolutionParameterfvEXT) (GLenum, GLenum, GLfloat *);
extern void (*glGetConvolutionParameterivEXT) (GLenum, GLenum, GLint *);
extern void (*glGetSeparableFilterEXT) (GLenum, GLenum, GLenum, GLvoid *, GLvoid *, GLvoid *);
extern void (*glSeparableFilter2DEXT) (GLenum, GLenum, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *, const GLvoid *);
#endif

#ifndef GL_EXT_color_matrix
#define GL_EXT_color_matrix 1
#endif

#ifndef GL_SGI_color_table
#define GL_SGI_color_table 1
extern void (*glColorTableSGI) (GLenum, GLenum, GLsizei, GLenum, GLenum, const GLvoid *);
extern void (*glColorTableParameterfvSGI) (GLenum, GLenum, const GLfloat *);
extern void (*glColorTableParameterivSGI) (GLenum, GLenum, const GLint *);
extern void (*glCopyColorTableSGI) (GLenum, GLenum, GLint, GLint, GLsizei);
extern void (*glGetColorTableSGI) (GLenum, GLenum, GLenum, GLvoid *);
extern void (*glGetColorTableParameterfvSGI) (GLenum, GLenum, GLfloat *);
extern void (*glGetColorTableParameterivSGI) (GLenum, GLenum, GLint *);
#endif

#ifndef GL_SGIX_pixel_texture
#define GL_SGIX_pixel_texture 1
extern void (*glPixelTexGenSGIX) (GLenum);
#endif

#ifndef GL_SGIS_pixel_texture
#define GL_SGIS_pixel_texture 1
extern void (*glPixelTexGenParameteriSGIS) (GLenum, GLint);
extern void (*glPixelTexGenParameterivSGIS) (GLenum, const GLint *);
extern void (*glPixelTexGenParameterfSGIS) (GLenum, GLfloat);
extern void (*glPixelTexGenParameterfvSGIS) (GLenum, const GLfloat *);
extern void (*glGetPixelTexGenParameterivSGIS) (GLenum, GLint *);
extern void (*glGetPixelTexGenParameterfvSGIS) (GLenum, GLfloat *);
#endif

#ifndef GL_SGIS_texture4D
#define GL_SGIS_texture4D 1
extern void (*glTexImage4DSGIS) (GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *);
extern void (*glTexSubImage4DSGIS) (GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
#endif

#ifndef GL_SGI_texture_color_table
#define GL_SGI_texture_color_table 1
#endif

#ifndef GL_EXT_cmyka
#define GL_EXT_cmyka 1
#endif

#ifndef GL_EXT_texture_object
#define GL_EXT_texture_object 1
extern GLboolean (*glAreTexturesResidentEXT) (GLsizei, const GLuint *, GLboolean *);
extern void (*glBindTextureEXT) (GLenum, GLuint);
extern void (*glDeleteTexturesEXT) (GLsizei, const GLuint *);
extern void (*glGenTexturesEXT) (GLsizei, GLuint *);
extern GLboolean (*glIsTextureEXT) (GLuint);
extern void (*glPrioritizeTexturesEXT) (GLsizei, const GLuint *, const GLclampf *);
#endif

#ifndef GL_SGIS_detail_texture
#define GL_SGIS_detail_texture 1
extern void (*glDetailTexFuncSGIS) (GLenum, GLsizei, const GLfloat *);
extern void (*glGetDetailTexFuncSGIS) (GLenum, GLfloat *);
#endif

#ifndef GL_SGIS_sharpen_texture
#define GL_SGIS_sharpen_texture 1
extern void (*glSharpenTexFuncSGIS) (GLenum, GLsizei, const GLfloat *);
extern void (*glGetSharpenTexFuncSGIS) (GLenum, GLfloat *);
#endif

#ifndef GL_EXT_packed_pixels
#define GL_EXT_packed_pixels 1
#endif

#ifndef GL_SGIS_texture_lod
#define GL_SGIS_texture_lod 1
#endif

#ifndef GL_SGIS_multisample
#define GL_SGIS_multisample 1
extern void (*glSampleMaskSGIS) (GLclampf, GLboolean);
extern void (*glSamplePatternSGIS) (GLenum);
#endif

#ifndef GL_EXT_rescale_normal
#define GL_EXT_rescale_normal 1
#endif

#ifndef GL_EXT_vertex_array
#define GL_EXT_vertex_array 1
extern void (*glArrayElementEXT) (GLint);
extern void (*glColorPointerEXT) (GLint, GLenum, GLsizei, GLsizei, const GLvoid *);
extern void (*glDrawArraysEXT) (GLenum, GLint, GLsizei);
extern void (*glEdgeFlagPointerEXT) (GLsizei, GLsizei, const GLboolean *);
extern void (*glGetPointervEXT) (GLenum, GLvoid* *);
extern void (*glIndexPointerEXT) (GLenum, GLsizei, GLsizei, const GLvoid *);
extern void (*glNormalPointerEXT) (GLenum, GLsizei, GLsizei, const GLvoid *);
extern void (*glTexCoordPointerEXT) (GLint, GLenum, GLsizei, GLsizei, const GLvoid *);
extern void (*glVertexPointerEXT) (GLint, GLenum, GLsizei, GLsizei, const GLvoid *);
#endif

#ifndef GL_EXT_misc_attribute
#define GL_EXT_misc_attribute 1
#endif

#ifndef GL_SGIS_generate_mipmap
#define GL_SGIS_generate_mipmap 1
#endif

#ifndef GL_SGIX_clipmap
#define GL_SGIX_clipmap 1
#endif

#ifndef GL_SGIX_shadow
#define GL_SGIX_shadow 1
#endif

#ifndef GL_SGIS_texture_edge_clamp
#define GL_SGIS_texture_edge_clamp 1
#endif

#ifndef GL_SGIS_texture_border_clamp
#define GL_SGIS_texture_border_clamp 1
#endif

#ifndef GL_EXT_blend_minmax
#define GL_EXT_blend_minmax 1
extern void (*glBlendEquationEXT) (GLenum);
#endif

#ifndef GL_EXT_blend_subtract
#define GL_EXT_blend_subtract 1
#endif

#ifndef GL_EXT_blend_logic_op
#define GL_EXT_blend_logic_op 1
#endif

#ifndef GL_SGIX_interlace
#define GL_SGIX_interlace 1
#endif

#ifndef GL_SGIX_pixel_tiles
#define GL_SGIX_pixel_tiles 1
#endif

#ifndef GL_SGIX_texture_select
#define GL_SGIX_texture_select 1
#endif

#ifndef GL_SGIX_sprite
#define GL_SGIX_sprite 1
extern void (*glSpriteParameterfSGIX) (GLenum, GLfloat);
extern void (*glSpriteParameterfvSGIX) (GLenum, const GLfloat *);
extern void (*glSpriteParameteriSGIX) (GLenum, GLint);
extern void (*glSpriteParameterivSGIX) (GLenum, const GLint *);
#endif

#ifndef GL_SGIX_texture_multi_buffer
#define GL_SGIX_texture_multi_buffer 1
#endif

#ifndef GL_EXT_point_parameters
#define GL_EXT_point_parameters 1
extern void (*glPointParameterfEXT) (GLenum, GLfloat);
extern void (*glPointParameterfvEXT) (GLenum, const GLfloat *);
extern void (*glPointParameterfSGIS) (GLenum, GLfloat);
extern void (*glPointParameterfvSGIS) (GLenum, const GLfloat *);
#endif

#ifndef GL_SGIX_instruments
#define GL_SGIX_instruments 1
extern GLint (*glGetInstrumentsSGIX) (void);
extern void (*glInstrumentsBufferSGIX) (GLsizei, GLint *);
extern GLint (*glPollInstrumentsSGIX) (GLint *);
extern void (*glReadInstrumentsSGIX) (GLint);
extern void (*glStartInstrumentsSGIX) (void);
extern void (*glStopInstrumentsSGIX) (GLint);
#endif

#ifndef GL_SGIX_texture_scale_bias
#define GL_SGIX_texture_scale_bias 1
#endif

#ifndef GL_SGIX_framezoom
#define GL_SGIX_framezoom 1
extern void (*glFrameZoomSGIX) (GLint);
#endif

#ifndef GL_SGIX_tag_sample_buffer
#define GL_SGIX_tag_sample_buffer 1
extern void (*glTagSampleBufferSGIX) (void);
#endif

#ifndef GL_SGIX_reference_plane
#define GL_SGIX_reference_plane 1
extern void (*glReferencePlaneSGIX) (const GLdouble *);
#endif

#ifndef GL_SGIX_flush_raster
#define GL_SGIX_flush_raster 1
extern void (*glFlushRasterSGIX) (void);
#endif

#ifndef GL_SGIX_depth_texture
#define GL_SGIX_depth_texture 1
#endif

#ifndef GL_SGIS_fog_function
#define GL_SGIS_fog_function 1
extern void (*glFogFuncSGIS) (GLsizei, const GLfloat *);
extern void (*glGetFogFuncSGIS) (const GLfloat *);
#endif

#ifndef GL_SGIX_fog_offset
#define GL_SGIX_fog_offset 1
#endif

#ifndef GL_HP_image_transform
#define GL_HP_image_transform 1
extern void (*glImageTransformParameteriHP) (GLenum, GLenum, GLint);
extern void (*glImageTransformParameterfHP) (GLenum, GLenum, GLfloat);
extern void (*glImageTransformParameterivHP) (GLenum, GLenum, const GLint *);
extern void (*glImageTransformParameterfvHP) (GLenum, GLenum, const GLfloat *);
extern void (*glGetImageTransformParameterivHP) (GLenum, GLenum, GLint *);
extern void (*glGetImageTransformParameterfvHP) (GLenum, GLenum, GLfloat *);
#endif

#ifndef GL_HP_convolution_border_modes
#define GL_HP_convolution_border_modes 1
#endif

#ifndef GL_SGIX_texture_add_env
#define GL_SGIX_texture_add_env 1
#endif

#ifndef GL_EXT_color_subtable
#define GL_EXT_color_subtable 1
extern void (*glColorSubTableEXT) (GLenum, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *);
extern void (*glCopyColorSubTableEXT) (GLenum, GLsizei, GLint, GLint, GLsizei);
#endif

#ifndef GL_PGI_vertex_hints
#define GL_PGI_vertex_hints 1
#endif

#ifndef GL_PGI_misc_hints
#define GL_PGI_misc_hints 1
extern void (*glHintPGI) (GLenum, GLint);
#endif

#ifndef GL_EXT_paletted_texture
#define GL_EXT_paletted_texture 1
extern void (*glColorTableEXT) (GLenum, GLenum, GLsizei, GLenum, GLenum, const GLvoid *);
extern void (*glGetColorTableEXT) (GLenum, GLenum, GLenum, GLvoid *);
extern void (*glGetColorTableParameterivEXT) (GLenum, GLenum, GLint *);
extern void (*glGetColorTableParameterfvEXT) (GLenum, GLenum, GLfloat *);
#endif

#ifndef GL_EXT_clip_volume_hint
#define GL_EXT_clip_volume_hint 1
#endif

#ifndef GL_SGIX_list_priority
#define GL_SGIX_list_priority 1
extern void (*glGetListParameterfvSGIX) (GLuint, GLenum, GLfloat *);
extern void (*glGetListParameterivSGIX) (GLuint, GLenum, GLint *);
extern void (*glListParameterfSGIX) (GLuint, GLenum, GLfloat);
extern void (*glListParameterfvSGIX) (GLuint, GLenum, const GLfloat *);
extern void (*glListParameteriSGIX) (GLuint, GLenum, GLint);
extern void (*glListParameterivSGIX) (GLuint, GLenum, const GLint *);
#endif

#ifndef GL_SGIX_ir_instrument1
#define GL_SGIX_ir_instrument1 1
#endif

#ifndef GL_SGIX_calligraphic_fragment
#define GL_SGIX_calligraphic_fragment 1
#endif

#ifndef GL_SGIX_texture_lod_bias
#define GL_SGIX_texture_lod_bias 1
#endif

#ifndef GL_SGIX_shadow_ambient
#define GL_SGIX_shadow_ambient 1
#endif

#ifndef GL_EXT_index_texture
#define GL_EXT_index_texture 1
#endif

#ifndef GL_EXT_index_material
#define GL_EXT_index_material 1
extern void (*glIndexMaterialEXT) (GLenum, GLenum);
#endif

#ifndef GL_EXT_index_func
#define GL_EXT_index_func 1
extern void (*glIndexFuncEXT) (GLenum, GLclampf);
#endif

#ifndef GL_EXT_index_array_formats
#define GL_EXT_index_array_formats 1
#endif

#ifndef GL_EXT_compiled_vertex_array
#define GL_EXT_compiled_vertex_array 1
extern void (*glLockArraysEXT) (GLint, GLsizei);
extern void (*glUnlockArraysEXT) (void);
#endif

#ifndef GL_EXT_cull_vertex
#define GL_EXT_cull_vertex 1
extern void (*glCullParameterdvEXT) (GLenum, GLdouble *);
extern void (*glCullParameterfvEXT) (GLenum, GLfloat *);
#endif

#ifndef GL_SGIX_ycrcb
#define GL_SGIX_ycrcb 1
#endif

#ifndef GL_SGIX_fragment_lighting
#define GL_SGIX_fragment_lighting 1
extern void (*glFragmentColorMaterialSGIX) (GLenum, GLenum);
extern void (*glFragmentLightfSGIX) (GLenum, GLenum, GLfloat);
extern void (*glFragmentLightfvSGIX) (GLenum, GLenum, const GLfloat *);
extern void (*glFragmentLightiSGIX) (GLenum, GLenum, GLint);
extern void (*glFragmentLightivSGIX) (GLenum, GLenum, const GLint *);
extern void (*glFragmentLightModelfSGIX) (GLenum, GLfloat);
extern void (*glFragmentLightModelfvSGIX) (GLenum, const GLfloat *);
extern void (*glFragmentLightModeliSGIX) (GLenum, GLint);
extern void (*glFragmentLightModelivSGIX) (GLenum, const GLint *);
extern void (*glFragmentMaterialfSGIX) (GLenum, GLenum, GLfloat);
extern void (*glFragmentMaterialfvSGIX) (GLenum, GLenum, const GLfloat *);
extern void (*glFragmentMaterialiSGIX) (GLenum, GLenum, GLint);
extern void (*glFragmentMaterialivSGIX) (GLenum, GLenum, const GLint *);
extern void (*glGetFragmentLightfvSGIX) (GLenum, GLenum, GLfloat *);
extern void (*glGetFragmentLightivSGIX) (GLenum, GLenum, GLint *);
extern void (*glGetFragmentMaterialfvSGIX) (GLenum, GLenum, GLfloat *);
extern void (*glGetFragmentMaterialivSGIX) (GLenum, GLenum, GLint *);
extern void (*glLightEnviSGIX) (GLenum, GLint);
#endif

#ifndef GL_IBM_rasterpos_clip
#define GL_IBM_rasterpos_clip 1
#endif

#ifndef GL_HP_texture_lighting
#define GL_HP_texture_lighting 1
#endif

#ifndef GL_EXT_draw_range_elements
#define GL_EXT_draw_range_elements 1
extern void (*glDrawRangeElementsEXT) (GLenum, GLuint, GLuint, GLsizei, GLenum, const GLvoid *);
#endif

#ifndef GL_WIN_phong_shading
#define GL_WIN_phong_shading 1
#endif

#ifndef GL_WIN_specular_fog
#define GL_WIN_specular_fog 1
#endif

#ifndef GL_EXT_light_texture
#define GL_EXT_light_texture 1
extern void (*glApplyTextureEXT) (GLenum);
extern void (*glTextureLightEXT) (GLenum);
extern void (*glTextureMaterialEXT) (GLenum, GLenum);
#endif

#ifndef GL_SGIX_blend_alpha_minmax
#define GL_SGIX_blend_alpha_minmax 1
#endif

#ifndef GL_EXT_bgra
#define GL_EXT_bgra 1
#endif

#ifndef GL_INTEL_parallel_arrays
#define GL_INTEL_parallel_arrays 1
extern void (*glVertexPointervINTEL) (GLint, GLenum, const GLvoid* *);
extern void (*glNormalPointervINTEL) (GLenum, const GLvoid* *);
extern void (*glColorPointervINTEL) (GLint, GLenum, const GLvoid* *);
extern void (*glTexCoordPointervINTEL) (GLint, GLenum, const GLvoid* *);
#endif

#ifndef GL_HP_occlusion_test
#define GL_HP_occlusion_test 1
#endif

#ifndef GL_EXT_pixel_transform
#define GL_EXT_pixel_transform 1
extern void (*glPixelTransformParameteriEXT) (GLenum, GLenum, GLint);
extern void (*glPixelTransformParameterfEXT) (GLenum, GLenum, GLfloat);
extern void (*glPixelTransformParameterivEXT) (GLenum, GLenum, const GLint *);
extern void (*glPixelTransformParameterfvEXT) (GLenum, GLenum, const GLfloat *);
#endif

#ifndef GL_EXT_pixel_transform_color_table
#define GL_EXT_pixel_transform_color_table 1
#endif

#ifndef GL_EXT_shared_texture_palette
#define GL_EXT_shared_texture_palette 1
#endif

#ifndef GL_EXT_separate_specular_color
#define GL_EXT_separate_specular_color 1
#endif

#ifndef GL_EXT_secondary_color
#define GL_EXT_secondary_color 1
extern void (*glSecondaryColor3bEXT) (GLbyte, GLbyte, GLbyte);
extern void (*glSecondaryColor3bvEXT) (const GLbyte *);
extern void (*glSecondaryColor3dEXT) (GLdouble, GLdouble, GLdouble);
extern void (*glSecondaryColor3dvEXT) (const GLdouble *);
extern void (*glSecondaryColor3fEXT) (GLfloat, GLfloat, GLfloat);
extern void (*glSecondaryColor3fvEXT) (const GLfloat *);
extern void (*glSecondaryColor3iEXT) (GLint, GLint, GLint);
extern void (*glSecondaryColor3ivEXT) (const GLint *);
extern void (*glSecondaryColor3sEXT) (GLshort, GLshort, GLshort);
extern void (*glSecondaryColor3svEXT) (const GLshort *);
extern void (*glSecondaryColor3ubEXT) (GLubyte, GLubyte, GLubyte);
extern void (*glSecondaryColor3ubvEXT) (const GLubyte *);
extern void (*glSecondaryColor3uiEXT) (GLuint, GLuint, GLuint);
extern void (*glSecondaryColor3uivEXT) (const GLuint *);
extern void (*glSecondaryColor3usEXT) (GLushort, GLushort, GLushort);
extern void (*glSecondaryColor3usvEXT) (const GLushort *);
extern void (*glSecondaryColorPointerEXT) (GLint, GLenum, GLsizei, GLvoid *);
#endif

#ifndef GL_EXT_texture_perturb_normal
#define GL_EXT_texture_perturb_normal 1
extern void (*glTextureNormalEXT) (GLenum);
#endif

#ifndef GL_EXT_multi_draw_arrays
#define GL_EXT_multi_draw_arrays 1
extern void (*glMultiDrawArraysEXT) (GLenum, GLint *, GLsizei *, GLsizei);
extern void (*glMultiDrawElementsEXT) (GLenum, const GLsizei *, GLenum, const GLvoid* *, GLsizei);
#endif

#ifndef GL_EXT_fog_coord
#define GL_EXT_fog_coord 1
extern void (*glFogCoordfEXT) (GLfloat);
extern void (*glFogCoordfvEXT) (const GLfloat *);
extern void (*glFogCoorddEXT) (GLdouble);
extern void (*glFogCoorddvEXT) (const GLdouble *);
extern void (*glFogCoordPointerEXT) (GLenum, GLsizei, const GLvoid *);
#endif

#ifndef GL_REND_screen_coordinates
#define GL_REND_screen_coordinates 1
#endif

#ifndef GL_EXT_coordinate_frame
#define GL_EXT_coordinate_frame 1
extern void (*glTangent3bEXT) (GLbyte, GLbyte, GLbyte);
extern void (*glTangent3bvEXT) (const GLbyte *);
extern void (*glTangent3dEXT) (GLdouble, GLdouble, GLdouble);
extern void (*glTangent3dvEXT) (const GLdouble *);
extern void (*glTangent3fEXT) (GLfloat, GLfloat, GLfloat);
extern void (*glTangent3fvEXT) (const GLfloat *);
extern void (*glTangent3iEXT) (GLint, GLint, GLint);
extern void (*glTangent3ivEXT) (const GLint *);
extern void (*glTangent3sEXT) (GLshort, GLshort, GLshort);
extern void (*glTangent3svEXT) (const GLshort *);
extern void (*glBinormal3bEXT) (GLbyte, GLbyte, GLbyte);
extern void (*glBinormal3bvEXT) (const GLbyte *);
extern void (*glBinormal3dEXT) (GLdouble, GLdouble, GLdouble);
extern void (*glBinormal3dvEXT) (const GLdouble *);
extern void (*glBinormal3fEXT) (GLfloat, GLfloat, GLfloat);
extern void (*glBinormal3fvEXT) (const GLfloat *);
extern void (*glBinormal3iEXT) (GLint, GLint, GLint);
extern void (*glBinormal3ivEXT) (const GLint *);
extern void (*glBinormal3sEXT) (GLshort, GLshort, GLshort);
extern void (*glBinormal3svEXT) (const GLshort *);
extern void (*glTangentPointerEXT) (GLenum, GLsizei, const GLvoid *);
extern void (*glBinormalPointerEXT) (GLenum, GLsizei, const GLvoid *);
#endif

#ifndef GL_EXT_texture_env_combine
#define GL_EXT_texture_env_combine 1
#endif

#ifndef GL_APPLE_specular_vector
#define GL_APPLE_specular_vector 1
#endif

#ifndef GL_APPLE_transform_hint
#define GL_APPLE_transform_hint 1
#endif

#ifndef GL_SGIX_fog_scale
#define GL_SGIX_fog_scale 1
#endif

#ifndef GL_SUNX_constant_data
#define GL_SUNX_constant_data 1
extern void (*glFinishTextureSUNX) (void);
#endif

#ifndef GL_SUN_global_alpha
#define GL_SUN_global_alpha 1
extern void (*glGlobalAlphaFactorbSUN) (GLbyte);
extern void (*glGlobalAlphaFactorsSUN) (GLshort);
extern void (*glGlobalAlphaFactoriSUN) (GLint);
extern void (*glGlobalAlphaFactorfSUN) (GLfloat);
extern void (*glGlobalAlphaFactordSUN) (GLdouble);
extern void (*glGlobalAlphaFactorubSUN) (GLubyte);
extern void (*glGlobalAlphaFactorusSUN) (GLushort);
extern void (*glGlobalAlphaFactoruiSUN) (GLuint);
#endif

#ifndef GL_SUN_triangle_list
#define GL_SUN_triangle_list 1
extern void (*glReplacementCodeuiSUN) (GLuint);
extern void (*glReplacementCodeusSUN) (GLushort);
extern void (*glReplacementCodeubSUN) (GLubyte);
extern void (*glReplacementCodeuivSUN) (const GLuint *);
extern void (*glReplacementCodeusvSUN) (const GLushort *);
extern void (*glReplacementCodeubvSUN) (const GLubyte *);
extern void (*glReplacementCodePointerSUN) (GLenum, GLsizei, const GLvoid* *);
#endif

#ifndef GL_SUN_vertex
#define GL_SUN_vertex 1
extern void (*glColor4ubVertex2fSUN) (GLubyte, GLubyte, GLubyte, GLubyte, GLfloat, GLfloat);
extern void (*glColor4ubVertex2fvSUN) (const GLubyte *, const GLfloat *);
extern void (*glColor4ubVertex3fSUN) (GLubyte, GLubyte, GLubyte, GLubyte, GLfloat, GLfloat, GLfloat);
extern void (*glColor4ubVertex3fvSUN) (const GLubyte *, const GLfloat *);
extern void (*glColor3fVertex3fSUN) (GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
extern void (*glColor3fVertex3fvSUN) (const GLfloat *, const GLfloat *);
extern void (*glNormal3fVertex3fSUN) (GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
extern void (*glNormal3fVertex3fvSUN) (const GLfloat *, const GLfloat *);
extern void (*glColor4fNormal3fVertex3fSUN) (GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
extern void (*glColor4fNormal3fVertex3fvSUN) (const GLfloat *, const GLfloat *, const GLfloat *);
extern void (*glTexCoord2fVertex3fSUN) (GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
extern void (*glTexCoord2fVertex3fvSUN) (const GLfloat *, const GLfloat *);
extern void (*glTexCoord4fVertex4fSUN) (GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
extern void (*glTexCoord4fVertex4fvSUN) (const GLfloat *, const GLfloat *);
extern void (*glTexCoord2fColor4ubVertex3fSUN) (GLfloat, GLfloat, GLubyte, GLubyte, GLubyte, GLubyte, GLfloat, GLfloat, GLfloat);
extern void (*glTexCoord2fColor4ubVertex3fvSUN) (const GLfloat *, const GLubyte *, const GLfloat *);
extern void (*glTexCoord2fColor3fVertex3fSUN) (GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
extern void (*glTexCoord2fColor3fVertex3fvSUN) (const GLfloat *, const GLfloat *, const GLfloat *);
extern void (*glTexCoord2fNormal3fVertex3fSUN) (GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
extern void (*glTexCoord2fNormal3fVertex3fvSUN) (const GLfloat *, const GLfloat *, const GLfloat *);
extern void (*glTexCoord2fColor4fNormal3fVertex3fSUN) (GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
extern void (*glTexCoord2fColor4fNormal3fVertex3fvSUN) (const GLfloat *, const GLfloat *, const GLfloat *, const GLfloat *);
extern void (*glTexCoord4fColor4fNormal3fVertex4fSUN) (GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
extern void (*glTexCoord4fColor4fNormal3fVertex4fvSUN) (const GLfloat *, const GLfloat *, const GLfloat *, const GLfloat *);
extern void (*glReplacementCodeuiVertex3fSUN) (GLenum, GLfloat, GLfloat, GLfloat);
extern void (*glReplacementCodeuiVertex3fvSUN) (const GLenum *, const GLfloat *);
extern void (*glReplacementCodeuiColor4ubVertex3fSUN) (GLenum, GLubyte, GLubyte, GLubyte, GLubyte, GLfloat, GLfloat, GLfloat);
extern void (*glReplacementCodeuiColor4ubVertex3fvSUN) (const GLenum *, const GLubyte *, const GLfloat *);
extern void (*glReplacementCodeuiColor3fVertex3fSUN) (GLenum, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
extern void (*glReplacementCodeuiColor3fVertex3fvSUN) (const GLenum *, const GLfloat *, const GLfloat *);
extern void (*glReplacementCodeuiNormal3fVertex3fSUN) (GLenum, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
extern void (*glReplacementCodeuiNormal3fVertex3fvSUN) (const GLenum *, const GLfloat *, const GLfloat *);
extern void (*glReplacementCodeuiColor4fNormal3fVertex3fSUN) (GLenum, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
extern void (*glReplacementCodeuiColor4fNormal3fVertex3fvSUN) (const GLenum *, const GLfloat *, const GLfloat *, const GLfloat *);
extern void (*glReplacementCodeuiTexCoord2fVertex3fSUN) (GLenum, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
extern void (*glReplacementCodeuiTexCoord2fVertex3fvSUN) (const GLenum *, const GLfloat *, const GLfloat *);
extern void (*glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN) (GLenum, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
extern void (*glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN) (const GLenum *, const GLfloat *, const GLfloat *, const GLfloat *);
extern void (*glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN) (GLenum, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat, GLfloat);
extern void (*glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN) (const GLenum *, const GLfloat *, const GLfloat *, const GLfloat *, const GLfloat *);
#endif

#ifndef GL_EXT_blend_func_separate
#define GL_EXT_blend_func_separate 1
extern void (*glBlendFuncSeparateEXT) (GLenum, GLenum, GLenum, GLenum);
#endif

#ifndef GL_INGR_color_clamp
#define GL_INGR_color_clamp 1
#endif

#ifndef GL_INGR_interlace_read
#define GL_INGR_interlace_read 1
#endif

#ifndef GL_EXT_stencil_wrap
#define GL_EXT_stencil_wrap 1
#endif

#ifndef GL_EXT_422_pixels
#define GL_EXT_422_pixels 1
#endif

#ifndef GL_NV_texgen_reflection
#define GL_NV_texgen_reflection 1
#endif

#ifndef GL_SUN_convolution_border_modes
#define GL_SUN_convolution_border_modes 1
#endif

#ifndef GL_EXT_texture_env_add
#define GL_EXT_texture_env_add 1
#endif

#ifndef GL_EXT_texture_lod_bias
#define GL_EXT_texture_lod_bias 1
#endif

#ifndef GL_EXT_texture_filter_anisotropic
#define GL_EXT_texture_filter_anisotropic 1
#endif

#ifndef GL_EXT_vertex_weighting
#define GL_EXT_vertex_weighting 1
extern void (*glVertexWeightfEXT) (GLfloat);
extern void (*glVertexWeightfvEXT) (const GLfloat *);
extern void (*glVertexWeightPointerEXT) (GLsizei, GLenum, GLsizei, const GLvoid *);
#endif

#ifndef GL_NV_light_max_exponent
#define GL_NV_light_max_exponent 1
#endif

#ifndef GL_NV_vertex_array_range
#define GL_NV_vertex_array_range 1
extern void (*glFlushVertexArrayRangeNV) (void);
extern void (*glVertexArrayRangeNV) (GLsizei, const GLvoid *);
#endif

#ifndef GL_NV_register_combiners
#define GL_NV_register_combiners 1
extern void (*glCombinerParameterfvNV) (GLenum, const GLfloat *);
extern void (*glCombinerParameterfNV) (GLenum, GLfloat);
extern void (*glCombinerParameterivNV) (GLenum, const GLint *);
extern void (*glCombinerParameteriNV) (GLenum, GLint);
extern void (*glCombinerInputNV) (GLenum, GLenum, GLenum, GLenum, GLenum, GLenum);
extern void (*glCombinerOutputNV) (GLenum, GLenum, GLenum, GLenum, GLenum, GLenum, GLenum, GLboolean, GLboolean, GLboolean);
extern void (*glFinalCombinerInputNV) (GLenum, GLenum, GLenum, GLenum);
extern void (*glGetCombinerInputParameterfvNV) (GLenum, GLenum, GLenum, GLenum, GLfloat *);
extern void (*glGetCombinerInputParameterivNV) (GLenum, GLenum, GLenum, GLenum, GLint *);
extern void (*glGetCombinerOutputParameterfvNV) (GLenum, GLenum, GLenum, GLfloat *);
extern void (*glGetCombinerOutputParameterivNV) (GLenum, GLenum, GLenum, GLint *);
extern void (*glGetFinalCombinerInputParameterfvNV) (GLenum, GLenum, GLfloat *);
extern void (*glGetFinalCombinerInputParameterivNV) (GLenum, GLenum, GLint *);
#endif

#ifndef GL_NV_fog_distance
#define GL_NV_fog_distance 1
#endif

#ifndef GL_NV_texgen_emboss
#define GL_NV_texgen_emboss 1
#endif

#ifndef GL_NV_blend_square
#define GL_NV_blend_square 1
#endif

#ifndef GL_NV_texture_env_combine4
#define GL_NV_texture_env_combine4 1
#endif

#ifndef GL_MESA_resize_buffers
#define GL_MESA_resize_buffers 1
extern void (*glResizeBuffersMESA) (void);
#endif

#ifndef GL_MESA_window_pos
#define GL_MESA_window_pos 1
extern void (*glWindowPos2dMESA) (GLdouble, GLdouble);
extern void (*glWindowPos2dvMESA) (const GLdouble *);
extern void (*glWindowPos2fMESA) (GLfloat, GLfloat);
extern void (*glWindowPos2fvMESA) (const GLfloat *);
extern void (*glWindowPos2iMESA) (GLint, GLint);
extern void (*glWindowPos2ivMESA) (const GLint *);
extern void (*glWindowPos2sMESA) (GLshort, GLshort);
extern void (*glWindowPos2svMESA) (const GLshort *);
extern void (*glWindowPos3dMESA) (GLdouble, GLdouble, GLdouble);
extern void (*glWindowPos3dvMESA) (const GLdouble *);
extern void (*glWindowPos3fMESA) (GLfloat, GLfloat, GLfloat);
extern void (*glWindowPos3fvMESA) (const GLfloat *);
extern void (*glWindowPos3iMESA) (GLint, GLint, GLint);
extern void (*glWindowPos3ivMESA) (const GLint *);
extern void (*glWindowPos3sMESA) (GLshort, GLshort, GLshort);
extern void (*glWindowPos3svMESA) (const GLshort *);
extern void (*glWindowPos4dMESA) (GLdouble, GLdouble, GLdouble, GLdouble);
extern void (*glWindowPos4dvMESA) (const GLdouble *);
extern void (*glWindowPos4fMESA) (GLfloat, GLfloat, GLfloat, GLfloat);
extern void (*glWindowPos4fvMESA) (const GLfloat *);
extern void (*glWindowPos4iMESA) (GLint, GLint, GLint, GLint);
extern void (*glWindowPos4ivMESA) (const GLint *);
extern void (*glWindowPos4sMESA) (GLshort, GLshort, GLshort, GLshort);
extern void (*glWindowPos4svMESA) (const GLshort *);
#endif

#ifndef GL_IBM_cull_vertex
#define GL_IBM_cull_vertex 1
#endif

#ifndef GL_IBM_multimode_draw_arrays
#define GL_IBM_multimode_draw_arrays 1
extern void (*glMultiModeDrawArraysIBM) (GLenum, const GLint *, const GLsizei *, GLsizei, GLint);
extern void (*glMultiModeDrawElementsIBM) (const GLenum *, const GLsizei *, GLenum, const GLvoid* *, GLsizei, GLint);
#endif

#ifndef GL_IBM_vertex_array_lists
#define GL_IBM_vertex_array_lists 1
extern void (*glColorPointerListIBM) (GLint, GLenum, GLint, const GLvoid* *, GLint);
extern void (*glSecondaryColorPointerListIBM) (GLint, GLenum, GLint, const GLvoid* *, GLint);
extern void (*glEdgeFlagPointerListIBM) (GLint, const GLboolean* *, GLint);
extern void (*glFogCoordPointerListIBM) (GLenum, GLint, const GLvoid* *, GLint);
extern void (*glIndexPointerListIBM) (GLenum, GLint, const GLvoid* *, GLint);
extern void (*glNormalPointerListIBM) (GLenum, GLint, const GLvoid* *, GLint);
extern void (*glTexCoordPointerListIBM) (GLint, GLenum, GLint, const GLvoid* *, GLint);
extern void (*glVertexPointerListIBM) (GLint, GLenum, GLint, const GLvoid* *, GLint);
#endif

#ifndef GL_SGIX_subsample
#define GL_SGIX_subsample 1
#endif

#ifndef GL_SGIX_ycrcba
#define GL_SGIX_ycrcba 1
#endif

#ifndef GL_SGIX_ycrcb_subsample
#define GL_SGIX_ycrcb_subsample 1
#endif

#ifndef GL_SGIX_depth_pass_instrument
#define GL_SGIX_depth_pass_instrument 1
#endif

#ifndef GL_3DFX_texture_compression_FXT1
#define GL_3DFX_texture_compression_FXT1 1
#endif

#ifndef GL_3DFX_multisample
#define GL_3DFX_multisample 1
#endif

#ifndef GL_3DFX_tbuffer
#define GL_3DFX_tbuffer 1
extern void (*glTbufferMask3DFX) (GLuint);
#endif

#ifndef GL_EXT_multisample
#define GL_EXT_multisample 1
extern void (*glSampleMaskEXT) (GLclampf, GLboolean);
extern void (*glSamplePatternEXT) (GLenum);
#endif

#ifndef GL_SGI_vertex_preclip
#define GL_SGI_vertex_preclip 1
#endif

#ifndef GL_SGIX_convolution_accuracy
#define GL_SGIX_convolution_accuracy 1
#endif

#ifndef GL_SGIX_resample
#define GL_SGIX_resample 1
#endif

#ifndef GL_SGIS_point_line_texgen
#define GL_SGIS_point_line_texgen 1
#endif

#ifndef GL_SGIS_texture_color_mask
#define GL_SGIS_texture_color_mask 1
extern void (*glTextureColorMaskSGIS) (GLboolean, GLboolean, GLboolean, GLboolean);
#endif

#ifndef GL_EXT_vertex_buffer
#define GL_EXT_vertex_buffer 1

#define GL_TRIBESMTVFMT_EXT							0x8702
#define GL_TRIBESMTNVFMT_EXT							0x8703
#define GL_TRIBESFTVFMT_EXT							0x8704
#define GL_TRIBESFMTVFMT_EXT							0x8705

extern GLboolean (*glAvailableVertexBufferEXT)();
extern GLint (*glAllocateVertexBufferEXT)(GLsizei size, GLint format, GLboolean preserve);
extern void* (*glLockVertexBufferEXT)(GLint handle, GLsizei size);
extern void (*glUnlockVertexBufferEXT)(GLint handle);
extern void (*glSetVertexBufferEXT)(GLint handle);
extern void (*glOffsetVertexBufferEXT)(GLint handle, GLuint offset);
extern void (*glFillVertexBufferEXT)(GLint handle, GLint first, GLsizei count);
extern void (*glFreeVertexBufferEXT)(GLint handle);
#endif


/*
 * GL state information.
 */
struct GLState
{
	bool suppARBMultitexture;
	bool suppPackedPixels;
	bool suppTexEnvAdd;
	bool suppLockedArrays;
	bool suppTextureEnvCombine;
	bool suppVertexArrayRange;
	bool suppFogCoord;
	bool suppEdgeClamp;
	bool suppTextureCompression;
	bool suppS3TC;
	bool suppFXT1;
	bool suppTexAnisotropic;
	bool suppPalettedTexture;
	unsigned int triCount[4];
	unsigned int primCount[4];
	unsigned int primMode; // 0-3

	GLfloat maxAnisotropy;
 	GLint   maxTextureUnits;
};

extern GLState gGLState;

extern bool gOpenGLDisablePT;
extern bool gOpenGLDisableCVA;
extern bool gOpenGLDisableTEC;
extern bool gOpenGLDisableARBMT;
extern bool gOpenGLDisableFC;
extern bool gOpenGLDisableTCompress;
extern bool gOpenGLNoEnvColor;
extern float gOpenGLGammaCorrection;
extern bool gOpenGLNoDrawArraysAlpha;

/* 
 * Inline state helpers.
 */
inline void dglSetRenderPrimType(unsigned int type)
{
   gGLState.primMode = type;
}

inline void dglClearPrimMetrics()
{
   for(int i = 0; i < 4; i++)
      gGLState.triCount[i] = gGLState.primCount[i] = 0;
}

inline bool dglDoesSupportPalettedTexture()
{
   return gGLState.suppPalettedTexture && (gOpenGLDisablePT == false);
}

inline bool dglDoesSupportCompiledVertexArray()
{
   return gGLState.suppLockedArrays && (gOpenGLDisableCVA == false);
}

inline bool dglDoesSupportTextureEnvCombine()
{
   return gGLState.suppTextureEnvCombine && (gOpenGLDisableTEC == false);
}

inline bool dglDoesSupportARBMultitexture()
{
   return gGLState.suppARBMultitexture && (gOpenGLDisableARBMT == false);
}

inline bool dglDoesSupportVertexArrayRange()
{
   return gGLState.suppVertexArrayRange;
}

inline bool dglDoesSupportFogCoord()
{
   return gGLState.suppFogCoord && (gOpenGLDisableFC == false);
}

inline bool dglDoesSupportEdgeClamp()
{
   return gGLState.suppEdgeClamp;
}

inline bool dglDoesSupportTextureCompression()
{
   return gGLState.suppTextureCompression && (gOpenGLDisableTCompress == false);
}

inline bool dglDoesSupportS3TC()
{
   return gGLState.suppS3TC;
}

inline bool dglDoesSupportFXT1()
{
   return gGLState.suppFXT1;
}

inline bool dglDoesSupportTexEnvAdd()
{
   return gGLState.suppTexEnvAdd;
}

inline bool dglDoesSupportTexAnisotropy()
{
   return gGLState.suppTexAnisotropic;
}

inline bool dglDoesSupportVertexBuffer()
{
	return false;
}

inline GLfloat dglGetMaxAnisotropy()
{
   return gGLState.maxAnisotropy;
}

inline GLint dglGetMaxTextureUnits()
{
   if (dglDoesSupportARBMultitexture())
      return gGLState.maxTextureUnits;
   else
      return 1; 
}

#endif
