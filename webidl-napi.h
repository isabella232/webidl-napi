#ifndef WEBIDL_NAPI
#define WEBIDL_NAPI

#include <string.h>
#include <string>
#include <memory>
#include <vector>

// TODO(gabrielschulhof): Once we no longer support Node.js 10, we can
// unconditionally switch to the js_native_api.h header.
#if defined(BUILDING_NODE_EXTENSION)
#include "node_api.h"
#else
#include "js_native_api.h"
#endif

// Empty value so that macros here are able to return NULL or void
#define NAPI_RETVAL_NOTHING  // Intentionally blank #define

// A line that looks like a stack frame from a JS exception
#define SOURCE_LOCATION                                        \
  (std::string("\n    at ") + std::string(__func__) +          \
   std::string(" (" __FILE__ ":") + std::to_string(__LINE__) + \
   std::string(")"))

#define GET_AND_THROW_LAST_ERROR(env)                                    \
  do {                                                                   \
    const napi_extended_error_info *error_info;                          \
    napi_get_last_error_info((env), &error_info);                        \
    bool is_pending;                                                     \
    napi_is_exception_pending((env), &is_pending);                       \
    /* If an exception is already pending, don't rethrow it */           \
    if (!is_pending) {                                                   \
      const char* error_message = error_info->error_message != NULL ?    \
        error_info->error_message :                                      \
        "empty error message";                                           \
      napi_throw_error((env), NULL, (std::string(error_message) +        \
          SOURCE_LOCATION).c_str());                                     \
    }                                                                    \
  } while (0)

#define NAPI_CALL_BASE(env, the_call, ret_val)                           \
  do {                                                                   \
    if ((the_call) != napi_ok) {                                         \
      GET_AND_THROW_LAST_ERROR((env));                                   \
      return ret_val;                                                    \
    }                                                                    \
  } while (0)

// Returns NULL if the_call doesn't return napi_ok.
#define NAPI_CALL(env, the_call)                                         \
  NAPI_CALL_BASE(env, the_call, NULL)

// Returns empty if the_call doesn't return napi_ok.
#define NAPI_CALL_RETURN_VOID(env, the_call)                             \
  NAPI_CALL_BASE(env, the_call, NAPI_RETVAL_NOTHING)

struct webidl_sig {
  bool candidate;
  std::vector<napi_valuetype> sig;
};

using DOMString = std::string;
using USVString = std::string;
using object = napi_value;
using bool_t = bool;

namespace WebIdlNapi {

template <typename T>
class Converter {
 public:
  static inline napi_status ToNative(napi_env env,
                                     napi_value value,
                                     T* result);
  static inline napi_status ToJS(napi_env env,
                                 const T& value,
                                 napi_value* result);
};


static inline napi_status
PickSignature(napi_env env,
              size_t argc,
              napi_value* argv,
              std::vector<webidl_sig> sigs,
              int* sig_idx) {

  // Advance through the signatures one argument type at a time and mark those
  // as non-candidates whose signature does not correspond to the sequence of
  // argument types found in the actual arguments.
  for (size_t idx = 0; idx < argc; idx++) {
    napi_valuetype val_type;
    napi_status status = napi_typeof(env, argv[idx], &val_type);
    if (status != napi_ok) return status;
    for (auto& sig: sigs)
      if (sig.candidate)
        if (idx >= sig.sig.size() || sig.sig[idx] != val_type)
          sig.candidate = false;
  }

  // If any signatures are left marked as candidates, return the first one. We
  // do not touch `sig_idx` if we do not find a candidate, so the caller can set
  // it to -1 to be informed after this call completes that no candidate was
  // found.
  for (size_t idx = 0; idx < sigs.size(); idx++)
    if (sigs[idx].candidate) {
      *sig_idx = idx;
      break;
    }

  return napi_ok;
}

template <typename T>
class Promise {
 public:
  static inline napi_status
  ToJS(napi_env env, const Promise<T>& promise, napi_value* val) {
    return napi_ok;
  }
  static inline napi_status
  ToNative(napi_env env, napi_value value, Promise<T>* promise) {
    return napi_ok;
  }
};

template <typename WebIdlNapiType>
class sequence {
 public:
  static inline napi_status
  ToJS(napi_env env, const sequence<WebIdlNapiType>& seq, napi_value* val) {
    return napi_ok;
  }
  static inline napi_status
  ToNative(napi_env env, napi_value val, sequence<WebIdlNapiType>* result) {
    return napi_ok;
  }
};

}  // end of namespace WebIdlNapi

#include "webidl-napi-inl.h"

#endif  // WEBIDL_NAPI