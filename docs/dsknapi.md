# NAPI functions and macros helpers

This header file includes functions prototypes and macros that
helps working with Node.js NAPI framework.

## Members

* [DSK_NAPI_CALL](#DSK_NAPI_CALL) - wraps its FN argument with code to check that a NAPI call succeded.
* [DSK_JS_FUNC](#DSK_JS_FUNC) - declares a js callback function prototype
* [DSK_JS_FUNC_INIT](#DSK_JS_FUNC_INIT) - initialize a scope with the following variables:
* [DSK_ONERROR_THROW_RET](#DSK_ONERROR_THROW_RET) - defines an error handler that throw a JavaScript error.
* [DSK_ONERROR_FATAL_RET](#DSK_ONERROR_FATAL_RET) - defines an error handler that bort the process
* [DSK_ONERROR_UNCAUGHT_RET](#DSK_ONERROR_UNCAUGHT_RET) - defines an error handler that throw an uncaught JavaScript error.
* [DSK_JS_FUNC_INIT_WITH_ARGS](#DSK_JS_FUNC_INIT_WITH_ARGS) - initialize a function scope with a set of standard variables
* [DSK_AT_LEAST_NARGS](#DSK_AT_LEAST_NARGS) - Throw a "EINVAL" errors if the callback is not called with at least N arguments.
* [DSK_EXACTLY_NARGS](#DSK_EXACTLY_NARGS) - Throw a "EINVAL" errors if the callback is not called with exactly N arguments.
* [Function and macro to exports function, objects, or classes.](#Function and macro to exports function, objects, or classes.) - 

### DSK_NAPI_CALL

> wraps its FN argument with code to check that a NAPI call succeded.

DSK_NAPI_CALL macro wraps its FN argument
with code to check that a napi call succeded.
FN argument must be an expression that evaluates
to an object of `napi_status` type.

The entire macro is wrapped in a do {} while, so it
doesn't pollute the calling stack with variables.

The macro does these assumptions on the calling scope:

 * an `dsk_env` variable is defined of type `napi_env` that contains the current
   NAPI environment to use.
 * a `dsk_error_msg` variable is declared of type char*.
   This variable will contain the error message in case an error
   occurs during the call.
 * a `dsk_error` label is defined. Execution will jump to that label
   after appropriately setting the dsk_error_msg variable.

If you are using the DSK_NAPI_CALL macro in a function that calls
the DSK_JS_FUNC_INIT* macro, that macro already appropriately defines
in the scope all variables needed by DSK_NAPI_CALL. Otherwise, it is
caller responsibility to appropriately prepare the scope before
the call to DSK_NAPI_CALL.

---

### DSK_JS_FUNC

> declares a js callback function prototype

DSK_JS_FUNC declares a js callback function prototype given the name.
It can be used to declare a function in a header file:

```c

 DSK_JS_FUNC(external_function);

```

or otherwise to define a function in a source file:

```c

DSK_JS_FUNC(external_function) {

 // do something interesting here...

 // and return something useful.
 return NULL;
}

```

---

### DSK_JS_FUNC_INIT

> initialize a scope with the following variables:

The macro does these assumptions on the calling scope:
 * an `env` variable is defined of type `napi_env` that contains the current
   NAPI environment to use.
 * an `info` variable is declared of type napi_callback_info.
   This variable will contain the error message in case an error
   occurs during the call.
This two requirement are already satisfied by the protype required by callback
functions.

The macro uses `env` and `info` to extract arguments of the call, and defines
in current scope the following additional variables:

 * a `dsk_error_msg` variable of type char* that is compiled by NAPI_CALL in case of errors.
 * a `dsk_error` label to which execution will jump in case of error. It contains code to
   throw a JS error with the message contained in `dsk_error_msg` variable, and return NULL
afterward;
 * an `argc` variable of type size_t that contains the actual number of argument
   passed to the function by the caller.
 * an `argv` array of type napi_value[] that contains the actual number of argument
   passed to the function by the caller. If the function is given more than 10 arguments, the
remaining one are ignored. If it's given less than 10 arguments, than argv elements from `argc`
to 10 are filled witth `napi_undefined`. If you have to use accepts more than 10 arguments, use
`DSK_JS_FUNC_INIT_WITH_ARGS` macro to specify a greater number.
 * a `this` variable, of type napi_value, that contains the receiver of the call (the JS `this`
value);
 * an `argc` variable of type size_t containing actual number of arguments received in the call.

---

### DSK_ONERROR_THROW_RET

> defines an error handler that throw a JavaScript error.

This macro defines following variables in current scope:

 * a `dsk_error_msg` variable of type char* that is compiled by DSK_NAPI_CALL in
   case of errors.
 * a `dsk_error` label to which execution will jump in case of error. It contains
   code to throw a JS error with the message contained in `dsk_error_msg` variable,
   and return `WHAT` argument afterward;

---

### DSK_ONERROR_FATAL_RET

> defines an error handler that bort the process

This macro defines following variables in current scope:

 * a `dsk_error_msg` variable of type char* that is compiled by DSK_NAPI_CALL in
   case of errors.
 * a `dsk_error` label to which execution will jump in case of error. It contains
   code to call `napi_fatal_error` with the message contained in `dsk_error_msg` variable,
   and return `WHAT` argument afterward. The call to `napi_fatal_error` causes the node process
	  to immediately exit.

---

### DSK_ONERROR_UNCAUGHT_RET

> defines an error handler that throw an uncaught JavaScript error.

This macro defines following variables in current scope:

 * a `dsk_error_msg` variable of type char* that is compiled by DSK_NAPI_CALL in
   case of errors.
 * a `dsk_error` label to which execution will jump in case of error. It contains
   code to throw an uncaught Javascript exception with the message contained in `dsk_error_msg`
   variable, and return `WHAT` argument afterward.

---

### DSK_JS_FUNC_INIT_WITH_ARGS

> initialize a function scope with a set of standard variables

You can use this function when you have to accepts more than 10 arguments in your
callback. Use `DSK_JS_FUNC_INIT` when <= 10 arguments are required.

---

### DSK_AT_LEAST_NARGS

> Throw a "EINVAL" errors if the callback is not called with at least N arguments.

The calling scope must contains `env`, `argc` and `dsk_error_msg` variables, and a `dsk_error`
label.

---

### DSK_EXACTLY_NARGS

> Throw a "EINVAL" errors if the callback is not called with exactly N arguments.

The calling scope must contains `env`, `argc` and `dsk_error_msg` variables, and a `dsk_error`
label.

---

### Function and macro to exports function, objects, or classes.

> 



---

