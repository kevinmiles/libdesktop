#include "libdesktop.h"

#include <yoga/Yoga.h>
DSK_EXTEND_MODULE(libdesktop);
DSK_EXTEND_CLASS(libdesktop, EdgedProp);

typedef YGValue GetterYGVALUE(const YGNodeConstRef node);
typedef YGValue GetterEdgedYGVALUE(const YGNodeConstRef node, YGEdge edge);
typedef float GetterEdgedF32(const YGNodeConstRef node, YGEdge edge);
typedef void SetterEdgedF32(const YGNodeRef node, YGEdge edge, const float value);

struct EdgedPropData {
	GetterEdgedYGVALUE *getter;
	SetterEdgedF32 *setter;
	GetterEdgedF32 *getter_f;
	YGNodeRef node;
	YGUnit unit;
};

DSK_JS_FUNC(getPropYGValue) {
	DSK_JS_FUNC_INIT()

	YGNodeRef node;
	DSK_NAPI_CALL(napi_unwrap(env, this, (void **)&node));

	void **fns;
	DSK_NAPI_CALL(napi_get_cb_info(env, info, NULL, NULL, NULL, (void **)&fns));

	GetterYGVALUE *getter = fns[0];
	char *unit = fns[2];
	YGValue result = getter(node);

	if (strcmp(unit, "AUTO") == 0) {
		napi_value ret;
		DSK_NAPI_CALL(napi_get_boolean(env, result.unit == YGUnitAuto, &ret));
		return ret;
	}

	if (strcmp(unit, "POINT") == 0) {
		if (result.unit != YGUnitPoint) {
			napi_value ret;
			DSK_NAPI_CALL(napi_create_double(env, NAN, &ret));
			return ret;
		}
		napi_value ret;
		DSK_NAPI_CALL(napi_create_double(env, result.value, &ret));
		return ret;
	}

	if (strcmp(unit, "PERCENT") == 0) {
		if (result.unit != YGUnitPercent) {
			napi_value ret;
			DSK_NAPI_CALL(napi_create_double(env, NAN, &ret));
			return ret;
		}
		napi_value ret;
		DSK_NAPI_CALL(napi_create_double(env, result.value, &ret));
		return ret;
	}

	napi_throw_error(env, "EINVAL", "Unknown unit");
	return NULL;
}

typedef void dsk_SetterI32(void *instance, const int32_t value, void **datas);
typedef int32_t dsk_GetterI32(void *instance, void **datas);

typedef void dsk_SetterF32(void *instance, const float value, void **datas);
typedef float dsk_GetterF32(void *instance, void **datas);

DSK_JS_FUNC(dsk_setPropI32) {
	DSK_JS_FUNC_INIT();
	DSK_AT_LEAST_NARGS(1);
	int32_t value;
	DSK_NAPI_CALL(napi_get_value_int32(env, argv[0], &value));

	void *instance;
	DSK_NAPI_CALL(napi_unwrap(env, this, (void **)&instance));

	void **datas;
	DSK_NAPI_CALL(napi_get_cb_info(env, info, NULL, NULL, NULL, (void **)&datas));

	dsk_SetterI32 *setter = datas[1];

	setter(instance, value, datas);
	return NULL;
}

DSK_JS_FUNC(dsk_getPropI32) {
	DSK_JS_FUNC_INIT()

	void *instance;
	DSK_NAPI_CALL(napi_unwrap(env, this, (void **)&instance));

	void **datas;

	DSK_NAPI_CALL(napi_get_cb_info(env, info, NULL, NULL, NULL, (void **)&datas));

	dsk_GetterI32 *getter = datas[0];
	int32_t result = getter(instance, datas);
	napi_value ret;
	DSK_NAPI_CALL(napi_create_int32(env, result, &ret));
	return ret;
}

