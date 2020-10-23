#ifndef DSK_CONTROL_H__
#define DSK_CONTROL_H__ 1

#include "yoga/Yoga.h"
#include "napi_utils.h"

YGNodeRef dsk_widget_get_node(napi_env env, napi_value widget);
void dsk_widget_set_node(napi_env env, napi_value widget, YGNodeRef node);


typedef void *UIHandle;
void dsk_platform_container_add_child(UIHandle parent, UIHandle child);
void dsk_add_child(napi_env env, UIHandle parentHandle, UIHandle childHandle);
void dsk_append_all_children(napi_env env, UIHandle widget,napi_value children);

void widget_finalize(napi_env env, void *finalize_data, void *finalize_hint);

void dsk_wrap_widget(napi_env env, UIHandle widget, napi_value this);
void dsk_widget_move(napi_env env, UIHandle container, UIHandle widget, float x, float y);
//UIHandle dsk_unwrap_widget(napi_env env, napi_value this);
napi_value dsk_widget_wrapper(napi_env env, UIHandle widget);
void dsk_calculate_layout(napi_env env, UIHandle container, YGNodeRef root);

#endif