DSK_JS_FUNC(dsk_setPropF32) {
	DSK_JS_FUNC_INIT()
	DSK_AT_LEAST_NARGS(1)
	double value;
	DSK_NAPI_CALL(napi_get_value_double(env, argv[0], &value));

	void *instance;
	DSK_NAPI_CALL(napi_unwrap(env, this, (void **)&instance));

	void **datas;
	DSK_NAPI_CALL(napi_get_cb_info(env, info, NULL, NULL, NULL, (void **)&datas));

	dsk_SetterF32 *setter = datas[1];
	setter(instance, (float)value, datas);
	return NULL;
}

DSK_JS_FUNC(dsk_getPropF32) {
	DSK_JS_FUNC_INIT()

	void *instance;
	DSK_NAPI_CALL(napi_unwrap(env, this, (void **)&instance));

	void **datas;
	DSK_NAPI_CALL(napi_get_cb_info(env, info, NULL, NULL, NULL, (void **)&datas));

	dsk_GetterF32 *getter = datas[0];
	float result = getter(instance, datas);

	napi_value ret;
	DSK_NAPI_CALL(napi_create_double(env, result, &ret));
	return ret;
}

#define PROP_YGVALUE_F32(NAME, UNIT, NATIVE_GETTER, NATIVE_SETTER)                                 \
	DSK_DEFINE_PROPERTY(libdesktop, Style, NAME, getPropYGValue, dsk_setPropF32,                   \
						((void *[]){NATIVE_GETTER, NATIVE_SETTER, #UNIT}))

napi_value mk_edged_prop(napi_env env, YGNodeRef node, YGUnit unit, GetterEdgedYGVALUE *getter,
						 SetterEdgedF32 *setter) {

	napi_value edgedProp = dsk_new_instance(env, libdesktop_EdgedProp_ref, 0, NULL);
	if (edgedProp == NULL) {
		return NULL;
	}

	struct EdgedPropData *data = dsk_unwrap(env, edgedProp);
	if (data == NULL) {
		return NULL;
	}
	data->getter = getter;
	data->setter = setter;
	data->node = node;
	data->unit = unit;

	return edgedProp;
}

napi_value mk_edged_prop_f(napi_env env, YGNodeRef node, YGUnit unit, GetterEdgedF32 *getter,
						   SetterEdgedF32 *setter) {
	napi_value edgedProp = dsk_new_instance(env, libdesktop_EdgedProp_ref, 0, NULL);
	if (edgedProp == NULL) {
		return NULL;
	}

	struct EdgedPropData *data = dsk_unwrap(env, edgedProp);
	if (data == NULL) {
		return NULL;
	}

	data->getter_f = getter;
	data->setter = setter;
	data->node = node;
	data->unit = unit;

	return edgedProp;
}

#define EDGED_PROP(NAME, UNIT, GETTER, SETTER)                                                     \
	napi_define_properties(                                                                        \
		env, this, 1,                                                                              \
		(napi_property_descriptor[]){{                                                             \
			.utf8name = #NAME,                                                                     \
			.value = mk_edged_prop(env, node, UNIT, GETTER, SETTER),                               \
			.attributes = napi_enumerable, /*!napi_configurable !napi_writable*/                   \
		}})

#define EDGED_PROP_F(NAME, GETTER, SETTER)                                                         \
	napi_define_properties(                                                                        \
		env, this, 1,                                                                              \
		(napi_property_descriptor[]){{                                                             \
			.utf8name = #NAME,                                                                     \
			.value = mk_edged_prop_f(env, node, 0xbabe, GETTER, SETTER),                           \
			.attributes = napi_enumerable, /*!napi_configurable !napi_writable*/                   \
		}})

static void set_margin_auto(YGNodeRef node, YGEdge edge, float margin) {
	if (margin == 0.0) {
		YGNodeStyleSetMargin(node, edge, 0.0);
	}
	YGNodeStyleSetMarginAuto(node, edge);
}

static void set_style_auto(const YGNodeRef node, float _) {
	YGNodeStyleSetFlexBasisAuto(node);
}

typedef void dsk_SetterI32(void *instance, const int32_t value, void **datas);
typedef int32_t dsk_GetterI32(void *instance, void **datas);

typedef void dsk_SetterF32(void *instance, const float value, void **datas);
typedef float dsk_GetterF32(void *instance, void **datas);

typedef void dsk_SetterSTR(void *instance, char *value, void **datas);
typedef char *dsk_GetterSTR(void *instance, void **datas);

typedef void dsk_SetterBOOL(void *instance, bool value, void **datas);
typedef bool dsk_GetterBOOL(void *instance, void **datas);

DSK_JS_FUNC(dsk_setPropI32);
DSK_JS_FUNC(dsk_getPropI32);
DSK_JS_FUNC(dsk_setPropBOOL);
DSK_JS_FUNC(dsk_getPropBOOL);
DSK_JS_FUNC(dsk_setPropF32);
DSK_JS_FUNC(dsk_getPropF32);
DSK_JS_FUNC(dsk_setPropSTR);
DSK_JS_FUNC(dsk_getPropSTR);

#define PROP_I32(NAME, NATIVE_GETTER, NATIVE_SETTER)                                               \
	DSK_DEFINE_PROPERTY(libdesktop, Style, NAME, dsk_getPropI32, dsk_setPropI32,                   \
						((void *[]){NATIVE_GETTER, NATIVE_SETTER}))

#define PROP_F32(NAME, NATIVE_GETTER, NATIVE_SETTER)                                               \
	DSK_DEFINE_PROPERTY(libdesktop, Style, NAME, dsk_getPropF32, dsk_setPropF32,                   \
						((void *[]){NATIVE_GETTER, NATIVE_SETTER}))

DSK_DEFINE_CLASS(libdesktop, Style) {
	DSK_JS_FUNC_INIT()
	DSK_EXACTLY_NARGS(2);
	napi_valuetype argType;

	YGNodeRef node;
	// printf("*****************1\n");
	if (argc < 2) {
		node = YGNodeNew();
	} else {
		// printf("*****************2\n");
		DSK_NAPI_CALL(napi_typeof(env, argv[1], &argType));
		// printf("*****************3\n");
		if (argType == napi_null) {
			// printf("*****************4\n");
			node = YGNodeNew();
		} else {
			// printf("*****************5\n");
			DskCtrlI *ctrl;
			DSK_NAPI_CALL(dsk_CtrlI_from_wrapper(env, argv[1], &ctrl));
			// printf("*****************6\n");
			node = ctrl->yg_node;
		}
	}

	// printf("*****************7\n");
	DSK_NAPI_CALL(napi_wrap(env, this, node, NULL, NULL, NULL));

	EDGED_PROP(position, YGUnitPoint, YGNodeStyleGetPosition, YGNodeStyleSetPosition);
	EDGED_PROP(positionPercent, YGUnitPercent, YGNodeStyleGetPosition,
			   YGNodeStyleSetPositionPercent);

	EDGED_PROP_F(border, YGNodeStyleGetBorder, YGNodeStyleSetBorder);

	EDGED_PROP(padding, YGUnitPoint, YGNodeStyleGetPadding, YGNodeStyleSetPadding);
	EDGED_PROP(paddingPercent, YGUnitPercent, YGNodeStyleGetPadding, YGNodeStyleSetPaddingPercent);

	EDGED_PROP(margin, YGUnitPoint, YGNodeStyleGetMargin, YGNodeStyleSetMargin);
	EDGED_PROP(marginPercent, YGUnitPercent, YGNodeStyleGetMargin, YGNodeStyleSetMarginPercent);
	EDGED_PROP(marginAuto, YGUnitAuto, YGNodeStyleGetMargin, set_margin_auto);

	return this;
}

PROP_I32(direction, YGNodeStyleGetDirection, YGNodeStyleSetDirection);
PROP_I32(flexDirection, YGNodeStyleGetFlexDirection, YGNodeStyleSetFlexDirection);
PROP_I32(justifyContent, YGNodeStyleGetJustifyContent, YGNodeStyleSetJustifyContent);
PROP_I32(alignContent, YGNodeStyleGetAlignContent, YGNodeStyleSetAlignContent);
PROP_I32(alignItems, YGNodeStyleGetAlignItems, YGNodeStyleSetAlignItems);
PROP_I32(alignSelf, YGNodeStyleGetAlignSelf, YGNodeStyleSetAlignSelf);
PROP_I32(positionType, YGNodeStyleGetPositionType, YGNodeStyleSetPositionType);
PROP_I32(flexWrap, YGNodeStyleGetFlexWrap, YGNodeStyleSetFlexWrap);
PROP_I32(overflow, YGNodeStyleGetOverflow, YGNodeStyleSetOverflow);
PROP_I32(display, YGNodeStyleGetDisplay, YGNodeStyleSetDisplay);
PROP_F32(flex, YGNodeStyleGetFlex, YGNodeStyleSetFlex);
PROP_F32(flexGrow, YGNodeStyleGetFlexGrow, YGNodeStyleSetFlexGrow);
PROP_F32(flexShrink, YGNodeStyleGetFlexShrink, YGNodeStyleSetFlexShrink);
PROP_YGVALUE_F32(flexBasis, POINT, YGNodeStyleGetFlexBasis, YGNodeStyleSetFlexBasis);
PROP_YGVALUE_F32(flexBasisPercent, PERCENT, YGNodeStyleGetFlexBasis,
				 YGNodeStyleSetFlexBasisPercent);
PROP_YGVALUE_F32(flexBasisAuto, AUTO, YGNodeStyleGetFlexBasis, set_style_auto);
PROP_YGVALUE_F32(width, POINT, YGNodeStyleGetWidth, YGNodeStyleSetWidth);
PROP_YGVALUE_F32(widthPercent, PERCENT, YGNodeStyleGetWidth, YGNodeStyleSetWidthPercent);
PROP_YGVALUE_F32(height, POINT, YGNodeStyleGetHeight, YGNodeStyleSetHeight);
PROP_YGVALUE_F32(heightPercent, PERCENT, YGNodeStyleGetHeight, YGNodeStyleSetHeightPercent);
PROP_YGVALUE_F32(minWidth, POINT, YGNodeStyleGetMinWidth, YGNodeStyleSetMinWidth);
PROP_YGVALUE_F32(minWidthPercent, PERCENT, YGNodeStyleGetMinWidth, YGNodeStyleSetMinWidthPercent);
PROP_YGVALUE_F32(minHeight, POINT, YGNodeStyleGetMinHeight, YGNodeStyleSetMinHeight);
PROP_YGVALUE_F32(minHeightPercent, PERCENT, YGNodeStyleGetMinHeight,
				 YGNodeStyleSetMinHeightPercent);
PROP_YGVALUE_F32(maxWidth, POINT, YGNodeStyleGetMaxWidth, YGNodeStyleSetMaxWidth);
PROP_YGVALUE_F32(maxWidthPercent, PERCENT, YGNodeStyleGetMaxWidth, YGNodeStyleSetMaxWidthPercent);
PROP_YGVALUE_F32(maxHeight, POINT, YGNodeStyleGetMaxHeight, YGNodeStyleSetMaxHeight);
PROP_YGVALUE_F32(maxHeightPercent, PERCENT, YGNodeStyleGetMaxHeight,
				 YGNodeStyleSetMinHeightPercent);

static DSK_JS_FUNC(edgedFloatSetter) {
	DSK_JS_FUNC_INIT();
	DSK_AT_LEAST_NARGS(1)

	double value;
	DSK_NAPI_CALL(napi_get_value_double(env, argv[0], &value));

	struct EdgedPropData *data;
	DSK_NAPI_CALL(napi_unwrap(env, this, (void **)&data));

	YGEdge *edge;
	DSK_NAPI_CALL(napi_get_cb_info(env, info, NULL, NULL, NULL, (void **)&edge));
	data->setter(data->node, *edge, (float)value);

	return NULL;
}

/*

static DSK_JS_FUNC(edgedFloatGetter) {
	DSK_JS_FUNC_INIT();


	struct EdgedPropData* data;
	napi_status status = napi_unwrap(env, this, (void**)&data);
	CHECK_STATUS_THROW(status, napi_unwrap);

	YGEdge* edge;
	status = napi_get_cb_info(env, info, NULL, NULL, NULL,(void**)&edge);
	CHECK_STATUS_THROW(status, napi_get_cb_info);

	float result = data->getter_f(data->node, *edge);
	return make_double(env,result);
}
*/
static DSK_JS_FUNC(edgedYgValueGetter) {
	DSK_JS_FUNC_INIT();

	struct EdgedPropData *data;
	DSK_NAPI_CALL(napi_unwrap(env, this, (void **)&data));

	YGEdge *edge;
	DSK_NAPI_CALL(napi_get_cb_info(env, info, NULL, NULL, NULL, (void **)&edge));

	if (data->unit == 0xbabe) {
		float result = data->getter_f(data->node, *edge);
		napi_value ret;
		DSK_NAPI_CALL(napi_create_double(env, result, &ret));
		return ret;
	}

	YGValue result = data->getter(data->node, *edge);

	if (data->unit == YGUnitAuto) {
		napi_value ret;
		DSK_NAPI_CALL(napi_get_boolean(env, result.unit == YGUnitAuto, &ret));
		return ret;
	}

	if (data->unit == YGUnitPoint) {
		napi_value ret;

		if (result.unit != YGUnitPoint) {
			DSK_NAPI_CALL(napi_create_double(env, NAN, &ret));
			return ret;
		}
		DSK_NAPI_CALL(napi_create_double(env, result.value, &ret));
		return ret;
	}

	if (data->unit == YGUnitPercent) {
		napi_value ret;
		if (result.unit != YGUnitPercent) {
			DSK_NAPI_CALL(napi_create_double(env, NAN, &ret));
			return ret;
		}
		DSK_NAPI_CALL(napi_create_double(env, result.value, &ret));
		return ret;
	}

	DSK_FAILURE("Unknown unit");
	return NULL;
}

#define EDGED_PROP_YGVALUE(NAME, EDGE)                                                             \
	DSK_DEFINE_PROPERTY(libdesktop, EdgedProp, NAME, edgedYgValueGetter, edgedFloatSetter,         \
						(YGEdge[]){EDGE})

static void finalize_edgedProp(napi_env env, void *finalize_data, void *finalize_hint) {
	free(finalize_data);
}

DSK_DEFINE_CLASS(libdesktop, EdgedProp) {
	DSK_JS_FUNC_INIT();

	struct EdgedPropData *data = malloc(sizeof(struct EdgedPropData));

	DSK_NAPI_CALL(napi_wrap(env, this, data, finalize_edgedProp, NULL, NULL));

	return this;
}

EDGED_PROP_YGVALUE(left, YGEdgeLeft)
EDGED_PROP_YGVALUE(top, YGEdgeTop)
EDGED_PROP_YGVALUE(right, YGEdgeRight)
EDGED_PROP_YGVALUE(bottom, YGEdgeBottom)
EDGED_PROP_YGVALUE(start, YGEdgeStart)
EDGED_PROP_YGVALUE(end, YGEdgeEnd)
EDGED_PROP_YGVALUE(horizontal, YGEdgeHorizontal)
EDGED_PROP_YGVALUE(vertical, YGEdgeVertical)
EDGED_PROP_YGVALUE(all, YGEdgeAll)

/*




// Yoga specific properties, not compatible with flexbox specification Aspect
// ratio control the size of the undefined dimension of a node. Aspect ratio is
// encoded as a floating point value width/height. e.g. A value of 2 leads to a
// node with a width twice the size of its height while a value of 0.5 gives the
// opposite effect.
//
// - On a node with a set width/height aspect ratio control the size of the
//   unset dimension
// - On a node with a set flex basis aspect ratio controls the size of the node
//   in the cross axis if unset
// - On a node with a measure function aspect ratio works as though the measure
//   function measures the flex basis
// - On a node with flex grow/shrink aspect ratio controls the size of the node
//   in the cross axis if unset
// - Aspect ratio takes min/max dimensions into account
WIN_EXPORT void YGNodeStyleSetAspectRatio(YGNodeRef node, float aspectRatio);
WIN_EXPORT float YGNodeStyleGetAspectRatio(YGNodeConstRef node);
*/